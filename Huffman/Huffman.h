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
        map<char, int> countBytes(const vector<char>& input);
        vector<Node*> createNodes(const map<char, int>& frequencies);
        Node* buildTree(vector<Node*>& nodes);
        void traverseHuffmanTree(Node* node, string code, map<char, string>& huffmanCodes);
        map<char, string> generateHuffmanCodes(const vector<char>& input);
        vector<char> encode(const vector<char>& input, const map<char, string>& huffmanCodes);
        vector<char> decode(const vector<char>& input, const map<char, string>& huffmanCodes);
    private:
        
};