#ifndef SPRITESHEETSPLITTER_SPLITTEROPTIONS_H
#define SPRITESHEETSPLITTER_SPLITTEROPTIONS_H

#include <string>
#include <iostream>
#include <limits>
#include "RegexWrapper.hpp"

struct SplitterOpts {
    std::string inDirectory; // for both --in and --directory. uses isPNGDirectory to decide which it is. Cannot have both.
    std::string outDirectory;
    RegexWrapper groundFilePattern;
    int workAmount;
    bool isPNGInDirectory;
    bool recursive;
    bool useSubFoldersInOutput;
    bool subtractAlphaSpritesFromIndex;

    SplitterOpts()
        :   groundFilePattern(RegexWrapper("/ground/i")), workAmount(0), isPNGInDirectory(false),
            recursive(false), useSubFoldersInOutput(true),
            subtractAlphaSpritesFromIndex(false) {}

    // This is more rigorously tested by the std::filesystem class further in execution (if the file exists & if it can be loaded).
    // All that matters for now, is if the user _intends_ to run it on a directory pointing to an alleged 'png'.
    inline void setIsPNGDirectory() {
        isPNGInDirectory = inDirectory.size() >= 4 && (0 == inDirectory.compare(inDirectory.size() - 4, 4, ".png"));
    }
};

inline std::ostream&operator<<(std::ostream &o, const SplitterOpts& s) {
    o << "SplitterOptions:\n";
    o << "\tinDir: " << s.inDirectory << "\n";
    o << "\toutDir: " << s.outDirectory << "\n";
    o << "\tgroundFilePattern: " << s.groundFilePattern << "\n";
    o << "\tworkAmount: " << (s.workAmount == std::numeric_limits<int>::max() ? "infinite" : std::to_string(s.workAmount)) << "\n";
    o << "\tisPathToPNG?: " << (s.isPNGInDirectory ? "true" : "false") << "\n";
    o << "\trecursive?: " << (s.recursive ? "true" : "false") << "\n";
    o << "\tuseSubFoldersInOutput?: " << (s.useSubFoldersInOutput ? "true" : "false") << "\n";
    o << "\tsubtractAlphaSpritesFromIndex?: " << (s.subtractAlphaSpritesFromIndex ? "true" : "false") << "\n";
    return o;
}

#endif //SPRITESHEETSPLITTER_SPLITTEROPTIONS_H
