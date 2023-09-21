//
// Created by 20173607 on 20/09/2023.
//

#include "JSONConfigParser.hpp"
#include "struct_mapping.h"

#include <fstream>

namespace sm = struct_mapping;
namespace logger = LoggerTags;

bool once_flag = false;

// todo: suboptimal, leads to reading the whole stream twice. . . need derived structs and union type maybe?
struct GroundFilePatternHandler {
    std::string regex_literal;
};

void registerJSONMappings() {
    if (once_flag) return;
    once_flag = true;

    sm::reg(&SplitterOpts::inDirectory, "in", sm::NotEmpty{});
    sm::reg(&SplitterOpts::outDirectory, "out", sm::NotEmpty{});
    // groundFilePattern: Cannot be mapped 1:1 from string, this is too complex for my sm::Remap.
    sm::reg(&SplitterOpts::workAmount, "cap", sm::Default{std::numeric_limits<int>::max()});
    // isPNGInDirectory: Is not allowed to be set by JSON, this is a computed property from in directory.
    sm::reg(&SplitterOpts::recursive, "recursive", sm::Default{false});

    // useSubFoldersInOutput: command line 'singleFolderOutput' is the inverse, so we should remap it.
    std::function<bool(const bool&)> invert = [](const bool& in) { return !in; };
    sm::reg(&SplitterOpts::useSubFoldersInOutput, "singleFolderOutput", sm::Default{true}, sm::Remap{invert});
    sm::reg(&SplitterOpts::subtractAlphaSpritesFromIndex, "subtractAlphaFromIndex", sm::Default{false});

    // groundFilePattern shall be handled in two steps: Extract the string, then manually insert the wrapper.
    sm::reg(&GroundFilePatternHandler::regex_literal, "groundFilePattern", sm::Default{"/ground/i"});
}

/**
 * @param pathToFile string path to json config file.
 * @param work vector of configurations to be extracted from the config file
 * @throws StructMappingException if there are JSON errors.
 */
// static
void JSONConfigParser::parseConfig(const std::string &pathToFile, std::vector<SplitterOpts> &work) {
    registerJSONMappings();

    std::ifstream jsonStream(pathToFile);

    if (! jsonStream.is_open()) {
        std::cout << logger::error << "Could not open config file '"  << pathToFile << "'.\n";
        exit(-1); // We will not proceed with other files or command line args, fix your file please :)
    }

    // todo: use library differently if array-like config, for now it's just a single json object.
    SplitterOpts s;
    GroundFilePatternHandler g;
    try {
        sm::map_json_to_struct(s, jsonStream);
        // reset the stream back to the start.
        jsonStream.clear();
        jsonStream.seekg(0);
        // parse the JSON again, extracting the regex literal.
        sm::map_json_to_struct(g, jsonStream);

        s.groundFilePattern = RegexWrapper(g.regex_literal);
    } catch (sm::StructMappingException& e) {
        throw sm::StructMappingException("There is likely a JSON parsing error with '" + pathToFile + "'. Received exception message:\n\t\"" + e.what() + "\"");
    }

    work.emplace_back(s);
}