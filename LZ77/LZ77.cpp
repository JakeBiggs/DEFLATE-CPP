#include "LZ77.h"
#include <functional>

vector<unsigned char> LZ77::loadFile(const string& filename) {
    // Open the file
    ifstream file(filename, ios::binary);
    if (!file.good()) {
        cout << "Error Opening File... FILE NOT GOOD?" << endl;
        //runtime_error("Error Opening File...");
        return {};
    }

    // Read the entire file into a vector of chars
    vector<unsigned char> input((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return input;
}

void LZ77::saveFile(const string& filename, const vector<unsigned char>& byteStream) {
    ofstream outfile(filename, ios::binary);
    if (!outfile) {
        throw std::runtime_error("Could not open file for writing");
    }
    outfile.write(reinterpret_cast<const char*>(&byteStream[0]), byteStream.size());
    outfile.close();
}
vector<unsigned char> LZ77::working_compress(const vector<unsigned char>& input, int window_size) {
    vector<LZ77Token> output;
    int i = 0; //i represents the current position in the input data
    // j represents the start of the match in the sliding window
    // k represents the length of the match
    //The sliding window is being defined implicitly from position i-window_size to i

    // Loop over the input data
    //int input_size = input.size();
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
        unsigned char next = (i + match_length < input.size()) ? input[i + match_length] : '\0';

        // Add the LZ77 token to the output
        output.push_back({ match_distance, match_length, next });

        // Move the window
        i += match_length + 1;
    }

    //Create a vector to hold a bytestream (needed for huffman)
    vector<unsigned char> byteStream = tokensToByteStream(output);

    return byteStream;
}




vector<unsigned char> LZ77::compress(const vector<unsigned char>& input, int window_size) {
    TrieNode* root = new TrieNode();
    vector<LZ77Token> output;

    for (int i = 0; i < input.size(); i++) {
        TrieNode* node = root;
        int j = i;

        while (j < input.size() && node->children.count(input[j])) {
            node = node->children[input[j]];
            j++;
        }

        if (node != root) {
            uint16_t match_distance = i - node->start;
            uint16_t match_length = node->end - node->start;
            unsigned char next = (i + match_length < input.size()) ? input[i + match_length] : '\0';
            output.push_back({ match_distance, match_length, next });
            i += match_length;
        } else {
            output.push_back({ 0, 0, input[i] });
        }

        node = root;
        for (int k = i; k < j; k++) {
            if (!node->children.count(input[k])) {
                node->children[input[k]] = new TrieNode();
            }
            node = node->children[input[k]];
            node->start = min(node->start, k);
            node->end = max(node->end, k);
        }
    }

    return tokensToByteStream(output);
}

vector<unsigned char> LZ77::decompressToBytes(const vector<LZ77Token>& compressed) {
    vector<unsigned char> output;
    for (const LZ77Token& token : compressed) {
        if (token.length > 0) {
            // Copy the match from the specified distance back in the output
            int start = output.size() - token.offset;
            if (start < 0 || token.length > output.size()) {
                throw std::runtime_error("Invalid LZ77 token");
            }
            for (int i = 0; i < token.length; ++i) {
                output.push_back(output[start + i]);
            }
        }
        // Append the next character
        output.push_back(token.next);
    }
    return output;
}

void LZ77::decompressToFile(const vector<unsigned char>& compressedData, const string& filename) {
    vector<LZ77Token> tokens = byteStreamToTokens(compressedData);

    vector<unsigned char> output = decompressToBytes(tokens);
    saveFile(filename, output);

}


vector<unsigned char> LZ77::tokensToByteStream(const vector<LZ77Token>& tokens) {
    //Create a vector to hold a bytestream (needed for huffman)
    vector<unsigned char> byteStream;

    // Convert the LZ77 tokens to a byte stream
    for (const LZ77Token& token : tokens) {
        if (token.offset > UINT16_MAX || token.length > UINT16_MAX) {
            throw std::runtime_error("Offset or length too large for 16 bits");
        }
        uint16_t offset = token.offset;
        uint16_t length = token.length;
        unsigned char next = token.next;

        // Convert the offset to bytes and add them to the byte stream
        byteStream.push_back(static_cast<unsigned char>(offset & 0xFF)); //Bitwise AND to keep 8 least significant digits
        byteStream.push_back(static_cast<unsigned char>((offset >> 8) & 0xFF));  //Bitwise shift to get the next 8 digits

        // Convert the length to bytes and add them to the byte stream
        byteStream.push_back(static_cast<unsigned char>(length & 0xFF)); //Bitwise AND to keep 8 least significant digits
        byteStream.push_back(static_cast<unsigned char>((length >> 8) & 0xFF)); //Bitwise shift to get the next 8 digits

        // Add the next character to the byte stream
        byteStream.push_back(next);
    }
    return byteStream;
}



vector<LZ77Token> LZ77::byteStreamToTokens(const vector<unsigned char>& byteStream) {
    vector<LZ77Token> tokens;

    // Check if the size of the input vector is a multiple of 5
    if (byteStream.size() % 5 != 0) {
        //throw std::runtime_error("Invalid byte stream size");
        cout <<"Stream size invalid, trying anyway..." <<endl;
    }

    // Convert the byte stream to LZ77 tokens
    for (size_t i = 0; i < byteStream.size(); i += 5) {
        // Convert the first 2 bytes to a 16-bit offset using bitwise OR
        uint16_t offset = static_cast<uint16_t>(static_cast<unsigned char>(byteStream[i])) |
                          (static_cast<uint16_t>(static_cast<unsigned char>(byteStream[i + 1])) << 8);

        // Convert bytes 3 and 4 to a 16-bit length using bitwise OR
        uint16_t length = static_cast<uint16_t>(static_cast<unsigned char>(byteStream[i + 2])) |
                          (static_cast<uint16_t>(static_cast<unsigned char>(byteStream[i + 3])) << 8);

        // Get next unsigned char
        unsigned char next = byteStream[i + 4];

        tokens.push_back(LZ77Token(offset, length, next));
    }

    return tokens;
}

