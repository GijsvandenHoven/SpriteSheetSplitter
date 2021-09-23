#ifndef SPRITESHEETSPLITTER_SPLITTERJOBHANDLER_H
#define SPRITESHEETSPLITTER_SPLITTERJOBHANDLER_H

#include "splitterOptions.h"
#include "IO/SpriteSheetLoader.h"

class SplitterJobHandler {
public:
    SplitterJobHandler() = default;
    void Work(std::vector<SplitterOpts>& jobs);

private:
    SpriteSheetLoader loader;
    SpriteSheetLoader saver;
    // todo: Splitter class
};


#endif //SPRITESHEETSPLITTER_SPLITTERJOBHANDLER_H
