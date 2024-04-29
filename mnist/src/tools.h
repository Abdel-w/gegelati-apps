#ifndef TOOLS_H
#define TOOLS_H

#include <filesystem>
#include <iostream>
#include <sstream>
#include <cstring>

constexpr size_t BUFFER_SIZE = 256;

// Concatenate two strings and return a new dynamically allocated char*
char* concatenateStrings(const char* str1, const char* str2);

// concatenates the current time in the provided buffer
void getCurrentTimeAsString(char* bufferTime, char* gen_per_agreg, char* init_seed);

// Create a specific folder for this training
char* createFolderWithCurrentTime(char* path, char* gen_per_agreg, char* init_seed);

// copy the source file into the dest file
void copyFile(char *filename_src, char *filename_dest);

#endif