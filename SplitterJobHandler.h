#ifndef SPRITESHEETSPLITTER_SPLITTERJOBHANDLER_H
#define SPRITESHEETSPLITTER_SPLITTERJOBHANDLER_H

#include "splitterOptions.h"
#include "IO/SpriteSheetIO.h"

class SplitterJobHandler {
public:
    SplitterJobHandler() = default;
    void Work(std::vector<SplitterOpts>& jobs);

private:
    SpriteSheetIO ssio;
    // todo: Splitter class
};


#endif //SPRITESHEETSPLITTER_SPLITTERJOBHANDLER_H
