#include <cstdlib>
#include <iostream>

void runTest(const char* params);
int main() {
    runTest("1 2");
    return 0;
}

void runTest(const std::string& params) {
    // Construct the command to run your C++ program with the given parameters
    std::string command = "./mnist ";
    command += params;

    system(command.c_str());

}
