#include "LZ77.h"


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

vector<unsigned char> LZ77::compress(const vector<unsigned char>& input, int window_size) {
    vector<LZ77Token> output; // Vector to hold the LZ77 tokens
    deque<TrieNode*> window; // Sliding window of TrieNodes as double ended queue
    TrieNode* root = new TrieNode(); // Root of the trie
    int i = 0; // Index of the current byte in the input

    // Iterate over the input bytes
    // Time complexity of O(n) where n is the number of bytes in the input
    while (i < input.size()) {
        TrieNode* node = root; // Start at the root of the trie
        int match_distance = 0, match_length = 0, j = i;

        // Search for the longest match in the sliding window (time complexity of O(1) and determined by window_size)
        //j is the index of the byte in the input
        // i+window_size is the maximum length of the match
        // input.size() is the maximum length of the input
        for (; j < i + window_size && j < input.size(); ++j) {
            if (node->children[input[j]] == nullptr) break; //If the byte is not in the trie, break so it can be added as a literal
            //If the byte is in the trie, move to the next node
            node = node->children[input[j]];
            if (!node->indices.empty() && j - node->indices[0] <= window_size) { //If the node has indices to use and the distance is within the window size
                match_distance = i - node->indices[0]; //Calculate the distance back to the match
                match_length = j - i + 1;  //Calculate the length of the match
            }
        }

        // Add the LZ77 token to the output
        if (match_length > 0 && i + match_length <= input.size()) {
            output.push_back(LZ77Token(match_distance, match_length, input[i + match_length - 1]));
            i += match_length;  // Increment i by match_length if a match was found to skip the matched bytes
        } else if (i < input.size()) { // If no match was found and there are still bytes left in the input
            output.push_back(LZ77Token(0, 0, input[i]));  // create a token with the current byte as literal
            i++; // Increment i to move to the next byte in the input
        }

        // Update the window
        if (window.size() > window_size) {
            TrieNode* old_node = window.front(); // Get the node that fell out of the window
            window.pop_front();
            delete old_node;  // Delete the TrieNode that fell out of the window
        }
        TrieNode* new_node = new TrieNode(); // Create a new TrieNode for the current byte
        window.push_back(new_node); // Add the new node to the window

        //Update the trie to match the new window
        node = root; // Reset the node to the root of the trie before traversing
        for (int k = i - match_length; k <= j && k < input.size(); ++k) { //k is the index of the byte in the input
            if (k >= 0) {
                if (node->children[input[k]] == nullptr) { //If current byte is not in the trie
                    node->children[input[k]] = new_node; //Add the new node to the trie
                }
                else{ //If current byte does have a corresponding node in the trie
                    node = node->children[input[k]]; //Move to the that node
                }
                node->indices.push_back(k); //Add the index of the byte to the node
            }
        }
    }

    // Cleanup: Delete remaining nodes in the window
    while (!window.empty()) {
        TrieNode* node = window.front();
        window.pop_front();
        delete node;
    }

    // Convert the output tokens to a byte stream
    vector<unsigned char> byteStream = tokensToByteStream(output);

    return byteStream;
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