#ifndef SPRITESHEETSPLITTER_SPLITTER_H
#define SPRITESHEETSPLITTER_SPLITTER_H

#include "util/splitterOptions.h"
#include "IO/SpriteSheetIO.h"

class Splitter {
public:
    Splitter() = default;
    void work(std::vector<SplitterOpts>& jobs);

private:
    SpriteSheetIO ssio;

    int workFolder(int workCap, std::queue<std::string>& pngFileNames);
    bool split(const std::string& fileName, std::basic_ostream<char>&);
    static bool validSpriteSheet(unsigned int width, unsigned int height, unsigned int columnCount);
    static void splitObjectSheet(unsigned char* imgData, unsigned int spriteSize, unsigned int spriteCount, unsigned char** out);
    static void splitCharSheet(std::vector<unsigned char>& img, unsigned int spriteSize, std::vector<unsigned char**>& out);

    // sprite count per row of type
    static const int OBJ_SHEET_ROW = 16;
    static const int CHAR_SHEET_ROW = 7;
};

#endif //SPRITESHEETSPLITTER_SPLITTER_H
