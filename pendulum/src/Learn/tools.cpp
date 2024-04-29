#include "tools.h"

// Concatenate two strings and return a new dynamically allocated char*
char* concatenateStrings(const char* str1, const char* str2) {
    // Calculate the length of the resulting string
    size_t len = strlen(str1) + strlen(str2) + 1; // +1 for the null terminator

    // Allocate memory for the result
    char* result = new char[len];

    // Copy the strings into the result buffer
    strcpy(result, str1);
    strcat(result, str2);

    return result;
}

// concatenates the current time in the provided buffer
void getCurrentTimeAsString(char* bufferTime, char* gen_per_agreg, char* init_seed) {
    // get currrent time
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    // convert time_t struct into char*
    char buff[BUFFER_SIZE];
    std::strftime(buff, BUFFER_SIZE, "%Y-%m-%d_%H:%M.%S", std::localtime(&now));
    sprintf(bufferTime,"nb_aggr_%s|seed_%s|%s",gen_per_agreg, init_seed, buff);
}

char* createFolderWithCurrentTime(char* path, char* gen_per_agreg, char* init_seed) {
    // the folder is named after the exact time it was created
	char bufferTime[BUFFER_SIZE*2];

	getCurrentTimeAsString(bufferTime,gen_per_agreg,init_seed);

    char* bufferPath = concatenateStrings(ROOT_DIR, path);
	char* saveFolderPath = concatenateStrings(bufferPath, bufferTime);

    // Create the folder
    std::filesystem::create_directory(saveFolderPath);

	// Create a subfolder for the dot files
	char* saveSubFolderPath = concatenateStrings(saveFolderPath, "/Graphs");
	std::filesystem::create_directory(saveSubFolderPath);

    // Print the created folder name
    std::cout << "Created folder: " << saveFolderPath << std::endl;

	//return pointer for freeing the allocated memory
	return saveFolderPath;
}

// copy the source file into the dest file
void copyFile(char *filename_src, char *filename_dest) {
	
	FILE *fptr_src, *fptr_dest;
	char c;

	// Open source file for reading
	fptr_src = fopen(filename_src, "r");
	if (fptr_src == NULL) {
		printf("Cannot open file %s\n", filename_src);
		exit(0);
	}

	// Open destination file for writing 
	fptr_dest = fopen(filename_dest, "w");
	if (fptr_dest == NULL) {
		printf("Cannot open file %s\n", filename_dest);
		exit(0);
	}

	// Read content from file
	c = fgetc(fptr_src);
	while (c != EOF) {
		fputc(c, fptr_dest);
		c = fgetc(fptr_src);
	}

	fclose(fptr_src);
	fclose(fptr_dest);
}