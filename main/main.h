#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <mio/mio.hpp>
#include <benchmark/benchmark.h>
#include <LZ77/LZ77.h>
#include <Huffman/Huffman.h>
#include <cstring>
#include <chrono>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
using namespace std;

void compress(string path, string outputFilename);
//void compress_chunk(const vector<unsigned char>&chunk);
void decompress(string path, string outputFilename);

void writeCompressedData(ofstream& outputFile, const vector<unsigned char>& compressedData){
    //Write the size of the compressed data
    size_t size = compressedData.size();
    outputFile.write(reinterpret_cast<const char*>(&size), sizeof(size));


    //Write the compressed data
    outputFile.write(reinterpret_cast<const char*>(&compressedData[0]), compressedData.size());
}

void writeHuffmanCodes(ofstream& outputFile, const unordered_map<unsigned char, string>& huffmanCodes){
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

void testWriteRead() {
    // Generate some test data
    unordered_map<unsigned char, string> huffmanCodes = {{'a', "0"}, {'b', "10"}, {'c', "110"}};
    vector<unsigned char> compressedData = {'a', 'b', 'c'};

    // Write the test data to a file
    ofstream outputFile("test.bin", ios::binary);
    writeCompressedData(outputFile, compressedData);
    outputFile.close();

    // Read the test data from the file
    ifstream inputFile("test.bin", ios::binary);
    unordered_map<unsigned char, string> huffmanCodesAfterRead = readHuffmanCodes(inputFile);
    vector<unsigned char> compressedDataAfterRead = readCompressedData(inputFile);
    inputFile.close();

    // Compare the read data with the original data
    if (huffmanCodes != huffmanCodesAfterRead){
        cout << "Huffman codes do not match" << endl;
    } else {
        cout << "Codes Test Passed" << endl;
    }
    if (compressedData != compressedDataAfterRead){
        cout << "Compressed data does not match" << endl;
    }else {
        cout << "Data Test Passed" << endl;
    }
    assert(huffmanCodes == huffmanCodesAfterRead);
    assert(compressedData == compressedDataAfterRead);
}


//Function to process chunks
tuple<int, unordered_map<unsigned char, int>, vector<unsigned char>> compress_chunk(const vector<unsigned char>&chunk, int chunk_index){
    cout<<"Processing chunk of size " << chunk.size() << endl;
    LZ77 lz;
    int window_size = 4096;
    Huffman huff;
// Compress the file
    vector<unsigned char> compressed = lz.compress(chunk, window_size);
    //unordered_map<unsigned char, string> huffmanCodes = huff.generateHuffmanCodes(compressed);
    unordered_map<unsigned char, int> frequencyTable = huff.countBytes(compressed);

    //vector<unsigned char> huffCompressed = huff.encode(compressed, huffmanCodes);
    //writeCompressedData(outputFile, huffmanCodes, huffCompressed);

    this_thread::sleep_for(chrono::seconds(1));

    return make_tuple(chunk_index, frequencyTable, compressed);
}

void compress_file_in_chunks(const string& filename, size_t chunk_size, size_t num_threads) {
    // Open the file for reading
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Failed to open file." << endl;
        return;
    }
    // Get the size of the file
    file.seekg(0, ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, ios::beg);

    // Ensure chunk_size does not exceed file_size
    chunk_size = min(chunk_size, file_size);

    //Open the output file
    ofstream outputFile("output.bin", ios::binary);

    // Mutex for synchronizing access to file and priority queue
    mutex file_mutex, queue_mutex;

    //Condition variable to notify threads when there is work to do
    condition_variable cv;

    // Boolean flag to indicate when the file has reached the end
    bool end_of_file = false;

    // Priority queue to store chunks
    auto compare = [](const tuple<int, unordered_map<unsigned char, int>, vector<unsigned char>>& a, const tuple<int, unordered_map<unsigned char, int>, vector<unsigned char>>& b) {
        return get<0>(a) > get<0>(b);
    };
    priority_queue<tuple<int, unordered_map<unsigned char, int>, vector<unsigned char>>, vector<tuple<int, unordered_map<unsigned char, int>, vector<unsigned char>>>, decltype(compare)> chunks(compare);

    // Vector to store thread objects
    vector<thread> threads;

    // Create threads to process chunks
    for (size_t i = 0; i < num_threads; ++i) {
        threads.emplace_back([&](size_t thread_id) {
            vector<unsigned char> chunk(chunk_size);
            int chunk_index = 0;
            while (true) {
                // Read chunk from file
                {
                    unique_lock<mutex> lock(file_mutex);
                    cv.wait(lock, [&] { return end_of_file || !file.eof(); });
                    if (!file.read(reinterpret_cast<char *>(chunk.data()), chunk_size)) {
                        end_of_file = true;
                        cv.notify_all();
                        if(file.gcount() == 0){
                            break; // End of file
                        }
                    }
                    cv.notify_one();
                }
                // Resize chunk to actual number of bytes read
                chunk.resize(file.gcount());
                // Compress the chunk
                tuple<int, unordered_map<unsigned char, int>, vector<unsigned char>> compressed_chunk = compress_chunk(chunk, chunk_index);
                // Add compressed chunk to priority queue
                {
                    lock_guard<mutex> lock(queue_mutex);
                    chunks.push(compressed_chunk);
                }
                chunk_index++;
            }
        }, i);
    }

    // Wait for all process_chunk threads to finish
    for (size_t i = 0; i < num_threads; ++i) {
        threads[i].join();
    }

    // Merge frequency tables and store chunks for later use
    unordered_map<unsigned char, int> mergedFrequencyTable;
    vector<tuple<int, unordered_map<unsigned char, int>, vector<unsigned char>>> storedChunks;
    while (!chunks.empty()) {
        auto chunk = chunks.top();
        chunks.pop();
        storedChunks.push_back(chunk);
        auto frequencyTable = get<1>(chunk);
        for (const auto& pair : frequencyTable) {
            mergedFrequencyTable[pair.first] += pair.second;
        }
    }
    // Sort storedChunks by chunk index
    sort(storedChunks.begin(), storedChunks.end(), [](const auto& a, const auto& b) {
        return get<0>(a) < get<0>(b);
    });

    // Build Huffman tree using merged frequency table
    Huffman huff;
    unordered_map<unsigned char, string> huffmanCodes = huff.generateHuffmanCodesFromFreq(mergedFrequencyTable);
    writeHuffmanCodes(outputFile, huffmanCodes);

    // Encode chunks using Huffman codes and write to output file
    for (const auto& chunk : storedChunks) {
        vector<unsigned char> huffCompressed = huff.encode(get<2>(chunk), huffmanCodes);
        outputFile.seekp(0, ios::end);
        writeCompressedData(outputFile, huffCompressed);
    }

    // Close files
    file.close();
    outputFile.close();
}

/*
//Function to split the data into chunks for each thread
vector<vector<unsigned char>> makeChunks(const vector<unsigned char>data, size_t chunk_size){
    vector<vector<unsigned char>> chunks;

    //Calculate number of chunks
    size_t num_chunks = (data.size() + chunk_size -1) /chunk_size; //Addition of chunk_size-1 to ensure enough for remaining data

    //Extract data for chunks
    for(size_t i =0; i<num_chunks; ++i){
        //Calculate the start and end index of current chunk
        size_t start = i * chunk_size;
        size_t end = min(start + chunk_size, data.size()); //data.size() if last chunk

        vector<unsigned char> chunk(data.begin() + start, data.begin() + end); //Gets data from chunk indices
        chunks.push_back(chunk);
    }
    return chunks;
}

*/