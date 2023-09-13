//
// Created by 20173607 on 25/08/2023.
//

#ifndef SPRITESHEETSPLITTER_LOGGERTAGS_HPP
#define SPRITESHEETSPLITTER_LOGGERTAGS_HPP

#include <iostream>

namespace LoggerTags {

struct ErrorTag {};
struct WarningTag {};
struct InfoTag {};

struct ThreadedInfoTag {};
struct ThreadedWarningTag {};
struct ThreadedErrorTag {};

// global variables declared in the .cpp file.
extern ErrorTag error;
extern ThreadedErrorTag threaded_error;

extern WarningTag warn;
extern ThreadedWarningTag threaded_warn;

extern InfoTag info;
extern ThreadedInfoTag threaded_info;

}

std::ostream& operator<<(std::ostream& os, const LoggerTags::ErrorTag &);
std::ostream& operator<<(std::ostream& os, const LoggerTags::WarningTag &);
std::ostream& operator<<(std::ostream& os, const LoggerTags::InfoTag &);

std::ostream& operator<<(std::ostream& os, const LoggerTags::ThreadedErrorTag &);
std::ostream& operator<<(std::ostream& os, const LoggerTags::ThreadedWarningTag &);
std::ostream& operator<<(std::ostream& os, const LoggerTags::ThreadedInfoTag &);

#endif //SPRITESHEETSPLITTER_LOGGERTAGS_HPP
