cmake_minimum_required(VERSION 3.12)
project(OS_2 C)

set(CMAKE_C_STANDARD 99)

#add_executable(Root main.c)
add_executable(Searcher searcher.c record.c statistics.c)
add_executable(Splitter_Merger splitter_merger.c record.c statistics.c)      # DEBUG remove record.c

add_dependencies(Splitter_Merger Searcher)