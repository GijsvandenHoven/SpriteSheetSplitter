#ifndef SPRITESHEETSPLITTER_SPRITESHEETIO_H
#define SPRITESHEETSPLITTER_SPRITESHEETIO_H

#include <filesystem>
#include <queue>
#include <map>
#include "lodepng.h"
#include "../util/SpriteSheetPNGData.h"
#include "../util/SpriteSheetType.h"
#include "../util/CharSheetInfo.h"
#include "../util/SpriteSplittingStatus.h"
#include "../util/SpriteSplittingData.h"
#include "IOOptions.hpp"

namespace fs = std::filesystem;

class ignorant_directory_iterator;

class SpriteSheetIO {
public:
    SpriteSheetIO() = default;
    ~SpriteSheetIO();
    void setIOOptions(const SplitterOpts &opts);
    void fillPNGQueue(std::queue<std::string>& q);
    static unsigned int loadPNG(const std::string& fileName, std::vector<unsigned char> &buffer, SpriteSheetPNGData& data);
    void saveSplits(SpriteSplittingData& ssd, std::basic_ostream<char>& outStream) const;
    [[nodiscard]] inline bool validOptions() const { return optionsOK_; }

private:
    IOOptions IOOpts_;
    ignorant_directory_iterator* directoryIterator_ = nullptr;
    bool optionsOK_ = false; // is written to by setIOOptions.

    [[nodiscard]] bool initializeDirectoryIterator(bool shouldBePNG, bool recursive);
    [[nodiscard]] bool initializeOutPath();
    [[nodiscard]] bool createCleanDirectory(const std::string& dir, std::error_code& ec) const noexcept;
    void saveObjectSplits(SpriteSplittingData &ssd, const std::string &folderName, std::basic_ostream<char>& outStream) const;
    void saveCharSplits(SpriteSplittingData& ssd, const std::string& folderName, std::basic_ostream<char>& outStream) const;
    void saveGroundSplits(SpriteSplittingData& ssd, const std::string& folderName, std::basic_ostream<char>& outStream) const;
    bool saveObjectSprite(const unsigned char* sprite, int index, unsigned int spriteSize, lodepng::State& lodeState, const std::string& folderName, std::basic_ostream<char>& outStream) const;
    unsigned int saveCharSprites(unsigned char* sprites [SPRITES_PER_CHAR], int index, unsigned int spriteSize, lodepng::State& lodeState, const std::string& folderName, std::basic_ostream<char>& outStream) const;
    static void checkLodePNGErrorCode(unsigned int code, std::basic_ostream<char>& outStream);
    static bool charSpritesAreAlpha(unsigned char* sprites [SPRITES_PER_CHAR], unsigned int spriteSize, const unsigned char* elongatedSprite);
    static std::string folderNameFromSheetName(const std::string& sheetPath, const SpriteSheetType& type);
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
