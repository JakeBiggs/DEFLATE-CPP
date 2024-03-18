// COMP203.cpp : Defines the entry point for the application.
//

#include "main.h"
#include "LZ77/LZ77.h"
#include "Huffman/Huffman.h"

using namespace std;

#include <iostream>
#include <vector>
#include <fstream>

int main() {

	// LEMPEL ZIV 77
	LZ77 lz;
	int window_size = 4096;

	// Compress the file
	vector<char> compressed = lz.compress(lz.loadFile("bee-movie.txt"), window_size);
	cout << "LZ77 Compression Complete" << endl;
	/*lz.saveFile("output.bin", compressed);

	// Decompress the file
	vector<LZ77Token> tokens = lz.byteStreamToTokens(lz.loadFile("output.bin"));
	vector<char> decompressed = lz.decompressToBytes(tokens);
	lz.saveFile("output.txt", decompressed);
	*/

	// HUFFMAN
	Huffman huff;
	//vector<char> huffTest = { 'a', 'b', 'c', 'a', 'b', 'c' , 'b', 'b', 'a', 'd'};
    //map<char, int> freq = huff.countBytes(huffTest);
    //vector<Node*> nodes;


	map<char, int> freq = huff.countBytes(compressed);
	cout << "Huff counted bytes" << endl;
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

