#ifndef SPRITESHEETSPLITTER_SPRITESPLITTINGDATA_H
#define SPRITESHEETSPLITTER_SPRITESPLITTINGDATA_H

struct SpriteSplittingData {
    unsigned char* spriteSheet; // pointer to the (decoded) RGBA pixels of a SpriteSheet
    unsigned char** splitSprites; // collection of byte pointers to [spriteSize] rows of individual sprites.
    const unsigned int spriteSize; // height of the sprite, usually also square size (except for e.g. frame 5 of character sheets)
    const unsigned int spriteCount; // amount of sprites on the sheet. Not necessarily the amount of drawn sprites actually on the sheet: the max amount of sprites that would fit on the SpriteSheet
    const SpriteSheetType& sheetType; // the type of sheet, e.g. Object sheet or Character sheet.
    lodepng::State& lodeState; // LodePNG library for encoding/decoding PNG files
    const std::string& originalFileName; // original (absolute) path to the SpriteSheet file.
    SpriteSplittingStatus& stats; // stat tracking object

    SpriteSplittingData() = delete;
    SpriteSplittingData(unsigned char* _spriteSheet, unsigned char** _splitSprites,
                        unsigned int _spriteSize, unsigned int _spriteCount,
                        const SpriteSheetType& _type, lodepng::State& _lodeState,
                        const std::string& _originalFileName, SpriteSplittingStatus& _stats) :

            spriteSheet(_spriteSheet), splitSprites(_splitSprites),
            spriteSize(_spriteSize), spriteCount(_spriteCount),
            sheetType(_type), lodeState(_lodeState),
            originalFileName(_originalFileName), stats(_stats)

            {/*end of constructor*/}
};

#endif //SPRITESHEETSPLITTER_SPRITESPLITTINGDATA_H
