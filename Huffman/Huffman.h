#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <queue>
using namespace std;


struct Node {
            char data;
            int freq;
            Node *left, *right;
            Node(char data, int freq, Node* left = nullptr, Node* right = nullptr)
                : data(data), freq(freq), left(left), right(right) {}
};

struct Compare {
    bool operator()(Node* l, Node*r){
        return l->freq, r->freq;
    }

};



class Huffman{
    public:
        unordered_map<unsigned char, int> countBytes(const vector<unsigned char>& input);
        priority_queue<Node*, vector<Node*>, Compare> createNodes(const unordered_map<unsigned char, int>& frequencies);
        Node* buildTree(priority_queue<Node*, vector<Node*>, Compare>& nodes);
        void traverseHuffmanTree(Node* node, string code, unordered_map<unsigned char, string>& huffmanCodes);
        unordered_map<unsigned char, string> generateHuffmanCodes(const vector<unsigned char>& input);
        vector<unsigned char> encode(const vector<unsigned char>& input, const unordered_map<unsigned char, string>& huffmanCodes);
        vector<unsigned char> decode(const vector<unsigned char>& input, const unordered_map<unsigned char, string>& huffmanCodes);
    private:
        
};