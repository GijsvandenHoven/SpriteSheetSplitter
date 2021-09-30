#ifndef SPRITESHEETSPLITTER_SPRITESHEETIO_H
#define SPRITESHEETSPLITTER_SPRITESHEETIO_H

#include <filesystem>
#include <queue>
#include "lodepng.h"
#include "../SpriteSheetData.h"

namespace fs = std::filesystem;

class ignorant_directory_iterator;

class SpriteSheetIO {
public:
    SpriteSheetIO() = default;
    ~SpriteSheetIO();
    bool setInPath(std::string& pathName, bool shouldBePNG, bool recursive);
    bool setOutPath(std::string& pathName);
    std::queue<std::string>& getPNGQueue(std::queue<std::string>& q);
    static unsigned int loadPNG(const std::string& fileName, std::vector<unsigned char> &buffer, SpriteSheetData& data);

private:
    fs::path inFilePath_;
    fs::path outFilePath_;
    ignorant_directory_iterator* directoryIterator_ = nullptr;
};

/**
 * An interface that obscures the type of iterator (fs::directory_iterator or fs::recursive_directory_iterator).
 * The IO class does not care, it just wants the next path to work on.
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
