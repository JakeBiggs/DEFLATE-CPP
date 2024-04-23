/*
The Algorithm

1> First count the amount of times each byte appears. This is the frequency of each byte.

2>Create a collection of n small, one-node trees (where n is the number of distinct characters in the input stream). 
Each of these n trees represent a distinct input character and have a weight corresponding to their count tallied in the analysis step.

3>From the collection, pick out the two trees with the smallest weights and remove them. 
Combine them into a new tree whose root has the weight equal to the sum of the weights of the two trees and with the two trees as its left and right subtrees.
Add the new combined tree back into the collection.

4>Continue this process - select the two trees (with anywhere from 1 to (n - 1) nodes) with lowest weight, join these by a new root node, set the root node's weight, and place the new tree back into the pool.
Repeat this process until one tree encompassing all the input weights has been constructed.

If at any point there is more than one way to choose the two trees of smallest weight, the algorithm chooses arbitrarily. 
The resultant large tree with a single root node is called a Huffman tree. 
This way, the nodes with the highest weight will be near the top of the tree, and have shorter codes.
*/
#include "Huffman.h"
#include <bitset>

unordered_map<unsigned char, int> Huffman::countBytes(const vector<unsigned char>& input) {
    unordered_map<unsigned char, int> freq;
    for (auto byte : input) {
        ++freq[byte];
    }
    return freq;
}

priority_queue<Node*, vector<Node*>, Compare> Huffman::createNodes(const unordered_map<unsigned char, int>& frequencies) {
    priority_queue<Node*, vector<Node*>, Compare> nodes;
    for (auto pair : frequencies) {
        nodes.push(new Node(pair.first, pair.second, nullptr, nullptr));
    }
    return nodes;
}

Node* Huffman::buildTree(priority_queue<Node*, vector<Node*>, Compare>& nodes){
    if (nodes.size() == 1) return nodes.top(); // If there is only one node, return it (base case)
    if (nodes.empty()) return nullptr; // If there are no nodes, return nullptr

    //Inefficient way to build the tree O(n^2 log n) 
    while (nodes.size() > 1) {
        // Get the two nodes with the smallest frequency
        Node* left = nodes.top(); nodes.pop();
        Node* right = nodes.top(); nodes.pop();

        // Create a new node with the two nodes as children
        Node* parent = new Node('\0', left->freq + right->freq, left, right);

        // Add the new node to the priority queue
        nodes.push(parent);
    }
    return nodes.top();

}

void Huffman::traverseHuffmanTree(Node* node, string code, unordered_map<unsigned char, string>& huffmanCodes) {
    if (node->left == nullptr && node->right == nullptr) { // Leaf node
        huffmanCodes[node->data] = code;
    }
    else { // Non-leaf node
        if(node->left != nullptr) {
            traverseHuffmanTree(node->left, code + "0", huffmanCodes);
        }
        if(node->right != nullptr){
            traverseHuffmanTree(node->right, code + "1", huffmanCodes);
        }
    }
}

unordered_map<unsigned char, string> Huffman::generateHuffmanCodes(const vector<unsigned char>& input) {
    unordered_map<unsigned char, int> freq = countBytes(input);
    priority_queue<Node*, vector<Node*>, Compare> nodes = createNodes(freq);
    Node* root = buildTree(nodes);
    unordered_map<unsigned char, string> huffmanCodes;
    traverseHuffmanTree(root, "", huffmanCodes);
    return huffmanCodes;
}

vector<unsigned char> Huffman::encode(const vector<unsigned char>& input, const unordered_map<unsigned char, string>& huffmanCodes) {
    vector<unsigned char> encoded;
    string temp;
    for (unsigned char byte : input) {
        temp += huffmanCodes.at(byte);
        while (temp.size() >= 8) {

            // Convert the first 8 bits to a char byte and add it to the encoded data
            unsigned char byte = static_cast<char>(bitset<8>(temp.substr(0, 8)).to_ulong());
            encoded.push_back(byte); //
            temp = temp.substr(8); // Remove the first 8 bits after processing 
        }
    }
    // Add the remaining bits
    if (!temp.empty()){
        size_t validBitsInLastByte = temp.size();
        while (temp.size() < 8){
            temp += "0"; //pads remaining byte with 0s
        }
        unsigned char byte = static_cast<unsigned char>(bitset<8>(temp).to_ulong());
        encoded.push_back(byte);

        // Add an extra byte that contains the number of valid bits in the last byte
        encoded.push_back(static_cast<unsigned char>(validBitsInLastByte));
    }
    return encoded;
}

vector<unsigned char> Huffman::decode(const vector<unsigned char>& input, const unordered_map<unsigned char, string>& huffmanCodes) {
    vector<unsigned char> decoded;

    unordered_map<string, unsigned char> reversedHuffmanCodes;
    for (const auto &pair : huffmanCodes) {
        reversedHuffmanCodes[pair.second] = pair.first;
    }

    // Read the extra byte that contains the number of valid bits in the last byte
    size_t validBitsInLastByte = static_cast<size_t>(input.back());
    vector<unsigned char> inputWithoutExtraByte(input.begin(), input.end() - 1);

    string code;
    for (size_t i = 0; i < inputWithoutExtraByte.size(); ++i) {
        unsigned char byte = inputWithoutExtraByte[i];

        // If this is the last byte, only include the valid bits
        size_t bitsToProcess = (i == inputWithoutExtraByte.size() - 1) ? validBitsInLastByte : 8;

        for (size_t j = 0; j < bitsToProcess; ++j) {
            // Get the j-th bit of the byte
            bool bit = (byte >> (7 - j)) & 1;
            code += bit ? '1' : '0';

            auto iterator = reversedHuffmanCodes.find(code);
            if (iterator != reversedHuffmanCodes.end()){ // If the current byte is a valid huffman code (will be end if not)
                unsigned char decodedByte = iterator->second;
                decoded.push_back(iterator->second);
                code.clear();
            }
        }
    }

    if (!code.empty()) {
        cout << "Invalid Huffman Code: " << code << endl;
        throw runtime_error("Invalid Huffman code");
    }

    return decoded;
}