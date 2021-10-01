#ifndef SPRITESHEETSPLITTER_SPRITESHEETIO_H
#define SPRITESHEETSPLITTER_SPRITESHEETIO_H

#include <filesystem>
#include <queue>
#include <map>
#include "lodepng.h"
#include "../util/SpriteSheetData.h"
#include "../Util/SpriteSheetType.h"
#include "../util/CharSheetNames.h"

namespace fs = std::filesystem;

class ignorant_directory_iterator;

class SpriteSheetIO {
public:
    SpriteSheetIO() = default;
    ~SpriteSheetIO();
    bool setInPath(const std::string& pathName, bool shouldBePNG, bool recursive);
    bool setOutPath(const std::string& pathName);
    void getPNGQueue(std::queue<std::string>& q);
    static unsigned int loadPNG(const std::string& fileName, std::vector<unsigned char> &buffer, SpriteSheetData& data);
    bool saveObjectSplits(unsigned char** data, unsigned int spriteSize, unsigned int spriteCount, lodepng::State& lodeState, const std::string& originalFileName);
    bool saveCharSplits(unsigned char** data, unsigned int spriteSize, unsigned int spriteCount, lodepng::State& lodeState, const std::string& originalFileName);

private:
    constexpr static int SPRITES_PER_CHAR = 5; // amount of sprites in a row of a charSheet. (idle, walk1, walk2, attack1, attack2).

    fs::path inFilePath_;
    fs::path outFilePath_;
    ignorant_directory_iterator* directoryIterator_ = nullptr;
    static bool charSpritesAreAlpha(unsigned char* sprites [SPRITES_PER_CHAR], unsigned int spriteSize, const unsigned char* elongatedSprite);
    bool saveObjectSprite(const unsigned char* sprite, int index, unsigned int spriteSize, lodepng::State& lodeState, const std::string& folderName);
    bool saveCharSprites(unsigned char* sprites [SPRITES_PER_CHAR], int index, unsigned int spriteSize, lodepng::State& lodeState, const std::string& folderName);
    static std::string folderNameFromSheetName(const std::string& sheet, const SpriteSheetType& type);

    static const std::map<CharSheetNames, std::string> CHAR_SHEET_TYPE_TO_NAME; // todo: constexpr map using c++20? no support yet.
    // needs c++20 constexpr containers, no compiler support yet (assuming map gets constexpr at all, not entirely clear on that! They mention containers _such as_ vector and string, but cant find examples of anything other than these two...).
    //static_assert(SPRITES_PER_CHAR == CHAR_SHEET_TYPE_TO_NAME.size());
    static_assert(SPRITES_PER_CHAR == to_integral(CharSheetNames::ATTACK_2) + 1);
};

/**
 * An interface that obscures the type of iterator (fs::directory_iterator or fs::recursive_directory_iterator).
 * The IO class does not care, it just wants the next path to evaluate.
 */
class ignorant_directory_iterator {
public:
    virtual const fs::path& operator*() = 0;
    virtual const fs::path* operator->() = 0;
    virtual ignorant_directory_iterator& operator++() = 0;
    virtual bool end() = 0;
    // aliases for the operator overloads
    [[maybe_unused]] virtual const fs::path& get() = 0;
    [[maybe_unused]] virtual void next() = 0;
    virtual ~ignorant_directory_iterator() = default;
};

class directory_iterator : public ignorant_directory_iterator {
    fs::directory_iterator* iterator;
public:
    directory_iterator() = delete;
    explicit directory_iterator(fs::directory_iterator* di) : iterator(di) {};
    const fs::path& operator*() final { return (*iterator)->path(); }
    const fs::path* operator->() final { return &((*iterator)->path()); }
    directory_iterator& operator++() final { ++(*iterator); return *this; }
    bool end() final { return *iterator == fs::end(*iterator); }
    const fs::path& get() final { return (*iterator)->path(); }
    void next() final { ++(*iterator); }
    ~directory_iterator() override { delete iterator; }
};

class recursive_directory_iterator : public ignorant_directory_iterator {
    fs::recursive_directory_iterator* iterator;
public:
    recursive_directory_iterator() = delete;
    explicit recursive_directory_iterator(fs::recursive_directory_iterator* rdi) : iterator(rdi) {};
    const fs::path& operator*() final { return (*iterator)->path(); }
    const fs::path* operator->() final { return &((*iterator)->path()); }
    recursive_directory_iterator& operator++() final { ++(*iterator); return *this; }
    bool end() final { return *iterator == fs::end(*iterator); }
    const fs::path& get() final { return (*iterator)->path(); }
    void next() final { ++(*iterator); }
    ~recursive_directory_iterator() override { delete iterator; }
};

#endif //SPRITESHEETSPLITTER_SPRITESHEETIO_H
