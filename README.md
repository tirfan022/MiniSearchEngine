# 📄 Mini Search Engine in C++ (TF-IDF Based)

A lightweight **file-based search engine** implemented in **C++**, featuring **inverted indexing**, **TF-IDF ranking**, **persistent indexing**, and **multi-word query support** over a document corpus such as *20 Newsgroups*.

---

## 🧠 Approach & Design

The project is designed in two main phases:

### 1️⃣ Indexing Phase (Offline)
- Recursively crawls the dataset directory.
- Extracts and preprocesses tokens from each document.
- Builds an **inverted index** mapping:

- Stores document paths and term frequencies.
- Serializes the index to disk (`index.bin`) to avoid re-indexing on future runs.

### 2️⃣ Search Phase (Online)
- Accepts **multi-word queries**.
- Preprocesses query terms (lowercasing, filtering, stopword removal).
- Computes **TF-IDF scores** for each document.
- Ranks documents by relevance.
- Displays results in paginated format (10 results per page).

---

## 🧱 Data Structures Used

| Data Structure | Purpose |
|----------------|---------|
| `unordered_map<string, vector<Posting>>` | Inverted index for fast keyword lookup |
| `unordered_map<int, string>` | Maps document IDs to file paths |
| `unordered_set<string>` | Stopword filtering |
| `vector<Posting>` | Stores (docID, term frequency) pairs |
| `unordered_map<int, double>` | Accumulates TF-IDF scores during search |
| `vector<SearchResult>` | Sorted list of ranked results |

---

## ⏱️ Time & Space Complexity

### Indexing Phase
- **Time Complexity:**  
`O(Total Tokens)`  
(Each token is processed once)

- **Space Complexity:**  
`O(V + P)`  
where:
- `V` = number of unique tokens  
- `P` = total postings across all documents

### Search Phase
- **Time Complexity:**  
`O(Q × Pq)`  
where:
- `Q` = number of query terms  
- `Pq` = average postings per term

- **Space Complexity:**  
`O(D)`  
where `D` is the number of matched documents.

---

## ▶️ How to Run the Project

### 1️⃣ Compile
```bash
g++ -std=c++17 main.cpp -o search_engine
./search_engine
