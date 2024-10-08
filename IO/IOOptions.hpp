//
// Created by 20173607 on 24/08/2023.
//

#ifndef SPRITESHEETSPLITTER_IOOPTIONS_HPP
#define SPRITESHEETSPLITTER_IOOPTIONS_HPP

#include "../util/SplitterOptions.h"
#include <set>

/**
 * IOOptions represents the options used in a SpriteSheetIO instance.
 *
 * The SpriteSheetIO instance typically takes ownership of one of these structs for each splitting job it is executing.
 *
 * IOOptions represents a subset of SplitterOpts, namely only the options relevant for the IO side.
 */
struct IOOptions {

    // default-constructed fs::path are empty string. This is fine, they will fail all checks such as fs::is_directory and fs::exists.
    IOOptions() : inDirectory(), outDirectory(), subtractAlphaFromIndex(false), useSubFolders(false) {}

    explicit IOOptions(const SplitterOpts & splitterOpts)
        :   inDirectory(std::filesystem::path(splitterOpts.inDirectory).make_preferred()),
            outDirectory(std::filesystem::path(splitterOpts.outDirectory).make_preferred()),
            groundIndexOffset(splitterOpts.groundIndexOffset.second),
            subtractAlphaFromIndex(splitterOpts.subtractAlphaSpritesFromIndex),
            useSubFolders(splitterOpts.useSubFoldersInOutput) {}

    // a note about using non-UTF8 strings as path name.
    // This means technically not all path names are supported,
    // but lodePNG (the png library this uses) does not support this either.
    // There are ways around that (obtain the encoded png bytes externally), but that is only worth doing if the need ever arises.
    std::filesystem::path inDirectory;
    std::filesystem::path outDirectory;
    std::set<SpriteSheetType> IOUsed; // used during splitting by SpriteSheetIO for single-folder mode, to warn about file overwrites. (e.g. double write of '0.png')
    int groundIndexOffset;
    bool subtractAlphaFromIndex;
    bool useSubFolders;

    // mark an enum type as 'used' for this SpriteSheetIO run.
    // returns true if the IO was used for the first time, for this enum value, for this instance of IOOptions.
    bool useIO (SpriteSheetType sst) {
        auto emplaceResult = IOUsed.emplace(sst);
        return emplaceResult.second;
    }
};

#endif //SPRITESHEETSPLITTER_IOOPTIONS_HPP
