#include <iostream>
#include <getopt.h>
#include "splitterOptions.h"

// https://linux.die.net/man/3/getopt
extern char* optarg; // NOLINT(readability-redundant-declaration)
// extern int optind; // NOLINT(readability-redundant-declaration)

// used by getopt, getopt_long etc. each character is the short name of an option. Colon after means it has a parameter. Double colon means optional parameter.
std::string& getOPT_STR() {
    static std::string OPT_STR = "d:ri:o::k::c::";
    return OPT_STR;
}

int main(int argc, char* argv[]) {

    const char* OPT_STR = getOPT_STR().c_str();
    static struct option long_options[] = {
            {"directory",   required_argument,  nullptr, 'd'},
            {"in",          required_argument,  nullptr, 'i'}, // equivalent to -d
            {"recursive",   no_argument,        nullptr, 'r'},
            {"out",         optional_argument,  nullptr, 'o'},
            {"keepworking", optional_argument,  nullptr, 'k'},
            {"useconfig",   optional_argument,  nullptr, 'c'},
            {nullptr,       no_argument,        nullptr, 0} // Documentation says last element must be empty.
    };

    int c;
    SplitterOpts options;
    while (EOF != (c = getopt_long(argc, argv, OPT_STR, long_options, nullptr))) {
        switch (c) {
            case 'd':
            case 'i': {
                if (!options.inDirectory.empty()) {
                    std::cout << "Warning! both -d and -i appear to be supplied. " <<
                    "This is not possible, only the latter ("<< (char) c << ") Will be used.\n";
                }
                options.inDirectory = optarg;
                std::string &dir = options.inDirectory;
                options.isPNGInDirectory = (0 == dir.compare(dir.size() - 4, 4, "png"));
                break;
            }
            case 'o': {
                options.outDirectory = optarg;
                break;
            }
            case 'k': {
                if (optarg == nullptr) {
                    options.workAmount = std::numeric_limits<int>::max();
                } else {
                    int amount;
                    try {
                        amount = std::stoi(optarg);
                    } catch (int e) {
                        std::cout << "Warning! -k expects numerical arguments (" << optarg << " was supplied). Using default of 1.\n";
                        amount = 1;
                    }
                    options.workAmount = amount;
                }
                break;
            }
            case 'c': {
                std::cout << "-c is not supported yet!\n";
                break;
            }
            case 'r':
                options.recursive = true;
                break;
            case '?':
            case ':':
            default:
                std::cout << "usage: (todo)\n";
                break;
        }
    }

    std::cout << options;

    return 0;
}


// possible options:
// (no flag provided, just a text on argv[1]): check if `.png`. Then use as --directory or --in depending on result.
// --directory (-d) OR --in (i) Directory to a folder of spritesheet(s).
// --recursive (-r) in case of folder, also check folders within folders while working. PNGS in current folder have precedence over sub-folders, i.e. folders checked last.
// --out (-o): Output directory. Optional. When none specified, output in spritesheets folder.
// --keepworking (-k): Optional Number. Amount of pngs to process in this folder. Stop when at amount or when run out. Default int_max.
// --useconfig (-c): Bool. Use config file. has parameter 'dir': Directory to config. Otherwise, looks for 'config.txt (.json?)' in current folder.

// config file:
// can provide a set of directories and/or files to process.
// each directory has a -k (optional, default 1).
// each directory or file has an -o (required)
// each would create the options struct and call the routine.