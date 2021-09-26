#include <iostream>
#include "SpriteSheetIO.h"
#include "lodepng.h"

bool SpriteSheetIO::setInPath(std::string& pathName, bool shouldBePNG, bool recursive) {

    delete directoryIterator;
    directoryIterator = nullptr;
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

// using lodepng, and the current file path, obtain decoded png file as byte buffer.
unsigned SpriteSheetIO::load(std::vector<unsigned char>& buffer) const {
    return 0;
}
