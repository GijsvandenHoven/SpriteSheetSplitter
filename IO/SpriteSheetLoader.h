#ifndef SPRITESHEETSPLITTER_SPRITESHEETLOADER_H
#define SPRITESHEETSPLITTER_SPRITESHEETLOADER_H

#include <filesystem>

using namespace std::filesystem;

class SpriteSheetLoader {
public:
    SpriteSheetLoader() = default;
    void setNewPath(std::string& pathName);

private:
    path currentFilePath_;
};


#endif //SPRITESHEETSPLITTER_SPRITESHEETLOADER_H
