//
// Created by 20173607 on 24/08/2023.
//

#ifndef SPRITESHEETSPLITTER_IOOPTIONS_HPP
#define SPRITESHEETSPLITTER_IOOPTIONS_HPP

#include "../util/SplitterOptions.h"

struct IOOptions {

    IOOptions() : subtractAlphaFromIndex(false), useSubFolders(false) {}

    explicit IOOptions(const SplitterOpts & splitterOpts)
        : subtractAlphaFromIndex(splitterOpts.subtractAlphaSpritesFromIndex),
          useSubFolders(splitterOpts.useSubFoldersInOutput) {}

    bool subtractAlphaFromIndex;
    bool useSubFolders;
};

#endif //SPRITESHEETSPLITTER_IOOPTIONS_HPP
