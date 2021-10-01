#include <vector>
#include <thread>
#include <syncstream>
#include <future>
#include "Splitter.h"

void Splitter::work(std::vector <SplitterOpts> &jobs) {
    // todo: better counting (fail count, complete count, possibly why something failed)
    int completedCount = 0;
    for (auto& job : jobs) {
        // todo: to really push the concurrency, any job could be its own process.
        bool inPathOK = ssio.setInPath(job.inDirectory, job.isPNGInDirectory, job.recursive);
        bool outPathOK = ssio.setOutPath(job.outDirectory);
        // only work the job if the input is OK. The only input error that can happen from now on is a png decode error or malformed spritesheet.
        if (! (inPathOK && outPathOK)) continue;

        std::queue<std::string> pngQueue;
        ssio.getPNGQueue(pngQueue);

        if (job.isPNGInDirectory) {
            std::string& onlyFile = pngQueue.front();
            completedCount += split(onlyFile, std::cout);
            pngQueue.pop();
        } else {
            workFolder(job.workAmount, pngQueue);
        }
    }
}

int Splitter::workFolder(int workCap, std::queue<std::string>& pngs) {
    while (workCap-- > 0 && ! pngs.empty()) {
        std::string& file = pngs.front();
        // for printing without data races
        std::osyncstream synced_out(std::cout);
        // todo: threading goes here
        split(file, synced_out);

        // remove from queue after being done with the string
        pngs.pop();
    }

    // wait for all threads to end.

    return 0;
}

/**
 *
 * @param imgBuffer A buffer to hold the image. This is first cleared when the function is called.
 *                  It is not created within the function to prevent re-allocating memory lots of times.
 * @param outStream stream for printing characters. Normally std::cout, but could be std::osyncstream from threading.
 * @return whether the image was successfully split
 */
bool Splitter::split(const std::string &fileName, std::basic_ostream<char>& outStream) {
    std::vector<unsigned char> img;
    SpriteSheetData ssd;

    SpriteSheetIO::loadPNG(fileName, img, ssd);

    if (ssd.error) {
        outStream << "[ERROR] LodePNG decode error: " << ssd.error << ". (Most likely a corrupt png)\n"; // if it's an incorrect path at this point then that is a bug!
        return false;
    }

    SpriteSheetType type;
    if (validSpriteSheet(ssd.width, ssd.height, OBJ_SHEET_ROW)) {
        type = SpriteSheetType::OBJECT;
        outStream << "Detected as Object sheet.\n";
    } else if (validSpriteSheet(ssd.width, ssd.height, CHAR_SHEET_ROW)) {
        type = SpriteSheetType::CHARACTER;
        outStream << "Detected as Char sheet\n";
    } else {
        outStream << "[ERROR] An image of size " << ssd.width << ", " << ssd.height << " is not a valid SpriteSheet.\n";
        return false;
    }

    unsigned int spriteSize; // size of a sprite (8, 16, 32..)
    unsigned int spriteCount; // amount of unsigned char* to expect back from splitting.
    unsigned char** spriteData; // array of SpriteSheet row indices, filled by upcoming split function.
    bool saveSuccess;
    switch(type) {
        case SpriteSheetType::OBJECT:
            spriteSize = ssd.width / OBJ_SHEET_ROW;
            spriteCount = (img.size() / 4) / (spriteSize * spriteSize);
            spriteData = new unsigned char* [spriteSize * spriteCount]; // rows per sprite * amount of sprites

            splitObjectSheet(img.data(), spriteSize, spriteCount, spriteData);

            saveSuccess = ssio.saveObjectSplits(spriteData, spriteSize, spriteCount, ssd.lodeState, fileName);
            break;
        case SpriteSheetType::CHARACTER:
            spriteSize = ssd.width / CHAR_SHEET_ROW;
            spriteData = nullptr;
            //splitCharSheet(img, spriteSize, spriteCount);
            //break;
            return false; // todo
        default: // did you add a new type to the enum?
            std::cout << "[ERROR] unknown SpriteSheetType" << type << "\n";
            exit(-1);
    }

    delete[] spriteData;
    spriteData = nullptr;

    // todo
    return saveSuccess;
}

void Splitter::splitObjectSheet(unsigned char* imgData, unsigned int spriteSize, unsigned int spriteCount, unsigned char** out) {
    // 16 rows, 4 uchar per pixel.
    unsigned int sheetPixelWidth = spriteSize * OBJ_SHEET_ROW * 4;
    // todo: (auto) vectorization? openMP go brr?
    for (int i = 0; i < spriteCount; ++i) {
        for (int j = 0; j < spriteSize; ++j) {
            //    (linear index)    =           (sprite row offset)                   + (sprite column offset)  + (pixel row offset)
            out[i * spriteSize + j] = imgData + (i/16) * spriteSize * sheetPixelWidth + (i%16) * spriteSize * 4 + j * sheetPixelWidth;
        }
    }
}

void Splitter::splitCharSheet(std::vector<unsigned char>& img, unsigned int spriteSize, std::vector<unsigned char**>& out) {
    /* Char sheets are special. They consist of seven columns, where each row belongs to a single char.
     * Column 0 is their idle frame.
     * Column 1 and 2 are their walking frames.
     * Column 3 is always empty. (Mightve been intended for a fancy walk frame. Never happened.)
     * Column 4 and (5+6) are attack frames.
     * Column 5 and 6 are joined as one sprite for i.e. an extended arm holding a sword.
     * Meaning all sprites here are square except for the 5th, it is a size x 2size rectangle!
     */
    int rowIndex = 0;
    auto sprite = new unsigned char*[spriteSize];
    unsigned char* imgData = img.data();
    // divide (bytes per pixel) (sprite height) (sprite width). THEN correct for 2 fewer sprites per row. (3 empty, 5+6 joined)
    unsigned int spriteCount = ((img.size() / 4) / spriteSize) / spriteSize;
    spriteCount = (spriteCount / CHAR_SHEET_ROW) * (CHAR_SHEET_ROW - 2);
    std::cout << "DEBUG: # sprites on this sheet: " << spriteCount << "\n";
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
    // equally sized columns?
    float columnSize = static_cast<float>(width) / static_cast<float>(columnCount);
    if (columnSize != static_cast<float>(static_cast<int>(columnSize))) {
        return false;
    }
    // columns are a power of 2 and at least 8?
    unsigned int spriteSize = width / columnCount;
    if (spriteSize < 8 || 0 != (spriteSize & (spriteSize - 1))) {
        return false;
    }
    // height correct in terms of spritesheet rows?
    float rowSize = static_cast<float>(height) / static_cast<float>(spriteSize);
    if (rowSize != static_cast<float>(static_cast<int>(rowSize))) {
        return false;
    }

    return true;
}
