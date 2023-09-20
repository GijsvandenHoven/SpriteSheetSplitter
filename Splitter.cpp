#include <vector>
#include <syncstream>
#include <omp.h>
#include "Splitter.h"
#include "util/SimpleTimer.h"
#include "logging/LoggerTags.hpp"

namespace logger = LoggerTags;

void Splitter::work(std::vector <SplitterOpts> &jobs) {
    SpriteSplittingStatus jobStats;

    int jobCounter = 0;
    for (auto& job : jobs) {
        // todo: to really push the concurrency, any job could be its own process.
        bool inPathOK = ssio.setInPath(job.inDirectory, job.isPNGInDirectory, job.recursive);
        bool outPathOK = ssio.setOutPath(job.outDirectory);
        ssio.setIOOptions(IOOptions(job));
        // only work the job if the input is OK. The only input error that can happen from now on is a png decode error or malformed SpriteSheet.
        if (! (inPathOK && outPathOK)) {
            std::cout << logger::error << "Something went wrong with setting the in or out path for this job. Please check the program output.\n";
            std::cout << "\tIn path provided:\t" << job.inDirectory << "\n";
            std::cout << "\tOut path provided:\t" << job.outDirectory << "\n";
            continue;
        }

        std::queue<std::string> pngQueue;
        ssio.fillPNGQueue(pngQueue);

        if (pngQueue.empty()) {
            std::cout << logger::warn << "Zero png files were found from the input directory:";
            std::cout << "\n\t\t" << job.inDirectory << "\n";
        }

        if (job.isPNGInDirectory) {
            std::cout << logger::info << "Begin working on file \"" << job.inDirectory << "\"with " << job;

            std::string& onlyFile = pngQueue.front();
            split(onlyFile, jobStats, std::cout);
            pngQueue.pop();
        } else {
            std::cout << logger::info << "Begin working on folder \"" << job.inDirectory << "\" with " << job;

            workFolder(job.workAmount, pngQueue, jobStats);
        }

        std::cout << logger::info << "DONE with job " << ++jobCounter << " out of " << jobs.size() << "\n";
    }

    std::cout << jobStats;
}

/**
 * Split all PNGs of a folder by following the string filepaths in the pngs queue.
 *
 * This is done by assigning one thread per file for adequate performance
 *
 * @param workCap the maximum amount of files to process before stopping
 * @param pngs the queue of FilePaths to SpriteSheets
 * @param jobStats stat tracking object
 */
void Splitter::workFolder(int workCap, std::queue<std::string> &pngs, SpriteSplittingStatus &jobStats) const {
    const int work = std::min(workCap, static_cast<int>(pngs.size()));

    SimpleTimer folder("Splitting this folder");
#pragma omp parallel for schedule(dynamic) shared(work, pngs, std::cout, jobStats, logger::info) default(none)
    for (int tid = 0; tid < work; ++tid) {
        if (tid == 0) std::cout << logger::info << " Begin working on a folder using " << omp_get_num_threads() << " threads\n";

        std::string file;
#pragma omp critical(queueAccess)
        {
            file = std::move(pngs.front());
            pngs.pop();
        }

        // for printing without data races. Downside, only prints when the object is destroyed (end of loop iteration).
        std::osyncstream synced_out(std::cout);

        SpriteSplittingStatus individualJobStats;
        split(file, individualJobStats, synced_out);

#pragma omp critical(updateStats)
        {
            jobStats += individualJobStats;
        }
    }
}

/**
 * Load a SpriteSheet from the given fileDirectory, split the data in single sprites with the correct name, then save.
 *
 * Automatically detects the SpriteSheet type (if any).
 * Does not split fully invisible (alpha 0 on every pixel) objects or chars. Chars with only some invisible frames are OK.
 *
 * Automatically determines the name of a folder based on the SpriteSheet name.
 *
 * @param fileDirectory A path to a .png SpriteSheet file.
 * @param outStream stream for printing characters. Normally std::cout, but could be std::osyncstream from threading.
 * @param jobStats struct for counting stats of splitting.
 */
void Splitter::split(const std::string &fileDirectory, SpriteSplittingStatus &jobStats, std::basic_ostream<char> &outStream) const {
    SimpleTimer timer {"Splitting this file", outStream};
    std::vector<unsigned char> img;
    SpriteSheetPNGData pngData;

    outStream << logger::threaded_info << "Loading " << fileDirectory << "\n";

    SpriteSheetIO::loadPNG(fileDirectory, img, pngData);

    if (pngData.error) {
        outStream << logger::threaded_error << "LodePNG decode error: " << pngData.error << ". (Most likely a corrupt png)\n"; // if it's an incorrect path at this point then that is a bug!
        jobStats.n_load_error += 1;
        return;
    }

    // Note to self: for types that are not discernible from sheet dimensions alone, this info could be supplied as optional parameter or function overload (latter needs loading routine above moved to dedicated function).
    // Then transform this into something where the default branch is this code, and supplied parameter simply set the type after calling validSpriteSheet of some dimension.
    SpriteSheetType type;
    if (validSpriteSheet(pngData.width, pngData.height, OBJ_SHEET_ROW)) {
        type = SpriteSheetType::OBJECT;
    } else if (validSpriteSheet(pngData.width, pngData.height, CHAR_SHEET_ROW)) {
        type = SpriteSheetType::CHARACTER;
    } else {
        outStream << logger::threaded_error << "An image of size " << pngData.width << ", " << pngData.height << " is not a valid SpriteSheet.\n";
        jobStats.n_load_error += 1;
        return;
    }

    outStream << logger::threaded_info << "Processing file as " << type << " sheet\n";

    unsigned int spriteSize; // size of a sprite (8, 16, 32..)
    unsigned int spriteCount; // amount of unsigned char* to expect back from splitting.

    // pointer representing the specific function to call splitting on. Strategy pattern go brr.
    // This is to prevent allocating spriteData in each switch branch, by doing it after because it always has the same size.
    // But also preventing the need for calling a generic entry point which then _again_ has a switch on SpriteSheetType.
    std::function<void(SpriteSplittingData&)> splitFunction;

    switch(type) {
        case SpriteSheetType::OBJECT:
            spriteSize = pngData.width / OBJ_SHEET_ROW;
            spriteCount = (img.size() / 4) / (spriteSize * spriteSize);
            // assign the correct function
            splitFunction = Splitter::splitObjectSheet;
            break;
        case SpriteSheetType::CHARACTER:
            spriteSize = pngData.width / CHAR_SHEET_ROW;
            // correct for column 3 being empty, and 5+6 being joined (see splitCharSheet function comment)
            spriteCount = ((img.size() / 4) / (spriteSize * spriteSize));
            spriteCount = (spriteCount / CHAR_SHEET_ROW) * (CHAR_SHEET_ROW - 2);
            // assign the correct function
            splitFunction = Splitter::splitCharSheet;
            break;
        default: // did you add a new type to the enum?
            outStream << logger::error << "unknown SpriteSheetType" << type << "\n";
            exit(-1);
    }

    // rows per sprite * amount of sprites that fit on the sheet
    auto spriteData = new unsigned char* [spriteSize * spriteCount];
    // bundle all these parameters into one struct
    SpriteSplittingData splitData(img.data(), spriteData, spriteSize, spriteCount, type, pngData.lodeState, fileDirectory, jobStats);
    // split the sprites
    splitFunction(splitData);
    // and save them
    ssio.saveSplits(splitData, outStream);

    outStream << logger::threaded_info << "Finished splitting SpriteSheet.\n";

    delete[] spriteData;
}

/**
 * Given a pointer to a SpriteSheets raw pixel data, and the amount of object sprites there are in it,
 * fills a collection of byte pointers such that every [spriteSize] pointers forms a singe sprite.
 * Each individual pointer is a row of sprite data.
 *
 * Assumes that the collection has enough space allocated to do this (spriteSize * spriteCount).
 *
 * @param ssd Struct containing all necessary data. See SpriteSplittingData.h.\n
 *            In particular, these members are used:\n
 *            ssd.spriteSheet, ssd.spriteSize, ssd.SpriteCount, ssd.splitSprites
 */
void Splitter::splitObjectSheet(SpriteSplittingData& ssd) {
    unsigned char* imgData = ssd.spriteSheet;
    const unsigned int spriteSize = ssd.spriteSize;
    const unsigned int spriteCount = ssd.spriteCount;
    unsigned char** out = ssd.splitSprites;
    // constant * SZ rows, 4 uchar per pixel.
    unsigned int sheetPixelWidth = spriteSize * OBJ_SHEET_ROW * 4;

#pragma omp simd collapse(2)
    for (int i = 0; i < spriteCount; ++i) {
        for (int j = 0; j < spriteSize; ++j) {
            //    (linear index)    =           (sprite row offset)                                + (sprite column offset)               + (pixel row offset)
            out[i * spriteSize + j] = imgData + (i / OBJ_SHEET_ROW) * spriteSize * sheetPixelWidth + (i % OBJ_SHEET_ROW) * spriteSize * 4 + j * sheetPixelWidth;
        }
    }
}

/**
 * Given a pointer to a SpriteSheets raw pixel data, and the amount of character sprites there are in it,
 * fills a collection of byte pointers such that every [spriteSize] pointers forms a singe sprite.
 * Each individual pointer is a row of sprite data.
 *
 * Assumes that the collection has enough space allocated to do this (spriteSize * spriteCount).
 *
 * @param ssd Struct containing all necessary data. See SpriteSplittingData.h.\n
 *            In particular, these members are used:\n
 *            ssd.spriteSheet, ssd.spriteSize, ssd.SpriteCount, ssd.splitSprites
 */
void Splitter::splitCharSheet(SpriteSplittingData& ssd) {
    /* Char sheets are special. They consist of seven columns, where each row belongs to a single char.
     * Column 0 is their idle frame.
     * Column 1 and 2 are their walking frames.
     * Column 3 is always empty. (Might've been intended for a fancy walk frame. Never happened.)
     * Column 4 and (5+6) are attack frames.
     * Column 5 and 6 are joined as one sprite for i.e. an extended arm holding a sword.
     * Meaning all sprites here are square except for the 5th, it is a size x 2size rectangle!
     */
    unsigned char* imgData = ssd.spriteSheet;
    const unsigned int spriteSize = ssd.spriteSize;
    const unsigned int spriteCount = ssd.spriteCount;
    unsigned char** out = ssd.splitSprites;
    unsigned int sheetPixelWidth = spriteSize * CHAR_SHEET_ROW * 4;
    const int CHARS_PER_ROW = CHAR_SHEET_ROW - 2;
    auto columnOffset = [](int i)->bool { return i % CHARS_PER_ROW >= 3; };

#pragma omp simd collapse(2)
    for (int i = 0; i < spriteCount; ++i) {
        for (int j = 0; j < spriteSize; ++j) {
            // linear index         =           (sprite row offset)                                + (sprite col offset   + (skips column 3))                 + (pixel row offset)
            out[i * spriteSize + j] = imgData + (i / CHARS_PER_ROW) * spriteSize * sheetPixelWidth + ((i % CHARS_PER_ROW) + columnOffset(i)) * spriteSize * 4 + j * sheetPixelWidth;
        }
    }
}

/**
 * tests if the given image dimensions are that of a correctly formed SpriteSheetData.
 * A SpriteSheetData has equally sized columns (objects vs chars), where each column is at least 8px wide, and the column width is a power of 2.
 * Furthermore, the height of the SpriteSheetData are divisible by the sprite size.
 *
 * @param width width of the png
 * @param height height of the png
 * @param columnCount amount of columns in the png
 * @return whether or not it is a valid SpriteSheetData.
 */
bool Splitter::validSpriteSheet(unsigned int width, unsigned int height, unsigned int columnCount) {
    // columns are equally sized?
    float columnSize = static_cast<float>(width) / static_cast<float>(columnCount);
    if (columnSize != static_cast<float>(static_cast<int>(columnSize))) {
        return false;
    }
    // columns are a power of 2 and at least 8?
    unsigned int spriteSize = width / columnCount;
    if (spriteSize < 8 || 0 != (spriteSize & (spriteSize - 1))) {
        return false;
    }
    // height correct in terms of SpriteSheet rows?
    float rowSize = static_cast<float>(height) / static_cast<float>(spriteSize);
    if (rowSize != static_cast<float>(static_cast<int>(rowSize))) {
        return false;
    }

    return true;
}
