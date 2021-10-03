#include <iostream>
#include "SpriteSheetIO.h"

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
unsigned int SpriteSheetIO::loadPNG(const std::string& fileName, std::vector<unsigned char> &buffer, SpriteSheetPNGData& data) {
    unsigned int& error = data.error;
    std::vector<unsigned char> encodedPixelBuffer;

    error = lodepng::load_file(encodedPixelBuffer, fileName);
    if (!error) error = lodepng::decode(buffer, data.width, data.height, data.lodeState, encodedPixelBuffer);

    return error;
}

/**
 * Generic entry point for saving a type of splits. Calls the correct saving method depending on the given SpriteSheetType.
 * @param data the split sprite data
 * @param spriteSize the size of the sprite (height in type of charsprites due to 2nd attack frame)
 * @param spriteCount amount of sprites
 * @param type type of SpriteSheet (e.g. object, char)
 * @param lodeState LodePNG Library state for encoding/decoding.
 * @param originalFileName FileName of the spriteSheet these splits came from
 * @param jobStats tracking object for splitting.
 */
void SpriteSheetIO::saveSplits(SpriteSplittingData& ssd) const {
    switch (ssd.sheetType) {
        case SpriteSheetType::OBJECT:
            saveObjectSplits(ssd);
            break;
        case SpriteSheetType::CHARACTER:
            saveCharSplits(ssd);
            break;
        default: // did you add a new SpriteSheetType?
            std::cout << "[ERROR]: Unknown SpriteSheetType " << ssd.sheetType << "\n"; // is at risk of being mangled in printing due to multithreading, but given this is a 'developer only' error, I think that is OK. No need to supply the synced outstream or create one.
            exit(-1);
    }
}

/**
 * Given a sprite amount and size,
 * saves a given collection of byte pointers as single sprite files on disk,
 * based on the LodePNGState and name of the original SpriteSheet.
 *
 * Every byte pointer in the collection points to a (spriteSize) sized row of RGBA bytes.
 * Assumes every sprite is a square shape.
 *
 * @param ssd Struct containing all needed information, see SpriteSplittingData.h
 */
void SpriteSheetIO::saveObjectSplits(SpriteSplittingData& ssd) const {

    std::string folderName = folderNameFromSheetName(ssd.originalFileName, SpriteSheetType::OBJECT);
    bool cleanedFolder = createCleanDirectory(folderName);
    if (! cleanedFolder) {
        ssd.stats.n_save_error += ssd.spriteCount; // mark every sprite as failed.
        return;
    }

    auto* sprite = new unsigned char[ssd.spriteSize * ssd.spriteSize * 4];
    int skippedSprites = 0;
    // for each sprite
    for (int i = 0; i < ssd.spriteCount; ++i) {
        // for each sprite row
        for (int j = 0; j < ssd.spriteSize; ++j) {
            // access the pointer to sprite rows in the data.
            unsigned char* spriteRow = ssd.splitSprites[i * ssd.spriteSize + j];
            memcpy(sprite + j * ssd.spriteSize * 4, spriteRow, ssd.spriteSize * 4);
        }
        // need to check if a sprite is pure alpha (then don't save it)
        bool transparent = true;
        for (int x = 3; transparent && x < ssd.spriteSize * ssd.spriteSize * 4; x += 4) {
            transparent = (0 == sprite[x]);
        }

        if (transparent) {
            skippedSprites++;
        } else {
            // unsigned char* sprite is now holding a spriteSize * spriteSize * 4 byte sprite. Finally!
            saveObjectSprite(sprite, i - skippedSprites, ssd.spriteSize, ssd.lodeState, folderName, ssd.stats);
        }
    }

    ssd.stats.n_skipped += skippedSprites;

    delete[] sprite;
}

/**
 * Encodes and saves the byte data of a single sprite to disk as png.
 *
 * @param sprite the byte data
 * @param index used for naming: index 0 would be called '0.png'.
 * @param spriteSize the size of the sprite
 * @param lodeState the LodePNG library encoder/decoder State.
 * @param folderName the name of the folder this should go into. An absolute path (using outFilePath_ and index) is generated.
 * @param jobStats tracking object for sprite splitting stats
 */
void SpriteSheetIO::saveObjectSprite(const unsigned char* sprite, int index, unsigned int spriteSize, lodepng::State& lodeState, const std::string& folderName, SpriteSplittingStatus& jobStats) const {
    bool error;
    std::string fileName = std::to_string(index) + ".png";
    std::vector<unsigned char> encodedPixels;

    error = lodepng::encode(encodedPixels, sprite, spriteSize, spriteSize, lodeState);
    if (!error) {
        error = lodepng::save_file(encodedPixels, (outFilePath_/folderName/fileName).string());
        jobStats.n_save_error +=    error;
        jobStats.n_success +=       ! error;
    }
}

/**
 * Encodes and saves the byte data of multiple sprites belonging to a single char to disk as png.
 *
 * Uses 'Character sheet' sprites, i.e. 5 sprites (idle walk walk attack attack). the second attack frame is twice as wide.
 *
 * @param ssd Struct containing all needed information, see SpriteSplittingData.h
 */
void SpriteSheetIO::saveCharSplits(SpriteSplittingData& ssd) const {

    std::string folderName = folderNameFromSheetName(ssd.originalFileName, SpriteSheetType::CHARACTER);
    bool cleanedFolder = createCleanDirectory(folderName);
    if (! cleanedFolder) {
        ssd.stats.n_save_error += ssd.spriteCount; // mark every sprite as failed
        return;
    }

    // for char sheets, one row = one character. If there exists invisible frames on that row (but not all are invisible),
    // then that is perfectly valid. For example, pet skins without attack frames.
    // I suspect Exalt still expects full alpha frames to slot into e.g. a pets attack frames.
    // Therefore, process one entire row of sprites, _then_ decide if it's an alpha (unlike saveObjectSplits, which is on a per-sprite basis)
    const unsigned int spriteBytes = ssd.spriteSize * ssd.spriteSize * 4;
    auto* sprite_0 = new unsigned char[spriteBytes]; // idle frame
    auto* sprite_1 = new unsigned char[spriteBytes]; // walk frame 1
    auto* sprite_2 = new unsigned char[spriteBytes]; // walk frame 2
    auto* sprite_3 = new unsigned char[spriteBytes]; // attack frame 1
    auto* sprite_4 = new unsigned char[spriteBytes * 2]; // attack frame 2, twice as wide
    unsigned char* charSprites[SPRITES_PER_CHAR] = {sprite_0, sprite_1, sprite_2, sprite_3, sprite_4};
    int skippedSprites = 0;

    // for each sprite
    for (int i = 0; i < ssd.spriteCount; ++i) {
        // fill sprite_0 through sprite_4 with a character
        unsigned char* sprite = charSprites[i % SPRITES_PER_CHAR];
        for (int j = 0; j < ssd.spriteSize; ++j) { //       twice as much when wide sprite!
            unsigned int spriteWidth = ssd.spriteSize * 4 * (sprite == sprite_4 ? 2 : 1);
            unsigned char* spriteRow = ssd.splitSprites[i * ssd.spriteSize + j];
            memcpy(sprite + j * spriteWidth, spriteRow, spriteWidth);
        }

        // sprite_0 through sprite_4 contains a new character. Check if it's all alpha, then save if not.
        if (i % SPRITES_PER_CHAR == SPRITES_PER_CHAR - 1) {
            if (charSpritesAreAlpha(charSprites, ssd.spriteSize, sprite_4)) {
                skippedSprites++;
            } else {
                // unsigned char** charSprites is now holding a chars' sprites. Finally!
                saveCharSprites(charSprites, (i / SPRITES_PER_CHAR) - skippedSprites, ssd.spriteSize, ssd.lodeState, folderName, ssd.stats);
            }
        }
    }

    ssd.stats.n_skipped += skippedSprites;

    delete[] sprite_0;
    delete[] sprite_1;
    delete[] sprite_2;
    delete[] sprite_3;
    delete[] sprite_4;
}

/**
 * Encodes and saves a single characters sprites to disk
 * @param sprites the sprites belonging to this character
 * @param index Used for naming. e.g. index 3 is called 3_[character_frame_name].png
 * @param spriteSize size of the (base) sprite
 * @param lodeState LodePNG Library encoder/decoder state
 * @param folderName the name of the folder this should go into. The folder is assumed to exist.
 * @param jobStats tracking object for sprite splitting stats
 */
void SpriteSheetIO::saveCharSprites(unsigned char *sprites [SPRITES_PER_CHAR], int index, unsigned int spriteSize, lodepng::State &lodeState, const std::string &folderName, SpriteSplittingStatus& jobStats) const {
    bool error;
    std::string baseFileName = std::to_string(index) + '_';

    for (const auto& kvp : CHAR_SHEET_TYPE_TO_NAME) {

        int spriteIndex = to_integral(kvp.first);
        unsigned int width = spriteIndex == CharSheetInfo::ATTACK_2 ? (2 * spriteSize) : spriteSize; // attack2 is twice as wide!

        std::string fileName = baseFileName + kvp.second + ".png"; // index_descriptor.png format needed

        std::vector<unsigned char> encodedPixels;
        error = lodepng::encode(encodedPixels, sprites[spriteIndex], width, spriteSize, lodeState);

        if (error) { // log the failure
            jobStats.n_save_error++;
        } else {
            error = lodepng::save_file(encodedPixels, (outFilePath_/folderName/fileName).string());
            jobStats.n_save_error +=    error;
            jobStats.n_success +=       ! error;
        }
    }

}

/**
 * Checks if a set of sprites belonging to a character is fully alpha.
 * @param sprites the collection of sprites.
 * @param spriteSize the (base) size of the sprites
 * @param elongatedSprite pointer to the sprite of the collection that is twice as wide.
 * @return whether or not the character is fully alpha.
 */
bool SpriteSheetIO::charSpritesAreAlpha(unsigned char* sprites [SPRITES_PER_CHAR], const unsigned int spriteSize, const unsigned char* elongatedSprite) {
    bool transparent = true;
    for (int i = 0; i < SPRITES_PER_CHAR; ++i) {
        const unsigned char* inspectedSprite = sprites[i];
        for (int j = 3; transparent && j < spriteSize * spriteSize * 4 * (inspectedSprite == elongatedSprite ? 2 : 1); j += 4) {
            transparent = (0 == inspectedSprite[j]);
        }
    }

    return transparent;
}

/**
 * Very little is known on the expected folder name of the sprite splitting.
 * Therefore an assumption is made it is the sheet name and dimension, specified at the end.
 *
 * Hence, for example "AbyssOfDemonsChars16x16.png" becomes "AbyssOfDemonsChars16".
 * Specifying the dimension is important, otherwise sprites from e.g. "AbyssOfDemonsChars8x8" could end up in the same folder.
 *
 * A realistic guess is that it's the same name as used in xml. Which is _usually_ this but not for some older files.
 * @param sheet filename or path to the original SpriteSheet file
 * @return suggested folder name by the above description.
 */
std::string SpriteSheetIO::folderNameFromSheetName(const std::string& sheet, const SpriteSheetType& type) {
    auto toLower = [](const std::string& s)->std::string { std::string o; for (const auto& c : s) o.push_back(static_cast<char>(std::tolower(c))); return o; };
    size_t index;
    std::string specifier;
    switch(type) {
        case SpriteSheetType::OBJECT: {
            specifier = "objects";
            break;
        }
        case SpriteSheetType::CHARACTER: {
            specifier = "chars";
            break;
        }
        default: return {"error_unknown_sheet_type"};
    }

    std::string lowerName {std::move(toLower(sheet))};
    index = lowerName.find(specifier);
    // now have index to start of specifier, but want to include dimension number as well.
    if (index != (size_t) -1) {
        index = index + specifier.size(); // (only do this now in case of not found) index points to end of specifier
        index = lowerName.find('x', index+1); // +1: includes self, what if a future specifier ends in 'x'? Dimensions are at least one char so this is OK.
    }

    if (index == (size_t) -1) { // no type specifier? at least split off the .png and suggest that as folder name.
        return sheet.substr(0, sheet.size() - 4);
    } else {
        return sheet.substr(0, index);
    }
}

/**
 * Cleans out or creates the directory given by appending the given dir to the outFilePath_
 * @param dir the folder(s) to append to outFilePath
 * @return whether the operation was successful
 */
bool SpriteSheetIO::createCleanDirectory(const std::string& dir) const {
    if (fs::exists(outFilePath_/dir)) {
        fs::remove_all(outFilePath_/dir); // don't need the return code, any fail here means create_directory will fail.
    }

    return fs::create_directory(outFilePath_/dir);
}

SpriteSheetIO::~SpriteSheetIO() {
    delete directoryIterator_;
}
