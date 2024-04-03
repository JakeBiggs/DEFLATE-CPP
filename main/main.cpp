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
    LZ77 lz;
    Huffman huff;
    int window_size= 4096;
    cout << "starting..." << endl;
 vector<char> data = lz.loadFile("bee-movie-100.txt");
    for (auto _ : s){
        //Lz77 compression:
        cout << "LZ77 Compression starting..." << endl;
        vector<char> lzCompressed = lz.compress(data, window_size);
        cout << "LZ77 Compression Complete..." << endl;

        //Huffman compression:

        map<char, string> huffmanCodes = huff.generateHuffmanCodes(lzCompressed);
        cout << "Generated Huffman Codes" << endl;
        vector<char> huffCompressed = huff.encode(lzCompressed, huffmanCodes);
        cout << "Huffman Encoded" << endl;

        //Decompression:
        cout << "Beginning Decompression..." << endl;
        vector<char> huffDecompressed = huff.decode(huffCompressed, huffmanCodes);
        cout << "Huffman decoded" << endl;

        vector<LZ77Token> tokens = lz.byteStreamToTokens(huffDecompressed);
        cout << "Tokens decoded" << endl;

        vector<char> decompressed = lz.decompressToBytes(tokens);
        cout << "LZ77 decoded" << endl;

        lz.saveFile("output.txt", decompressed);
        cout << "Output Saved!" <<endl;
    }
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

