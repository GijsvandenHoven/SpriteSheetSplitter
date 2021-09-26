#include <iostream>
#include "SpriteSheetIO.h"

bool SpriteSheetIO::setInPath(std::string& pathName, bool shouldBePNG, bool recursive) {

    delete directoryIterator;
    directoryIterator = nullptr;
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
            directoryIterator = new recursive_directory_iterator(iterator);
        } else {
            auto *iterator = new fs::directory_iterator(inFilePath_);
            directoryIterator = new directory_iterator(iterator);
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
 * Advances the directoryIterator until it points to a PNG.
 * When no directoryIterator exists, then true is returned because the inFile must be pointing to a png, so it was found.
 *
 * Note that this iterator first looks at itself, so repeated calls to just findNext would result in an infinite loop.
 *
 * @return whether or not another PNG could be found.
 */
bool SpriteSheetIO::findNextPNG() {
    if (directoryIterator == nullptr) return true; // no directory iterator was set, so input path itself is already pointing at a PNG.

    while (! directoryIterator->end()) {
        auto& dir = directoryIterator->get();
        if (dir.extension() == ".png") return true;
        directoryIterator->next();
    }
    // no more pngs.
    return false;
}

bool SpriteSheetIO::hasNextPNG() {
    return ! (directoryIterator == nullptr || directoryIterator->end());
}

/**
 * Using the LodePNG Library, attempts to decode the current inPath as PNG.
 * The current inPath is the directoryIterator (if any), or the inFilePath string.
 *
 * Calling this function writes the LodePNG::State to TODO member variable,
 * so that the correct encoding method is preserved for the split sprites.
 *
 * @param buffer vector to-be-filled with the raw pixels of the PNG
 * @return error code from lodePNG (0 = OK)
 */
unsigned int SpriteSheetIO::load(std::vector<unsigned char>& buffer) {
    std::string fileName;
    if (directoryIterator == nullptr) {
        fileName = inFilePath_.string();
    } else {
        fileName = directoryIterator->get().string();
        // also advance the iterator, loading effectively consumes the file.
        directoryIterator->next();
    }

    // todo savestate
    std::vector<unsigned char> encodedPixelBuffer;
    unsigned int error;
    unsigned int width;
    unsigned int height;
    error = lodepng::load_file(encodedPixelBuffer, fileName);
    if (!error) error = lodepng::decode(buffer, width, height, encodedPixelBuffer);

    std::cout << "loaded image with " << width << ", " << height << "\n";

    return error;
}