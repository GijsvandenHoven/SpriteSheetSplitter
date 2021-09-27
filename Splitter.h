#ifndef SPRITESHEETSPLITTER_SPLITTER_H
#define SPRITESHEETSPLITTER_SPLITTER_H

#include "splitterOptions.h"
#include "IO/SpriteSheetIO.h"

class Splitter {
public:
    Splitter() = default;
    void work(std::vector<SplitterOpts>& jobs);
    static bool split(const std::string& fileName, std::basic_ostream<char>&);

private:
    SpriteSheetIO ssio;

    int workFolder(int workCap);
    static bool validSpriteSheet(unsigned int width, unsigned int height, unsigned int columnCount);
    bool splitObjectSheet(std::vector<unsigned char>& img, unsigned int spriteSize);
    bool splitCharSheet(std::vector<unsigned char>& img, unsigned int spriteSize);

    enum SpriteSheetType {
        OBJECT = 0,
        CHARACTER = 1,
    };
};


#endif //SPRITESHEETSPLITTER_SPLITTER_H
