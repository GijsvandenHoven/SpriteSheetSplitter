//
// Created by 20173607 on 20/09/2023.
//

#ifndef SPRITESHEETSPLITTER_JSONCONFIGPARSER_HPP
#define SPRITESHEETSPLITTER_JSONCONFIGPARSER_HPP

#include <vector>
#include "../util/SplitterOptions.h"

class JSONConfigParser {
public:
    static void parseConfig(const std::string &pathToFile, std::vector<SplitterOpts> &work);
};


#endif //SPRITESHEETSPLITTER_JSONCONFIGPARSER_HPP
