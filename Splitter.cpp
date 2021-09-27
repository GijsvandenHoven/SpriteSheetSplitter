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

        if (job.isPNGInDirectory) {
            completedCount += split(ssio.pathName(), std::cout);
        } else {
            workFolder(job.workAmount);
        }
    }
}

int Splitter::workFolder(int workCap) {
    while (workCap-- > 0 && ssio.hasUncheckedFiles()) {
        if (ssio.findPNG()) {
            // found png, get path name, advance internal file iterator for next png search.
            std::string fileName = ssio.pathName();
            ssio.consumeFile();
            // for printing without any data races.
            std::osyncstream synced_out(std::cout);

            // todo: thread
        }
    }

    return 0;
}

/**
 *
 * @param imgBuffer A buffer to hold the image. This is first cleared when the function is called.
 *                  It is not created within the function to prevent re-allocating memory lots of times.
 * @param out stream for printing characters. Normally std::cout, but could be std::osyncstream from threading.
 * @return whether the image was successfully split
 */
bool Splitter::split(const std::string &fileName, std::basic_ostream<char>& out) {
    std::vector<unsigned char> img;
    SpriteSheetData ssd;

    SpriteSheetIO::loadPNG(fileName, img, ssd);

    if (ssd.error) {
        std::cout << "[ERROR] LodePNG decode error: " << ssd.error << ". (Most likely a corrupt png)\n"; // if it's an incorrect path at this point then that is a bug!
        return false;
    }

    SpriteSheetType type;
    if (validSpriteSheet(ssd.width, ssd.height, 16)) {
        type = SpriteSheetType::OBJECT;
        std::cout << "object sheet\n";
    } else if (validSpriteSheet(ssd.width, ssd.height, 7)) {
        type = SpriteSheetType::CHARACTER;
        std::cout << "char sheet\n";
    } else {
        std::cout << "[ERROR] An image of size " << ssd.width << ", " << ssd.height << " is not a valid SpriteSheetData.\n";
        return false;
    }

    switch(type) {
        case SpriteSheetType::OBJECT:
            return true;
        case SpriteSheetType::CHARACTER:
            return true;
    }

    // did you add a SpriteSheetType without handling in type?
    return false;
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
    // 16 equally sized columns?
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

bool Splitter::splitObjectSheet(std::vector<unsigned char> &img, unsigned int spriteSize) {
    return false;
}

bool Splitter::splitCharSheet(std::vector<unsigned char> &img, unsigned int spriteSize) {
    return false;
}
