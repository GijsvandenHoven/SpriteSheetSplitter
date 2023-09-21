#include <iostream>
#include <getopt.h>
#include <vector>
#include "util/SplitterOptions.h"
#include "util/RegexWrapper.hpp"
#include "Splitter.h"
#include "logging/LoggerTags.hpp"
#include "IO/JSONConfigParser.hpp"

namespace logger = LoggerTags;

// https://linux.die.net/man/3/getopt
extern char* optarg; // NOLINT(readability-redundant-declaration)
[[maybe_unused]] // it is modified in getopt.h. Silly compiler.
extern int optind; // NOLINT(readability-redundant-declaration)

// printed when the user gives illegal input
std::string& getHELP_STRING() {
    static std::string HELP_STRING = "Use --help for more information.\n";
    return HELP_STRING;
}

// used by getopt, getopt_long etc. each character is the short name of an option. Colon after means it has a parameter. Double colon means optional parameter.
std::string& getOPT_STR() {
    static std::string OPT_STR = "hrsda:i:o::g::k::c::";
    return OPT_STR;
}

// SpriteSheetIO.h is agnostic to workload; expecting only a sequence of jobs,
// hence even the single instance of SplitterOpts from parseCommandLine should be a vector.
bool readConfig(int argc, char* argv[], option* long_options, [[maybe_unused]] std::vector<SplitterOpts>& work);
void parseCommandLine(int argc, char* argv[], option* long_options, std::vector<SplitterOpts>& work);

bool validateOptions(SplitterOpts& options);
void parseSingleParameter(int c, SplitterOpts& options);

int main(int argc, char* argv[]) {
    std::string& HELP_STRING = getHELP_STRING();

    static struct option long_options[] = {
            {"directory",   required_argument,  nullptr, 'd'},
            {"in",          required_argument,  nullptr, 'i'}, // equivalent to -d
            {"recursive",   no_argument,        nullptr, 'r'},
            {"out",         optional_argument,  nullptr, 'o'},
            {"groundPattern", optional_argument, nullptr, 'g'},
            {"keepworking", optional_argument,  nullptr, 'k'},
            {"useconfig",   optional_argument,  nullptr, 'c'},
            {"singleFolderOutput", no_argument, nullptr, 's'},
            {"subtractAlphaFromIndex", no_argument, nullptr, 'a'},
            {"help",        no_argument,        nullptr, 'h'},
            {nullptr,       no_argument,        nullptr, 0} // Documentation says last element must be empty.
    };

    if (argc <= 1) {
        std::cout << logger::error << "No input given.\n";
        std::cout << HELP_STRING;
        exit(-1);
    }

    std::vector<SplitterOpts> jobs;
    bool hasConfig = readConfig(argc, argv, long_options, jobs);
    if (!hasConfig) { // we're only doing command line, if there is no config to work with.
        parseCommandLine(argc, argv, long_options, jobs);
    }

    if (!jobs.empty()) {
        Splitter worker;
        worker.work(jobs);
    } else {
        std::cout << logger::error << "There are no usable options, stopping.\n";
    }

    return 0;
}

/**
 * Read from '-c' (defaulting to config.json) a file path to an assumed JSON file, which is parsed into options.
 * @param argc
 * @param argv
 * @param long_options
 * @param work
 * @return if a config file was read.
 */
bool readConfig(int argc, char* argv[], option* long_options, std::vector<SplitterOpts> &work) {

    const char* OPT_STR = getOPT_STR().c_str();
    int c;
    // check for config file specifically before parsing command line parameters
    while (EOF != (c = getopt_long(argc, argv, OPT_STR, long_options, nullptr))) {
        if (c == 'c') {
            // try to find the config file
            std::string fileName;
            if (optarg == nullptr) {
                fileName = "config.json";
            } else {
                fileName = optarg;
            }

            std::vector<SplitterOpts> unvalidated_work;
            JSONConfigParser::parseConfig(fileName, unvalidated_work);

            int dropped = 0;
            for (auto& opt : unvalidated_work) {
                if (validateOptions(opt)) {
                    work.emplace_back(opt);
                } else {
                    dropped++;
                }
            }

            if (dropped) {
                std::cout << logger::warn << "Dropped " << dropped << " configurations due to invalidity. Proceeding.\n";
            }

            return true;
        }
    }
    optind = 1;

    return false;
}

/**
 *
 * @param argc from main, amount of arguments in argv
 * @param argv argument vector
 * @param long_options existing long options (see get_long_opts)
 * @param work output parameter to place finished SplitterOpts into.
 */
void parseCommandLine(int argc, char* argv[], option* long_options, std::vector<SplitterOpts>& work) {
    SplitterOpts options;

    const char* OPT_STR = getOPT_STR().c_str();
    int c;

    while (EOF != (c = getopt_long(argc, argv, OPT_STR, long_options, nullptr))) {
        parseSingleParameter(c, options);
    }

    if (optind >= 0 && optind < argc) { // in parameter may be supplied raw instead of as option.
        options.inDirectory = argv[optind];
        options.setIsPNGDirectory(); // in directory is definitively set, compute this boolean.
    }

    bool valid = validateOptions(options);

    if (valid) {
        work.emplace_back(options);
    }

    // Uncomment these lines if you want to debug all your command line parameters being read correctly.
    // std::cout << logger::info << " Added job with these options:\n";
    // std::cout << options << "\n"; // intentional double newline (one from operator<< of SplitterOpts).

    std::cout << logger::info << "Input path is set to: " << options.inDirectory << "\n";
    std::cout << logger::info << "Output path is set to: " << options.outDirectory << "\n";
}

/**
 * Handle a single input character specified by c. Typically extracts something to place into options.
 * Also displays warnings of bad input, when detected.
 *
 * @param c the (short) name of the command.
 * @param options struct to place any extracted parameters into.
 */
void parseSingleParameter(int c, SplitterOpts& options) {
    switch (c) {
        case 'd':
        case 'i': {
            if (!options.inDirectory.empty()) {
                std::cout << logger::warn << "Duplicate in directory appears to be supplied. " <<
                          "This is not possible, only the latter ("<< (char) c << ") Will be used.\n";
            }
            if (optarg == nullptr) {
                std::cout << logger::warn << "-i used without parameter. inDirectory will not be set.\n";
            } else {
                options.inDirectory = optarg;
                options.setIsPNGDirectory();
            }
            break;
        }
        case 'o': {
            if (optarg == nullptr) {
                std::cout << logger::warn << "-o used without parameter. outDirectory will not be set.\n";
            } else {
                options.outDirectory = optarg;
            }
            break;
        }
        case 'k': {
            if (optarg == nullptr) {
                std::cout << logger::info << "-k has no amount supplied: The entire directory will be processed.\n";
                options.workAmount = std::numeric_limits<int>::max();
            } else {
                int amount;
                try {
                    amount = std::stoi(optarg);
                } catch (std::invalid_argument& e) {
                    std::cout << logger::warn << "-k expects numerical arguments (" << optarg << " was supplied). Using default of 1.\n";
                    amount = 1;
                }
                options.workAmount = amount;
            }
            break;
        }
            /* --config is checked before the parsing of this section; because existence of a config overrides all command line parameters.
            case 'c': {
                break;
            }
            */
        case 'r':
            options.recursive = true;
            break;
        case 's':
            options.useSubFoldersInOutput = false;
            break;
        case 'a':
            options.subtractAlphaSpritesFromIndex = true;
            break;
        case 'g':
            if (optarg == nullptr) {
                std::cout << logger::warn << "-g specified without regex literal string. Not setting -g.\n";
            } else {
                options.groundFilePattern = RegexWrapper(std::string(optarg));
            }
            break;
        case 'h':
            std::cout << "--directory (-d):          " << "Input directory.\n";
            std::cout << "                           " << "When this is a .png file, processes just this file.\n";
            std::cout << "                           " << "When a folder is specified, processes the folder based on -k.\n";
            std::cout << "--in (-i):                 " << "Alias for -d.\n";
            std::cout << "--out (-o):                " << "The output directory.\n";
            std::cout << "                           " << "When not specified, outputs to the input directory.\n";
            std::cout << "--recursive (-r):          " << "Used when processing folders.\n";
            std::cout << "                           " << "When enabled, also checks subfolders for pngs.\n";
            std::cout << "--singleFolderOutput (-s)  " << "Used when outputting split files.\n";
            std::cout << "                           " << "When enabled, creates a subfolder in the output directory,\n";
            std::cout << "                           " << "For every sprite sheet that is being split.\n";
            std::cout << "                           " << "The name of the folder is automatically determined.\n";
            std::cout << "--subtractalphafromindex (-a)\t" << "Used when outputting split files.\n";
            std::cout << "                           " << "When enabled, 'empty' space in the sprite sheet\n";
            std::cout << "                           " << "is subtracted from the index (numerical file name). \n";
            std::cout << "                           " << "Enabling this leads to a contiguous index,\n";
            std::cout << "                           " << "but numbers no longer directly map to positions on the original sheet.\n";
            std::cout << "--keepworking (-k) ('cap' in config):\t" << "Amount of files to process in a folder before stopping.\n";
            std::cout << "                           " << "Defaults to process the entire folder unless specified otherwise.\n";
            std::cout << "                           " << "When a k is specified, this amount of files are processed before halting.\n";
            std::cout << "                           " << "To process specific files in a folder only, use --config.\n";
            std::cout << "--useconfig (-c):          " << "Use a config file instead of command line options.\n";
            std::cout << "                           " << "Expects either a 'config.json' in the same directory,\n";
            std::cout << "                           " << "or a parameter specifying the config directory.\n";
            std::cout << "--groundPattern (-g):      " << "String representing an ECMAScript RegEx literal.\n";
            std::cout << "                           " << "When a sprite sheet matches the RegEx, it is processed as a ground file.\n";
            std::cout << "                           " << "This is necessary because Object and Ground sheets are indistinguishable\n";
            std::cout << "                           " << "By dimensions. When unspecified, the default value used is '/ground/i'.\n";
            std::cout << "--help (-h):               " << "Display this message\n";

            // assume the user either wants to use the program, or get information on commands. Not at the same time!
            exit(-1);
        case '?':
        case ':':
        default:
            // ignore
            break;
    }
}

/**
 * check SplitterOpts struct for any irregularities,
 * invoke defaults, inform the user of changes to the struct (their input!),
 * remove if illegal input.
 *
 * @param options the SplitterOpts to evaluate
 * @return whether or not the SplitterOpts are correctly formed
 */
bool validateOptions(SplitterOpts& options) {
    std::string& HELP_STRING = getHELP_STRING();

    if (options.inDirectory.empty()) {
        std::cout << logger::error << "No input directory specified.\n";
        std::cout << HELP_STRING;
        return false;
    }

    if (options.outDirectory.empty()) { // default to indirectory when not specified.
        std::cout << logger::warn << "No output directory given, using input directory as output.\n";
        options.outDirectory = options.inDirectory;
    }

    if (!options.isPNGInDirectory && options.workAmount == 0) {
        std::cout << logger::info << "A folder (instead of a file) was given, but no -k specified. Defaulting to process the entire folder.\n";
        options.workAmount = std::numeric_limits<int>::max();
    }

    if (options.isPNGInDirectory && options.workAmount > 0 && options.workAmount != std::numeric_limits<int>::max()) {
        std::cout << logger::warn << "A .png file was given as input, but -k was specified. -k only works for folders, and will be ignored.\n";
        options.workAmount = 1; // just in case
    }

    return true;
}
