//
// Created by 20173607 on 24/08/2023.
//

#ifndef SPRITESHEETSPLITTER_IOOPTIONS_HPP
#define SPRITESHEETSPLITTER_IOOPTIONS_HPP

#include "../util/SplitterOptions.h"

struct IOOptions {

    // default-constructed fs::path are empty string. This is fine, they will fail all checks such as fs::is_directory and fs::exists.
    IOOptions() : inDirectory(), outDirectory(), subtractAlphaFromIndex(false), useSubFolders(false) {}

    explicit IOOptions(const SplitterOpts & splitterOpts)
        :   inDirectory(std::filesystem::path(splitterOpts.inDirectory).make_preferred()),
            outDirectory(std::filesystem::path(splitterOpts.outDirectory).make_preferred()),
            subtractAlphaFromIndex(splitterOpts.subtractAlphaSpritesFromIndex),
            useSubFolders(splitterOpts.useSubFoldersInOutput) {}

    // a note about using non-UTF8 strings as path name.
    // This means technically not all path names are supported,
    // but lodePNG (the png library this uses) does not support this either.
    // There are ways around that (obtain the encoded png bytes externally), but that is only worth doing if the need ever arises.
    std::filesystem::path inDirectory;
    std::filesystem::path outDirectory;
    bool subtractAlphaFromIndex;
    bool useSubFolders;
};

#endif //SPRITESHEETSPLITTER_IOOPTIONS_HPP
