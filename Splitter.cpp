#include <vector>
#include <thread>
#include <syncstream>
#include <future>
#include "Splitter.h"

void Splitter::work(std::vector <SplitterOpts> &jobs) {
    SpriteSplittingStatus jobStats{};

    int jobCounter = 0;
    for (auto& job : jobs) {
        // todo: to really push the concurrency, any job could be its own process.
        bool inPathOK = ssio.setInPath(job.inDirectory, job.isPNGInDirectory, job.recursive);
        bool outPathOK = ssio.setOutPath(job.outDirectory);
        // only work the job if the input is OK. The only input error that can happen from now on is a png decode error or malformed SpriteSheet.
        if (! (inPathOK && outPathOK)) continue;

        std::queue<std::string> pngQueue;
        ssio.fillPNGQueue(pngQueue);

        if (job.isPNGInDirectory) {
            std::string& onlyFile = pngQueue.front();
            split(onlyFile, jobStats, std::cout);
            pngQueue.pop();
        } else {
            workFolder(job.workAmount, pngQueue, jobStats);
        }

        std::cout << "DONE with job " << ++jobCounter << " out of " << jobs.size() << "\n";
    }

    std::cout << jobStats;
}

/**
 * Split all PNGs of a folder by following the string filepaths in the pngs queue.
 *
 * (todo) this is done with one thread per file for adequate performance
 *
 * @param workCap the maximum amount of files to process before stopping
 * @param pngs the queue of FilePaths to SpriteSheets
 * @param jobStats stat tracking object
 */
void Splitter::workFolder(int workCap, std::queue<std::string> &pngs, SpriteSplittingStatus &jobStats) const {

    // todo: transform to forloop, pragma omp parallel for
    while (workCap-- > 0 && ! pngs.empty()) {
        std::string& file = pngs.front(); // todo pragma omp atomic
        // for printing without data races. Downside, only prints when the object is destroyed (end of loop iteration).
        std::osyncstream synced_out(std::cout);

        SpriteSplittingStatus individualJobStats{};
        split(file, individualJobStats, synced_out);

        // remove from queue after being done with the string
        pngs.pop();
        // update status todo: pragma omp atomic
        jobStats += individualJobStats;
    }

    // wait for all threads to end.
}

/**
 * Load a SpriteSheet from the given fileName, split the data in single sprites with the correct name, then save.
 *
 * Automatically detects the SpriteSheet type (if any).
 * Does not split fully invisible (alpha 0 on every pixel) objects or chars. Chars with only some invisible frames are OK.
 *
 * Automatically determines the name of a folder based on the SpriteSheet name, unless (todo specified otherwise)
 *
 * @param fileName A path to a .png SpriteSheet file.
 * @param outStream stream for printing characters. Normally std::cout, but could be std::osyncstream from threading.
 * @param jobStats struct for counting stats of splitting.
 */
void Splitter::split(const std::string &fileName, SpriteSplittingStatus &jobStats, std::basic_ostream<char> &outStream) const {
    std::vector<unsigned char> img;
    SpriteSheetData ssd;

    outStream << "[INFO] Loading " << fileName << "\n";

    SpriteSheetIO::loadPNG(fileName, img, ssd);

    if (ssd.error) {
        outStream << "[ERROR] LodePNG decode error: " << ssd.error << ". (Most likely a corrupt png)\n"; // if it's an incorrect path at this point then that is a bug!
        jobStats.n_load_error += 1;
        return;
    }

    // todo: for types that are not discernible from sheet dimensions alone, this info could be supplied as optional parameter or function overload (latter needs loading routine above moved to dedicated function).
    // todo: Then transform this into something where the default branch is this code, and supplied parameter simply set the type after calling validSpriteSheet.
    SpriteSheetType type;
    if (validSpriteSheet(ssd.width, ssd.height, OBJ_SHEET_ROW)) {
        type = SpriteSheetType::OBJECT;
    } else if (validSpriteSheet(ssd.width, ssd.height, CHAR_SHEET_ROW)) {
        type = SpriteSheetType::CHARACTER;
    } else {
        outStream << "[ERROR] An image of size " << ssd.width << ", " << ssd.height << " is not a valid SpriteSheet.\n";
        jobStats.n_load_error += 1;
        return;
    }

    outStream << "[INFO] Processing file as " << type << " sheet\n";

    unsigned int spriteSize; // size of a sprite (8, 16, 32..)
    unsigned int spriteCount; // amount of unsigned char* to expect back from splitting.
    unsigned char** spriteData; // array of SpriteSheet row indices, filled by upcoming split function.
    switch(type) {
        case SpriteSheetType::OBJECT:
            spriteSize = ssd.width / OBJ_SHEET_ROW;
            spriteCount = (img.size() / 4) / (spriteSize * spriteSize);
            spriteData = new unsigned char* [spriteSize * spriteCount]; // rows per sprite * amount of sprites

            splitObjectSheet(img.data(), spriteSize, spriteCount, spriteData);

            ssio.saveObjectSplits(spriteData, spriteSize, spriteCount, ssd.lodeState, fileName, jobStats);
            break;
        case SpriteSheetType::CHARACTER:
            spriteSize = ssd.width / CHAR_SHEET_ROW;
            // correct for column 3 being empty, and 5+6 being joined (see splitCharSheet function comment)
            spriteCount = ((img.size() / 4) / (spriteSize * spriteSize));
            spriteCount = (spriteCount / CHAR_SHEET_ROW) * (CHAR_SHEET_ROW - 2);
            spriteData = new unsigned char* [spriteSize * spriteCount]; // rows per sprite * amount of sprites

            splitCharSheet(img.data(), spriteSize, spriteCount, spriteData);

            ssio.saveCharSplits(spriteData, spriteSize, spriteCount, ssd.lodeState, fileName, jobStats);
            break;
        default: // did you add a new type to the enum?
            outStream << "[ERROR] unknown SpriteSheetType" << type << "\n";
            exit(-1);
    }

    outStream << "[INFO] Finished splitting SpriteSheet.\n";

    delete[] spriteData;
}

/**
 * Given a pointer to a SpriteSheets raw pixel data, and the amount of object sprites there are in it,
 * fills a collection of byte pointers such that every [spriteSize] pointers forms a singe sprite.
 * Each individual pointer is a row of sprite data.
 *
 * Assumes that the collection has enough space allocated to do this (spriteSize * spriteCount).
 *
 * @param imgData pointer to the raw bytes of the SpriteSheet
 * @param spriteSize Size of a single sprite (8,16,32,...)
 * @param spriteCount Amount of sprites that fit on the sheet
 * @param out The collection of pointers to fill.
 */
void Splitter::splitObjectSheet(unsigned char* imgData, unsigned int spriteSize, unsigned int spriteCount, unsigned char** out) {
    // 16 rows, 4 uchar per pixel.
    unsigned int sheetPixelWidth = spriteSize * OBJ_SHEET_ROW * 4;
#pragma omp simd collapse(2)
    for (int i = 0; i < spriteCount; ++i) {
        for (int j = 0; j < spriteSize; ++j) {
            //    (linear index)    =           (sprite row offset)                              + (sprite column offset)             + (pixel row offset)
            out[i * spriteSize + j] = imgData + (i/OBJ_SHEET_ROW) * spriteSize * sheetPixelWidth + (i%OBJ_SHEET_ROW) * spriteSize * 4 + j * sheetPixelWidth;
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
 * @param imgData pointer to the raw bytes of the SpriteSheet
 * @param spriteSize Size of a single sprite (8,16,32,...)
 * @param spriteCount Amount of sprites that fit on the sheet.
 *                    Keep in mind skipped and conjoined columns for Char sheets, the function accounts for this.
 * @param out The collection of pointers to fill.
 */
void Splitter::splitCharSheet(unsigned char* imgData, unsigned int spriteSize, unsigned int spriteCount, unsigned char** out) {
    /* Char sheets are special. They consist of seven columns, where each row belongs to a single char.
     * Column 0 is their idle frame.
     * Column 1 and 2 are their walking frames.
     * Column 3 is always empty. (Might've been intended for a fancy walk frame. Never happened.)
     * Column 4 and (5+6) are attack frames.
     * Column 5 and 6 are joined as one sprite for i.e. an extended arm holding a sword.
     * Meaning all sprites here are square except for the 5th, it is a size x 2size rectangle!
     */
    unsigned int sheetPixelWidth = spriteSize * CHAR_SHEET_ROW * 4;
    const int CHARS_PER_ROW = CHAR_SHEET_ROW - 2;
    auto columnOffset = [](int i)->bool { return i % CHARS_PER_ROW >= 3; };
#pragma omp simd collapse(2)
    for (int i = 0; i < spriteCount; ++i) {
        for (int j = 0; j < spriteSize; ++j) {
            // linear index         =           (sprite row offset)                                + (sprite column offset + (extra offset, skips column 3))  + (pixel row offset)
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
