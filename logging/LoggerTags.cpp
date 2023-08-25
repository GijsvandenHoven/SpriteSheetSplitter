//
// Created by 20173607 on 25/08/2023.
//

#include "LoggerTags.hpp"
#include <omp.h>

namespace LoggerTags {
    ErrorTag error;
    ThreadedErrorTag threaded_error;

    WarningTag warn;
    ThreadedWarningTag threaded_warn;

    InfoTag info;
    ThreadedInfoTag threaded_info;
}

std::ostream& operator<<(std::ostream& os, const LoggerTags::ErrorTag &) {
    return os << "[ERROR] ";
}

std::ostream& operator<<(std::ostream& os, const LoggerTags::WarningTag &) {
    return os << "[WARNING] ";
}

std::ostream& operator<<(std::ostream& os, const LoggerTags::InfoTag &) {
    return os << "[INFO] ";
}

std::ostream& operator<<(std::ostream& os, const LoggerTags::ThreadedErrorTag &) {
    return os << "[ERROR][T_" << omp_get_thread_num() << "] ";
}

std::ostream& operator<<(std::ostream& os, const LoggerTags::ThreadedWarningTag &) {
    return os << "[WARNING][T_" << omp_get_thread_num() << "] ";
}

std::ostream& operator<<(std::ostream& os, const LoggerTags::ThreadedInfoTag &) {
    return os << "[INFO][T_" << omp_get_thread_num() << "] ";
}