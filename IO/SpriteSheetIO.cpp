#include <iostream>
#include "SpriteSheetIO.h"

bool SpriteSheetIO::setInPath(std::string& pathName, bool shouldBePNG, bool recursive) {

    delete directoryIterator_;
    directoryIterator_ = nullptr;
    // a note about using non-UTF8 strings as path name. This means technically not all path names are supported, but lodePNG (the png library this uses) does not support this either. There are ways around that (obtain the encoded png bytes externally), but that is only worth doing if the need ever arises.
    inFilePath_ = fs::path(pathName).make_preferred();

    if (! fs::exists(inFilePath_)) {
        std::cout << "[ERROR] The provided input directory does not exist:\n\t\t" << inFilePath_ << "\n";
        return false;
    }

    // not a directory and not a png? that's an error
    if (! (shouldBePNG || fs::is_directory(inFilePath_))) {
        std::cout << "[ERROR] The provided input directory is not a png file or folder:\n\t\t" << inFilePath_ << "\n";
        return false;
    }

    if (fs::is_directory(inFilePath_)) {
        if (recursive) {
            auto *iterator = new fs::recursive_directory_iterator(inFilePath_);
            directoryIterator_ = new recursive_directory_iterator(iterator);
        } else {
            auto *iterator = new fs::directory_iterator(inFilePath_);
            directoryIterator_ = new directory_iterator(iterator);
        }
    }

    return true;
}

bool SpriteSheetIO::setOutPath(std::string &pathName) {
    outFilePath_ = fs::path(pathName).make_preferred();

    if (! fs::is_directory(outFilePath_)) {
        outFilePath_ = outFilePath_.parent_path();
    }

    if(! fs::exists(outFilePath_)) {
        std::cout << "[ERROR] The provided output directory does not exist.\n\t\t" << outFilePath_ << "\n";
        return false;
    }

    if (! fs::is_directory(outFilePath_)) {
        std::cout << "[ERROR] The output directory must be a folder.\n\t\t" << outFilePath_ << "\n";
        return false;
    }

    return true;
}

/**
 * Advances the directoryIterator_ until it points to a PNG.
 * When no directoryIterator_ exists, then true is returned because the inFile must be pointing to a png, so it was found.
 *
 * Note that this iterator first looks at itself, so repeated calls to just findNext would result in an infinite loop.
 * Use consumeFile() to advance the iterator by one.
 *
 * @return whether or not another PNG could be found.
 */
bool SpriteSheetIO::findPNG() {
    if (directoryIterator_ == nullptr) return true; // no directory iterator was set, so input path itself is already pointing at a PNG.

    while (! directoryIterator_->end()) {
        auto& dir = **directoryIterator_;
        if (dir.extension() == ".png") return true;
        ++*directoryIterator_;
    }
    // no more pngs.
    return false;
}

void SpriteSheetIO::consumeFile() {
    if (directoryIterator_ == nullptr || directoryIterator_->end()) return;

    ++*directoryIterator_;
}

bool SpriteSheetIO::hasUncheckedFiles() {
    return ! (directoryIterator_ == nullptr || directoryIterator_->end());
}

std::string SpriteSheetIO::pathName() {
    if (directoryIterator_ == nullptr) {
        return inFilePath_.string();
    } else {
        return (*directoryIterator_)->string();
    }
}

/**
 * Using the LodePNG Library, attempts to decode the fileName as PNG.
 *
 * @param fileName path to the PNG.
 * @param buffer Vector to-be-filled with the raw pixels of the PNG
 * @param data struct containing metadata from the spritesheet, like dimensions and lodepng decode state.
 * @return error code from lodePNG (0 = OK)
 */
unsigned int SpriteSheetIO::loadPNG(const std::string& fileName, std::vector<unsigned char> &buffer, SpriteSheetData& data) {
    std::cout << "[INFO] Loading " << fileName << "\n";
    unsigned int& error = data.error;
    std::vector<unsigned char> encodedPixelBuffer;

    error = lodepng::load_file(encodedPixelBuffer, fileName);
    if (!error) error = lodepng::decode(buffer, data.width, data.height, data.lodeState, encodedPixelBuffer);

    return error;
}

SpriteSheetIO::~SpriteSheetIO() {
    delete directoryIterator_;
}