#include "json.hpp"
#include <fstream>
#include <iostream>

#include <chrono>
using namespace std::chrono;

#include <cassert>
static const uint32_t ntimes = 10;

int main(int argc, char** argv) {
    
    if (argc < 2)
        return -1;
    
    std::ifstream inputfile(argv[1]);
    std::string to_parse;
    int count;

    inputfile.seekg(0, std::ios::end);   
    to_parse.reserve(inputfile.tellg());
    inputfile.seekg(0, std::ios::beg);

    to_parse.assign((std::istreambuf_iterator<char>(inputfile)),
                     std::istreambuf_iterator<char>());
    
    steady_clock::time_point start_time = steady_clock::now();
    for (unsigned i = 0; i < ntimes; ++i) {
        JSON::Value parsed = JSON::parse(to_parse);
    }
    steady_clock::time_point end_time = steady_clock::now();
    microseconds us = duration_cast<microseconds>(end_time - start_time);
    std::cout << "[+] Finished successfully with an average of: " << (us.count() / ntimes) << " us\n";
}
