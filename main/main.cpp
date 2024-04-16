// COMP203.cpp : Defines the entry point for the application.
//

#include "main.h"
#include "LZ77/LZ77.h"
#include "Huffman/Huffman.h"

using namespace std;



static void BM_Deflate(benchmark::State &s){

    for (auto _ : s){
        compress();
        decompress();
        //testWriteRead();
        //readUntilEndOfCodes();
    }
}

void testWriteRead() {
    // Generate some test data
    map<unsigned char, string> huffmanCodes = {{'a', "0"}, {'b', "10"}, {'c', "110"}};
    vector<unsigned char> compressedData = {'a', 'b', 'c'};

    // Write the test data to a file
    ofstream outputFile("test.bin", ios::binary);
    writeCompressedData(outputFile, huffmanCodes, compressedData);
    outputFile.close();

    // Read the test data from the file
    ifstream inputFile("test.bin", ios::binary);
    map<unsigned char, string> huffmanCodesAfterRead = readHuffmanCodes(inputFile);
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



void compress(){
    /*
    LZ77 lz;
    Huffman huff;
    int window_size = 4096;

    ifstream inputFile("bee-movie.txt", ios::binary);
    ofstream outputFile("output.bin", ios::binary);

    if (!inputFile) {
        cout << "Failed to open input file" << endl;
        return;
    }

    vector<unsigned char> buffer(window_size); // Buffer size is equal to the window size
    vector<unsigned char> lzCompressed;
    while (inputFile) {
        // Read a chunk of data from the file
        streampos oldPos = inputFile.tellg();
        inputFile.read(reinterpret_cast<char*>(&buffer[0]), buffer.size());
        if (!inputFile) {
            break;
        }
        streamsize bytesRead = inputFile.gcount();

        // LZ77 compression
        vector<unsigned char> chunkCompressed = lz.compress(buffer, window_size);
        lzCompressed.insert(lzCompressed.end(), chunkCompressed.begin(), chunkCompressed.end());

        // Slide the buffer window over the file
        inputFile.seekg(oldPos + bytesRead, ios::beg);

    }

    // Huffman compression
    map<unsigned char, string> huffmanCodes = huff.generateHuffmanCodes(lzCompressed);
    vector<unsigned char> huffCompressed = huff.encode(lzCompressed, huffmanCodes);
    
    
    // Write the Huffman codes to the output file



    // Write the compressed data to the output file
    //outputFile.write(&huffCompressed[0], huffCompressed.size());
    outputFile.write(reinterpret_cast<const char*>(&huffCompressed[0]), huffCompressed.size());
    

    writeCompressedData(outputFile, huffmanCodes, huffCompressed);
    inputFile.close();
    outputFile.close();

    cout << "Compression Complete" << endl;
    */
    // LEMPEL ZIV 77
    LZ77 lz;
    int window_size = 4096;
    ofstream outputFile("output.txt", ios::binary);

    // Compress the file
    vector<unsigned char> compressed = lz.compress(lz.loadFile("bee-movie.txt"), window_size);
    cout << "LZ77 Compression Complete" << endl;

    // HUFFMAN
    Huffman huff;

    //map<char, int> freq = huff.countBytes(compressed);
    //cout << "Huff counted bytes" << endl;
    map<unsigned char, string> huffmanCodes = huff.generateHuffmanCodes(compressed);
    cout << "Generated Huffman Codes" << endl;
    vector<unsigned char> huffCompressed = huff.encode(compressed, huffmanCodes);
    cout << "Huffman Encoded" << endl;
    writeCompressedData(outputFile, huffmanCodes, huffCompressed);
    cout << "Compressed File Saved" << endl;
}

void decompress(){
    /*
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
    map<unsigned char, string> huffmanCodes = readHuffmanCodes(inputFile);

    // Read the compressed data from the input file
    vector<unsigned char> huffCompressed = readCompressedData(inputFile);

    // Huffman decompression
    //map<char, string> huffmanCodes = huff.generateHuffmanCodes(huffCompressed);
    vector<unsigned char> lzCompressed = huff.decode(huffCompressed, huffmanCodes);

    // LZ77 decompression
    for (size_t i = 0; i < lzCompressed.size(); i += window_size) {
        vector<unsigned char> chunk(lzCompressed.begin() + i, lzCompressed.begin() + min(i + window_size, lzCompressed.size()));
        vector<LZ77Token> tokens = lz.byteStreamToTokens(chunk);
        vector<unsigned char> decompressed = lz.decompressToBytes(tokens);

        // Write the decompressed data to the output file
        //outputFile.write(&decompressed[0], decompressed.size());
        outputFile.write(reinterpret_cast<const char*>(&decompressed[0]), decompressed.size());
    }

    inputFile.close();
    outputFile.close();

    cout << "Decompression Complete" << endl;
    */
    LZ77 lz;
    Huffman huff;
    int window_size = 4096;
    ifstream inputFile("output.bin", ios::binary);
    //ofstream outputFile("output.txt", ios::binary);

    map<unsigned char, string> huffmanCodes = readHuffmanCodes(inputFile);
    vector<unsigned char> huffCompressed = readCompressedData(inputFile);

    cout << "Beginning Decompression..." << endl;
    // Decompress the huffman encoding
    vector<unsigned char> huffDecompressed = huff.decode(huffCompressed, huffmanCodes);
    cout << "Huffman decoded" << endl;

    // Decompress the LZ77 encoding
    vector<LZ77Token> tokens = lz.byteStreamToTokens(huffDecompressed);
    cout << "Converted Bytestream back to tokens" << endl;

    vector<unsigned char> decompressed = lz.decompressToBytes(tokens);
    cout << "Decompressed LZ77" << endl;
    lz.saveFile("output.txt", decompressed);



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

