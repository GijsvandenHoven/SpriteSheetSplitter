//
// Created by 20173607 on 21/09/2023.
//

#ifndef SPRITESHEETSPLITTER_REGEXWRAPPER_HPP
#define SPRITESHEETSPLITTER_REGEXWRAPPER_HPP

#include <string>
#include <regex>
#include <iostream>
#include <exception>
#include "../logging/LoggerTags.hpp"

class RegexWrapper {
public:
    RegexWrapper() = delete;
    explicit RegexWrapper(const std::string& s) {
        // JS literal rules
        if (!s.starts_with('/')) {
            throw std::logic_error("'" + s + "' is not an ECMAScript Regex literal.");
        }
        // now that we know starts_with '/', std::string::npos is out.
        // Hence, we just want to make sure there is another '/' not at the start.
        const size_t flag_pos = s.find_last_of('/');
        if (flag_pos == 0) {
            throw std::logic_error("'" + s + "' is not an ECMAScript Regex literal.");
        }

        const std::string regex_string = s.substr(1, flag_pos - 1);

        // extract flags past the final '/' character.
        auto flags = std::regex_constants::ECMAScript;
        for (size_t flag_char_pos = flag_pos + 1; flag_char_pos < s.size(); ++flag_char_pos) {
            const char flag = s[flag_char_pos];
            switch (flag) { // NOLINT(hicpp-multiway-paths-covered)
                case 'i':
                    flags |= std::regex_constants::icase;
                    break;
                default:
                    std::cout << LoggerTags::warn << "char '" << flag << "' is not a supported regex literal flag.\n";
                    break;
            }
        }

        expression = std::regex(regex_string, flags);
        source_literal = s;
    }

    friend std::ostream& operator<<(std::ostream&, const RegexWrapper&);

    inline const std::regex& get() { return expression; }

private:
    std::regex expression;
    std::string source_literal;
};

inline std::ostream& operator<<(std::ostream& os, const RegexWrapper& rw) {
    os << "RegexWrapper( " << rw.source_literal << " )";

    return os;
}

#endif //SPRITESHEETSPLITTER_REGEXWRAPPER_HPP
