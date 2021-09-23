#include <iostream>
#include "SpriteSheetLoader.h"

void SpriteSheetLoader::setNewPath(std::string& pathName) {
    // todo: look into c++20 feature utf8 for properly loading _any_ file. Is this sufficient?
    currentFilePath_ = path(pathName);
}

