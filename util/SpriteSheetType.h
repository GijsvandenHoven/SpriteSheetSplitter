#ifndef SPRITESHEETSPLITTER_SPRITESHEETTYPE_H
#define SPRITESHEETSPLITTER_SPRITESHEETTYPE_H

enum class SpriteSheetType {
    OBJECT = 0,
    CHARACTER = 1,
    GROUND = 2,
};

inline std::ostream& operator<<(std::ostream& os, const SpriteSheetType& sst) {
    switch (sst) {
        case SpriteSheetType::OBJECT:
            os << "OBJECT";
            break;
        case SpriteSheetType::CHARACTER:
            os << "CHARACTER";
            break;
        case SpriteSheetType::GROUND:
            os << "GROUND";
            break;
    }
    return os;
}

#endif //SPRITESHEETSPLITTER_SPRITESHEETTYPE_H
