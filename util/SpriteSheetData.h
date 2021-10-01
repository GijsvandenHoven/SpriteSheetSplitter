#ifndef SPRITESHEETSPLITTER_SPRITESHEETDATA_H
#define SPRITESHEETSPLITTER_SPRITESHEETDATA_H
#include "lodepng.h"

struct SpriteSheetData {
    unsigned int width;
    unsigned int height;
    unsigned int error;
    lodepng::State lodeState;

    SpriteSheetData() {
        // lodepng settings to encode images with the exact same settings as the source image.
        // I have only an elementary grasp on what these settings do.
        // Taken 1:1 from lodeng/examples/example_reencode.cpp
        lodeState.decoder.color_convert = 0;
        lodeState.decoder.remember_unknown_chunks = 1; //make it reproduce even unknown chunks in the saved image
        lodeState.encoder.text_compression = 1;
        // to satiate the compiler warnings. Empirically lodepng::decode will always set these.
        width = 0;
        height = 0;
        error = 0;
    }
};
#endif //SPRITESHEETSPLITTER_SPRITESHEETDATA_H
