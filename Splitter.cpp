#include <vector>
#include <cassert>
#include "Splitter.h"

void Splitter::work(std::vector <SplitterOpts> &jobs) {
    std::vector<unsigned char> image;
    image.reserve(16*16*16*32); // reserves enough space for a 16x16 spritesheet with 32 rows. (256x512)
    int completedCount = 0;
    for (auto& job : jobs) {

        bool inPathOK = ssio.setInPath(job.inDirectory, job.isPNGInDirectory, job.recursive);
        bool outPathOK = ssio.setOutPath(job.outDirectory);
        // only work the job if the input is OK. The only input error that can happen from now on is a png decode error or malformed spritesheet.
        if (! (inPathOK && outPathOK)) continue;

        if (job.isPNGInDirectory) {
            completedCount += splitNext(image);
        } else {
            int work = job.workAmount;
            while (work-- > 0 && ssio.hasNextPNG()) {
                completedCount += splitNext(image);
            }
        }
    }
}

/**
 *
 * @param imgBuffer A buffer to hold the image. This is first cleared when the function is called.
 *                  It is not created within the function to prevent re-allocating memory lots of times.
 * @return whether the image was successfully split
 */
bool Splitter::splitNext(std::vector<unsigned char>& imgBuffer) {
    if (! ssio.findNextPNG()) return false; // no more PNGs to work on in this path.
// todo: possible concurrency here after findNext: send off a thread to split the sheet. Because loading from disk is slow!!
    imgBuffer.clear();
    unsigned int width = 0;
    unsigned int height = 0;
    unsigned int error = ssio.load(imgBuffer, width, height);
    if (error) {
        std::cout << "[ERROR] LodePNG decode error: " << error << ". (Most likely a corrupt png)\n"; // if it's an incorrect path at this point then that is a bug!
        return false;
    }

    SpriteSheetType type;
    if (validSpriteSheet(width, height, 16)) {
        type = SpriteSheetType::OBJECT;
        std::cout << "object sheet\n";
    } else if (validSpriteSheet(width, height, 7)) {
        type = SpriteSheetType::CHARACTER;
        std::cout << "char sheet\n";
    } else {
        std::cout << "[ERROR] An image of size " << width << ", " << height << " is not a valid SpriteSheet.\n";
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
 * tests if the given image dimensions are that of a correctly formed SpriteSheet.
 * A SpriteSheet has equally sized columns (objects vs chars), where each column is at least 8px wide, and the column width is a power of 2.
 * Furthermore, the height of the SpriteSheet are divisible by the sprite size.
 *
 * @param width width of the png
 * @param height height of the png
 * @param columnCount amount of columns in the png
 * @return whether or not it is a valid SpriteSheet.
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

// todo: concurrency possible here on sheet rows.
bool Splitter::splitObjectSheet(std::vector<unsigned char> &img, unsigned int spriteSize) {
    return false;
}

bool Splitter::splitCharSheet(std::vector<unsigned char> &img, unsigned int spriteSize) {
    return false;
}
