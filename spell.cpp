#include <iostream>
#include <vector>
#include <queue>
#include <fstream>
#include <unordered_set>
#include <string>
using namespace std;

struct TrieNode {
    bool terminating = false;
    TrieNode* children[26] = {NULL}; // each node has up to 26 children, one for each letter
};

class Trie {
    TrieNode* root;
public:
    Trie();
    void insertWord(string word);
    bool searchWord(string word);
    bool deleteWord(string word);
    bool updateWord(string oldWord, string newWord);
    vector<string> prefixSearch(string prefix);
    vector<string> spellCheck(string word);
    void loadDictionary(const string& filename);

private:
    void dfs(TrieNode* node, string prefix, vector<string>& result);
    bool isOneEditDistance(string& word1, string& word2);
};

Trie::Trie() {
    root = new TrieNode();
}

// Insert a word into the Trie
void Trie::insertWord(string word) {
    TrieNode* currNode = root;
    for (char c : word) {
        if (currNode->children[c - 'a'] == NULL) {
            currNode->children[c - 'a'] = new TrieNode();
        }
        currNode = currNode->children[c - 'a'];
    }
    currNode->terminating = true;
}

// Search for an exact word in the Trie
bool Trie::searchWord(string word) {
    TrieNode* currNode = root;
    for (char c : word) {
        if (currNode->children[c - 'a'] == NULL) {
            return false;
        }
        currNode = currNode->children[c - 'a'];
    }
    return currNode->terminating;
}

// Delete a word from the Trie (simple version)
bool Trie::deleteWord(string word) {
    TrieNode* currNode = root;
    for (char c : word) {
        if (currNode->children[c - 'a'] == NULL) {
            return false;
        }
        currNode = currNode->children[c - 'a'];
    }
    currNode->terminating = false;
    return true;
}

// Update a word by deleting the old one and inserting the new one
bool Trie::updateWord(string oldWord, string newWord) {
    bool deleted = deleteWord(oldWord);
    if (deleted) {
        insertWord(newWord);
    }
    return deleted;
}

// DFS helper function to collect all words starting with a prefix
void Trie::dfs(TrieNode* node, string prefix, vector<string>& result) {
    if (node->terminating) {
        result.push_back(prefix);
    }
    for (int i = 0; i < 26; ++i) {
        if (node->children[i] != NULL) {
            dfs(node->children[i], prefix + char(i + 'a'), result);
        }
    }
}

// Get all words that start with the given prefix
vector<string> Trie::prefixSearch(string prefix) {
    vector<string> result;
    TrieNode* currNode = root;
    for (char c : prefix) {
        if (currNode->children[c - 'a'] == NULL) {
            return result;  // No words with this prefix
        }
        currNode = currNode->children[c - 'a'];
    }
    dfs(currNode, prefix, result);
    return result;
}

// Check if two words are one edit distance apart (for spell checking)
bool Trie::isOneEditDistance(string& word1, string& word2) {
    int len1 = word1.size(), len2 = word2.size();
    if (abs(len1 - len2) > 1) return false;

    int i = 0, j = 0, edits = 0;
    while (i < len1 && j < len2) {
        if (word1[i] != word2[j]) {
            edits++;
            if (edits > 1) return false;
            if (len1 > len2) i++;
            else if (len1 < len2) j++;
            else { i++; j++; }
        } else {
            i++; j++;
        }
    }
    return true;
}

// Spell check using BFS to suggest similar words
vector<string> Trie::spellCheck(string word) {
    vector<string> result;
    unordered_set<string> suggestions;
    queue<pair<TrieNode*, string>> q;
    q.push(make_pair(root, ""));

    while (!q.empty()) {
        pair<TrieNode*, string> p = q.front();
        TrieNode* node = p.first;
        string prefix = p.second;
        q.pop();
        
        if (node->terminating && isOneEditDistance(word, prefix)) {
            suggestions.insert(prefix);
        }
        
        for (int i = 0; i < 26; ++i) {
            if (node->children[i] != NULL) {
                q.push(make_pair(node->children[i], prefix + char(i + 'a')));
            }
        }
    }

    result.insert(result.end(), suggestions.begin(), suggestions.end());
    return result;
}

void Trie::loadDictionary(const string& filename) {
    ifstream file(filename);
    string word;
    if (file.is_open()) {
        while (getline(file, word)) {
            insertWord(word);
        }
        file.close();
    } else {
        cerr << "Unable to open the file!" << endl;
    }
}

#define underlineOn "\033[4m"
#define underlineOff "\033[0m"

int main() {
    Trie oTrie;

    // Insert sample words
    oTrie.loadDictionary("dictionary.txt");

    // Spell checker interactive loop
    string s = "";
    while (true) {
        cout << "Enter a word (or 'exit' to quit): ";
        cin >> s;
        if (s == "exit") break;

        // Suggest words based on the current input
        vector<string> prefixResults = oTrie.prefixSearch(s);
        if (!prefixResults.empty()) {
            cout << "Suggestions for prefix '" << s << "': ";
            for (string suggestion : prefixResults) {
                cout << suggestion << " ";
            }
            cout << endl;
        }

        // Check if the word exists in the Trie
        if (oTrie.searchWord(s)) {
            cout << "Word found: " << s << endl;
        } else {
            cout << underlineOn << "Word not found: " << s << underlineOff << endl;

            vector<string> corrections = oTrie.spellCheck(s);
            if (!corrections.empty()) {
                cout << "Did you mean: ";
                for (string suggestion : corrections) {
                    cout << suggestion << " ";
                }
                cout << endl;
            } else {
                cout << "No suggestions found." << endl;
            }
        }
    }

    return 0;
}
