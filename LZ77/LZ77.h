#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <map>
#include <deque>
#include<xxhash.h>
#include <divsufsort.h>
#include <divsufsort64.h>
#include <sdsl/suffix_arrays.hpp>
#include <sdsl/lcp.hpp>
using namespace std;


struct LZ77Token {
    uint16_t offset;
    uint16_t length;
    unsigned char next;

    LZ77Token(uint16_t offset, uint16_t length,unsigned char next) : offset(offset), length(length), next(next) {}
};

class LZ77 {
public:
    vector<unsigned char> loadFile(const string& filename);
    void saveFile(const string& filename, const vector<unsigned char>& byteStream);
    vector<unsigned char> compress(const vector<unsigned char>& input, int window_size);
    vector<unsigned char> working_compress(const vector<unsigned char>& input, int window_size);
    vector<unsigned char> deque_compress(const vector<unsigned char>& input, int window_size);
    vector<unsigned char> decompressToBytes(const vector<LZ77Token>& compressed);
    void decompressToFile(const vector<unsigned char>& compressedData, const string& filename);
    vector<unsigned char> tokensToByteStream(const vector<LZ77Token>& tokens);
    vector<LZ77Token> byteStreamToTokens(const vector<unsigned char>& byteStream);


    pair<int, int> sa_binary_search(const sdsl::csa_wt<>& sa, const vector<unsigned char>& pattern, int current_position_in_data) {
        int left = 0;
        int right = sa.size() - 1;
        int longest_match_length = 0;
        int longest_match_position = -1;

        while (left <= right) {
            int mid = left + (right - left) / 2;
            int mid_value = sa[mid];
            int cmp = 0;
            for (int i = 0; i < pattern.size() && mid_value + i < sa.size(); ++i) {
                if (pattern[i] != sa[mid_value + i]) {
                    cmp = pattern[i] - sa[mid_value + i];
                    break;
                }
            }

            if (cmp == 0) {
                // If a match is found, check if it's the longest match
                int match_length = 0;
                while (mid_value + match_length < sa.size() && pattern[match_length] == sa[mid_value + match_length]) {
                    ++match_length;
                }
                if (match_length > longest_match_length) {
                    longest_match_position = mid;
                    longest_match_length = match_length;
                }
                // Continue searching in the right half
                left = mid + 1;
            } else if (cmp < 0) {
                // If the pattern is less than the mid value, search in the left half
                right = mid - 1;
            } else {
                // If the pattern is greater than the mid value, search in the right half
                left = mid + 1;
            }
        }

        if (longest_match_position == -1) {
            return {0, 0};  // No match found
        } else {
            int offset = sa[longest_match_position] - current_position_in_data;
            return {offset, longest_match_length};  // Return the offset and length of the longest match
        }
    }

    vector<unsigned char> rabin_karp_compress(const vector<unsigned char> &input, int window_size);
};

