// COMP203.cpp : Defines the entry point for the application.
//

#include "main.h"
#include "LZ77/LZ77.h"
#include "Huffman/Huffman.h"

using namespace std;


static void BM_Deflate(benchmark::State &s){
    //vector<string> files = {"bee-movie.txt", "bee-movie-10.txt","bee-movie-20.txt","bee-movie-30.txt","bee-movie-40.txt","bee-movie-50.txt","bee-movie-60.txt","bee-movie-70.txt","bee-movie-80.txt","bee-movie-90.txt","bee-movie-100.txt","bee-movie-200.txt","bee-movie-300.txt","bee-movie-400.txt","bee-movie-500.txt"};
    vector<string> files = {"bee-movie-500.txt"};//,"bee-movie-300.txt","bee-movie-400.txt"};//,"bee-movie-500.txt"};
    ofstream csvFile("results.csv");
    csvFile << "File, Compression Time, Decompression Time" << endl;
    int i = 0;

    size_t chunk_size = 1024 * 1024 * 10; //10MB
    size_t num_threads = 5;

    for (auto _ : s){
        for (const auto& file : files) {
            auto start = chrono::high_resolution_clock::now();
            compress_file_in_chunks(file,chunk_size,num_threads);
            auto end = chrono::high_resolution_clock::now();
            chrono::duration<double> compressTime = end - start;


            start = chrono::high_resolution_clock::now();
            //decompress("output.bin", "output.txt");
            end = chrono::high_resolution_clock::now();
            chrono::duration<double> decompressTime = end - start;

            csvFile << file << "," << compressTime.count() << "," << decompressTime.count() << "\n";

        }
        //testWriteRead();
        //readUntilEndOfCodes();
    }
    csvFile.close();
}

void process_file_in_chunks(const string& filename, size_t chunk_size, size_t num_threads){

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

    // Compress the file
    vector<unsigned char> compressed = lz.compress(data, window_size);
    cout << "LZ77 Compression Complete" << endl;



    //map<char, int> freq = huff.countBytes(compressed);
    //cout << "Huff counted bytes" << endl;
    unordered_map<unsigned char, string> huffmanCodes = huff.generateHuffmanCodes(compressed);
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
    /*
    std::error_code error;
    mio::mmap_source mmap(path,0, mio::map_entire_file);
    mmap.map(path, error);
    if (error) {
        std::cout << "Error mapping file: " << error.message() << std::endl;
        return;
    }

    // Convert the memory-mapped data to a vector of unsigned chars
    // std::vector<unsigned char> data(mmap.begin(), mmap.end());
    */
    // Get the data from the memory mapped file
    unordered_map<unsigned char, string> huffmanCodes = readHuffmanCodes(inputFile);
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

void decompress_chunk(const vector<unsigned char>& chunk){
    LZ77 lz;
    Huffman huff;
    int window_size = 4096;
}



BENCHMARK(BM_Deflate);
BENCHMARK_MAIN();


