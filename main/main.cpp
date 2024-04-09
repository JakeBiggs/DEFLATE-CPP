// COMP203.cpp : Defines the entry point for the application.
//

#include "main.h"
#include "LZ77/LZ77.h"
#include "Huffman/Huffman.h"

using namespace std;

#include <iostream>
#include <vector>
#include <fstream>
#include <benchmark/benchmark.h>


static void BM_Deflate(benchmark::State &s){

    for (auto _ : s){
        compress();
        decompress();
    }
}

void compress(){
    LZ77 lz;
    Huffman huff;
    int window_size = 4096;

    ifstream inputFile("bee-movie.txt", ios::binary);
    ofstream outputFile("output.bin", ios::binary);

    if (!inputFile) {
        cout << "Failed to open input file" << endl;
        return;
    }

    vector<char> buffer(window_size); // Buffer size is equal to the window size
    vector<char> lzCompressed;
    while (inputFile) {
        inputFile.read(&buffer[0], buffer.size());
        if (!inputFile) {
            break;
        }

        // LZ77 compression
        vector<char> chunkCompressed = lz.compress(buffer, window_size);
        lzCompressed.insert(lzCompressed.end(), chunkCompressed.begin(), chunkCompressed.end());

        // Slide the buffer window over the file
        inputFile.seekg(window_size, ios::cur);
    }

    // Huffman compression
    map<char, string> huffmanCodes = huff.generateHuffmanCodes(lzCompressed);
    vector<char> huffCompressed = huff.encode(lzCompressed, huffmanCodes);

    // Write the Huffman codes to the output file
    for (const auto &pair : huffmanCodes) {
        outputFile << static_cast<int>(pair.first) << ' ' << pair.second << '\n';
    }
    outputFile << "END_OF_CODES\n";


    // Write the compressed data to the output file
    outputFile.write(&huffCompressed[0], huffCompressed.size());

    inputFile.close();
    outputFile.close();

    cout << "Compression Complete" << endl;
}

void decompress(){
    LZ77 lz;
    Huffman huff;
    int window_size = 4096;

    ifstream inputFile("output.bin", ios::binary);
    ofstream outputFile("output.txt", ios::binary);

    if (!inputFile) {
        cout << "Failed to open input file" << endl;
        return;
    }

    // Read the Huffman codes from the input file
    map<char, string> huffmanCodes;
    string line;
    while (getline(inputFile, line)) {
        if (line == "END_OF_CODES") {
            break;
        }
        char key = static_cast<char>(stoi(line.substr(0, line.find(' '))));
        string value = line.substr(line.find(' ') + 1);
        huffmanCodes[key] = value;
    }


    // Read the entire compressed file
    vector<char> huffCompressed((istreambuf_iterator<char>(inputFile)), istreambuf_iterator<char>());

    // Huffman decompression
    //map<char, string> huffmanCodes = huff.generateHuffmanCodes(huffCompressed);
    vector<char> lzCompressed = huff.decode(huffCompressed, huffmanCodes);

    // LZ77 decompression
    for (size_t i = 0; i < lzCompressed.size(); i += window_size) {
        vector<char> chunk(lzCompressed.begin() + i, min(lzCompressed.begin() + i + window_size, lzCompressed.end()));
        vector<LZ77Token> tokens = lz.byteStreamToTokens(chunk);
        vector<char> decompressed = lz.decompressToBytes(tokens);

        // Write the decompressed data to the output file
        outputFile.write(&decompressed[0], decompressed.size());
    }

    inputFile.close();
    outputFile.close();

    cout << "Decompression Complete" << endl;
}

BENCHMARK(BM_Deflate);
BENCHMARK_MAIN();



/*
int main() {

	// LEMPEL ZIV 77
	LZ77 lz;
	int window_size = 4096;

	// Compress the file
	vector<char> compressed = lz.compress(lz.loadFile("bee-movie.txt"), window_size);
	cout << "LZ77 Compression Complete" << endl;

	// HUFFMAN
	Huffman huff;

	//map<char, int> freq = huff.countBytes(compressed);
	//cout << "Huff counted bytes" << endl;
	map<char, string> huffmanCodes = huff.generateHuffmanCodes(compressed);
	cout << "Generated Huffman Codes" << endl;
	vector<char> huffCompressed = huff.encode(compressed, huffmanCodes);
	cout << "Huffman Encoded" << endl;
	lz.saveFile("output.bin", huffCompressed);
	cout << "Compressed File Saved" << endl;

	cout << "Beginning Decompression..." << endl;
	// Decompress the huffman encoding 
	vector<char> huffDecompressed = huff.decode(huffCompressed, huffmanCodes);
	cout << "Huffman decoded" << endl;

	// Decompress the LZ77 encoding
	vector<LZ77Token> tokens = lz.byteStreamToTokens(huffDecompressed);
	cout << "Converted Bytestream back to tokens" << endl;

	vector<char> decompressed = lz.decompressToBytes(tokens);
	cout << "Decompressed LZ77" << endl;

	lz.saveFile("output.txt", decompressed);
	cout << "Saved Output" << endl;

}
*/

