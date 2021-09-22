#ifndef SPRITESHEETSPLITTER_SPLITTEROPTIONS_H
#define SPRITESHEETSPLITTER_SPLITTEROPTIONS_H

struct SplitterOpts {
    std::string inDirectory; // for both --in and --directory. uses isPNGDirectory to decide which it is. Cannot have both.
    std::string outDirectory;
    int workAmount;
    bool isPNGInDirectory;
    bool recursive;

    SplitterOpts() : workAmount(0), isPNGInDirectory(false), recursive(false) {}
};

std::ostream&operator<<(std::ostream &o, const SplitterOpts& s) {
    o << "inDir: " << s.inDirectory << "\n";
    o << "outDir: " << s.outDirectory << "\n";
    o << "workAmount: " << s.workAmount << "\n";
    o << "isPNGDir?: " << (s.isPNGInDirectory ? "true" : "false") << "\n";
    o << "recursive?: " << (s.recursive ? "true" : "false") << "\n";
    return o;
}

#endif //SPRITESHEETSPLITTER_SPLITTEROPTIONS_H
