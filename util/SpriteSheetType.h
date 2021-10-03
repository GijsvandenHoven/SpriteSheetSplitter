#ifndef SPRITESHEETSPLITTER_SPRITESHEETTYPE_H
#define SPRITESHEETSPLITTER_SPRITESHEETTYPE_H
// no longer a member of Splitter: also used in SpriteSheetIO
enum class SpriteSheetType {
    OBJECT = 0,
    CHARACTER = 1,
};

inline std::ostream& operator<<(std::ostream& os, const SpriteSheetType& sst) {
    switch (sst) {
        case SpriteSheetType::OBJECT:
            os << "OBJECT";
            break;
        case SpriteSheetType::CHARACTER:
            os << "CHARACTER";
            break;
    }
    return os;
}

#endif //SPRITESHEETSPLITTER_SPRITESHEETTYPE_H
