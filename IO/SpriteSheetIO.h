#ifndef SPRITESHEETSPLITTER_SPRITESHEETIO_H
#define SPRITESHEETSPLITTER_SPRITESHEETIO_H

#include <filesystem>

namespace fs = std::filesystem;

class ignorant_directory_iterator;

class SpriteSheetIO {
public:
    SpriteSheetIO() = default;
    bool setInPath(std::string& pathName, bool shouldBePNG, bool recursive);
    bool setOutPath(std::string& pathName);
    unsigned int load(std::vector<unsigned char> &buffer) const;

private:
    fs::path inFilePath_;
    fs::path outFilePath_;

    ignorant_directory_iterator* directoryIterator = nullptr;
};

/**
 * An interface that obscures the type of iterator (fs::directory_iterator or fs::recursive_directory_iterator).
 * The IO class does not care, it just wants the next path to work on.
 */
class ignorant_directory_iterator {
public:
    virtual const fs::directory_entry& get() = 0;
    virtual bool end() = 0;
    virtual void next() = 0;
    virtual ~ignorant_directory_iterator() = default;
};

class directory_iterator : public ignorant_directory_iterator {
    fs::directory_iterator* iterator;
public:
    directory_iterator() = delete;
    explicit directory_iterator(fs::directory_iterator* di) : iterator(di) {};
    const fs::directory_entry& get() final { return **iterator; }
    bool end() final { return *iterator == fs::end(*iterator); }
    void next() final { (*iterator)++; }
    ~directory_iterator() override { delete iterator; }
};

class recursive_directory_iterator : public ignorant_directory_iterator {
    fs::recursive_directory_iterator* iterator;
public:
    recursive_directory_iterator() = delete;
    explicit recursive_directory_iterator(fs::recursive_directory_iterator* rdi) : iterator(rdi) {};
    const fs::directory_entry& get() final { return **iterator; }
    bool end() final { return *iterator == fs::end(*iterator); }
    void next() final { (*iterator)++; }
    ~recursive_directory_iterator() override { delete iterator; }
};


#endif //SPRITESHEETSPLITTER_SPRITESHEETIO_H
