#ifndef SPRITESHEETSPLITTER_SPLITTER_H
#define SPRITESHEETSPLITTER_SPLITTER_H

#include "util/SplitterOptions.h"
#include "IO/SpriteSheetIO.h"
#include "util/SpriteSplittingStatus.h"
#include "util/SpriteSplittingData.h"

class Splitter {
public:
    Splitter() = default;
    void work(std::vector<SplitterOpts>& jobs);

private:
    SpriteSheetIO ssio;

    void workFolder(int workCap, std::queue<std::string> &pngs, SpriteSplittingStatus &jobStats) const;
    void split(const std::string &fileName, SpriteSplittingStatus &jobStats, std::basic_ostream<char> &outStream) const;
    static bool validSpriteSheet(unsigned int width, unsigned int height, unsigned int columnCount);
    static void splitObjectSheet(SpriteSplittingData& ssd);
    static void splitCharSheet(SpriteSplittingData& ssd);

    // sprite count per row of type
    static const int OBJ_SHEET_ROW = 16;
    static const int CHAR_SHEET_ROW = 7;
};

#endif //SPRITESHEETSPLITTER_SPLITTER_H
