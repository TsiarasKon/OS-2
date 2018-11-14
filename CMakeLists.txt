cmake_minimum_required(VERSION 3.12)
project(OS_2 C)

set(CMAKE_C_STANDARD 99)

add_executable(Searcher searcher.c record.c statistics.c util.c)
add_executable(Splitter_Merger splitter_merger.c statistics.c util.c)
add_executable(Root root.c record.c statistics.c util.c)

add_dependencies(Splitter_Merger Searcher)
add_dependencies(Root Splitter_Merger)
