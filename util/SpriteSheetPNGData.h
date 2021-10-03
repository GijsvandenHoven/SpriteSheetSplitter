#ifndef SPRITESHEETSPLITTER_SPRITESHEETPNGDATA_H
#define SPRITESHEETSPLITTER_SPRITESHEETPNGDATA_H
#include "lodepng.h"

struct SpriteSheetPNGData {
    unsigned int width;
    unsigned int height;
    unsigned int error;
    lodepng::State lodeState;

    SpriteSheetPNGData() {
        lodeState.decoder.remember_unknown_chunks = 1; //make it reproduce even unknown chunks in the saved image
        lodeState.encoder.text_compression = 1; // something about better compression? I don't really know. Taken from lodepng/examples/reencode.cpp
        // Splitter DEPENDS on this. It expects to get the pixels in RGBA format, 1 byte each.
        lodeState.info_raw.colortype = LCT_RGBA;
        // Output settings. It appears all files in dev/frontend/data/images are 32 bit LCT_RGBA.
        lodeState.info_png.color.colortype = LCT_RGBA;
        lodeState.info_png.color.bitdepth = 8;
        lodeState.encoder.auto_convert = 0; // without th is it will automatically pick the best value for us.
        // to satiate the compiler warnings. Empirically lodepng::decode will always set these.
        width = 0;
        height = 0;
        error = 0;
    }
};
#endif //SPRITESHEETSPLITTER_SPRITESHEETPNGDATA_H
