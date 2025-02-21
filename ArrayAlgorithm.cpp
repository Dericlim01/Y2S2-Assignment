#include <windows.h>        // Windows-specific API functions
#include <psapi.h>          // Process Status API for memory information
#include <iostream>         // Input/Output stream for console I/O
#include <fstream>          // File stream for reading CSV files
#include <string>           // String class for text manipulation
#include <algorithm>        // Algorithms (e.g., transform, remove_if)
#include <cctype>           // Character classification functions (e.g., ispunct, toupper)
#include <sstream>          // String stream for parsing strings
#include <chrono>           // High-resolution clock for time measurement

using namespace std;
using namespace std::chrono;

// ---------------------------------------------------------------------------
// Structure: News
// Purpose: To hold individual news article information.
// Fields:
//    - title: The title of the article.
//    - text: The full content of the article.
//    - subject: The category/subject (e.g., politics, government news).
//    - date: Publication date in the format "DD-MM-YYYY".
//    - isTrue: Boolean flag (true if article is true, false if fake; logic inverted).
//    - year: Year extracted from the date string.
// ---------------------------------------------------------------------------
struct News {
    string title;    
    string text;     
    string subject;  
    string date;     
    bool isTrue;     
    int year;        
};

// ---------------------------------------------------------------------------
// Structure: WordFrequency
// Purpose: To store a word (token) and its frequency count.
// ---------------------------------------------------------------------------
struct WordFrequency {
    string word;  
    int count;    
};

// ---------------------------------------------------------------------------
// Global Variable: recursionDepth
// Purpose: To track the recursion depth in the Quick Sort algorithm.
// ---------------------------------------------------------------------------
int recursionDepth = 0;

// ---------------------------------------------------------------------------
// Function: getProcessMemoryUsage
// Purpose: Uses PSAPI to return the current working set size (physical memory usage)
//          of this process.
// Returns: Memory usage in bytes.
// ---------------------------------------------------------------------------
size_t getProcessMemoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize; // Return physical memory usage in bytes.
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Function: loadArticles
// Purpose: Load articles from a CSV file into a dynamically allocated array.
// Parameters:
//    - filename: Name of the CSV file.
//    - articles: Reference to a pointer that will point to the allocated News array.
// Returns: The total number of articles loaded.
// ---------------------------------------------------------------------------
int loadArticles(const string &filename, News *&articles) {
    const int MAX_ARTICLES = 50000;               // Maximum number of articles to load
    articles = new News[MAX_ARTICLES];            // Allocate array of News on the heap.
    int articleCount = 0;
    
    ifstream file(filename);                      // Open CSV file for reading.
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return 0;
    }
    
    // Skip the header line (CSV header).
    string header;
    getline(file, header);
    
    string line;
    // Loop through each line until end-of-file or MAX_ARTICLES reached.
    while (getline(file, line) && articleCount < MAX_ARTICLES) {
        // --- Multi-line record handling ---
        // Count quotes in the current line.
        int quoteCount = 0;
        for (char c : line)
            if (c == '"')
                quoteCount++;
        // If the number of quotes is odd (unbalanced), append additional lines.
        while ((quoteCount % 2) != 0 && !file.eof()) {
            string extra;
            if (!getline(file, extra))
                break;
            line += "\n" + extra;
            // Recalculate quote count after appending.
            quoteCount = 0;
            for (char c : line)
                if (c == '"')
                    quoteCount++;
        }
        
        // --- Parse the record into fields ---
        // Create a temporary dynamic array to hold fields.
        const int TEMP_FIELD_SIZE = 20;
        string* tempFields = new string[TEMP_FIELD_SIZE];
        int fieldCount = 0;
        bool inQuotes = false;
        string currentField = "";
        
        // Loop over each character to split the line by commas, while handling quotes.
        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];
            if (c == '"') {
                // If inside quotes and next character is also a quote, it's an escaped quote.
                if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    currentField.push_back('"');
                    i++; // Skip the escaped quote.
                } else {
                    inQuotes = !inQuotes; // Toggle the inQuotes flag.
                }
            } else if (c == ',' && !inQuotes) {
                // Comma outside quotes signals end of a field.
                tempFields[fieldCount++] = currentField;
                currentField = "";
            } else {
                currentField.push_back(c); // Append character to current field.
            }
        }
        // Add the final field after the loop.
        tempFields[fieldCount++] = currentField;
        
        // --- Collapse extra fields if there are more than expected ---
        // Expected CSV format: title, text, subject, date, and T/F field (5 columns).
        if (fieldCount < 5) {
            delete[] tempFields;  // Malformed record; free temporary array.
            continue;             // Skip this record.
        }
        if (fieldCount > 5) {
            // Combine any extra fields (beyond index 4) into one field.
            string combined = tempFields[4];
            for (int i = 5; i < fieldCount; i++) {
                combined += "," + tempFields[i];
            }
            tempFields[4] = combined;
            fieldCount = 5;
        }
        
        // Replace any empty fields with "NA".
        for (int i = 0; i < 5; i++) {
            if (tempFields[i].empty())
                tempFields[i] = "NA";
        }
        
        // --- Populate a News object with the parsed fields ---
        News article;
        article.title   = tempFields[0];
        article.text    = tempFields[1];
        article.subject = tempFields[2];
        article.date    = tempFields[3];
        
        // Extract the year from the date string (assumes format "DD-MM-YYYY").
        if (article.date != "NA" && article.date.size() >= 10) {
            try {
                article.year = stoi(article.date.substr(article.date.size() - 4, 4));
            } catch (...) {
                article.year = 0;
            }
        } else {
            article.year = 0;
        }
        
        // Process the True/False field (column 5).
        string tfField = tempFields[4];
        for (size_t i = 0; i < tfField.size(); i++) {
            tfField[i] = toupper(tfField[i]);    // Convert to uppercase.
        }
        // Invert the logic: if field is "FAKE", set isTrue to false; otherwise, true.
        article.isTrue = !(tfField == "FAKE");
        
        // Store the article in the articles array.
        articles[articleCount++] = article;
        
        // Free the temporary fields array.
        delete[] tempFields;
    }
    file.close();
    return articleCount;  // Return the number of articles loaded.
}

// ---------------------------------------------------------------------------
// Function: swap
// Purpose: Swap the values of two News objects.
// Parameters:
//    - a: First News object.
//    - b: Second News object.
// ---------------------------------------------------------------------------
void swap(News &a, News &b) {
    News temp = a;
    a = b;
    b = temp;
}

// ---------------------------------------------------------------------------
// Function: partition
// Purpose: Partition the array for Quick Sort using the publication year as pivot.
// Parameters:
//    - articles: Array of News.
//    - left: Left index of the subarray.
//    - right: Right index of the subarray.
// Returns: Partition index where the pivot element is placed.
// ---------------------------------------------------------------------------
int partition(News *articles, int left, int right) {
    int pivot = articles[right].year; // Choose pivot from the rightmost element.
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (articles[j].year < pivot) {
            i++;
            swap(articles[i], articles[j]);
        }
    }
    swap(articles[i + 1], articles[right]);
    return i + 1;
}

// ---------------------------------------------------------------------------
// Function: quickSort
// Purpose: Recursively sort the News array by publication year using Quick Sort.
// Parameters:
//    - articles: Array of News.
//    - left: Left index of subarray.
//    - right: Right index of subarray.
// ---------------------------------------------------------------------------
void quickSort(News *articles, int left, int right) {
    recursionDepth++; // Increment recursion depth for analysis.
    if (left < right) {
        int pi = partition(articles, left, right); // Partition the array.
        quickSort(articles, left, pi - 1);           // Recursively sort left subarray.
        quickSort(articles, pi + 1, right);          // Recursively sort right subarray.
    }
}

// ---------------------------------------------------------------------------
// Function: insertionSort
// Purpose: Sort the News array by publication year using Insertion Sort.
// Parameters:
//    - articles: Array of News.
//    - count: Number of articles.
// ---------------------------------------------------------------------------
void insertionSort(News *articles, int count) {
    for (int i = 1; i < count; i++) {
        News key = articles[i];
        int j = i - 1;
        // Shift elements that are greater than key to one position ahead.
        while (j >= 0 && articles[j].year > key.year) {
            articles[j + 1] = articles[j];
            j--;
        }
        articles[j + 1] = key;
    }
}

// ---------------------------------------------------------------------------
// Function: selectionSort
// Purpose: Sort an array of WordFrequency in descending order of frequency using Selection Sort.
// Parameters:
//    - wordFreq: Array of WordFrequency.
//    - wordCount: Number of words in the array.
// ---------------------------------------------------------------------------
void selectionSort(WordFrequency* wordFreq, int wordCount) {
    for (int i = 0; i < wordCount - 1; i++) {
        int maxIdx = i;
        // Find the index with maximum frequency in the unsorted portion.
        for (int j = i + 1; j < wordCount; j++) {
            if (wordFreq[j].count > wordFreq[maxIdx].count) {
                maxIdx = j;
            }
        }
        swap(wordFreq[i], wordFreq[maxIdx]);
    }
}

// ---------------------------------------------------------------------------
// Function: linearSearch
// Purpose: Search for a specific word in the WordFrequency array.
// Parameters:
//    - wordFreq: Array of WordFrequency.
//    - wordCount: Number of words in the array.
//    - word: Word to search for.
// Returns: Index of the word if found; otherwise, -1.
// ---------------------------------------------------------------------------
int linearSearch(WordFrequency* wordFreq, int wordCount, const string &word) {
    for (int i = 0; i < wordCount; i++) {
        if (wordFreq[i].word == word) {
            return i;
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Function: traverseAndCountArticles
// Purpose: Print each article's title and year, and count the total, fake, and true articles.
// Parameters:
//    - articles: Array of News.
//    - count: Number of articles.
// ---------------------------------------------------------------------------
void traverseAndCountArticles(News *articles, int count) {
    int fakeCount = 0;
    int trueCount = 0;

    // Iterate over each article.
    for (int i = 0; i < count; i++) {
        cout << "Title: " << articles[i].title << ", Year: " << articles[i].year << endl;
        if (articles[i].isTrue) {
            trueCount++;
        } else {
            fakeCount++;
        }
    }
    cout << "Total articles: " << count << endl;
    cout << "Total FAKE articles: " << fakeCount << endl;
    cout << "Total TRUE articles: " << trueCount << endl;
}

// ---------------------------------------------------------------------------
// Function: calculateFakePoliticalNewsPercentage
// Purpose: Calculate and display the percentage of political news articles (from 2016)
//          that are fake.
// Parameters:
//    - articles: Array of News.
//    - count: Number of articles.
// ---------------------------------------------------------------------------
void calculateFakePoliticalNewsPercentage(News *articles, int count) {
    int totalPolitics2016 = 0;
    int fakePolitics2016 = 0;

    // Process each article.
    for (int i = 0; i < count; i++) {
        // Check if article is from 2016 and subject contains "politics".
        if (articles[i].year == 2016 && articles[i].subject.find("politics") != string::npos) {
            totalPolitics2016++;
            if (!articles[i].isTrue) {
                fakePolitics2016++;
            }
        }
    }

    // Calculate and display the percentage.
    if (totalPolitics2016 > 0) {
        double percentage = (static_cast<double>(fakePolitics2016) / totalPolitics2016) * 100;
        cout << "Percentage of fake political news articles in 2016: " << percentage << "%" << endl;
    } else {
        cout << "No political news articles found for the year 2016." << endl;
    }
}

// ---------------------------------------------------------------------------
// Function: tokenize
// Purpose: Split a text string into tokens (words), remove punctuation, and convert to lowercase.
// Parameters:
//    - text: The input text to tokenize.
//    - tokens: Array to store the resulting tokens.
//    - tokenCount: Output parameter to store the number of tokens found.
// ---------------------------------------------------------------------------
void tokenize(const string &text, string* tokens, int &tokenCount) {
    stringstream ss(text);
    string word;
    tokenCount = 0;
    while (ss >> word) {
        // Remove punctuation from the word.
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        // Convert the word to lowercase.
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        tokens[tokenCount++] = word;
    }
}

// ---------------------------------------------------------------------------
// Function: insertAtLastPosition
// Purpose: Insert a word into the WordFrequency array with an initial count of 1,
//          or increment its count if it already exists.
// Parameters:
//    - wordFreq: Array of WordFrequency.
//    - wordCount: Reference to the current count of words (will be updated).
//    - word: The word to insert.
// ---------------------------------------------------------------------------
void insertAtLastPosition(WordFrequency* wordFreq, int &wordCount, const string &word) {
    wordFreq[wordCount].word = word;
    wordFreq[wordCount].count = 1;
    wordCount++;
}

// ---------------------------------------------------------------------------
// Function: findMostFrequentWords
// Purpose: Filter fake news articles with "government" in the subject, tokenize their text,
//          count word frequencies, sort them, and display the top N words.
// Parameters:
//    - articles: Array of News.
//    - count: Number of articles.
//    - topN: Number of top frequent words to display.
// ---------------------------------------------------------------------------
void findMostFrequentWords(News *articles, int count, int topN) {
    const int MAX_WORDS = 10000;
    // Dynamically allocate the WordFrequency array.
    WordFrequency* wordFreq = new WordFrequency[MAX_WORDS];
    int wordCount = 0;

    // Process each article.
    for (int i = 0; i < count; i++) {
        // Case-insensitive check for "government" in the subject.
        if (!articles[i].isTrue && articles[i].subject.find("Government News") != string::npos) {
            const int MAX_TOKENS = 10000;
            // Allocate temporary array for tokens.
            string* tokens = new string[MAX_TOKENS];
            int tokenCount;
            // Tokenize the text field of the article.
            tokenize(articles[i].text, tokens, tokenCount);
            // Update word frequencies.
            for (int j = 0; j < tokenCount; j++) {
                int index = linearSearch(wordFreq, wordCount, tokens[j]);
                if (index != -1) {
                    wordFreq[index].count++;
                } else {
                    insertAtLastPosition(wordFreq, wordCount, tokens[j]);
                }
            }
            // Free tokens array after processing.
            delete[] tokens;
        }
    }

    // Sort the WordFrequency array in descending order.
    selectionSort(wordFreq, wordCount);

    // Display the top N most frequent words.
    cout << "Top " << topN << " most frequent words in fake government news:" << endl;
    for (int i = 0; i < topN && i < wordCount; i++) {
        cout << wordFreq[i].word << ": " << wordFreq[i].count << " occurrences" << endl;
    }

    // Free the dynamically allocated WordFrequency array.
    delete[] wordFreq;
}

// ---------------------------------------------------------------------------
// Function: measureEfficiency
// Purpose: Measure the execution time of a given function.
// Parameters:
//    - operationName: A name for the operation (for logging purposes).
//    - func: The function to execute.
//    - args: Arguments to pass to the function.
// Returns: Execution time in microseconds.
// ---------------------------------------------------------------------------
template<typename Func, typename... Args>
long long measureEfficiency(const string& operationName, Func func, Args&&... args) {
    auto start = high_resolution_clock::now();           // Start time
    func(forward<Args>(args)...);                          // Execute the function
    auto end = high_resolution_clock::now();             // End time
    auto duration = duration_cast<microseconds>(end - start); // Duration in microseconds
    return duration.count();
}

// ---------------------------------------------------------------------------
// Function: main
// Purpose: Program entry point. Displays a menu to the user to perform various tasks:
//          1. Sort articles by year and display them.
//          2. Calculate the percentage of fake political news in 2016.
//          3. Perform word frequency analysis on fake government news.
//          4. Exit the program.
// ---------------------------------------------------------------------------
int main() {
    News* articles = nullptr;
    // Load articles from the CSV file into the articles array.
    int count = loadArticles("DataCleaned.csv", articles);
    if (count == 0) {
        cerr << "No articles loaded." << endl;
        return 1;
    }
    
    bool running = true;
    while (running) {
        // Display the user menu.
        cout << "\nEnter Your Option:" << endl
             << "1. Sort the news articles by year and display all articles" << endl
             << "2. Percentage of political news articles from 2016" << endl
             << "3. Most frequently words used in fake government news" << endl
             << "4. Exit" << endl
             << "Option: ";
             
        string option;
        cin >> option;
        
        // Get a baseline for process memory usage (optional).
        size_t baseMemory = getProcessMemoryUsage();
        
        if (option == "1") {
            // Option 1: Sort articles using Quick Sort.
            long long sortingTime = measureEfficiency("Quick Sort", quickSort, articles, 0, count - 1);
            double sortingTimeSec = sortingTime / 1e6;
            traverseAndCountArticles(articles, count);
        
            // Calculate memory used by the articles array.
            size_t arrayMemory = count * sizeof(News);
            // Get updated process memory usage.
            size_t processMemory = getProcessMemoryUsage();
        
            cout << "\n=== Quick Sort (All Articles) ===" << endl;
            cout << "Sorting Time: " << sortingTime << " µs (" << sortingTimeSec << " seconds)" << endl;
            cout << "Memory Used by Array (Calculated): " << arrayMemory << " bytes" << endl;
            cout << "Process Memory Usage (Working Set): " << processMemory << " bytes" << endl;
            cout << "Recursion Depth: " << recursionDepth << endl;
        
        } else if (option == "2") {
            // Option 2: Sort using Insertion Sort and calculate political news statistics.
            long long sortingTime = measureEfficiency("Insertion Sort", insertionSort, articles, count);
            double sortingTimeSec = sortingTime / 1e6;
            calculateFakePoliticalNewsPercentage(articles, count);
        
            size_t arrayMemory = count * sizeof(News);
            size_t processMemory = getProcessMemoryUsage();
        
            cout << "\n=== Insertion Sort (All Articles) ===" << endl;
            cout << "Sorting Time: " << sortingTime << " µs (" << sortingTimeSec << " seconds)" << endl;
            cout << "Memory Used by Array (Calculated): " << arrayMemory << " bytes" << endl;
            cout << "Process Memory Usage (Working Set): " << processMemory << " bytes" << endl;
        
        } else if (option == "3") {
            // Option 3: Word Frequency Analysis for fake government news.
            int topN;
            cout << "Enter the number of top frequent words to display: ";
            cin >> topN;
        
            const int MAX_WORDS = 100000;
            // Dynamically allocate an array for WordFrequency structures.
            WordFrequency* wordFreq = new WordFrequency[MAX_WORDS];
            int wordCount = 0;
        
            // Populate the word frequency array by processing each article.
            for (int i = 0; i < count; i++) {
                // Convert subject to lowercase for case-insensitive matching.
                string subjectLower = articles[i].subject;
                transform(subjectLower.begin(), subjectLower.end(), subjectLower.begin(), ::tolower);
                
                // Check if the article is fake and its subject contains "government news".
                if (!articles[i].isTrue && subjectLower.find("government news") != string::npos) {
                    const int MAX_TOKENS = 10000;
                    // Dynamically allocate an array for tokens.
                    string* tokens = new string[MAX_TOKENS];
                    int tokenCount = 0;
                    
                    // Tokenize the title field.
                    tokenize(articles[i].title, tokens, tokenCount);
                    for (int j = 0; j < tokenCount; j++) {
                        int index = linearSearch(wordFreq, wordCount, tokens[j]);
                        if (index != -1) {
                            wordFreq[index].count++;
                        } else {
                            insertAtLastPosition(wordFreq, wordCount, tokens[j]);
                        }
                    }
                    
                    // Tokenize the text field.
                    tokenize(articles[i].text, tokens, tokenCount);
                    for (int j = 0; j < tokenCount; j++) {
                        int index = linearSearch(wordFreq, wordCount, tokens[j]);
                        if (index != -1) {
                            wordFreq[index].count++;
                        } else {
                            insertAtLastPosition(wordFreq, wordCount, tokens[j]);
                        }
                    }
                    // Free the tokens array after processing the article.
                    delete[] tokens;
                }
            }
        
            // Sort the word frequency array using Selection Sort.
            long long sortingTime = measureEfficiency("Selection Sort", selectionSort, wordFreq, wordCount);
            double sortingTimeSec = sortingTime / 1e6;
        
            // Measure the time taken to display the top words.
            auto startSearch = high_resolution_clock::now();
            cout << "\nTop " << topN << " most frequent words in fake government news:" << endl;
            for (int i = 0; i < topN && i < wordCount; i++) {
                cout << wordFreq[i].word << ": " << wordFreq[i].count << " occurrences" << endl;
            }
            auto endSearch = high_resolution_clock::now();
            long long searchingTime = duration_cast<microseconds>(endSearch - startSearch).count();
            double searchingTimeSec = searchingTime / 1e6;
        
            // Calculate memory usage for the word frequency array.
            size_t arrayMemory = wordCount * sizeof(WordFrequency);
            size_t processMemory = getProcessMemoryUsage();
        
            cout << "\n=== Selection Sort (Word Frequency Analysis) ===" << endl;
            cout << "Sorting Time: " << sortingTime << " µs (" << sortingTimeSec << " seconds)" << endl;
            cout << "Searching Time: " << searchingTime << " µs (" << searchingTimeSec << " seconds)" << endl;
            cout << "Memory Used by Word Frequency Array (Calculated): " << arrayMemory << " bytes" << endl;
            cout << "Process Memory Usage (Working Set): " << processMemory << " bytes" << endl;
        
            // Free the dynamically allocated word frequency array.
            delete[] wordFreq;        
        } else if (option == "4") {
            // Option 4: Exit the program.
            cout << "Exiting program..." << endl;
            running = false;
        } else {
            // Handle invalid menu options.
            cout << "Invalid option. Please try again." << endl;
        }
    }
    
    // Clean up: Free the dynamically allocated articles array.
    delete[] articles;
    return 0;
}
