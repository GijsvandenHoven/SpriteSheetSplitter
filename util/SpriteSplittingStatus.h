#ifndef SPRITESHEETSPLITTER_SPRITESPLITTINGSTATUS_H
#define SPRITESHEETSPLITTER_SPRITESPLITTINGSTATUS_H

struct SpriteSplittingStatus {
    unsigned int n_error;
    unsigned int n_success;
    unsigned int n_skipped; // e.g. fully alpha.
};

#endif //SPRITESHEETSPLITTER_SPRITESPLITTINGSTATUS_H
