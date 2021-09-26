#ifndef SPRITESHEETSPLITTER_SPRITESHEETIO_H
#define SPRITESHEETSPLITTER_SPRITESHEETIO_H

#include <filesystem>
#include "lodepng.h"

namespace fs = std::filesystem;

class ignorant_directory_iterator;

class SpriteSheetIO {
public:
    SpriteSheetIO() = default;
    bool setInPath(std::string& pathName, bool shouldBePNG, bool recursive);
    bool setOutPath(std::string& pathName);
    bool findNextPNG();
    bool hasNextPNG();
    unsigned int load(std::vector<unsigned char> &buffer, unsigned int& width, unsigned int& height);

private:
    fs::path inFilePath_;
    fs::path outFilePath_;

    ignorant_directory_iterator* directoryIterator = nullptr;

    struct pngState {
        // todo: must it remember width and height? saving doesnt use this, it uses sprite size.
        unsigned int width;
        unsigned int height;
        unsigned int error;
        lodepng::State lodeState;
        std::vector<unsigned char> encodedPixelBuffer;

        pngState(){
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
    } pngState;
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
