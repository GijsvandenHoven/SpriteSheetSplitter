#include <iostream>
#include "SpriteSheetIO.h"
#include "lodepng.h"

void SpriteSheetIO::setNewPath(std::string& pathName) {
    // todo: look into c++20 feature utf8 for properly loading _any_ file. Is this sufficient?
    currentFilePath_ = path(pathName);
}

// using lodepng, and the current file path, obtain decoded png file as byte buffer.
char *SpriteSheetIO::getNextFile() {
    return nullptr;
}

