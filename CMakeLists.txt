cmake_minimum_required(VERSION 3.24)
project(Search_Engine)

set(CMAKE_CXX_STANDARD 17)

file(COPY sample_data/ DESTINATION sample_data/)

# show compiler output and enable warnings
set(CMAKE_VERBOSE_MAKEFILE ON)
add_compile_options(-Wall -Wextra -pedantic)

add_executable(rapidJSONExample rapidJSONExample.cpp)

# this makes sure we also link rapidjson
target_include_directories(rapidJSONExample PRIVATE rapidjson/)

set(CMAKE_VERBOSE_MAKEFILE OFF)

include(CTest)
add_executable(tests test.cpp CountingWord.cpp indexHandler.cpp documentParser.cpp queryEngine.cpp Porter2Stemmer/porter2_stemmer.cpp)
add_executable(Search_Engine main.cpp searchEngine.cpp indexHandler.cpp CountingWord.cpp documentParser.cpp queryEngine.cpp Porter2Stemmer/porter2_stemmer.cpp)
add_test(NAME SimpleTest COMMAND tests)
