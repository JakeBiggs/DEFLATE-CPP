#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <map>
#include <deque>
#include <xxhash.h>
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
    vector<unsigned char> decompressToBytes(const vector<LZ77Token>& compressed);
    void decompressToFile(const vector<unsigned char>& compressedData, const string& filename);
    vector<unsigned char> tokensToByteStream(const vector<LZ77Token>& tokens);
    vector<LZ77Token> byteStreamToTokens(const vector<unsigned char>& byteStream);
};

class RollingHash {
    // BASE is a prime number close to the number of characters in the input alphabet (255 or 256 in ascii i think)
    static const unsigned long long BASE = 257;
    // MOD is a large prime number to reduce the chance of hash collisions
    static const unsigned long long MOD = 1000000007;
    // hashValue is the current hash value
    unsigned long long hashValue;
    // basePower is the current power of BASE, equal to BASE raised to the power of the length of the substring
    unsigned long long basePower;
    // length is the current length of the substring
    int length;

public:
    // Constructor initializes hashValue, basePower, and length to their initial values
    RollingHash() : hashValue(0), basePower(1), length(0) {}

    // Constructor that takes an initial size for the hash
    RollingHash(int initialSize) : hashValue(0), basePower(1), length(0) {
        for (int i = 0; i < initialSize; i++) {
            basePower = (basePower * BASE) % MOD;
        }
    }
    // append method adds a character to the end of the substring and updates the hash value accordingly
    void append(char c) {
        hashValue = (hashValue * BASE + c) % MOD;
        basePower = (basePower * BASE) % MOD;
        ++length;
    }

    // skip method removes a character from the beginning of the substring and updates the hash value accordingly
    void skip(char c) {
        if (length == 0) {
            throw std::runtime_error("Cannot skip a character from an empty hash");
        }
        hashValue = ((hashValue - c * basePower % MOD + MOD) * BASE) % MOD;
        basePower = (basePower / BASE) % MOD;
        --length;
    }

    // Reset method resets hashValue, basePower, and length to their initial values
    void clear() {
        hashValue = 0;
        basePower = 1;
        length = 0;
    }

    // hash method returns the current hash value
    unsigned long long hash() const {
        return hashValue;
    }

    // size method returns the current length of the substring
    int size() const {
        return length;
    }
};