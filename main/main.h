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
#include <chrono>
using namespace std;

void testWriteRead();

void writeCompressedData(ofstream& outputFile, const unordered_map<unsigned char, string>& huffmanCodes, const std::vector<unsigned char>& compressedData){
    //Write the size of the metadata first
    size_t size = huffmanCodes.size();
    outputFile.write(reinterpret_cast<const char*>(&size), sizeof(size));

    //Write the metadata
    for (const auto& pair : huffmanCodes) {
        outputFile.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first)); //Write the byte
        size_t length = pair.second.size(); //Get the length of the huffman code
        outputFile.write(reinterpret_cast<const char*>(&length), sizeof(length)); //Write the length of the huffman code

        // Write the Huffman code bit by bit
        unsigned char byte = 0;
        int bitCount = 0;
        for (char bit : pair.second) {
            byte = (byte << 1) | (bit == '1');
            if (++bitCount == 8) {
                outputFile.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
                byte = 0;
                bitCount = 0;
            }
        }
        // Write any remaining bits
        if (bitCount > 0) {
            byte <<= (8 - bitCount);
            outputFile.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
        }
    }

    //Write the size of the compressed data
    size = compressedData.size();
    outputFile.write(reinterpret_cast<const char*>(&size), sizeof(size));

    //Write the compressed data
    outputFile.write(reinterpret_cast<const char*>(&compressedData[0]), compressedData.size());
}

unordered_map<unsigned char, string> readHuffmanCodes(ifstream& inputFile){
    //Read the size of the huffman codes
    size_t size;
    inputFile.read(reinterpret_cast<char*>(&size), sizeof(size));

    //Read the huffman codes
    unordered_map<unsigned char, string> huffmanCodes;
    for (size_t i = 0; i < size; ++i){
        unsigned char byte;
        inputFile.read(reinterpret_cast<char*>(&byte), sizeof(byte)); //Read the byte

        size_t length;
        inputFile.read(reinterpret_cast<char*>(&length), sizeof(length)); //Read the length of the huffman code

        // Read the Huffman code bit by bit
        string value;
        int bitCount = 0;
        unsigned char c;
        for (size_t j = 0; j < length; ++j) {
            if (bitCount == 0) {
                inputFile.read(reinterpret_cast<char*>(&c), sizeof(c));
                bitCount = 8;
            }
            value += ((c >> (bitCount - 1)) & 1) ? '1' : '0';
            --bitCount;
        }

        huffmanCodes[byte] = value;
    }

    return huffmanCodes;
}


vector<unsigned char> readCompressedData(ifstream& inputFile) {
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

void compress(string path, string outputFilename, int cv);
void LZ77compress(string path, string outputFilename){
    LZ77 lz;
    int window_size = 4096;

    //Memory Mapping
    std::error_code error;
    //const auto path = "bee-movie.txt";
    mio::mmap_source mmap(path, 0, mio::map_entire_file);
    mmap.map(path, error);
    if (error) {
        std::cout << "Error mapping file: " << error.message() << std::endl;
        return;
    }

    // Get the data from the memory mapped file
    vector<unsigned char> data(mmap.data(), mmap.end());
    // Compress the file
    vector<unsigned char> compressed = lz.compress(data, window_size);
    cout << "LZ77 Compression Complete" << endl;
    lz.saveFile("output.bin", compressed);
};

void huffmanCompress(string path, string outputFilename){
    ofstream outputFile(outputFilename, ios::binary);
    Huffman huff;

    //Memory Mapping
    std::error_code error;
    //const auto path = "bee-movie.txt";
    mio::mmap_source mmap(path, 0, mio::map_entire_file);
    mmap.map(path, error);
    if (error) {
        std::cout << "Error mapping file: " << error.message() << std::endl;
        return;
    }
    // Get the data from the memory mapped file
    vector<unsigned char> data(mmap.data(), mmap.end());

    unordered_map<unsigned char, string> huffmanCodes = huff.generateHuffmanCodes(data);
    cout << "Generated Huffman Codes" << endl;
    vector<unsigned char> huffCompressed = huff.encode(data, huffmanCodes);
    cout << "Huffman Encoded" << endl;
    writeCompressedData(outputFile, huffmanCodes, huffCompressed);
    cout << "Compressed File Saved" << endl;

}

void decompress(string path, string outputFilename);
void LZ77decompress(string path, string outputFilename){
    LZ77 lz;
    int window_size = 4096;
    ifstream inputFile(path, ios::binary);
    vector<unsigned char> data = lz.loadFile(path);
    // Decompress the LZ77 encoding
    vector<LZ77Token> tokens = lz.byteStreamToTokens(data);
    cout << "Converted Bytestream back to tokens" << endl;

    vector<unsigned char> decompressed = lz.decompressToBytes(tokens);
    cout << "Decompressed LZ77" << endl;
    lz.saveFile(outputFilename, decompressed);
    cout << "Saved Output" << endl;
};
void huffmanDecompress(string path, string outputFilename){
    Huffman huff;
    LZ77 lz;
    ifstream inputFile(path, ios::binary);
    // Get the data from the memory mapped file
    unordered_map<unsigned char, string> huffmanCodes = readHuffmanCodes(inputFile);
    vector<unsigned char> huffCompressed = readCompressedData(inputFile);

    cout << "Beginning Decompression..." << endl;
    // Decompress the huffman encoding
    vector<unsigned char> huffDecompressed = huff.decode(huffCompressed, huffmanCodes);
    cout << "Huffman decoded" << endl;
    lz.saveFile(outputFilename, huffDecompressed);
};