#include "benchmark.hpp"
#include "cli.hpp"

#include <string>
#include <vector>

namespace {

struct Options {
    std::string inputPath;
    std::string outputPath;
    std::string benchmarkPath;
    bool append = false;
};

Options parseOptions(int argc, char* argv[]) {
    Options options;
    std::vector<std::string> positional;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--benchmark-csv" && i + 1 < argc) {
            options.benchmarkPath = argv[++i];
        } else if (arg == "--append") {
            options.append = true;
        } else {
            positional.push_back(arg);
        }
    }
    options.inputPath = positional.empty() ? "data/contacts_50.csv" : positional[0];
    options.outputPath = positional.size() > 1 ? positional[1] : options.inputPath;
    return options;
}

}  // namespace

int main(int argc, char* argv[]) {
    const auto options = parseOptions(argc, argv);
    if (!options.benchmarkPath.empty()) {
        return runSearchBenchmarkBatch(options.inputPath, options.benchmarkPath,
                                       options.append);
    }
    return runPhoneBookCli(options.inputPath, options.outputPath);
}
