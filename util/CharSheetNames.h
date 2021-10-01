#ifndef SPRITESHEETSPLITTER_CHARSHEETNAMES_H
#define SPRITESHEETSPLITTER_CHARSHEETNAMES_H

// enum must (!) be ordered like they are in char sheets, as well as zero-indexed.
// built-in operator< is used for maintaining the ordering in a map, as well as indexing an array of sprites belonging to a Character.
enum class CharSheetNames : int {
    IDLE = 0,
    WALK_1 = 1,
    WALK_2 = 2,
    ATTACK_1 = 3,
    ATTACK_2 = 4,
};

inline constexpr bool operator==(const CharSheetNames& e, int x) { return static_cast<std::underlying_type<CharSheetNames>::type>(e) == x; }
inline constexpr bool operator==(int x, const CharSheetNames& e) { return e == x; }
inline constexpr int to_integral(const CharSheetNames& e) { return static_cast<std::underlying_type<CharSheetNames>::type>(e); };

// keep me updated
static_assert(CharSheetNames::IDLE == 0);
static_assert(CharSheetNames::WALK_1 == 1);
static_assert(CharSheetNames::WALK_2 == 2);
static_assert(CharSheetNames::ATTACK_1 == 3);
static_assert(CharSheetNames::ATTACK_2 == 4);

#endif //SPRITESHEETSPLITTER_CHARSHEETNAMES_H
