#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <map>
#include <deque>
using namespace std;
/*
struct TrieNode {
    TrieNode* parent;
    map<unsigned char, TrieNode*> children;
    deque<int> indices;
    int numChildren = 0;

    TrieNode(TrieNode* parent = nullptr) : parent(parent) {}
};
*/
struct TrieNode {
    map<char, TrieNode*> children;
    int start = -1;
    int end = -1;
};

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
    vector<unsigned char> decompressToBytes(const vector<LZ77Token>& compressed);
    void decompressToFile(const vector<unsigned char>& compressedData, const string& filename);
    vector<unsigned char> tokensToByteStream(const vector<LZ77Token>& tokens);
    vector<LZ77Token> byteStreamToTokens(const vector<unsigned char>& byteStream);
};
