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




priority_queue<Node*, vector<Node*>, Compare> Huffman::createNodes(const unordered_map<unsigned char, int>& frequencies) {
    priority_queue<Node*, vector<Node*>, Compare> nodes;
    for (const auto& pair : frequencies) {
        nodes.push(new Node(pair.first, pair.second, nullptr, nullptr));
    }
    return nodes;
}

Node* Huffman::buildTree(priority_queue<Node*, vector<Node*>, Compare>& nodes) {
    if (nodes.size() == 1) return nodes.top(); // If there is only one node, return it (base case)
    if (nodes.empty()) return nullptr; // If there are no nodes, return nullptr

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
    priority_queue<Node*, vector<Node*>, Compare> nodes = createNodes(freq);
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



TrieNode* Huffman::buildTrie(const unordered_map<unsigned char, string>& huffmanCodes) {
    TrieNode* root = new TrieNode();
    for (const auto& pair : huffmanCodes) {
        TrieNode* node = root;
        for (char bit : pair.second) {
            int index = bit - '0';
            if (node->children[index] == nullptr) {
                node->children[index] = new TrieNode();
            }
            node = node->children[index];
        }
        node->isEndOfCode = true;
        node->data = pair.first;
    }
    return root;
}

vector<unsigned char> Huffman::decode(const vector<unsigned char>& input, TrieNode* root) {
    vector<unsigned char> decoded;

    // Always read the extra byte that contains the number of valid bits in the last byte
    size_t validBitsInLastByte = static_cast<size_t>(input.back());
    size_t inputSize = input.size() - 1; // Subtract 1 to exclude the extra byte

    size_t totalBits = 0;
    size_t totalBitsInOriginalData = 8 * (inputSize - 1) + validBitsInLastByte - (8 - validBitsInLastByte);

    TrieNode* node = root;
    for (size_t i = 0; i < inputSize && totalBits < totalBitsInOriginalData; ++i) {
        unsigned char byte = input[i];
        size_t bits = (i == inputSize - 1) ? validBitsInLastByte : 8; // Use validBitsInLastByte for the last byte

        for (size_t j = 0; j < bits && totalBits < totalBitsInOriginalData; ++j) {
            // Get the j-th bit of the byte
            bool bit = (byte >> (7 - j)) & 1;
            node = node->children[bit];

            if (node->isEndOfCode) {
                decoded.push_back(node->data);
                node = root;
            }
        }
    }

    // Only throw an error if there are remaining bits that do not form a valid Huffman code
    if (node != root) {
        cout << "Invalid Huffman Code (padding bits if all 0): " << endl;
        //throw runtime_error("Invalid Huffman code"); //Dont throw this unless u hate yourself, we all know you cant catch
    }

    return decoded;
}