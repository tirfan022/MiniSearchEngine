#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map> 
#include <unordered_set>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <chrono> 

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

using namespace std;
using namespace std::chrono;

struct Posting {
    int docID;
    int count;
};

// Optimized with Unordered structures for O(1) average lookup
unordered_map<string, vector<Posting>> invertedIndex; 
unordered_map<int, string> docIdToPath;
unordered_set<string> stopWords; 
int totalDocuments = 0;

void initStopWords() {
    string words[] = {"the", "is", "at", "on", "and", "a", "an", "to", "in", "of", "it", "from", "for", "with", "that"};
    for(const string& w : words) stopWords.insert(w);
}

string preprocess(const string& word) {
    string cleaned = "";
    for (char c : word) {
        if (isalnum(static_cast<unsigned char>(c))) {
            cleaned += static_cast<char>(tolower(static_cast<unsigned char>(c)));
        }
    }
    return cleaned;
}

void indexFile(string path, int id) {
    ifstream file(path);
    if (!file.is_open()) return;

    docIdToPath[id] = path;
    string line, word;
    bool inBody = false;
    unordered_map<string, int> localFreq;

    while (getline(file, line)) {
        if (line.empty() && !inBody) { inBody = true; continue; }
        if (!inBody) continue;

        stringstream ss(line);
        while (ss >> word) {
            string clean = preprocess(word);
            if (!clean.empty() && stopWords.find(clean) == stopWords.end()) {
                localFreq[clean]++;
            }
        }
    }
    
    for (auto const& pair : localFreq) {
        invertedIndex[pair.first].push_back({id, pair.second});
    }
    totalDocuments++;
}

#ifdef _WIN32
void crawl(string path) {
    string searchPath = path + "\\*";
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(searchPath.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        string fullPath = path + "\\" + name;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) crawl(fullPath);
        else indexFile(fullPath, totalDocuments);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
}
#else
void crawl(string path) {
    DIR *dir = opendir(path.c_str());
    if (!dir) return;
    struct dirent *ent;
    while ((ent = readdir(dir))) {
        string name = ent->d_name;
        if (name == "." || name == "..") continue;
        string fullPath = path + "/" + name;
        struct stat st;
        stat(fullPath.c_str(), &st);
        if (S_ISDIR(st.st_mode)) crawl(fullPath);
        else indexFile(fullPath, totalDocuments);
    }
    closedir(dir);
}
#endif

// --- Index Persistence ---
void saveIndex(const string& filename) {
    ofstream out(filename, ios::binary);
    int mapSize = invertedIndex.size();
    out.write((char*)&mapSize, sizeof(mapSize));
    for (auto& pair : invertedIndex) {
        int len = pair.first.size();
        out.write((char*)&len, sizeof(len));
        out.write(pair.first.c_str(), len);
        int psize = pair.second.size();
        out.write((char*)&psize, sizeof(psize));
        for (auto& p : pair.second) {
            out.write((char*)&p.docID, sizeof(p.docID));
            out.write((char*)&p.count, sizeof(p.count));
        }
    }
    int docSize = docIdToPath.size();
    out.write((char*)&docSize, sizeof(docSize));
    for (auto& pair : docIdToPath) {
        int id = pair.first;
        int len = pair.second.size();
        out.write((char*)&id, sizeof(id));
        out.write((char*)&len, sizeof(len));
        out.write(pair.second.c_str(), len);
    }
    out.write((char*)&totalDocuments, sizeof(totalDocuments));
    out.close();
}

bool loadIndex(const string& filename) {
    ifstream in(filename, ios::binary);
    if (!in.is_open()) return false;
    int mapSize;
    in.read((char*)&mapSize, sizeof(mapSize));
    for (int i = 0; i < mapSize; i++) {
        int len; in.read((char*)&len, sizeof(len));
        string word(len, ' ');
        in.read(&word[0], len);
        int psize; in.read((char*)&psize, sizeof(psize));
        vector<Posting> postings(psize);
        for (int j = 0; j < psize; j++) {
            in.read((char*)&postings[j].docID, sizeof(int));
            in.read((char*)&postings[j].count, sizeof(int));
        }
        invertedIndex[word] = postings;
    }
    int docSize; in.read((char*)&docSize, sizeof(docSize));
    for (int i = 0; i < docSize; i++) {
        int id, len;
        in.read((char*)&id, sizeof(id));
        in.read((char*)&len, sizeof(len));
        string path(len, ' ');
        in.read(&path[0], len);
        docIdToPath[id] = path;
    }
    in.read((char*)&totalDocuments, sizeof(totalDocuments));
    in.close();
    return true;
}

// --- Multi-word Search & Ranking ---


struct SearchResult {
    int docID;
    double score;
};

void performSearch() {
    string rawQuery;
    cout << "\nSearch: ";
    getline(cin, rawQuery);

    if (preprocess(rawQuery) == "exit") {
        cout << "Goodbye!" << endl; exit(0);
    }

    auto start = high_resolution_clock::now();
    stringstream ss(rawQuery);
    string word;
    unordered_map<int, double> docScores;

    while (ss >> word) {
        string clean = preprocess(word);
        if (clean.empty() || stopWords.count(clean)) continue;

        if (invertedIndex.count(clean)) {
            const auto& postings = invertedIndex[clean];
            double idf = log10((double)totalDocuments / (double)postings.size());
            for (const auto& p : postings) {
                // Manual TF-IDF: Frequency * IDF
                docScores[p.docID] += (double)p.count * idf;
            }
        }
    }

    if (docScores.empty()) {
        cout << "No results found." << endl; return;
    }

    vector<SearchResult> finalResults;
    for (auto const& pair : docScores) finalResults.push_back({pair.first, pair.second});
    
    sort(finalResults.begin(), finalResults.end(), [](const SearchResult& a, const SearchResult& b){
        return a.score > b.score;
    });

    auto stop = high_resolution_clock::now();
    cout << "Found " << finalResults.size() << " results in " 
         << duration_cast<microseconds>(stop - start).count() << " us.\n";

    int currentIdx = 0;
    while (currentIdx < finalResults.size()) {
        int end = min((int)finalResults.size(), currentIdx + 10);
        for (int i = currentIdx; i < end; i++) {
            cout << i + 1 << ". [Score: " << finalResults[i].score << "] " << docIdToPath[finalResults[i].docID] << endl;
        }
        currentIdx = end;

        if (currentIdx < finalResults.size()) {
            string cmd;
            cout << "\n'more' for next 10, or enter for new search: ";
            getline(cin, cmd);
            if (preprocess(cmd) != "more") break;
        }
    }
}

int main() {
    initStopWords();
    string dataPath = "data/20_newsgroups/20_newsgroups"; 
    
    if (!loadIndex("index.bin")) {
        cout << "Building index (first time setup)..." << endl;
        crawl(dataPath);
        saveIndex("index.bin");
    } else {
        cout << "Index loaded successfully." << endl;
    }

    if (totalDocuments == 0) {
        cout << "ERROR: Check your data path." << endl; return 1;
    }

    cout << "Ready. Indexed " << totalDocuments << " documents." << endl;
    while (true) performSearch();
    return 0;
}