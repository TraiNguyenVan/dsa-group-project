# ============================================================
# Phonebook Lookup — CMake build
#
# Usage (any OS: Linux, macOS, Windows):
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release  # configure once
#   cmake --build build                             # build the demo binary
#   cmake --build build --target run-50             # build + run on 50-row CSV
#   ./build/demo data/contacts_100k.csv             # run manually (build/demo.exe on Windows)
#
# This file mirrors the old Makefile (TARGET=build/cpp/demo,
# SOURCES=src/*.cpp, INCLUDES=include/, STD=c++17).
# ============================================================

# Minimum CMake version we rely on. 3.16 is old enough to exist on most
# systems but new enough for C++17 support and $<TARGET_FILE:...> genexes.
cmake_minimum_required(VERSION 3.16)

# Define the project: its name, version, and language(s).
# This also detects a working C++ compiler (g++, clang++, or MSVC cl).
project(phonebook_lookup VERSION 1.0 LANGUAGES CXX)

# --- Language standard ------------------------------------------------
# CMAKE_CXX_STANDARD 17 ........ ask for C++17 (same as Makefile -std=c++17).
# CMAKE_CXX_STANDARD_REQUIRED ON  fail loudly if the compiler lacks C++17
#                                 instead of silently falling back.
# CMAKE_CXX_EXTENSIONS OFF ...... use strict -std=c++17, not -std=gnu++17,
#                                 so compiler-specific extensions stay off.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- The binary --------------------------------------------------------
# add_executable(<name> <sources...>): compile these .cpp files and link
# them into one program called "demo" (demo.exe on Windows, automatically).
# Listed explicitly (not file(GLOB ...)) so new files can't silently
# change the build without editing this file.
add_executable(demo
  src/main.cpp
  src/phonebook.cpp
  src/hashtable.cpp
)

# Tell the compiler where to find our headers (#include "phonebook.hpp").
# PRIVATE means only this target needs it (nothing links against demo).
# Equivalent of the Makefile's -Iinclude flag.
target_include_directories(demo PRIVATE include)

# --- Warnings ------------------------------------------------------------
# Warnings catch bugs early; we keep them on but portable:
# MSVC (Visual Studio) uses /W4, everyone else (GCC/Clang) uses -Wall -Wextra.
# Optimisation is NOT forced here — pick it at configure time with
# -DCMAKE_BUILD_TYPE=Release (like the Makefile's -O2) or Debug.
if(MSVC)
  target_compile_options(demo PRIVATE /W4)
else()
  target_compile_options(demo PRIVATE -Wall -Wextra)
endif()

# When you press F5 in Visual Studio, run from the project root so the
# relative paths (data/contacts_*.csv) resolve. No effect on Make/Ninja.
set_target_properties(demo PROPERTIES
  VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# --- Run helpers (mirror the Makefile's run/run-50/run-100k/...) --------
# add_custom_target(<name> ...): create a `cmake --build build --target <name>`
# shortcut that builds demo if needed, then runs it on a dataset.
#
# Inside the COMMAND:
#   $<TARGET_FILE:demo> ... a "generator expression": CMake substitutes the
#                           real path of the binary at build time, including
#                           the .exe suffix on Windows.
#   "${CMAKE_SOURCE_DIR}/..."  absolute path to the CSV, so the target works
#                           no matter where you invoke the build from.
#   WORKING_DIRECTORY ..... run from the project root, matching the Makefile's
#                           `./demo data/...` behaviour.
#   DEPENDS demo .......... ensure the binary is rebuilt before running.
#   VERBATIM .............. quote arguments safely on every shell.
function(add_demo_run_target target_name csv_relpath)
  add_custom_target(${target_name}
    COMMAND $<TARGET_FILE:demo> "${CMAKE_SOURCE_DIR}/${csv_relpath}"
    DEPENDS demo
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    COMMENT "Running demo with ${csv_relpath}"
    VERBATIM
  )
endfunction()

# One shortcut per dataset, same names as `make run-50` etc.
add_demo_run_target(run data/contacts_100k.csv)
add_demo_run_target(run-50 data/contacts_50.csv)
add_demo_run_target(run-100k data/contacts_100k.csv)
add_demo_run_target(run-200k data/contacts_200k.csv)
add_demo_run_target(run-1m data/contacts_1m.csv)

# Batch benchmark (C++ rows only): every run -> benchmark/results-cpp.csv
# + plot-cpp.png (needs matplotlib; `make run-benchmark` skips plotting
# gracefully instead). The full 5-language matrix lives in `make run-benchmark`.
add_custom_target(run-benchmark-cpp
  COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/benchmark"
  COMMAND $<TARGET_FILE:demo> --benchmark-csv "${CMAKE_SOURCE_DIR}/benchmark/results-cpp.csv" "${CMAKE_SOURCE_DIR}/data/contacts_100k.csv"
  COMMAND ${CMAKE_COMMAND} -E env python3 "${CMAKE_SOURCE_DIR}/benchmark/plot.py" "${CMAKE_SOURCE_DIR}/benchmark/results-cpp.csv" "${CMAKE_SOURCE_DIR}/benchmark/plot-cpp.png"
  DEPENDS demo
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  COMMENT "Writing C++ benchmark rows to benchmark/results-cpp.csv"
  VERBATIM
)
