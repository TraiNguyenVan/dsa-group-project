#ifndef PHONEBOOK_BENCHMARK_HPP
#define PHONEBOOK_BENCHMARK_HPP

#include <string>

class PhoneBook;

// Both entry points use the same search targets and measurements.
void runSearchBenchmark(PhoneBook& phonebook, const std::string& inputPath);
int runSearchBenchmarkBatch(const std::string& inputPath,
                            const std::string& outputPath, bool append);

#endif
