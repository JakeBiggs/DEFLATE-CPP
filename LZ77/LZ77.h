#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <map>

using namespace std;

struct TrieNode {
    map<char, TrieNode*> children;
    vector<int> indices;
};


struct LZ77Token {
    uint16_t offset;
    uint16_t length;
    char next;

    LZ77Token(uint16_t offset, uint16_t length, char next) : offset(offset), length(length), next(next) {}
};

class LZ77 {
public:
    vector<char> loadFile(const string& filename);
    void saveFile(const string& filename, const vector<char>& byteStream); 
    vector<char> compress(const vector<char>& input, int window_size);
    vector<char> decompressToBytes(const vector<LZ77Token>& compressed);
    void decompressToFile(const vector<char>& compressedData, const string& filename);
    vector<char> tokensToByteStream(const vector<LZ77Token>& tokens);
    vector<LZ77Token> byteStreamToTokens(const vector<char>& byteStream);
};