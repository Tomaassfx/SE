#include "Dictionary.h"
#include <fstream>
#include <iostream>

Dictionary::Dictionary() {
    _cuttor = new SplitToolCppJieba();
}

void Dictionary::loadDict(const string &dictFile) {
    ifstream infile(dictFile);
    if (!infile) {
        cerr << "Failed to open " << dictFile << "\n";
        return;
    }

    _dict.clear();
    string line;
    while (getline(infile, line)) {
        size_t pos = line.find(':');
        if (pos != string::npos) {
            string word = line.substr(0, pos);
            string freq_str = line.substr(pos + 1);

            try {
                int freq = std::stoi(freq_str);
                _dict.emplace_back(word, freq);
            } catch (const std::invalid_argument &e) {
                cerr << "Invalid argument while converting frequency: " << freq_str << "\n";
            } catch (const std::out_of_range &e) {
                cerr << "Out of range while converting frequency: " << freq_str << "\n";
            }
        } else {
            cerr << "Line format error: " << line << "\n";
        }
    }

    infile.close();
}

void Dictionary::loadIndex(const string &indexFile) {
    ifstream infile(indexFile);
    if (!infile) {
        cerr << "Failed to open " << indexFile << "\n";
        return;
    }

    _index.clear();
    string line;
    while (getline(infile, line)) {
        size_t pos = line.find(':');
        if (pos != string::npos) {
            string key = line.substr(0, pos);
            string indices_str = line.substr(pos + 1);

            istringstream iss(indices_str);
            set<int> index_set;
            int index;
            while (iss >> index) {
                index_set.insert(index);
            }
            _index[key] = index_set;
        } else {
            cerr << "Line format error: " << line << "\n";
        }
    }

    infile.close();
}

int Dictionary::getFrequency(const string &word) {
    auto it = std::find_if(_dict.begin(), _dict.end(),[&word](const pair<string, int> &entry) {
                return entry.first == word;
    });

    if (it != _dict.end()) {
        return it->second;
    }

    return 0; 
}