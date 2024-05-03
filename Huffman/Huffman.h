#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <deque>
#include <queue>

using namespace std;

struct Node {
    unsigned char data;
    int freq;
    Node *left, *right;
    Node(unsigned char data, int freq, Node* left = nullptr, Node* right = nullptr)
            : data(data), freq(freq), left(left), right(right) {}
};

struct TrieNode {
    TrieNode* children[2] = {nullptr, nullptr};
    unsigned char data;
    bool isEndOfCode = false;
};

struct Compare {
    bool operator()(Node* a, Node* b) {
        if (a->freq == b->freq) {
            return a->data > b->data;
        }
        return a->freq > b->freq;
    }
};

class Huffman {
public:
    unordered_map<unsigned char, int> countBytes(const vector<unsigned char>& input);
    priority_queue<Node *, vector<Node *>, Compare> createNodes(const unordered_map<unsigned char, int>& frequencies);

    void traverseHuffmanTree(Node* node, string& code, int length, unordered_map<unsigned char, string>& huffmanCodes);
    unordered_map<unsigned char, string> generateHuffmanCodes(const vector<unsigned char>& input);
    vector<unsigned char> encode(const vector<unsigned char>& input, const unordered_map<unsigned char, string>& huffmanCodes);
    vector<unsigned char> decode(const vector<unsigned char>& input, TrieNode* root);

    Node *buildTree(priority_queue<Node *, vector<Node *>, Compare> &nodes);
    TrieNode* buildTrie(const unordered_map<unsigned char, string>& huffmanCodes);
    void traverseHuffmanTree(Node *node, const string &code, unordered_map<unsigned char, string> &huffmanCodes);

    Node *buildTree(deque<Node *> &nodes);

    deque<Node *> deque_createNodes(const unordered_map<unsigned char, int> &frequencies);

    Node *deque_buildTree(deque<Node *> &nodes);

    void
    deque_traverseHuffmanTree(Node *node, string &code, int length, unordered_map<unsigned char, string> &huffmanCodes);

    unordered_map<unsigned char, string> deque_generateHuffmanCodes(const vector<unsigned char> &input);

    vector<unsigned char>
    deque_encode(const vector<unsigned char> &input, const unordered_map<unsigned char, string> &huffmanCodes);

    vector<unsigned char>
    deque_decode(const vector<unsigned char> &input, const unordered_map<unsigned char, string> &huffmanCodes);
};