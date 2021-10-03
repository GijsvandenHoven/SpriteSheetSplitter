#ifndef SPRITESHEETSPLITTER_SPRITESPLITTINGSTATUS_H
#define SPRITESHEETSPLITTER_SPRITESPLITTINGSTATUS_H

struct SpriteSplittingStatus {
    unsigned int n_load_error;
    unsigned int n_save_error;
    unsigned int n_success;
    unsigned int n_skipped; // e.g. fully alpha.

    SpriteSplittingStatus() : n_load_error(0), n_save_error(0), n_success(0), n_skipped(0) {}
};

inline SpriteSplittingStatus& operator+(SpriteSplittingStatus& lhs, const SpriteSplittingStatus& rhs) {
    lhs.n_load_error += rhs.n_load_error;
    lhs.n_save_error += rhs.n_save_error;
    lhs.n_success += rhs.n_success;
    lhs.n_skipped += rhs.n_skipped;

    return lhs;
}

inline SpriteSplittingStatus& operator+=(SpriteSplittingStatus& lhs, const SpriteSplittingStatus& rhs) {
    return lhs + rhs;
}

inline std::ostream& operator<<(std::ostream &o, const SpriteSplittingStatus& sst) {
    o   << "Sprite Splitting Stats:\n"
        << "\t"     << sst.n_success << " Sprites created from splitting."
        << "\n\t"   << sst.n_skipped << " Pure alpha sprites ignored."
        << "\n\t"   << sst.n_load_error << " File loading errors."
        << "\n\t"   << sst.n_save_error << " File saving errors." << "\n";

    return o;
}

#endif //SPRITESHEETSPLITTER_SPRITESPLITTINGSTATUS_H
