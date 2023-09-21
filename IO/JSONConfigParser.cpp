//
// Created by 20173607 on 20/09/2023.
//

#include "JSONConfigParser.hpp"
#include "struct_mapping.h"

#include <fstream>

namespace sm = struct_mapping;
namespace logger = LoggerTags;

/**
 * The JSON-to-struct mappings need to be registered to the struct_mapping library only once.
 */
bool once_flag = false;

/**
 * The config is represented as an Object with a field 'jobs'.
 * 'jobs' holds an Array of SplitterOpt objects represented as JSON.
 */
struct SplitterOptsArray {
    std::vector<SplitterOpts> jobs;
};

/**
 * This struct is necessary to extract the ECMAScript Regex Literal in SplitterOpts,
 * because the struct_mapping library is not capable to natively wrap the std::string to a RegexWrapper class.
 * And it will naturally throw an error if you try to insert a string into it.
 */
struct SplitterOptsComplexTypeHandler {
    std::string groundFilePattern;
};

/** Because the config is an array of jobs, the handler has to follow the same convention. */
struct SplitterOptsComplexTypeHandlerArray {
    std::vector<SplitterOptsComplexTypeHandler> jobs;
};

/**
 * Register to the struct_mapping library mappings from JSON to struct.
 * SplitterOpts, SplitterOptsArray, SplitterOptsComplexTypeHandler, and SplitterOptsComplexTypeHandlerArray are registered.
 *
 * This function needs to be called only once, and the global variable once_flag ensures this.
 */
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

    sm::reg(&SplitterOptsArray::jobs, "jobs", sm::Required{});
    sm::reg(&SplitterOptsComplexTypeHandlerArray::jobs, "jobs", sm::Required{});

    // groundFilePattern shall be handled in two steps: Extract the string, then manually insert the wrapper.
    sm::reg(&SplitterOptsComplexTypeHandler::groundFilePattern, "groundFilePattern", sm::Default{"/ground/i"});
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

    SplitterOptsArray soa;
    SplitterOptsComplexTypeHandlerArray socta;
    try {
        sm::map_json_to_struct(soa, jsonStream);
        // reset the stream back to the start.
        jsonStream.clear();
        jsonStream.seekg(0);
        // parse the JSON again, extracting the complex types.
        sm::map_json_to_struct(socta, jsonStream);

        if (soa.jobs.size() != socta.jobs.size()) {
            // This should be impossible due to every job having a groundFilePattern assigned: The field is optional but has a default assignment.
            throw sm::StructMappingException("job reading mismatch\n");
        }
    } catch (sm::StructMappingException& e) {
        throw sm::StructMappingException("There is likely a JSON parsing error with '" + pathToFile + "'. Received exception message:\n\t\"" + e.what() + "\"");
    }

    // finally, map the complex types, extracted as some primitive representation, and insert them into soa.
    for (size_t index = 0; index < soa.jobs.size(); ++index) {
        soa.jobs[index].groundFilePattern = RegexWrapper(socta.jobs[index].groundFilePattern);
        soa.jobs[index].setIsPNGDirectory();
    }

    work = std::move(soa.jobs);
    jsonStream.close();
}