#ifndef SPRITESHEETSPLITTER_SPLITTEROPTIONS_H
#define SPRITESHEETSPLITTER_SPLITTEROPTIONS_H
#include <string>
#include <iostream>
#include <limits>

struct SplitterOpts {
    std::string inDirectory; // for both --in and --directory. uses isPNGDirectory to decide which it is. Cannot have both.
    std::string outDirectory;
    int workAmount;
    bool isPNGInDirectory;
    bool recursive;
    bool useSubFoldersInOutput;
    bool subtractAlphaSpritesFromIndex;

    SplitterOpts() : workAmount(0), isPNGInDirectory(false), recursive(false), useSubFoldersInOutput(true), subtractAlphaSpritesFromIndex(false) {}
};

inline std::ostream&operator<<(std::ostream &o, const SplitterOpts& s) {
    o << "inDir: " << s.inDirectory << "\n";
    o << "outDir: " << s.outDirectory << "\n";
    o << "workAmount: " << (s.workAmount == std::numeric_limits<int>::max() ? "infinite" : std::to_string(s.workAmount)) << "\n";
    o << "isPathToPNG?: " << (s.isPNGInDirectory ? "true" : "false") << "\n";
    o << "recursive?: " << (s.recursive ? "true" : "false") << "\n";
    o << "useSubFoldersInOutput?: " << (s.useSubFoldersInOutput ? "true" : "false") << "\n";
    o << "subtractAlphaSpritesFromIndex?: " << (s.subtractAlphaSpritesFromIndex ? "true" : "false") << "\n";
    return o;
}

#endif //SPRITESHEETSPLITTER_SPLITTEROPTIONS_H
