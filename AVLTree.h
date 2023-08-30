#ifndef AVLTREE_H
#define AVLTREE_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;

template <typename Comparable>
class AVLTree {
private:

    struct Node {
        Comparable word; // word examined
        Node* left_node;
        Node* right_node;
        vector<string> documents; // vector of documents containing word
        int height;

        Node(const Comparable& w, Node* l, Node* r, vector<string> d = {}, int h = 0)
                : word(w), left_node(l), right_node(r), documents(d), height(h) {}
    };

    Node* root;
    int treeSize;

public:
    //Constructor
    AVLTree() { root = nullptr; }

    //Copy Constructor
    AVLTree(const AVLTree &rhs) : root{nullptr} {
        root = clone(rhs.root);
    }

    //Destructor
    ~AVLTree() { makeEmpty(); }

    //Copy Assignment Operator
    AVLTree &operator=(const AVLTree &rhs) {
        makeEmpty();
        root = clone(rhs.root);
        return *this;
    }

    bool contains(Comparable& x) { return contains(x, root); }

    bool isEmpty() { return (root == nullptr); }

    void makeEmpty() { return makeEmpty(root); }

    void insert(Comparable& x) { return insert(x, root); }

    int size() {
        treeSize = 0;
        size(root);
        return treeSize;
    }

    void addDocument(Comparable& x, string document) {
        if (!contains(x, root)) {
            insert(x);
        }
        return addDocument(x, root, document);
    }

    vector<string> getDocuments(Comparable& x) { return getDocuments(x, root); }

    void prettyPrintTree() const { prettyPrintTree("", root, false); }

    void saveTree(string filePath) const { saveTree(root, true, filePath); }

private: // Internal methods

    //recursive function inserts an element at the root of a subtree
    void insert(Comparable& x, Node*& subroot) {
        if (subroot == nullptr) {
            subroot = new Node(x,nullptr,nullptr);
            return;
        }

        if (x < subroot->word) {
            insert(x, subroot->left_node);
        } else if (subroot->word < x) {
            insert(x, subroot->right_node);
        } else {}

        balance(subroot);
    }

    //recursive function cycles from root node and searches tree
    bool contains(Comparable& x, Node* subroot) {
        if (subroot == nullptr) {
            return false;
        }

        if (x < subroot->word) {
            return contains(x, subroot->left_node);
        } else if (x > subroot->word) {
            return contains(x, subroot->right_node);
        } else {
            return true;
        }
    }

    //recursive function cycles from root node and adds for size
    void size(Node* subroot) {
        if (subroot == nullptr) {
            return;
        }

        size(subroot->left_node);
        size(subroot->right_node);
        treeSize++;
        return;
    }

    //recursive function cycles from root node and searches tree to add word
    void addDocument(Comparable& x, Node* subroot, string document) {
        if (subroot == nullptr || contains(x, subroot) == false) {
            return;
        }

        if (x < subroot->word) {
            return addDocument(x, subroot->left_node, document);
        } else if (x > subroot->word) {
            return addDocument(x, subroot->right_node, document);
        } else {
            (subroot->documents).push_back(document);
            return;
        }
    }

    vector<string> getDocuments(Comparable& x, Node* subroot) {
        if (subroot == nullptr || contains(x, subroot) == false) {
            return {};
        }

        if (x < subroot->word) {
            return getDocuments(x, subroot->left_node);
        } else if (x > subroot->word) {
            return getDocuments(x, subroot->right_node);
        } else {
            return (subroot->documents);
        }
    }

    //recursive function cycles through entire tree and deletes each node
    void makeEmpty(Node*& subroot) {
        if (subroot == nullptr) { return; }

        makeEmpty(subroot->left_node);
        makeEmpty(subroot->right_node);

        delete subroot;
        subroot = nullptr;
    }

    Node* clone(Node* node) {
        if (node == nullptr) { return nullptr; }

        return new Node(node->word,clone(node->left_node),clone(node->right_node),node->documents,node->height);
    }

    int height(Node* subroot) {
        if (subroot == nullptr)
            return -1;

        return subroot->height;
    }

    void balance(Node*& subroot) {
        if (subroot == nullptr) { return; }
        int imbalance_threshold = 1;

        int bf = height(subroot->left_node) - height(subroot->right_node);

        if (bf > imbalance_threshold) { // unbalancing insertion was left_node
            if (height(subroot->left_node->left_node) >= height(subroot->left_node->right_node))
                rotateWithleft_nodeChild(subroot); // case 1 (outside)
            else
                doubleWithleft_nodeChild(subroot); // case 2 (inside)
        }
        else if (bf < imbalance_threshold) { // unbalancing insertion was right_node
            if (height(subroot->right_node->right_node) >= height(subroot->right_node->left_node))
                rotateWithright_nodeChild(subroot); // case 4 (outside)
            else
                doubleWithright_nodeChild(subroot); // case 3 (inside)
        }

        // readjusting height
        subroot->height = max(height(subroot->left_node), height(subroot->right_node)) + 1;
    }

    int max(int one, int two) {
        if (one > two) {
            return one;
        } else {
            return two;
        }
    }

    void rotateWithleft_nodeChild(Node *&k2) {
        Node *k1 = k2->left_node;
        k2->left_node = k1->right_node;
        k1->right_node = k2;
        k2->height = max(height(k2->left_node), height(k2->right_node)) + 1;
        k1->height = max(height(k1->left_node), k2->height) + 1;
        k2 = k1;
    }

    /**
     * Rotate binary tree node with right_node child.
     * For AVL trees, this is a single rotation for case 4.
     * Update heights, then set new root.
     */
    void rotateWithright_nodeChild(Node *&k1) {
        Node *k2 = k1->right_node;
        k1->right_node = k2->left_node;
        k2->left_node = k1;
        k1->height = max(height(k1->left_node), height(k1->right_node)) + 1;
        k2->height = max(height(k2->right_node), k1->height) + 1;
        k1 = k2;
    }

    /**
     * Double rotate binary tree node: first left_node child.
     * with its right_node child; then node k3 with new left_node child.
     * For AVL trees, this is a double rotation for case 2.
     * Update heights, then set new root.
     */
    void doubleWithleft_nodeChild(Node *&k3) {
        rotateWithright_nodeChild(k3->left_node);
        rotateWithleft_nodeChild(k3);
    }

    /**
     * Double rotate binary tree node: first right_node child.
     * with its left_node child; then node k1 with new right_node child.
     * For AVL trees, this is a double rotation for case 3.
     * Update heights, then set new root.
     */
    void doubleWithright_nodeChild(Node *&k1) {
        rotateWithleft_nodeChild(k1->right_node);
        rotateWithright_nodeChild(k1);
    }

    // Modified from: https://stackoverflow.com/questions/36802354/print-binary-tree-in-a-pretty-way-using-c
    void prettyPrintTree(const std::string &prefix, const Node *node, bool isRight) const
    {
        if (node == nullptr)
        {
            return;
        }

        std::cout << prefix;
        std::cout << (isRight ? "├──" : "└──");
        // print the value of the node
        std::cout << node->word << std::endl;

        // enter the next tree level - left and right branch
        prettyPrintTree(prefix + (isRight ? "│   " : "    "), node->right_node, true);
        prettyPrintTree(prefix + (isRight ? "│   " : "    "), node->left_node, false);
    }

    void saveTree(const Node *node, bool outputOut, string filePath) const
    {
        if (node == nullptr)
        {
            return;
        }

        if (node->word.size() > 0) {
            fstream file;
            if (outputOut)
            {
                file.open(filePath, ios::out);
            }
            else
            {
                file.open(filePath, ios::app);
            }

            if(!file.is_open())
            {
                cerr << "File not open: " << filePath << endl;
            }

            file << node->word << "^";
            vector<string> docs = node->documents;
            if (!docs.empty())
            {
                file << docs.size();
                for (size_t i = 0; i < docs.size(); ++i)
                {
                    file << "^" << docs[i];
                }
                file << endl;
            }
            else
            {
                file << "0^" << endl;
            }
            file.close();
        }

        // enter the next tree level - left and right branch
        saveTree(node->right_node, false, filePath);
        saveTree(node->left_node, false, filePath);
    }
};

#endif
