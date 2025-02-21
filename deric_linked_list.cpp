#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <chrono>

using namespace std;
using namespace std::chrono;

struct News {
    string title, text, subject, date, identify, word;
    int frequency;
    News *next, *head;
    News() : next(nullptr) {}
    News(string title, string text, string subject, string date, string identify)
    : title(title), text(text), subject(subject), date(date), identify(identify), next(nullptr) {}
    News(string w, int f) : word(w), frequency(f), next(nullptr) {}
};

/**
 * Helper functions to calculate memory usage
 */
struct MemoryStats {
    size_t structSize;     // Size of News struct nodes
    size_t stringSize;     // Size of string contents
    size_t pointerSize;    // Size of pointers
    size_t totalSize;      // Total memory usage
    double timeElapsed;    // Time taken for operation

    MemoryStats() : structSize(0), stringSize(0), pointerSize(0), totalSize(0), timeElapsed(0) {}
};

/**
 * Helper function to calculate memory usage for a linked list
 */
struct WordNode {
    string word;
    int frequency;
    WordNode* next;
    WordNode(string w, int f) : word(w), frequency(f), next(nullptr) {}
};

/**
 * Linked List for Word Frequency
 */
class WordList {
    public:
        WordNode* head;
        WordList() : head(nullptr) {}

        WordNode* find(const string &word) {
            WordNode* current = head;
            while (current) {
                if (current -> word == word) {
                    return current;
                }
                current = current -> next;
            }
            return nullptr;
        }

        void insertOrUpdate(const string &word) {
            WordNode* node = find(word);
            if (node) {
                node -> frequency++;
            } else {
                WordNode* newNode = new WordNode(word, 1);
                newNode -> next = head;
                head = newNode;
            }
        }

        // Helper function: Get the tail of a linked list starting at 'cur'
        WordNode* getTail(WordNode* cur) {
            while (cur && cur -> next)
                cur = cur -> next;
            return cur;
        }

        // Partition the list using the last element as pivot.
        // This partitions the list into nodes with frequency greater than the pivot (for descending order)
        // and nodes with lower frequency.
        WordNode* partition_word_freq(WordNode* head, WordNode* end, WordNode** newHead, WordNode** newEnd) {
            WordNode* pivot = end;
            WordNode *prev = nullptr, *cur = head, *tail = pivot;
            
            // During partition, newHead and newEnd will be updated
            while (cur != pivot) {
                if (cur -> frequency > pivot -> frequency) { // For descending order
                    if ((*newHead) == nullptr)
                        *newHead = cur;
                    prev = cur;
                    cur = cur -> next;
                } else {
                    // Move 'cur' node to after tail
                    if (prev)
                        prev -> next = cur -> next;
                    WordNode* tmp = cur -> next;
                    cur -> next = nullptr;
                    tail -> next = cur;
                    tail = cur;
                    cur = tmp;
                }
            }
            
            if ((*newHead) == nullptr)
                *newHead = pivot;
            
            *newEnd = tail;
            return pivot;
        }

        // Recursive quick sort for linked list.
        WordNode* quickSortRecur(WordNode* head, WordNode* end) {
            if (!head || head == end)
                return head;

            WordNode *newHead = nullptr, *newEnd = nullptr;
            // Partition the list and get the pivot.
            WordNode* pivot = partition_word_freq(head, end, &newHead, &newEnd);

            // If pivot is not the smallest element, recursively sort the left part.
            if (newHead != pivot) {
                WordNode* tmp = newHead;
                while (tmp -> next != pivot)
                    tmp = tmp -> next;
                tmp -> next = nullptr; // Disconnect left sublist

                newHead = quickSortRecur(newHead, tmp);

                // Reconnect pivot
                tmp = getTail(newHead);
                tmp -> next = pivot;
            }

            // Recursively sort the list after the pivot.
            pivot -> next = quickSortRecur(pivot -> next, newEnd);
            return newHead;
        }

        // Add this member function to your WordList class:
        void quickSort_word_freq() {
            head = quickSortRecur(head, getTail(head));
        }

        void display() {
            WordNode* current = head;
            while (current) {
                cout << current -> word << ": " << current -> frequency << endl;
                current = current -> next;
            }
        }

        ~WordList() {
            WordNode* current = head;
            while (current) {
                WordNode* tmp = current;
                current = current -> next;
                delete tmp;
            }
        }
};

string cleanWord(const string &word) {
    string cleaned;
    for (char c : word) {
        if (isalnum(c)) cleaned.push_back(tolower(c));
    }
    return cleaned;
}

bool parseAllCSVLine(const string &line, string &title, string &text, string &subject, string &date, string &identify) {
    if (line.empty())
        return false;
    stringstream ss(line);
    getline(ss, title, ',');
    getline(ss, text, ',');
    getline(ss, subject, ',');
    getline(ss, date, ',');
    getline(ss, identify, ',');
    
    // Remove surrounding quotes if present
    auto removeQuotes = [](string &s) {
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
            s = s.substr(1, s.size() - 2);
        }
    };
    removeQuotes(title);
    removeQuotes(text);
    removeQuotes(subject);
    removeQuotes(date);
    removeQuotes(identify);
    
    return true;
}

/**
 * Insert at End of Linked List
 */
void insertAtEnd(News** head, string title, string text, string subject, string date, string identify) {
    News* newNews = new News; // Create a new News
    newNews -> title = title;
    newNews -> text = text;
    newNews -> subject = subject;
    newNews -> date = date;
    newNews -> identify = identify;

    if (*head == nullptr) {
        *head = newNews;
    } else {
        News* currentNews = *head;
        while (currentNews -> next != nullptr) {
            currentNews = currentNews -> next;
        }
        currentNews -> next = newNews;
    }
}

/**
 * Function to parse a CSV line into its individual columns while correctly handling quoted fields.
 * This function extracts the first four columns: title, text, subject, and date.
 * It manually parses the CSV line without using vector or list.
 */
void parseCSVLine(const string &line, string &title, string &text, string &subject, string &date, string &identify) {
    string field;          // To accumulate characters for a field
    int col = 0;           // Column counter
    bool inQuotes = false; // Flag to track if we're inside quotes

    // Iterate over each character in the line
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];

        if (c == '"') {  // If we encounter a quote
            if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                field.push_back('"');
                i++; // Skip the next quote
            } else {
                inQuotes = !inQuotes; // Toggle the inQuotes flag
            }
        } else if (c == ',' && !inQuotes) { // Comma delimiter outside quotes
            // Assign the accumulated field to the appropriate column
            switch(col) {
                case 0: title = field; break;
                case 1: text = field; break;
                case 2: subject = field; break;
                case 3: date = field; break;
                case 4: identify = field; break;
                // We ignore any extra columns here
            }
            col++;         // Move to next column
            field.clear(); // Clear the field accumulator
        } else {
            field.push_back(c); // Append character to the field
        }
    }
    // After the loop, assign the last field (if any)
    if (!field.empty() || col == 4) {
        switch(col) {
            case 0: title = field; break;
            case 1: text = field; break;
            case 2: subject = field; break;
            case 3: date = field; break;
            case 4: identify = field; break;
            default: break;
        }
    }
}

/**
 * Preload News into Linked List
 * @param infile The input file stream
 * @param newsBook The linked list of news
 */
void preloadNews(ifstream& infile, News*& newsBook) {
    string line;
    getline(infile, line); // Skip the header line
    while (getline(infile, line)) {
        string title, news, subject, date, identify;
        parseCSVLine(line, title, news, subject, date, identify);
        insertAtEnd(&newsBook, title, news, subject, date, identify);
    }
}

/**
 * Helper function to validate date format (dd-mm-yyyy)
 * @param date The date string to validate
 * @return true if date format is valid, false otherwise
 */
bool isValidDate(const string& date) {
    if (date.length() != 10) return false;
    if (date[2] != '-' || date[5] != '-') return false;
    
    for (int i = 0; i < 10; i++) {
        if (i != 2 && i != 5 && !isdigit(date[i])) return false;
    }
    
    string day = date.substr(0, 2);
    string month = date.substr(3, 2);
    string year = date.substr(6, 4);
    
    try {
        int d = stoi(day);
        int m = stoi(month);
        int y = stoi(year);
        
        if (d < 1 || d > 31 || m < 1 || m > 12 || y < 1900) return false;
    } catch (const exception& e) {
        return false;
    }
    
    return true;
}

/**
 * Helper function to compare dates in dd-mm-yyyy format
 * @param date1 The first date string
 * @param date2 The second date string
 * @return true if date1 <= date2
 */
bool compareDate(const string& date1, const string& date2) {
    //cout << "Comparing dates: " << date1 << " and " << date2 << endl;
    int day1, month1, year1, day2, month2, year2;
    char dash;  // for the '-' separator
    stringstream ss1(date1), ss2(date2);
    
    ss1 >> day1 >> dash >> month1 >> dash >> year1;
    ss2 >> day2 >> dash >> month2 >> dash >> year2;
    
    if (year1 != year2) return year1 < year2;
    if (month1 != month2) return month1 < month2;
    return day1 <= day2;
}

/**
 * Partition function for Quick Sort
 * @param head The head of the linked list
 * @param end The end of the linked list
 * @param newHead The new head of the linked list
 * @param newEnd The new end of the linked list
 * @return the pivot node
 */
News* partition(News* head, News* end, News** newHead, News** newEnd) {
    News* pivot = end;
    News* prev = nullptr, *cur = head, *tail = pivot;

    while (cur != pivot) {
        if (isValidDate(cur -> date) && isValidDate(pivot -> date)) {
            if (compareDate(cur -> date, pivot -> date)) {  // Changed comparison
                if ((*newHead) == nullptr) (*newHead) = cur;
                prev = cur;
                cur = cur -> next;
            } else {
                if (prev) prev -> next = cur -> next;
                News* tmp = cur -> next;
                cur -> next = nullptr;
                tail -> next = cur;
                tail = cur;
                cur = tmp;
            }
        } else {
            cur = cur -> next;
        }
    }
    if ((*newHead) == nullptr) (*newHead) = pivot;
    (*newEnd) = tail;
    return pivot;
}

/**
 * Recursive Quick Sort Function
 * @param head The head of the linked list
 * @param end The end of the linked list
 * @return the new head of the linked list
 */
News* quickSortRec(News* head, News* end) {
    if (!head || head == end) return head;
    News* newHead = nullptr, *newEnd = nullptr;
    News* pivot = partition(head, end, &newHead, &newEnd);
    if (newHead != pivot) {
        News* tmp = newHead;
        while (tmp -> next != pivot) tmp = tmp -> next;
        tmp -> next = nullptr;
        newHead = quickSortRec(newHead, tmp);
        tmp = newHead;
        while (tmp -> next) tmp = tmp -> next;
        tmp -> next = pivot;
    }
    pivot -> next = quickSortRec(pivot -> next, newEnd);
    return newHead;
}

/**
 * QuickSort driver function
 * @param headRef The head of the linked list
 */
void quickSort(News** headRef) {
    if (!headRef || !(*headRef) || !(*headRef) -> next) return;
    
    News* last = *headRef;
    while (last -> next) last = last -> next; // Find the last node

    *headRef = quickSortRec(*headRef, last);
}

/**
 * Iterative function to count news articles
 * @param head The head of the linked list
 */
void iterativeCount(News* head) {
    int count = 0;
    News* current = head;
    while (current != nullptr) {
        count++;
        current = current -> next;
    }
    cout << "Total news articles: " << count << endl;
}

/**
 * Calculate Total Number of Political News
 * @param news The linked list of news
 */
void countPoliticNews(News** news) {
    News* currentNews = *news;
    // Initialize variables
    double totalPoliticalNews = 0;
    double fakePoliticalNews2016 = 0;

    // Read through each line
    while (currentNews != nullptr) {
        stringstream ss(currentNews -> date);
        // Trim the date
        string day, month, year;
        getline(ss, day, '-');
        getline(ss, month, '-');
        getline(ss, year);
        
        // If subject is politics and year is 2016
        if ((currentNews -> subject == "politics" || currentNews -> subject == "politicsNews") && year == "2016") {
            // If identify is TRUE
            if (currentNews -> identify == "FAKE") {
                fakePoliticalNews2016++;
            }
            totalPoliticalNews++;
        }
        currentNews = currentNews -> next;
    }
    // Print the results
    cout << "Total number of political news articles: " << totalPoliticalNews << endl;
    cout << "Total number of political news articles in 2016: " << fakePoliticalNews2016 << endl;
    // If total news is greater than 0
    if (totalPoliticalNews > 0) {
        // Calculate the percentage of fake news articles in 2016
        double percentage = (fakePoliticalNews2016 / totalPoliticalNews) * 100;
        cout << "Percentage of fake news articles in 2016: " << percentage << "%" << endl;
    } else { // No political news articles found
        cout << "No political news articles found" << endl;
    }
}

/**
 * Calculate memory usage for a single node
 * @param node Pointer to News node
 * @return Memory usage in bytes
 */
size_t calculateNodeMemory(News* node) {
    if (!node) return 0;
    
    size_t memory = sizeof(News);  // Struct size
    // String content sizes
    memory += node -> title.capacity();
    memory += node -> text.capacity();
    memory += node -> subject.capacity();
    memory += node -> date.capacity();
    memory += node -> identify.capacity();
    // Pointer sizes
    memory += sizeof(News*) * 2;  // next and head pointers
    
    return memory;
}

/**
 * Calculate detailed memory statistics for a linked list
 * @param head Pointer to head of linked list
 * @return MemoryStats struct with detailed memory information
 */
MemoryStats calculateDetailedMemory(News* head) {
    MemoryStats stats;
    News* current = head;
    
    while (current) {
        stats.structSize += sizeof(News);
        stats.stringSize += current -> title.capacity() +
                            current -> text.capacity() +
                            current -> subject.capacity() +
                            current -> date.capacity() +
                            current -> identify.capacity();
        stats.pointerSize += sizeof(News*) * 2;  // next and head pointers
        current = current->next;
    }
    
    stats.totalSize = stats.structSize + stats.stringSize + stats.pointerSize;
    return stats;
}

/**
 * Display memory statistics in human-readable format
 * @param stats MemoryStats struct containing memory information
 * @param operationName Name of the operation being measured
 */
void displayMemoryStats(const MemoryStats& stats, const string& operationName) {
    cout << "\nMemory Usage for " << operationName << ":" << endl;
    cout << "Structure Size: " << stats.structSize << " B" << endl;
    cout << "String Content: " << stats.stringSize << " B" << endl;
    cout << "Pointer Overhead: " << stats.pointerSize << " B" << endl;
    cout << "Total Memory: " << stats.totalSize << " B" << endl;
    if (stats.timeElapsed > 0) {
        cout << "Time Elapsed: " << stats.timeElapsed << " seconds" << endl;
    }
    cout << "----------------------------------------" << endl;
}

/**
 * Write the linked list to a CSV file
 * @param filename The name of the CSV file
 * @param newsBook The linked list of news
 */
void writeCSV(const string& filename, News* newsBook) {
    ofstream file(filename);
    if (!file.is_open()) { cerr << "Error opening file: " << filename << endl; return; }

    // Write the data
    while (newsBook) {
        file << "\"" << newsBook -> title << "\","
            << "\"" << newsBook -> text << "\","
            << "\"" << newsBook -> subject << "\","
            << "\"" << newsBook -> date << "\","
            << "\"" << newsBook -> identify << "\"\n";
        newsBook = newsBook -> next;
    }

    file.close();
    cout << "Data written to " << filename << " successfully!" << endl;
}

int main(int argc, char const *argv[]) {
    News* newsBook = nullptr;
    News* news = new News;

    ifstream infile("DataCleaned.csv"); // Open file for reading

    // Check the file open successfully
    if (!infile) { cerr << "Error opening file" << endl; return 1; }

    cout << "Loading news into Linked List..." << endl;
    preloadNews(infile, newsBook); // Preload news into linked list
    cout << "News loaded successfully!\n" << endl;
    infile.close(); // Close the file

/**
 * 1. How can you efficiently sort the news articles by year and display the total number of articles in both datasets?
 * 2. What percentage of political news articles (including fake and true news) from the year of 2016 are fake? 
    (Hint: To answer this, you will need to search through both datasets and analyse the date and category of each article.)
 * 3. Which words are most frequently used in fake news articles related to government topics?
    (Hint: You are required to extract the most common words from these articles, sort them by frequency, and present the results.)
 */

    bool running = true;
    while (running) {
        // User Menu
        cout << "<<< User Menu >>>" << endl;
        cout << "1. Sort By Year and display the total number of articles in both datasets" << endl;
        cout << "2. Percentage of political news article (including fake and true news) from year 2016 are fake" << endl;
        cout << "3. Most frequently word used in fake news article related to government topics" << endl;

        // User Input
        int choice;
        cout << "\nEnter your choice: ";
        cin >> choice;
        switch (choice) {
            // Sort by year
            case 1: {
                cout << "\nSorting by year..." << endl;
                auto startMem_sort = calculateDetailedMemory(newsBook);
                auto timeStart_sort = high_resolution_clock::now();
                // Sort by year and display the total number of articles
                quickSort(&newsBook);
                cout << "Done sorting" << endl;
                iterativeCount(newsBook);
                MemoryStats stats_sort = calculateDetailedMemory(newsBook);
                auto timeEnd_sort = high_resolution_clock::now();
                stats_sort.timeElapsed = duration<double>(timeEnd_sort - timeStart_sort).count();
                displayMemoryStats(stats_sort, "Sorting by Year");
                writeCSV("SortedNews.csv", newsBook);
                break;
            }

            // Percentage of political news article (including fake and true news) from year 2016 are fake
            case 2: {
                cout << "Calculating political news article..." << endl;
                auto startMem_calcPol = calculateDetailedMemory(newsBook);
                auto timeStart_calcPol = chrono::high_resolution_clock::now();
                countPoliticNews(&newsBook);
                MemoryStats stats_calcPol = calculateDetailedMemory(newsBook);
                auto timeEnd_calcPol = chrono::high_resolution_clock::now();
                stats_calcPol.timeElapsed = chrono::duration<double>(timeEnd_calcPol - timeStart_calcPol).count();
                displayMemoryStats(stats_calcPol, "Political News Calculation");
                break;
            }

            // Most frequently word used in fake news article related to government topics
            case 3: {
                // Calculate memory statistics for word frequency calculation
                auto startMem_wordFreq = calculateDetailedMemory(newsBook);
                auto timeStart_wordFreq = chrono::high_resolution_clock::now();

                WordList wordList;
                News* newsPtr = newsBook;
                while (newsPtr) {
                    string subjectLower;
                    for (char c : newsPtr -> subject) {
                        subjectLower.push_back(tolower(c));
                    }
                    // `
                    if (newsPtr -> identify =="FAKE" && (subjectLower.find("government") != string::npos)) {
                        // Convert all text to lowercase
                        string titleLower, textLower, subjectLower;
                        for (char c : newsPtr -> title) {
                            titleLower.push_back(tolower(c));
                        }

                        for (char c : newsPtr -> text) {
                            textLower.push_back(tolower(c));
                        }

                        for (char c : newsPtr -> subject) {
                            subjectLower.push_back(tolower(c));
                        }

                        stringstream ss(titleLower);
                        string word;
                        while (ss >> word) {
                            string cleaned = cleanWord(word);
                            if (!cleaned.empty()) {
                                wordList.insertOrUpdate(cleaned);
                            }
                        }

                        // Convert text to lowercase and split into words
                        stringstream sss(textLower);
                        while (sss >> word) {
                            string cleaned = cleanWord(word);
                            if (!cleaned.empty()) {
                                wordList.insertOrUpdate(cleaned);
                            }
                        }
                    }
                    newsPtr = newsPtr -> next;
                }
                wordList.quickSort_word_freq();

                // Calculate memory statistics for word frequency calculation
                MemoryStats stats_wordFreq = calculateDetailedMemory(newsBook);
                auto timeEnd_wordFreq = chrono::high_resolution_clock::now();
                stats_wordFreq.timeElapsed = chrono::duration<double>(timeEnd_wordFreq - timeStart_wordFreq).count();

                // User input to display the top N most frequent words
                int topCount;
                cout << "\nEnter the number of most frequent words to display: ";
                cin >> topCount;

                if (topCount > 0) {
                    cout << "\nTop " << topCount << " most frequent words:" << endl;
                    cout << "------------------------------------------" << endl;
                    int count = 0;
                    WordNode* wordPtr = wordList.head;
                    while (wordPtr && count < topCount) {
                        cout << count + 1 << ". " << wordPtr -> word << " (" << wordPtr -> frequency << "times)" << endl;
                        wordPtr = wordPtr -> next;
                        count++;
                    }
                }

                // Display memory statistics for word frequency calculation
                displayMemoryStats(stats_wordFreq, "Word Frequency Calculation");

                break;
            }

            // Invalid Choice
            default: cout << "Invalid choice" << endl; break;
        }

        if (running && choice >= 1 && choice <= 4) {
            cout << "\nPress Enter to return to menu...";
            cin.ignore();   // Ignore newline character
            cin.get();      // Wait for user input
        }
    }

    // Free up memory
    delete news;
    return 0;
}
