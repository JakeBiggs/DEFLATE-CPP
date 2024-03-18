// COMP203.cpp : Defines the entry point for the application.
//

#include "lz_old.h"

using namespace std;

#include <iostream>
#include <vector>
#include <fstream>

// Define a struct to hold the LZ77 tokens
// These act as references to substrings within the input data
struct LZ77Token {
    uint16_t offset;
    uint16_t length;
    char next;

    LZ77Token(uint16_t offset, uint16_t length, char next) : offset(offset), length(length), next(next) {}
};

// Function to compress the input data using the LZ77 algorithm
vector<LZ77Token> lz77_compress(const vector<char>& input, int window_size) {
    vector<LZ77Token> output;
    int i = 0; //i represents the current position in the input data
               // j represents the start of the match in the sliding window
               // k represents the length of the match
    //The sliding window is being defined implicitly from position i-window_size to i

    // Loop over the input data
    while (i < input.size()) {
        uint16_t match_distance = 0;
        uint16_t match_length = 0;

        // Search for a match in the  sliding window
        for (int j = i - window_size; j < i; j++) {
            if (j < 0) continue; // Skip the invalid index

            int k = 0; //k represents the length of the match
            // Extend the match as far as possible:
            // j+k = distance to end of the match
            // i+k = distance to end of the match
            // Checking against input.size() ensures we don't go out of bounds of the data
            while (j + k < input.size() && i + k < input.size() && input[j + k] == input[i + k] && k < window_size) {
                k++; // Increment the length of the match
            }
            // If this match is longer than the previous best match, update the best match
            if (k > match_length) {
                match_distance = i - j;
                match_length = k;
            }
        }

        // Get the next character after the match
        // If at the end of the input, use a null character
        //char next = input[i + match_length];
        char next = (i + match_length < input.size()) ? input[i + match_length] : '\0'; 

        // Add the LZ77 token to the output
        output.push_back({ match_distance, match_length, next });

        // Move the window
        i += match_length + 1;
    }

    /*
    // Save the compressed data to a file
    ofstream outfile("compressed.bin", ios::binary);
    for (const LZ77Token& token : output) {
        uint16_t offset = token.offset;
        uint16_t length = token.length;
        char next = token.next;

        outfile.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
        outfile.write(reinterpret_cast<const char*>(&length), sizeof(length));
        outfile.write(&next, sizeof(next));
    }
    outfile.close();
    */

    //Create a vector to hold a bytestream (needed for huffman)
    vector<char> byteStream;

    // Convert the LZ77 tokens to a byte stream
    for (const LZ77Token& token : output) {
        uint16_t offset = token.offset;
        uint16_t length = token.length;
        char next = token.next;

        // Convert the offset to bytes and add them to the byte stream
        byteStream.push_back(static_cast<char>(offset & 0xFF)); //Bitwise AND to keep 8 least significant digits
        byteStream.push_back(static_cast<char>((offset >> 8) & 0xFF));  //Bitwise shift to get the next 8 digits

        // Convert the length to bytes and add them to the byte stream
        byteStream.push_back(static_cast<char>(length & 0xFF)); //Bitwise AND to keep 8 least significant digits
        byteStream.push_back(static_cast<char>((length >> 8) & 0xFF)); //Bitwise shift to get the next 8 digits

        // Add the next character to the byte stream
        byteStream.push_back(next);
    }

    // Save the byte stream to a file
    ofstream outfile("compressed.bin", ios::binary);
    outfile.write(&byteStream[0], byteStream.size());
    outfile.close();


    return output;
}

vector<LZ77Token> byteStreamToTokens(const vector<char>& byteStream){
    vector<LZ77Token> tokens;

    /*
    for (size_t i = 0; i< byteStream.size(); i += 5){

        // Convert the first 2 bytes to a 16-bit offset using bitwise OR
        uint16_t offset = static_cast<uint16_t>(byteStream[i]) | (static_cast<uint16_t>(byteStream[i + 1]) << 8);

        // Convert bytes 3 and 4 to a 16-bit length using bitwise OR
        uint16_t length = static_cast<uint16_t>(byteStream[i + 2]) | (static_cast<uint16_t>(byteStream[i + 3]) << 8);
    
        //Get next char
        char next = byteStream[i + 4];

        tokens.push_back(LZ77Token(offset, length, next));
    }
    */

    for (size_t i = 0; i< byteStream.size(); i += 5){
        // Convert the first 2 bytes to a 16-bit offset using bitwise OR
        // Static casts to unsigned char before uint16_t to avoid fuckery (sign extension?)
        uint16_t offset = static_cast<uint16_t>(static_cast<unsigned char>(byteStream[i])) | 
                          (static_cast<uint16_t>(static_cast<unsigned char>(byteStream[i + 1])) << 8);

        // Convert bytes 3 and 4 to a 16-bit length using bitwise OR
        uint16_t length = static_cast<uint16_t>(static_cast<unsigned char>(byteStream[i + 2])) | 
                          (static_cast<uint16_t>(static_cast<unsigned char>(byteStream[i + 3])) << 8);
    
        //Get next char
        char next = byteStream[i + 4];

        tokens.push_back(LZ77Token(offset, length, next));
    }
    return tokens;
}

string lz77_decompress(const vector<LZ77Token>& compressed) {
    string output;
    for (const LZ77Token& token : compressed) {
        if (token.length > 0) {
            // Copy the match from the specified distance back in the output
            int start = output.length() - token.offset;
            for (int i = 0; i < token.length && start + i < output.size(); ++i) {
                
                output += output[start + i];
                
            }

            //string substring = output.substr(start, token.length);
            //output += substring;
        }
        // Append the next character
        output += token.next;
    }
    return output;
}

int main() {
    // Open the file in binary mode
    ifstream file("bee-moviex100.txt", ios::binary);
    if (!file.good()) {
        cout << "Error Opening File..." << strerror(errno) << endl;
        return 1;
    }
    // Read the entire file into a vector of chars
    vector<char> input((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();

    int window_size = 8192;//4096;

    // Compress the data
    vector<LZ77Token> compressed_data = lz77_compress(input, window_size);



    // Print the compressed data
    //for (const LZ77Token& token : compressed_data) {
      //  cout << "(" << token.offset << ", " << token.length << ", " << token.next << ")" << endl;
    //}

    // Read the compressed data from file
    /*
    vector<LZ77Token> compressed;
    ifstream infile("compressed.bin", ios::binary);
    while (infile.good()) {
        uint16_t offset;
        uint16_t length;
        char next;

        infile.read(reinterpret_cast<char*>(&offset), sizeof(offset));
        infile.read(reinterpret_cast<char*>(&length), sizeof(length));
        infile.read(&next, sizeof(next));

        if (infile.good()) {
            compressed.push_back({ offset, length, next });
        }
    }
    infile.close();
    */
    ifstream infile("compressed.bin", ios::binary);
    vector<char> byteStream((istreambuf_iterator<char>(infile)), istreambuf_iterator<char>());
    infile.close();

    //Convert the byte stream to tokens
    vector<LZ77Token> compressed = byteStreamToTokens(byteStream);

    //Decompress the data
    string decompressed = lz77_decompress(compressed);
    cout << "Decompressed Data: " << decompressed << endl;

    return 0;
}

