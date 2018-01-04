
#ifndef CPPSTRTOK_H_
#define CPPSTRTOK_H_

#include <string>
using namespace std;
#include "cppstrtok.h"
#include "auxlib.h"
#include "string_set.h"

const string CPP = "/usr/bin/cpp -nostdinc";
constexpr size_t LINESIZE = 1024;

// Chomp the last character from a buffer if it is delim.
void chomp(char* string, char delim);

// Run cpp against the lines of the file.
void cpplines(FILE* tok_file, string_set& set);

#endif
