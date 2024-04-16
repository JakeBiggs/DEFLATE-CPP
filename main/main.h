#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <mio/mio.hpp>
#include <benchmark/benchmark.h>
#include <LZ77/LZ77.h>
#include <Huffman/Huffman.h>
#include <cstring>
using namespace std;
void compress();
void decompress();

void testWriteRead();

void writeCompressedData(ofstream& outputFile, const map<unsigned char, string>& huffmanCodes, const std::vector<unsigned char>& compressedData){
    //Write the size of the metadata first
    size_t size = huffmanCodes.size();
    outputFile.write(reinterpret_cast<const char*>(&size), sizeof(size));

    //Write the metadata
    for (const auto& pair : huffmanCodes) {
        outputFile.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first)); //Write the byte
        size_t length = pair.second.size(); //Get the length of the huffman code
        outputFile.write(reinterpret_cast<const char*>(&length), sizeof(length)); //Write the length of the huffman code
        for (char c : pair.second) { //Write each character of the huffman code
            outputFile.write(&c, sizeof(c));
        }
    }


    //Write separator
    unsigned char separator = 255;
    for (int i = 0; i < 8; ++i) {
        outputFile.write(reinterpret_cast<const char*>(&separator), sizeof(separator));
    }
    //Write the size of the compressed data
    size = compressedData.size();
    outputFile.write(reinterpret_cast<const char*>(&size), sizeof(size));


    //Write the compressed data
    outputFile.write(reinterpret_cast<const char*>(&compressedData[0]), compressedData.size());
}

map<unsigned char, string> readHuffmanCodes(ifstream& inputFile){
    //Read the size of the huffman codes
    size_t size;
    inputFile.read(reinterpret_cast<char*>(&size), sizeof(size));

    //Read the huffman codes
    map<unsigned char, string> huffmanCodes;
    for (size_t i = 0; i < size; ++i){
        unsigned char byte;
        inputFile.read(reinterpret_cast<char*>(&byte), sizeof(byte)); //Read the byte

        size_t length;
        inputFile.read(reinterpret_cast<char*>(&length), sizeof(length)); //Read the length of the huffman code

        string value;
        for (size_t j = 0; j < length; ++j) { //Read each character of the huffman code
            char c;
            inputFile.read(&c, sizeof(c));
            value += c;
        }

        huffmanCodes[byte] = value;
    }

    return huffmanCodes;
}


vector<unsigned char> readCompressedData(ifstream& inputFile) {
    /*
    // Get the current position of the file pointer
    streampos currentPos = inputFile.tellg();

    // Go to the end of the file
    inputFile.seekg(0, std::ios::end);

    // Get the end position and calculate the size of the compressed data
    streampos endPos = inputFile.tellg();
    size_t size = endPos - currentPos;

    // Go back to the current position
    inputFile.seekg(currentPos);

    // Read the compressed data
    vector<unsigned char> compressedData(size);
    inputFile.read(reinterpret_cast<char*>(&compressedData[0]), size);

    return compressedData;
    */
    // Read until separator
    unsigned char separator = 255;
    unsigned char byte;
    int count = 0;
    while (count < 8) {
        inputFile.read(reinterpret_cast<char*>(&byte), sizeof(byte));
        if (byte == separator) {
            ++count;
        } else {
            count = 0;
        }
    }
    // Read the size of the compressed data
    size_t size;
    inputFile.read(reinterpret_cast<char*>(&size), sizeof(size));

    // Read the compressed data
    vector<unsigned char> compressedData(size);
    inputFile.read(reinterpret_cast<char*>(&compressedData[0]), size);

    return compressedData;
}


void readUntilEndOfCodes() {
    ifstream inputFile("output.txt");

    if (!inputFile) {
        cout << "Failed to open output file" << endl;
        return;
    }

    string line;
    while (getline(inputFile, line)) {
        if (line == "END_OF_CODES") {
            break;
        }
        cout << line << endl;
    }

    inputFile.close();
}