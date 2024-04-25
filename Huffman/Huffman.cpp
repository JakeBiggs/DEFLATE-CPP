#include "Huffman.h"
#include <bitset>
#include <deque>
#include <algorithm>

unordered_map<unsigned char, int> Huffman::countBytes(const vector<unsigned char>& input) {
    unordered_map<unsigned char, int> freq;
    for (const auto& byte : input) {
        ++freq[byte];
    }
    return freq;
}

deque<Node*> Huffman::createNodes(const unordered_map<unsigned char, int>& frequencies) {
    deque<Node*> nodes;
    for (const auto& pair : frequencies) {
        nodes.push_back(new Node(pair.first, pair.second, nullptr, nullptr));
    }
    return nodes;
}

Node* Huffman::buildTree(deque<Node*>& nodes) {
    if (nodes.size() == 1) return nodes.front(); // If there is only one node, return it (base case)
    if (nodes.empty()) return nullptr; // If there are no nodes, return nullptr

    // More efficient way to build the tree O(n log n)
    while (nodes.size() > 1) {
        // Sort the nodes by frequency
        sort(nodes.begin(), nodes.end(), [](const Node* a, const Node* b) {
            return a->freq > b->freq;
        });

        // Get the two nodes with the smallest frequency
        Node* left = nodes.back(); nodes.pop_back();
        Node* right = nodes.back(); nodes.pop_back();

        // Create a new node with the two nodes as children
        Node* parent = new Node('\0', left->freq + right->freq, left, right);

        // Add the new node to the deque
        nodes.push_back(parent);
    }
    return nodes.front();
}

void Huffman::traverseHuffmanTree(Node* node, string& code, int length, unordered_map<unsigned char, string>& huffmanCodes) {
    if (node->left == nullptr && node->right == nullptr) { // Leaf node
        huffmanCodes[node->data] = code.substr(0, length);
    }
    else { // Non-leaf node
        if(node->left != nullptr) {
            code[length] = '0';
            traverseHuffmanTree(node->left, code, length + 1, huffmanCodes);
        }
        if(node->right != nullptr){
            code[length] = '1';
            traverseHuffmanTree(node->right, code, length + 1, huffmanCodes);
        }
    }
}

unordered_map<unsigned char, string> Huffman::generateHuffmanCodes(const vector<unsigned char>& input) {
    unordered_map<unsigned char, int> freq = countBytes(input);
    deque<Node*> nodes = createNodes(freq);
    Node* root = buildTree(nodes);
    unordered_map<unsigned char, string> huffmanCodes;
    string code(256, '\0');
    traverseHuffmanTree(root, code, 0, huffmanCodes);
    return huffmanCodes;
}

vector<unsigned char> Huffman::encode(const vector<unsigned char>& input, const unordered_map<unsigned char, string>& huffmanCodes) {
    vector<unsigned char> encoded;
    int length = 0;
    unsigned char byte = 0;
    for (unsigned char data : input) {
        const string& code = huffmanCodes.at(data);
        for (char bit : code) {
            byte = (byte << 1) | (bit - '0');
            if (++length == 8) {
                encoded.push_back(byte);
                length = 0;
            }
        }
    }
    // Add the remaining bits
    if (length > 0){
        byte <<= (8 - length);
        encoded.push_back(byte);

        // Add an extra byte that contains the number of valid bits in the last byte
        encoded.push_back(length);
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
    auto validBitsInLastByte = static_cast<size_t>(input.back());
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
            if (iterator != reversedHuffmanCodes.end()) {
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