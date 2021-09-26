#include <iostream>
#include <getopt.h>
#include <vector>
#include "splitterOptions.h"
#include "Splitter.h"

// https://linux.die.net/man/3/getopt
extern char* optarg; // NOLINT(readability-redundant-declaration)
[[maybe_unused]] // it is modified in getopt.h. Silly compiler.
extern int optind; // NOLINT(readability-redundant-declaration)

// printed when the user gives illegal input
std::string& getHELP_STRING() {
    static std::string HELP_STRING = "usage: (todo)\nUse --help for more information.\n";
    return HELP_STRING;
}

// used by getopt, getopt_long etc. each character is the short name of an option. Colon after means it has a parameter. Double colon means optional parameter.
std::string& getOPT_STR() {
    static std::string OPT_STR = "hrd:i:o::k::c::";
    return OPT_STR;
}

// This is more rigorously tested by the std::filesystem class further in execution (if the file exists & if it can be loaded).
// All that matters for now, is if the user _intends_ to run it on a directory pointing to an alleged 'png'.
bool checkIsPNGDirectory(std::string& dir) {
    return dir.size() >= 4 && (0 == dir.compare(dir.size() - 4, 4, ".png"));
}

// SpriteSheetIO.h is agnostic to workload; expecting only a sequence of jobs,
// hence even the single instance of SplitterOpts from parseCommandLine should be a vector.
bool readConfig(int argc, char* argv[], option* long_options, std::vector<SplitterOpts>& work);
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
            {"keepworking", optional_argument,  nullptr, 'k'},
            {"useconfig",   optional_argument,  nullptr, 'c'},
            {"help",        no_argument,        nullptr, 'h'},
            {nullptr,       no_argument,        nullptr, 0} // Documentation says last element must be empty.
    };

    if (argc <= 1) {
        std::cout << "[ERROR] No input given.\n";
        std::cout << HELP_STRING;
        exit(-1);
    }

    std::vector<SplitterOpts> jobs;
    bool hasConfig = readConfig(argc, argv, long_options, jobs);
    if (!hasConfig) {
        parseCommandLine(argc, argv, long_options, jobs);
    }

    if (!jobs.empty()) {
        Splitter worker{};
        worker.work(jobs);
    }

    return 0;
}

/**
 * todo.
 * @param argc
 * @param argv
 * @param long_options
 * @param work
 * @return if reading the config was successful.
 */
bool readConfig(int argc, char* argv[], option* long_options, std::vector<SplitterOpts> &work) {
    // todo: foreach config entry create splitteropts, validate, and add to vector if valid.
    const char* OPT_STR = getOPT_STR().c_str();
    int c;
    // check for config file specifically before parsing command line parameters
    while (EOF != (c = getopt_long(argc, argv, OPT_STR, long_options, nullptr))) {
        if (c == 'c') {
            // todo: read config
            std::cout << "configs are not supported yet.\n";

            // todo: before returning, report on the total amount of dropped jobs (from validation failures).
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
    std::string& HELP_STRING = getHELP_STRING();

    SplitterOpts options;

    const char* OPT_STR = getOPT_STR().c_str();
    int c;

    while (EOF != (c = getopt_long(argc, argv, OPT_STR, long_options, nullptr))) {
        parseSingleParameter(c, options);
    }

    if (optind >= 0 && optind < argc) { // in parameter may be supplied raw instead of as option.
        options.inDirectory = argv[optind];
        std::cout << options.inDirectory << "\n";
        options.isPNGInDirectory = checkIsPNGDirectory(options.inDirectory);
    }

    bool valid = validateOptions(options);

    if (valid) {
        work.emplace_back(options);
    }
}

/**
 * Handle a single input character specified by c. Typically extracts something to place into options.
 * Also displays warnings of bad input, when detected.
 *
 * @param c the (short) name of the command.
 * @param options struct to place any extracted parameters into.
 */
 //todo: with configs, optarg should not be used directly here (const char* instead, with the command line version passing optarg)
void parseSingleParameter(int c, SplitterOpts& options) {
    std::string& HELP_STRING = getHELP_STRING();
    switch (c) {
        case 'd':
        case 'i': {
            if (!options.inDirectory.empty()) {
                std::cout << "[WARNING] duplicate in directory appears to be supplied. " <<
                          "This is not possible, only the latter ("<< (char) c << ") Will be used.\n";
            }
            if (optarg == nullptr) {
                std::cout << "[WARNING] -i used without parameter. inDirectory will not be set.\n";
            } else {
                options.inDirectory = optarg;
                options.isPNGInDirectory = checkIsPNGDirectory(options.inDirectory);
            }
            break;
        }
        case 'o': {
            if (optarg == nullptr) {
                std::cout << "[WARNING] -o used without parameter. outDirectory will not be set.\n";
            } else {
                options.outDirectory = optarg;
            }
            break;
        }
        case 'k': {
            if (optarg == nullptr) {
                std::cout << "[INFO] -k has no amount supplied: The entire directory will be processed.\n";
                options.workAmount = std::numeric_limits<int>::max();
            } else {
                int amount;
                try {
                    amount = std::stoi(optarg);
                } catch (std::invalid_argument& e) {
                    std::cout << "[WARNING] -k expects numerical arguments (" << optarg << " was supplied). Using default of 1.\n";
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
        case 'h':
            std::cout << "--directory (-d):          " << "Input directory. When this is a .png file, processes just this file.\n";
            std::cout << "                           " << "When a folder is specified, processes the folder based on -k.\n";
            std::cout << "--in (-i):                 " << "Alias for -d.\n";
            std::cout << "--out (-o):                " << "The output directory. When not specified, outputs to the input directory.\n";
            // TODO: consider how to handle errors / warnings / prevent the case of trying to split a spritesplit output folder. Easy on the current run (just remember the folder name!) but what about subsequent runs?
            std::cout << "--recursive (-r):          " << "Used when processing folders. When enabled, also checks subfolders for pngs.\n";
            std::cout << "--keepworking (-k):        " << "Amount of files to process in a folder before stopping.\n";
            std::cout << "                           " << "Defaults to process the entire folder unless specified otherwise.\n";
            std::cout << "                           " << "When a k is specified, this amount of files are processed before halting.\n";
            std::cout << "                           " << "To process specific files in a folder only, use --config.\n";
            std::cout << "-- useconfig (-c):         " << "Use a config file instead of command line options.\n";
            std::cout << "                           " << "Expects either a 'config.json' in the same directory,\n";
            std::cout << "                           " << "or a parameter specifying the config directory.\n";
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
        std::cout << "[ERROR] No input directory specified.\n";
        std::cout << HELP_STRING;
        return false;
    }

    if (options.outDirectory.empty()) { // default to indirectory when not specified.
        std::cout << "[WARNING] No output directory given, using input directory as output.\n";
        options.outDirectory = options.inDirectory;
    }

    if (!options.isPNGInDirectory && options.workAmount == 0) {
        std::cout << "[INFO] A folder (instead of a file) was given, but no -k specified. Defaulting to process the entire folder.\n";
        options.workAmount = std::numeric_limits<int>::max();
    }

    if (options.isPNGInDirectory && options.workAmount > 0) {
        std::cout << "[WARNING] a .png file was given as input, but -k was specified. -k only works for folders, and will be ignored.\n";
        options.workAmount = 1; // just in case
    }

    return true;
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