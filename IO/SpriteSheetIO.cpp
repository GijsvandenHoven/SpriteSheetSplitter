#include <iostream>
#include "SpriteSheetIO.h"

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
void SpriteSheetIO::getPNGQueue(std::queue<std::string> &q) {
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
 * @param data struct containing metadata from the spritesheet, like dimensions and lodepng decode state.
 * @return error code from lodePNG (0 = OK)
 */
unsigned int SpriteSheetIO::loadPNG(const std::string& fileName, std::vector<unsigned char> &buffer, SpriteSheetData& data) {
    std::cout << "[INFO] Loading " << fileName << "\n";
    unsigned int& error = data.error;
    std::vector<unsigned char> encodedPixelBuffer;

    error = lodepng::load_file(encodedPixelBuffer, fileName);
    if (!error) error = lodepng::decode(buffer, data.width, data.height, data.lodeState, encodedPixelBuffer);

    return error;
}

bool SpriteSheetIO::saveObjectSplits(unsigned char** data, const unsigned int spriteSize, const unsigned int spriteCount, lodepng::State& lodeState, const std::string& originalFileName) {
    auto* sprite = new unsigned char[spriteSize * spriteSize * 4];
    bool error = false;
    std::string folderName = folderNameFromSheetName(originalFileName, SpriteSheetType::OBJECT);
    // for each sprite
    for (int i = 0; i < spriteCount; ++i) {
        // for each sprite row
        for (int j = 0; j < spriteSize; ++j) {
            // access the pointer to sprite rows in the data.
            unsigned char* spriteRow = data[i * spriteSize + j];
            memcpy(sprite + j * spriteSize * 4, spriteRow, spriteSize * 4);
        }
        // unsigned char* sprite is now holding a spriteSize * spriteSize * 4 byte sprite. Finally!
        error = saveSprite(sprite, i, spriteSize, lodeState, folderName);
    }
    delete[] sprite;

    return error;
}

bool SpriteSheetIO::saveSprite(unsigned char* sprite, int index, const unsigned int spriteSize, lodepng::State& lodeState, const std::string& folderName) {
    if (! fs::exists(outFilePath_/folderName)) {
        fs::create_directory(outFilePath_/folderName);
    }

    std::string fileName = std::to_string(index) + ".png";
    std::vector<unsigned char> encodedPixels;
    bool error = lodepng::encode(encodedPixels, sprite, spriteSize, spriteSize, lodeState);
    if (!error) error = lodepng::save_file(encodedPixels, (outFilePath_/folderName/fileName).string());

    return error;
}

/**
 * Very little is known on the expected folder name of the sprite splitting.
 * Therefore an assumption is made it is the sheet name minus dimension, specified at the end.
 *
 * A realistic guess is that it's the same name as used in xml. Which is _usually_ this but not for some older files.
 * @param sheet filename or path to the original spritesheet file
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
