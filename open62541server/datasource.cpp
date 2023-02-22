#include "datasource.h"
#include <iostream>
#include <fstream>
#include <string>


void writeInFile(const std::string& text) {
    // Create an instance of std::ofstream with the file name
    std::ofstream file("example.txt", std::ios::app);

    // Check if the file was opened successfully
    if (!file.is_open()) {
        std::cerr << "Error opening the file" << std::endl;
        return;
    }

    // Write the text to the file
    file << text << std::endl;

    // Close the file
    file.close();
}

int countLines(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return -1;
    }

    int num_lines = 0;
    std::string line;
    while (std::getline(file, line)) {
        num_lines++;
    }

    file.close();
    return num_lines;
}