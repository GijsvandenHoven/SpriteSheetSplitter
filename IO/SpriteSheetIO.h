#ifndef SPRITESHEETSPLITTER_SPRITESHEETIO_H
#define SPRITESHEETSPLITTER_SPRITESHEETIO_H

#include <filesystem>

using namespace std::filesystem;

class SpriteSheetIO {
public:
    SpriteSheetIO() = default;
    void setNewPath(std::string& pathName);
    char* getNextFile();

private:
    path currentFilePath_;
};


#endif //SPRITESHEETSPLITTER_SPRITESHEETIO_H
