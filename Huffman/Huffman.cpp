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

map<char, int> Huffman::countBytes(const vector<char>& input) {
    map<char, int> freq;
    for (char byte : input) {
        ++freq[byte];
    }
    return freq;
}

vector<Node*> Huffman::createNodes(const map<char, int>& frequencies) {
    vector<Node*> nodes;
    for (auto pair : frequencies) {
        nodes.push_back(new Node(pair.first, pair.second, nullptr, nullptr));
    }
    return nodes;
}

Node* Huffman::buildTree(vector<Node*>& nodes){
    if (nodes.size() == 1) return nodes[0]; // If there is only one node, return it (base case
    if (nodes.empty()) return nullptr; // If there are no nodes, return nullptr

    //Inefficient way to build the tree O(n^2 log n) 
    while (nodes.size() > 1) {
        // Sort the nodes by frequency
        sort(nodes.begin(), nodes.end(), [](Node* a, Node* b) { return a->freq < b->freq; });

        if (nodes.size() < 2) {
            cout << "Not enough nodes to build tree\n" << endl;
            // Handle error
            throw std::runtime_error("Not enough nodes to build tree");
        }
        // Get the two nodes with the smallest frequency
        Node* left = nodes[0];
        Node* right = nodes[1];

        // Create a new node with the two nodes as children
        Node* parent = new Node('\0', left->freq + right->freq, left, right);

        // Remove the two nodes from the list and add the new node
        nodes.erase(nodes.begin(), nodes.begin() + 2);
        nodes.push_back(parent);
    }
    return nodes[0];

}

void Huffman::traverseHuffmanTree(Node* node, string code, map<char, string>& huffmanCodes) {
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

map<char, string> Huffman::generateHuffmanCodes(const vector<char>& input) {
    map<char, int> freq = countBytes(input);
    vector<Node*> nodes = createNodes(freq);
    Node* root = buildTree(nodes);
    map<char, string> huffmanCodes;
    traverseHuffmanTree(root, "", huffmanCodes);
    return huffmanCodes;
}

vector<char> Huffman::encode(const vector<char>& input, const map<char, string>& huffmanCodes) {
    vector<char> encoded;
    string temp;
    for (char byte : input) {
        temp += huffmanCodes.at(byte);
        while (temp.size() >= 8) {

            // Convert the first 8 bits to a char byte and add it to the encoded data
            char byte = static_cast<char>(bitset<8>(temp.substr(0, 8)).to_ulong());
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
        char byte = static_cast<char>(bitset<8>(temp).to_ulong());
        encoded.push_back(byte);

        // Add an extra byte that contains the number of valid bits in the last byte
        encoded.push_back(static_cast<char>(validBitsInLastByte));
    }
    return encoded;
}

vector<char> Huffman::decode(const vector<char>& input, const map<char, string>& huffmanCodes) {
    string encoded;
    vector<char> decoded;

    map<string, char> reversedHuffmanCodes;
    for (const auto &pair : huffmanCodes) {
        reversedHuffmanCodes[pair.second] = pair.first;
    }

    // Read the extra byte that contains the number of valid bits in the last byte
    size_t validBitsInLastByte = static_cast<size_t>(input.back());
    vector<char> inputWithoutExtraByte(input.begin(), input.end() - 1);

    for (size_t i = 0; i < inputWithoutExtraByte.size(); ++i) {
        char byte = inputWithoutExtraByte[i];
        bitset<8> bits(byte);

        // If this is the last byte, only include the valid bits
        if (i == inputWithoutExtraByte.size() - 1) {
            encoded += bits.to_string().substr(0, validBitsInLastByte);
        } else {
            encoded += bits.to_string();
        }
    }

    string code;
    for(char bit : encoded){
        code += bit;
        auto iterator = reversedHuffmanCodes.find(code);
        if (iterator != reversedHuffmanCodes.end()){ // If the current byte is a valid huffman code (will be end if not)
            decoded.push_back(iterator->second);
            code.clear();
        }
    }

    if (!code.empty()) {
        cout << "Invalid Huffman Code: " << code << endl;
        throw runtime_error("Invalid Huffman code");
    }

    return decoded;
}