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

vector<unsigned char> LZ77::working_compress(const vector<unsigned char> &input, int window_size) {
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
    ofstream tokenfile("tokens_working.txt");
    for (const auto& token: output){
        tokenfile << "Token: (Offset: "<< token.offset << ", Length: "<< token.length<<", Next: "<< token.next<<")\n";
    }
    tokenfile.close();
    //Create a vector to hold a bytestream (needed for huffman)
    vector<unsigned char> byteStream = tokensToByteStream(output);

    return byteStream;
}
vector<unsigned char> LZ77::deque_compress(const vector<unsigned char>& input, int window_size) {
    deque<LZ77Token> output;
    unordered_map<int, deque<int>> window;

    int input_ptr = 0;

    while (input_ptr < input.size()) {
        uint16_t match_distance = 0;
        uint16_t match_length = 0;
        int best_match_index = -1;

        // Search for a match in the sliding window
        if (window.find(input[input_ptr]) != window.end()) {
            for (auto it = window[input[input_ptr]].rbegin(); it != window[input[input_ptr]].rend(); ++it) {
                int k = 0;
                while (input_ptr + k < input.size() && input[*it + k] == input[input_ptr + k] && k < window_size) {
                    k++;
                }

                if (k > match_length) {
                    match_length = k;
                    best_match_index = *it;
                }
            }

            if (best_match_index != -1) {
                match_distance = input_ptr - best_match_index;
            }
        }

        // Get the next character after the match
        unsigned char next = (input_ptr + match_length < input.size()) ? input[input_ptr + match_length] : '\0';

        // Add the LZ77 token to the output
        output.push_back({ match_distance, match_length, next });

        // Move the window
        for (int i = 0; i < match_length + 1; i++) {
            if (window[input[input_ptr + i]].size() == window_size) {
                window[input[input_ptr + i]].pop_front();
            }
            window[input[input_ptr + i]].push_back(input_ptr + i);
        }

        input_ptr += match_length + 1;
    }

    // Convert the output to a byte stream
    vector<unsigned char> byteStream = tokensToByteStream(output);

    return byteStream;
}

vector<unsigned char> LZ77::compress(const vector<unsigned char> &input, int window_size) {
    vector<LZ77Token> output;
    ifstream file("bee-movie.txt", ios::binary);

    vector<unsigned char> text((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());

    // Construct lexicographically sorted suffix array
    sdsl::csa_wt<> csa;
    sdsl::construct_im(csa, text, 0);

    for (int i = 0; i < input.size(); ) {
        // Start with a pattern of one character
        vector<unsigned char> pattern(input.begin() + i, input.begin() + i + 1);
        int match = -1;

        // Extend the pattern until no match is found
        while (i + pattern.size() <= input.size()) {
            int new_match = sa_binary_search(csa, pattern);
            if (new_match == -1) {
                break;
            }
            match = new_match;
            pattern.push_back(input[i + pattern.size()]);
        }

        // If no match is found for the single character pattern
        if (match == -1) {
            output.push_back(LZ77Token(0, 0, input[i]));
            ++i;
        } else {
            // Output token (position, length, S[j])
            output.push_back(LZ77Token(match, pattern.size() - 1, input[i + pattern.size() - 1]));

            // Move to the next position after the match
            i += pattern.size();
        }
    }

    ofstream tokenfile("tokens_SA.txt");
    for (const auto& token: output){
        tokenfile << "Token: (Offset: "<< token.offset << ", Length: "<< token.length<<", Next: "<< token.next<<")\n";
    }
    tokenfile.close();
    return tokensToByteStream(output);
}





/*
vector<unsigned char> LZ77::compress(const vector<unsigned char>& input, int window_size) {
    vector<LZ77Token> output;
    int n = input.size();

    // Adjust window_size if input is smaller
    if (n < window_size) {
        window_size = n;
    }

    // Construct the compressed suffix array for the entire input
    sdsl::int_vector<8> text(n);
    for (int i = 0; i < n; ++i) {
        text[i] = input[i];
    }
    sdsl::csa_wt<> csa;
    sdsl::construct_im(csa, text, 0);

// Construct the LCP array for the entire input
    sdsl::lcp_wt<> lcp;
    sdsl::construct_im(lcp, csa);

    int i = 0; // i represents the current position in the input data
    while (i < n) {
        uint16_t match_distance = 0;
        uint16_t match_length = 0;

        // Find the longest match in the window
        for (int j = max(0, i - window_size); j < i; ++j) {
            int l = 0;
            while (i + l < n && input[j + l] == input[i + l]) {
                ++l;
            }
            if (l > match_length) {
                match_length = l;
                match_distance = i - j;
            }
        }

        // Get the next character after the match
        unsigned char next = (i + match_length < n) ? input[i + match_length] : '\0';

        // Add the LZ77 token to the output
        output.push_back({match_distance, match_length, next});

        // Move the window
        i += match_length + 1;
    }

    return tokensToByteStream(output);
}
 */





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

