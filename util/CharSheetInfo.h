#ifndef SPRITESHEETSPLITTER_CHARSHEETINFO_H
#define SPRITESHEETSPLITTER_CHARSHEETINFO_H

constexpr static int SPRITES_PER_CHAR = 5; // amount of sprites in a row of a charSheet. (idle, walk1, walk2, attack1, attack2).

// enum must (!) be ordered like they are in char sheets, as well as zero-indexed.
// built-in operator< is used for maintaining the ordering in a map, as well as indexing an array of sprites belonging to a Character.
enum class CharSheetInfo : int {
    IDLE = 0,
    WALK_1 = 1,
    WALK_2 = 2,
    ATTACK_1 = 3,
    ATTACK_2 = 4,
};

inline constexpr bool operator==(const CharSheetInfo& e, int x) { return static_cast<std::underlying_type<CharSheetInfo>::type>(e) == x; }
inline constexpr bool operator==(int x, const CharSheetInfo& e) { return e == x; }
inline constexpr int to_integral(const CharSheetInfo& e) { return static_cast<std::underlying_type<CharSheetInfo>::type>(e); };

// keep me updated
static_assert(CharSheetInfo::IDLE == 0);
static_assert(CharSheetInfo::WALK_1 == 1);
static_assert(CharSheetInfo::WALK_2 == 2);
static_assert(CharSheetInfo::ATTACK_1 == 3);
static_assert(CharSheetInfo::ATTACK_2 == 4);

// todo: constexpr map using c++20? no support yet.
static const std::map<CharSheetInfo, std::string> CHAR_SHEET_TYPE_TO_NAME { // NOLINT(cert-err58-cpp) (Warns that this can throw an exception before main is called. It's true, map constructor isn't noexcept, but...)
        {CharSheetInfo::IDLE, "Right_Walk_0"},
        {CharSheetInfo::WALK_1, "Right_Walk_1"},
        {CharSheetInfo::WALK_2, "Right_Walk_2"},
        {CharSheetInfo::ATTACK_1, "Right_Attack_0"},
        {CharSheetInfo::ATTACK_2, "Right_Attack_1"},
};

// needs c++20 constexpr containers, no compiler support yet (assuming map gets constexpr at all, not entirely clear on that! They mention containers _such as_ vector and string, but cant find examples of anything other than these two...).
//static_assert(SPRITES_PER_CHAR == CHAR_SHEET_TYPE_TO_NAME.size());
static_assert(SPRITES_PER_CHAR == to_integral(CharSheetInfo::ATTACK_2) + 1);

#endif //SPRITESHEETSPLITTER_CHARSHEETINFO_H
