#include "LZ77.h"


vector<char> LZ77::loadFile(const string& filename) {
    // Open the file
    ifstream file(filename, ios::binary);
    if (!file.good()) {
        cout << "Error Opening File... FILE NOT GOOD?" << endl;
        //runtime_error("Error Opening File...");
        return {};
    }

    // Read the entire file into a vector of chars
    vector<char> input((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    file.close();
    return input;
}

void LZ77::saveFile(const string& filename, const vector<char>& byteStream) {
    ofstream outfile(filename, ios::binary);
    if (!outfile) {
        throw std::runtime_error("Could not open file for writing");
    }
    outfile.write(&byteStream[0], byteStream.size());
    outfile.close();
}

vector<char> LZ77::compress(const vector<char>& input, int window_size) {
    vector<LZ77Token> output;
    deque<TrieNode*> window;
    TrieNode* root = new TrieNode();
    int i = 0;

    while (i < input.size()) {
        TrieNode* node = root;
        int match_distance = 0, match_length = 0, j = i;

        // Search for the longest match in the sliding window
        for (; j < i + window_size && j < input.size(); ++j) {
            if (node->children[input[j]] == nullptr) break;
            node = node->children[input[j]];
            if (!node->indices.empty() && j - node->indices[0] <= window_size) {
                match_distance = i - node->indices[0];
                match_length = j - i + 1;  // Include the last character that was matched
            }
        }

        // Add the LZ77 token to the output
        if (match_length > 0 && i + match_length <= input.size()) {
            output.push_back(LZ77Token(match_distance, match_length, input[i + match_length - 1]));
            i += match_length;  // Increment i by match_length if a match was found
        } else if (i < input.size()) {
            output.push_back(LZ77Token(0, 0, input[i]));  // If no match was found, create a token with the current character
            i++;
        }

        // Update the window
        if (window.size() > window_size) {
            TrieNode* old_node = window.front();
            window.pop_front();
            delete old_node;  // Delete the TrieNode that fell out of the window
        }
        TrieNode* new_node = new TrieNode();
        window.push_back(new_node);
        node = root;
        for (int k = i - match_length; k <= j && k < input.size(); ++k) {
            if (k >= 0) {
                if (node->children[input[k]] == nullptr) {
                    node->children[input[k]] = new_node;
                }
                else{
                    node = node->children[input[k]];
                }
                node->indices.push_back(k);
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
    vector<char> byteStream = tokensToByteStream(output);

    return byteStream;
}
vector<char> LZ77::decompressToBytes(const vector<LZ77Token>& compressed) {
    vector<char> output;
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

void LZ77::decompressToFile(const vector<char>& compressedData, const string& filename) {
    vector<LZ77Token> tokens = byteStreamToTokens(compressedData);

    vector<char> output = decompressToBytes(tokens);
    saveFile(filename, output);

}


vector<char> LZ77::tokensToByteStream(const vector<LZ77Token>& tokens) {
    //Create a vector to hold a bytestream (needed for huffman)
    vector<char> byteStream;

    // Convert the LZ77 tokens to a byte stream
    for (const LZ77Token& token : tokens) {
        if (token.offset > UINT16_MAX || token.length > UINT16_MAX) {
            throw std::runtime_error("Offset or length too large for 16 bits");
        }
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
    return byteStream;
}



vector<LZ77Token> LZ77::byteStreamToTokens(const vector<char>& byteStream) {
    vector<LZ77Token> tokens;

    // Check if the size of the input vector is a multiple of 5
    if (byteStream.size() % 5 != 0) {
        throw std::runtime_error("Invalid byte stream size");
    }

    // Convert the byte stream to LZ77 tokens
    for (size_t i = 0; i < byteStream.size(); i += 5) {
        // Convert the first 2 bytes to a 16-bit offset using bitwise OR
        uint16_t offset = static_cast<uint16_t>(static_cast<unsigned char>(byteStream[i])) |
                          (static_cast<uint16_t>(static_cast<unsigned char>(byteStream[i + 1])) << 8);

        // Convert bytes 3 and 4 to a 16-bit length using bitwise OR
        uint16_t length = static_cast<uint16_t>(static_cast<unsigned char>(byteStream[i + 2])) |
                          (static_cast<uint16_t>(static_cast<unsigned char>(byteStream[i + 3])) << 8);

        // Get next char
        char next = byteStream[i + 4];

        tokens.push_back(LZ77Token(offset, length, next));
    }

    return tokens;
}