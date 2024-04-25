#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <map>
#include <string>
#include <algorithm>
using namespace std;


struct Node {
            char data;
            int freq;
            Node *left, *right;
            Node(char data, int freq, Node* left = nullptr, Node* right = nullptr)
                : data(data), freq(freq), left(left), right(right) {}
        };

class Huffman{
    public:
        map<unsigned char, int> countBytes(const vector<unsigned char>& input);
        vector<Node*> createNodes(const map<unsigned char, int>& frequencies);
        Node* buildTree(vector<Node*>& nodes);
        void traverseHuffmanTree(Node* node, string code, map<unsigned char, string>& huffmanCodes);
        map<unsigned char, string> generateHuffmanCodes(const vector<unsigned char>& input);
        vector<unsigned char> encode(const vector<unsigned char>& input, const map<unsigned char, string>& huffmanCodes);
        vector<unsigned char> decode(const vector<unsigned char>& input, const map<unsigned char, string>& huffmanCodes);
    private:
        
};