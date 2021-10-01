#ifndef SPRITESHEETSPLITTER_SPLITTER_H
#define SPRITESHEETSPLITTER_SPLITTER_H

#include "util/SplitterOptions.h"
#include "IO/SpriteSheetIO.h"

class Splitter {
public:
    Splitter() = default;
    void work(std::vector<SplitterOpts>& jobs);

private:
    SpriteSheetIO ssio;

    unsigned int workFolder(int workCap, std::queue<std::string>& pngs);
    unsigned int split(const std::string& fileName, std::basic_ostream<char> &outStream);
    static bool validSpriteSheet(unsigned int width, unsigned int height, unsigned int columnCount);
    static void splitObjectSheet(unsigned char* imgData, unsigned int spriteSize, unsigned int spriteCount, unsigned char** out);
    static void splitCharSheet(unsigned char* imgData, unsigned int spriteSize, unsigned int spriteCount, unsigned char** out);

    // sprite count per row of type
    static const int OBJ_SHEET_ROW = 16;
    static const int CHAR_SHEET_ROW = 7;
};

#endif //SPRITESHEETSPLITTER_SPLITTER_H
