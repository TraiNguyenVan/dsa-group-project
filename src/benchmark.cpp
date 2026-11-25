#include "benchmark.hpp"
#include "phonebook.hpp"
#include "timer.hpp"

#include <algorithm>
#include <array>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

namespace {

constexpr std::size_t kRepeats = 5;
constexpr const char* kMissPhone = "0000000000";

struct Target {
    const char* key;
    const char* label;
    long long index;
    std::string phone;
};

using Targets = std::array<Target, 4>;

Targets makeTargets(const PhoneBook& book) {
    const std::size_t middle = book.size() / 2;
    const std::size_t last = book.size() - 1;
    return {{
        {"first", "first (linear best case)", 0, book.getPhoneAt(0)},
        {"middle", "middle (linear avg / binary best)",
         static_cast<long long>(middle), book.getPhoneAt(middle)},
        {"last", "last (linear worst case)",
         static_cast<long long>(last), book.getPhoneAt(last)},
        {"miss", "miss (linear worst / hash & binary worst)", -1, kMissPhone},
    }};
}

struct Measurement {
    const char* key;
    const char* label;
    const char* indexNote;
    int index = -1;
    std::array<double, kRepeats> samples{};
};

template <typename Search>
Measurement measure(const char* key, const char* label, const char* indexNote,
                    Search search) {
    Measurement result{key, label, indexNote};
    for (auto& sample : result.samples) {
        sample = timeIt([&] { result.index = search(); });
    }
    return result;
}

std::array<Measurement, 3> measureSearches(const PhoneBook& book,
                                          const std::string& phone) {
    return {{
        measure("linear", "Linear", "", [&] { return book.searchLinearByPhone(phone); }),
        measure("hash", "Hash", ", position-independent",
                [&] { return book.searchHashByPhone(phone); }),
        measure("binary", "Binary", ", sorted index, position-independent",
                [&] { return book.searchBinaryByPhone(phone); }),
    }};
}

std::string utcTimestamp() {
    const auto now = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    gmtime_s(&tm, &now);
#else
    gmtime_r(&now, &tm);
#endif
    char stamp[32]{};
    std::strftime(stamp, sizeof(stamp), "%Y-%m-%dT%H:%M:%SZ", &tm);
    return stamp;
}

std::string compilerLabel() {
#if defined(__clang__)
    return std::string("clang ") + __clang_version__;
#elif defined(_MSC_VER)
    return "MSVC " + std::to_string(_MSC_FULL_VER);
#elif defined(__GNUC__)
    return std::string("g++ ") + __VERSION__;
#else
    return "unknown compiler";
#endif
}

std::string escapeCsvField(const std::string& field) {
    if (field.find_first_of(",\"\r\n") == std::string::npos) {
        return field;
    }
    std::string quoted = "\"";
    for (char ch : field) {
        if (ch == '"') {
            quoted += '"';
        }
        quoted += ch;
    }
    return quoted + '"';
}

bool hasCsvHeader(const std::string& path) {
    std::ifstream input(path);
    std::string firstLine;
    return std::getline(input, firstLine) && !firstLine.empty();
}

}  // namespace

void runSearchBenchmark(PhoneBook& book, const std::string& inputPath) {
    if (book.size() == 0) {
        std::cout << "Phonebook empty, auto-loading from: " << inputPath << '\n';
        const int loaded = book.loadfromCSV(inputPath);
        if (loaded == -1) {
            std::cout << "Cannot open file.\n";
            return;
        }
        if (book.size() == 0) {
            std::cout << "No contacts to benchmark. Load contacts first (option 1).\n";
            return;
        }
        std::cout << "Auto-loaded " << loaded << " contacts.\n";
    }

    std::cout << "\nBenchmarking phone search (linear: first=best / middle / last,miss=worst; "
                 "hash ~O(1) and binary O(log n), both position-independent; miss=worst for hash/binary)"
              << " (" << book.size() << " contacts, " << kRepeats
              << " runs each, best reported).\n";

    for (const auto& target : makeTargets(book)) {
        if (target.phone.empty()) {
            std::cout << '[' << target.label << "] index " << target.index
                      << ": cannot pick target.\n";
            continue;
        }
        const auto results = measureSearches(book, target.phone);
        std::cout << '[' << target.label << " index " << target.index
                  << " phone " << target.phone << "]\n";
        for (const auto& result : results) {
            const auto best = *std::min_element(result.samples.begin(), result.samples.end());
            std::cout << "  " << result.label << " best of " << kRepeats << ": " << best
                      << "ms. (index " << result.index << result.indexNote << ")\n";
        }
    }
    std::cout << "Linear: first=best, last/miss=worst. Hash/binary: ~constant; "
                 "miss is worst (full chain / log n probes).\n";
}

int runSearchBenchmarkBatch(const std::string& inputPath,
                            const std::string& outputPath, bool append) {
    PhoneBook book;
    if (book.loadfromCSV(inputPath) == -1) {
        std::cerr << "Cannot open file: " << inputPath << '\n';
        return 1;
    }
    if (book.size() == 0) {
        std::cerr << "No contacts to benchmark.\n";
        return 1;
    }

    const bool needHeader = !append || !hasCsvHeader(outputPath);
    std::ofstream out(outputPath, append ? std::ios::app : std::ios::trunc);
    if (!out) {
        std::cerr << "Cannot open output file: " << outputPath << '\n';
        return 1;
    }
    if (needHeader) {
        out << "language,dataset,n,case,algo,run,ms,timestamp,toolchain,target_index,phone\n";
    }
    out << std::setprecision(17);

    const auto timestamp = utcTimestamp();
    const auto datasetField = escapeCsvField(inputPath);
    const auto toolchainField = escapeCsvField(compilerLabel());
    std::size_t rows = 0;
    for (const auto& target : makeTargets(book)) {
        if (target.phone.empty()) {
            continue;
        }
        for (const auto& result : measureSearches(book, target.phone)) {
            for (std::size_t run = 0; run < result.samples.size(); ++run) {
                out << "cpp," << datasetField << ',' << book.size() << ',' << target.key << ','
                    << result.key << ',' << (run + 1) << ',' << result.samples[run] << ','
                    << timestamp << ',' << toolchainField << ',' << target.index << ','
                    << target.phone << '\n';
                ++rows;
            }
        }
    }
    out.close();
    if (!out) {
        std::cerr << "Cannot write benchmark output: " << outputPath << '\n';
        return 1;
    }
    std::cout << "Wrote " << rows << " rows -> " << outputPath
              << " (" << book.size() << " contacts).\n";
    return 0;
}
