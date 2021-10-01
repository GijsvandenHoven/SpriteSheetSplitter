#include <iostream>
#include "SpriteSheetIO.h"

// did anyone ever mention the naming scheme of these frames is stupid? There, I said it :).
const std::map<CharSheetNames, std::string> SpriteSheetIO::CHAR_SHEET_TYPE_TO_NAME { // NOLINT(cert-err58-cpp) (Warns that this can throw an exception before main is called. It's true, map constructor isn't noexcept, but...)
        {CharSheetNames::IDLE, "Right_Walk_0"},
        {CharSheetNames::WALK_1, "Right_Walk_1"},
        {CharSheetNames::WALK_2, "Right_Walk_2"},
        {CharSheetNames::ATTACK_1, "Right_Attack_0"},
        {CharSheetNames::ATTACK_2, "Right_Attack_1"},
};
/**
 * Validates the given path and sets the inFilePath member variable. Note, the member variable is set even if invalid.
 * @param pathName the pathName to set the in path to.
 * @param shouldBePNG whether the given path should be a .png file (or a directory if not).
 * @param recursive (only used for directory paths) whether folders within this folder are also to be considered.
 * @return if the path was valid.
 */
bool SpriteSheetIO::setInPath(const std::string& pathName, bool shouldBePNG, bool recursive) {

    delete directoryIterator_;
    directoryIterator_ = nullptr;
    // a note about using non-UTF8 strings as path name. This means technically not all path names are supported, but lodePNG (the png library this uses) does not support this either. There are ways around that (obtain the encoded png bytes externally), but that is only worth doing if the need ever arises.
    inFilePath_ = fs::path(pathName).make_preferred();

    if (! fs::exists(inFilePath_)) {
        std::cout << "[ERROR] The provided input directory does not exist:\n\t\t" << inFilePath_ << "\n";
        return false;
    }

    // not a directory and not a png? that's an error
    if (! (shouldBePNG || fs::is_directory(inFilePath_))) {
        std::cout << "[ERROR] The provided input directory is not a png file or folder:\n\t\t" << inFilePath_ << "\n";
        return false;
    }

    if (fs::is_directory(inFilePath_)) {
        if (recursive) {
            auto *iterator = new fs::recursive_directory_iterator(inFilePath_);
            directoryIterator_ = new recursive_directory_iterator(iterator);
        } else {
            auto *iterator = new fs::directory_iterator(inFilePath_);
            directoryIterator_ = new directory_iterator(iterator);
        }
    }

    return true;
}

/**
 * Validates the given path and sets the outFilePath member variable. Note that the member variable is set even if the path is invalid.
 * @param pathName the path to validate and set
 * @return whether the path was valid.
 */
bool SpriteSheetIO::setOutPath(const std::string &pathName) {
    outFilePath_ = fs::path(pathName).make_preferred();

    if (! fs::is_directory(outFilePath_)) {
        outFilePath_ = outFilePath_.parent_path();
    }

    if(! fs::exists(outFilePath_)) {
        std::cout << "[ERROR] The provided output directory does not exist.\n\t\t" << outFilePath_ << "\n";
        return false;
    }

    if (! fs::is_directory(outFilePath_)) {
        std::cout << "[ERROR] The output directory must be a folder.\n\t\t" << outFilePath_ << "\n";
        return false;
    }

    return true;
}

/**
 * Creates a queue of all png files in the directory/directories represented by directoryIterator.
 * When directoryIterator is nullptr, uses only the InfilePath instead (e.g. when infile is a .png itself)
 *
 * @param q the queue to fill
 */
void SpriteSheetIO::fillPNGQueue(std::queue<std::string> &q) {
    if (directoryIterator_ == nullptr) {
        q.emplace(std::move(inFilePath_.string()));
    } else {
        for (auto& dirIter = *directoryIterator_ ; ! dirIter.end() ; ++dirIter) {
            auto& dir = *dirIter;
            if (dir.extension() == ".png") {
                q.emplace(std::move(dirIter->string()));
            }
        }
    }
}

/**
 * Using the LodePNG Library, attempts to decode the fileName as PNG.
 *
 * @param fileName path to the PNG.
 * @param buffer Vector to-be-filled with the raw pixels of the PNG
 * @param data struct containing metadata from the SpriteSheet, like dimensions and lodePNG decode state.
 * @return error code from lodePNG (0 = OK)
 */
unsigned int SpriteSheetIO::loadPNG(const std::string& fileName, std::vector<unsigned char> &buffer, SpriteSheetData& data) {
    unsigned int& error = data.error;
    std::vector<unsigned char> encodedPixelBuffer;

    error = lodepng::load_file(encodedPixelBuffer, fileName);
    if (!error) error = lodepng::decode(buffer, data.width, data.height, data.lodeState, encodedPixelBuffer);

    return error;
}

/**
 * Given a sprite amount and size,
 * saves a given collection of byte pointers as single sprite files on disk,
 * based on the LodePNGState and name of the original SpriteSheet.
 *
 * Every byte pointer in the collection points to a (spriteSize) sized row of RGBA bytes.
 * Assumes every sprite is a square shape.
 *
 * @param data the collection of byte pointers
 * @param spriteSize the (square) size of the sprite
 * @param spriteCount the amount of sprites in the collection.
 * @param lodeState the LodePNG library 'State' object, used for encoding and decoding files.
 * @param originalFileName the name of the SpriteSheet these bytes originally came from.
 * @return the amount of errors (todo: success and fail and alpha(?) struct)
 */
unsigned int SpriteSheetIO::saveObjectSplits(unsigned char** data, const unsigned int spriteSize, const unsigned int spriteCount, lodepng::State& lodeState, const std::string& originalFileName) {
    auto* sprite = new unsigned char[spriteSize * spriteSize * 4];
    int skippedSprites = 0;
    unsigned int error = 0;
    std::string folderName = folderNameFromSheetName(originalFileName, SpriteSheetType::OBJECT);
    // for each sprite
    for (int i = 0; i < spriteCount; ++i) {
        // for each sprite row
        for (int j = 0; j < spriteSize; ++j) {
            // access the pointer to sprite rows in the data.
            unsigned char* spriteRow = data[i * spriteSize + j];
            memcpy(sprite + j * spriteSize * 4, spriteRow, spriteSize * 4);
        }
        // need to check if a sprite is pure alpha (then don't save it)
        uint32_t alpha = 0; // safe to add with until square sprites of size 2^31
#pragma omp simd reduction(+:alpha)
        for (int x = 3; x < spriteSize * spriteSize * 4; x += 4) {
            alpha += sprite[x];
        }

        if (alpha == 0) {
            skippedSprites++;
        } else {
            // unsigned char* sprite is now holding a spriteSize * spriteSize * 4 byte sprite. Finally!
            error += saveObjectSprite(sprite, i - skippedSprites, spriteSize, lodeState, folderName);
        }
    }
    delete[] sprite;

    return error;
}

/**
 * Encodes and saves the byte data of a single sprite to disk as png.
 *
 * @param sprite the byte data
 * @param index used for naming: index 0 would be called '0.png'.
 * @param spriteSize the size of the sprite
 * @param lodeState the LodePNG library encoder/decoder State.
 * @param folderName the name of the folder this should go into. An absolute path (using outFilePath_ and index) is generated.
 * @return whether or not saving the sprite was successful.
 */
bool SpriteSheetIO::saveObjectSprite(const unsigned char* sprite, int index, unsigned int spriteSize, lodepng::State& lodeState, const std::string& folderName) {
    bool error = false;

    if (! fs::exists(outFilePath_/folderName)) {
        error = ! fs::create_directory(outFilePath_/folderName); // returns true when success
    }

    if (!error) {
        std::string fileName = std::to_string(index) + ".png";
        std::vector<unsigned char> encodedPixels;
        error = lodepng::encode(encodedPixels, sprite, spriteSize, spriteSize, lodeState);
        if (!error) {
            error = lodepng::save_file(encodedPixels, (outFilePath_/folderName/fileName).string());
        }
    }

    return error;
}

/**
 * Encodes and saves the byte data of multiple sprites belonging to a single char to disk as png.
 *
 * Uses 'Character sheet' sprites, i.e. 5 sprites (idle walk walk attack attack). the second attack frame is twice as wide.
 *
 * @param data collection of byte pointers to sprites. Every (spriteSize) pointers constitutes one sprite.
 * @param spriteSize The (normal) size of the sprite.
 * @param spriteCount The amount of sprites that are in the collection.
 * @param lodeState the LodePNG library encoder/decoder state.
 * @param originalFileName the original name of the SpriteSheet these sprites come from.
 * @return the amount of sprites that failed to save.
 */
unsigned int SpriteSheetIO::saveCharSplits(unsigned char** data, unsigned int spriteSize, unsigned int spriteCount, lodepng::State& lodeState, const std::string& originalFileName) {
    // for char sheets, one row = one character. If there exists invisible frames on that row (but not all are invisible),
    // then that is perfectly valid. For example, pet skins without attack frames.
    // I suspect Exalt still expects full alpha frames to slot into e.g. a pets attack frames.
    // Therefore, process one entire row of sprites, _then_ decide if it's an alpha (unlike saveObjectSplits, which is on a per-sprite basis)
    auto* sprite_0 = new unsigned char[spriteSize * spriteSize * 4]; // idle frame
    auto* sprite_1 = new unsigned char[spriteSize * spriteSize * 4]; // walk frame 1
    auto* sprite_2 = new unsigned char[spriteSize * spriteSize * 4]; // walk frame 2
    auto* sprite_3 = new unsigned char[spriteSize * spriteSize * 4]; // attack frame 1
    auto* sprite_4 = new unsigned char[spriteSize * spriteSize * 4 * 2]; // attack frame 2, twice as wide
    unsigned char* charSprites[SPRITES_PER_CHAR] = {sprite_0, sprite_1, sprite_2, sprite_3, sprite_4};

    int skippedSprites = 0;
    unsigned int error = 0;

    std::string folderName = folderNameFromSheetName(originalFileName, SpriteSheetType::CHARACTER);
    // for each sprite
    for (int i = 0; i < spriteCount; ++i) {
        // fill sprite_0 through sprite_4 with a character
        unsigned char* sprite = charSprites[i % SPRITES_PER_CHAR];
        for (int j = 0; j < spriteSize; ++j) {
            unsigned int spriteWidth = spriteSize * 4 * (sprite == sprite_4 ? 2 : 1);
            unsigned char* spriteRow = data[i * spriteSize + j]; //                    twice as much when wide sprite.
            memcpy(sprite + j * spriteWidth, spriteRow, spriteWidth);
        }

        // sprite_0 through sprite_4 contains a new character. Check if it's all alpha, then save if not.
        if (i % SPRITES_PER_CHAR == SPRITES_PER_CHAR - 1) {
            if (charSpritesAreAlpha(charSprites, spriteSize, sprite_4)) {
                skippedSprites++;
            } else {
                // unsigned char** charSprites is now holding a chars' sprites. Finally!
                error += saveCharSprites(charSprites, (i / SPRITES_PER_CHAR) - skippedSprites, spriteSize, lodeState, folderName);
            }
        }
    }

    delete[] sprite_0;
    delete[] sprite_1;
    delete[] sprite_2;
    delete[] sprite_3;
    delete[] sprite_4;

    return error;
}

/**
 * Encodes and saves a single characters sprites to disk
 * @param sprites the sprites belonging to this character
 * @param index Used for naming. e.g. index 3 is called 3_[character_frame_name].png
 * @param spriteSize size of the (base) sprite
 * @param lodeState LodePNG Library encoder/decoder state
 * @param folderName the name of the folder this should go into. An absolute path (using outFilePath_ and index) is generated.
 * @return the amount of sprites that failed to save.
 */
unsigned int SpriteSheetIO::saveCharSprites(unsigned char *sprites [SPRITES_PER_CHAR], int index, unsigned int spriteSize, lodepng::State &lodeState, const std::string &folderName) {
    unsigned int error = 0;
    bool singleError;

    if (! fs::exists(outFilePath_/folderName)) {
        error = ! fs::create_directory(outFilePath_/folderName); // returns true when success
    }

    if (!error) {
        std::string baseFileName = std::to_string(index) + '_';

        for (const auto& kvp : CHAR_SHEET_TYPE_TO_NAME) {
            int spriteIndex = to_integral(kvp.first);
            std::string fileName = baseFileName + kvp.second + ".png"; // index_descriptor.png format needed
            std::vector<unsigned char> encodedPixels;
            unsigned int width = spriteIndex == CharSheetNames::ATTACK_2 ? (2 * spriteSize) : spriteSize; // attack2 is twice as wide!
            singleError = lodepng::encode(encodedPixels, sprites[spriteIndex], width, spriteSize, lodeState);
            if (!singleError) {
                singleError = lodepng::save_file(encodedPixels, (outFilePath_/folderName/fileName).string());
            }
            error += singleError; // discards error codes, just counts failures.
        }
    }

    return error;
}

/**
 * Checks if a set of sprites belonging to a character is fully alpha.
 * @param sprites the collection of sprites.
 * @param spriteSize the (base) size of the sprites
 * @param elongatedSprite pointer to the sprite of the collection that is twice as wide.
 * @return whether or not the character is fully alpha.
 */
bool SpriteSheetIO::charSpritesAreAlpha(unsigned char* sprites [SPRITES_PER_CHAR], const unsigned int spriteSize, const unsigned char* elongatedSprite) {
    int alpha = 0;
    for (int i = 0; i < SPRITES_PER_CHAR; ++i) {
        const unsigned char* inspectedSprite = sprites[i];
#pragma omp simd reduction(+:alpha)
        for (int j = 3; j < spriteSize * spriteSize * 4 * (inspectedSprite == elongatedSprite ? 2 : 1); j += 4) {
            alpha += inspectedSprite[j];
        }
    }

    return alpha == 0;
}

/**
 * Very little is known on the expected folder name of the sprite splitting.
 * Therefore an assumption is made it is the sheet name minus dimension, specified at the end.
 *
 * A realistic guess is that it's the same name as used in xml. Which is _usually_ this but not for some older files.
 * @param sheet filename or path to the original SpriteSheet file
 * @return suggested folder name by the above description.
 */
std::string SpriteSheetIO::folderNameFromSheetName(const std::string &sheet, const SpriteSheetType& type) {
    auto name = fs::path(sheet).filename();
    auto toLower = [](std::string&& s)->std::string { std::string o; for (const auto& c : s) o.push_back(static_cast<char>(std::tolower(c))); return o; };
    size_t index;
    std::string specifier;
    switch(type) {
        case SpriteSheetType::OBJECT: {
            specifier = "objects";
            std::string lowerName = {(toLower(std::move(name.string())))};
            index = lowerName.find(specifier);
            break;
        }
        case SpriteSheetType::CHARACTER: {
            specifier = "chars";
            std::string lowerName{toLower(std::move(name.string()))};
            index = lowerName.find(specifier);
            break;
        }
        default: return {"error_unknown_sheet_type"};
    }

    if (index == (size_t) -1) { // no type specifier? at least split off the .png and suggest that as folder name.
        std::string returns = name.string();
        return returns.substr(0, returns.size() - 4);
    } else {
        return name.string().substr(0, index + specifier.size());
    }
}

SpriteSheetIO::~SpriteSheetIO() {
    delete directoryIterator_;
}
