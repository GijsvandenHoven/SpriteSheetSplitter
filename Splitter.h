#ifndef SPRITESHEETSPLITTER_SPLITTER_H
#define SPRITESHEETSPLITTER_SPLITTER_H

#include "splitterOptions.h"
#include "IO/SpriteSheetIO.h"

class Splitter {
public:
    Splitter() = default;
    void work(std::vector<SplitterOpts>& jobs);
    bool splitNext(std::vector<unsigned char>& imgBuffer);

private:
    SpriteSheetIO ssio;
};


#endif //SPRITESHEETSPLITTER_SPLITTER_H
