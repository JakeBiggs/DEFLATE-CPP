// COMP203.cpp : Defines the entry point for the application.
//

#include "main.h"
#include "LZ77/LZ77.h"
#include "Huffman/Huffman.h"

using namespace std;


static void BM_Deflate(benchmark::State &s){
    //Define list of files to test
    //vector<string> files = {"bee-movie.txt", "bee-movie-10.txt","bee-movie-20.txt","bee-movie-30.txt","bee-movie-40.txt","bee-movie-50.txt","bee-movie-60.txt","bee-movie-70.txt","bee-movie-80.txt","bee-movie-90.txt","bee-movie-100.txt"};//,"bee-movie-500.txt"};
    vector<string> files = {"bee-movie-500.txt"};
    ofstream csvFile("results.csv");
    csvFile << "File, Compression Time, Decompression Time, Compression Ratio" << endl;
    int i = 0;
    for (auto _ : s){
        for (const auto& file : files) {
            auto start = chrono::high_resolution_clock::now();
            compress(file, "output.bin");
            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double> compressTime = end - start;

            start = chrono::high_resolution_clock::now();
            decompress("output.bin", "output.txt");
            end = chrono::high_resolution_clock::now();
            chrono::duration<double> decompressTime = end - start;

            csvFile << file << "," << compressTime.count() << "," << decompressTime.count() << "\n";

        }
        //testWriteRead();
        //readUntilEndOfCodes();
    }
    csvFile.close();
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



void compress(string path, string outputFilename){

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


    // Initialise
    LZ77 lz;
    int window_size = 4096;
    ofstream outputFile(outputFilename, ios::binary);
    Huffman huff;

    /*
    // Process the file in chunks
    const size_t chunk_size = 1024 * 1024; // 1MB
    for (size_t i = 0; i < mmap.size(); i += chunk_size) {
        // Get the current chunk from the memory mapped file
        size_t current_chunk_size = std::min(chunk_size, mmap.size() - i);
        std::vector<unsigned char> data(mmap.data() + i, mmap.data() + i + current_chunk_size);

        // Compress the chunk
        vector<unsigned char> compressed = lz.compress(data, window_size);
        cout << "LZ77 Compression Complete for chunk " << i / chunk_size << endl;

        // Compress the LZ77 output using Huffman coding
        map<unsigned char, string> huffmanCodes = huff.generateHuffmanCodes(compressed);
        vector<unsigned char> huffCompressed = huff.encode(compressed, huffmanCodes);
        cout << "Huffman Compression Complete for chunk " << i / chunk_size << endl;

        // Write the compressed data and huffman codes
        writeCompressedData(outputFile, huffmanCodes, huffCompressed);

    }


    outputFile.close();
    */

    // Compress the file
    vector<unsigned char> compressed = lz.compress(data, window_size);
    cout << "LZ77 Compression Complete" << endl;



    //map<char, int> freq = huff.countBytes(compressed);
    //cout << "Huff counted bytes" << endl;
    map<unsigned char, string> huffmanCodes = huff.generateHuffmanCodes(compressed);
    cout << "Generated Huffman Codes" << endl;
    vector<unsigned char> huffCompressed = huff.encode(compressed, huffmanCodes);
    cout << "Huffman Encoded" << endl;
    writeCompressedData(outputFile, huffmanCodes, huffCompressed);
    cout << "Compressed File Saved" << endl;

}

void decompress(string path, string outputFilename){

    LZ77 lz;
    Huffman huff;
    int window_size = 4096;
    ifstream inputFile(path, ios::binary);
    //ofstream outputFile("output.txt", ios::binary);

    // Memory Mapping
    std::error_code error;
    mio::mmap_source mmap(path,0, mio::map_entire_file);
    mmap.map(path, error);
    if (error) {
        std::cout << "Error mapping file: " << error.message() << std::endl;
        return;
    }
    // Convert the memory-mapped data to a vector of unsigned chars
    // std::vector<unsigned char> data(mmap.begin(), mmap.end());

    /*
    // Process the file in chunks
    const size_t chunk_size = 1024 * 1024; // 1MB
    while(inputFile){
        //Read the huffman codes for the chunk
        map<unsigned char, string> huffmanCodes = readHuffmanCodes(inputFile);

        //Read the compressed data for the chunk
        vector<unsigned char> huffCompressed;
        for (size_t i = 0; i <chunk_size; i++){
            unsigned char byte;
            inputFile.read(reinterpret_cast<char*>(&byte), sizeof(byte));
            huffCompressed.push_back(byte);
        }

        // Decompress the huffman encoding
        vector<unsigned char> huffDecompressed = huff.decode(huffCompressed, huffmanCodes);
        cout << "Huffman decoded" << endl;

        // Decompress the LZ77 encoding
        vector<LZ77Token> tokens = lz.byteStreamToTokens(huffDecompressed);
        cout << "Converted Bytestream back to tokens" << endl;
        vector<unsigned char> decompressed = lz.decompressToBytes(tokens);
        cout << "Decompressed LZ77" << endl;

        // Write the decompressed data to the output file
        for (unsigned char byte : decompressed) {
            outputFile.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
        }

    }
    */


    // Get the data from the memory mapped file
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
    lz.saveFile(outputFilename, decompressed);
    cout << "Saved Output" << endl;


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

