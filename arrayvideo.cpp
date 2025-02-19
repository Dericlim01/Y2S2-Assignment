#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <functional>
#include <utility>
#include <windows.h>
#include <psapi.h>

using namespace std;
using namespace std::chrono;

// ----------------------------------------------------------------
// Structures
// ----------------------------------------------------------------
struct News {
    string title;    // Title of the article
    string text;     // Full text content of the article
    string subject;  // Subject/category (e.g., politics, government news)
    string date;     // Publication date as a string ("DD-MM-YYYY")
    bool isTrue;     // true if article is true; false if fake
    int year;        // Year extracted from the date
};

struct WordFrequency {
    string word;
    int count;
};

// Global variable to track recursion depth in Quick Sort
int recursionDepth = 0;

// ----------------------------------------------------------------
// getCurrentMemoryUsage: Uses PSAPI to measure the current process memory usage.
// Returns the working set size in bytes.
// ----------------------------------------------------------------
size_t getCurrentMemoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;  // Memory in bytes
    }
    return 0;
}

// ----------------------------------------------------------------
// measurePerformance: Measures the time taken and the change in memory usage 
// by a function. It returns a pair: {time in seconds, memory used in bytes}.
// ----------------------------------------------------------------
template<typename Func, typename... Args>
pair<double, size_t> measurePerformance(const string& operationName, Func func, Args&&... args) {
    cout << "Starting " << operationName << endl;
    size_t memoryBefore = getCurrentMemoryUsage();
    auto start = high_resolution_clock::now();
    
    // Call the function (for void functions, the return value is discarded)
    func(forward<Args>(args)...);
    
    auto end = high_resolution_clock::now();
    size_t memoryAfter = getCurrentMemoryUsage();
    double timeTaken = duration<double>(end - start).count(); // time in seconds
    cout << "Completed " << operationName << endl;
    
    size_t memoryUsed = (memoryAfter > memoryBefore) ? (memoryAfter - memoryBefore) : 0;
    return {timeTaken, memoryUsed};
}

// ----------------------------------------------------------------
// loadArticles: Load articles from a CSV file into a dynamic array.
// Returns the number of articles loaded.
// ----------------------------------------------------------------
int loadArticles(const string &filename, News *&articles) {
    const int MAX_ARTICLES = 50000;
    articles = new News[MAX_ARTICLES];
    int articleCount = 0;
    
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return 0;
    }
    
    // Skip header.
    string header;
    getline(file, header);
    
    string line;
    while (getline(file, line) && articleCount < MAX_ARTICLES) {
        // --- Handle multi-line records ---
        int quoteCount = 0;
        for (char c : line)
            if (c == '"') quoteCount++;
        while ((quoteCount % 2) != 0 && !file.eof()) {
            string extra;
            if (!getline(file, extra))
                break;
            line += "\n" + extra;
            quoteCount = 0;
            for (char c : line)
                if (c == '"') quoteCount++;
        }
        
        // --- Parse CSV fields ---
        const int TEMP_FIELD_SIZE = 20;
        string* tempFields = new string[TEMP_FIELD_SIZE];
        int fieldCount = 0;
        bool inQuotes = false;
        string currentField = "";
        
        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];
            if (c == '"') {
                if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    currentField.push_back('"');
                    i++; // Skip escaped quote.
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                tempFields[fieldCount++] = currentField;
                currentField = "";
            } else {
                currentField.push_back(c);
            }
        }
        tempFields[fieldCount++] = currentField;
        
        // --- Collapse extra fields if needed ---
        if (fieldCount < 5) {
            delete[] tempFields;
            continue;
        }
        if (fieldCount > 5) {
            string combined = tempFields[4];
            for (int i = 5; i < fieldCount; i++) {
                combined += "," + tempFields[i];
            }
            tempFields[4] = combined;
            fieldCount = 5;
        }
        
        for (int i = 0; i < 5; i++) {
            if (tempFields[i].empty())
                tempFields[i] = "NA";
        }
        
        // --- Populate a News object ---
        News article;
        article.title   = tempFields[0];
        article.text    = tempFields[1];
        article.subject = tempFields[2];
        article.date    = tempFields[3];
        if (article.date != "NA" && article.date.size() >= 10) {
            try {
                article.year = stoi(article.date.substr(article.date.size() - 4, 4));
            } catch (...) {
                article.year = 0;
            }
        } else {
            article.year = 0;
        }
        string tfField = tempFields[4];
        transform(tfField.begin(), tfField.end(), tfField.begin(), ::toupper);
        article.isTrue = !(tfField == "FAKE");
        
        articles[articleCount++] = article;
        delete[] tempFields;
    }
    file.close();
    return articleCount;
}

// ----------------------------------------------------------------
// Swap two News objects.
// ----------------------------------------------------------------
void swap(News &a, News &b) {
    News temp = a;
    a = b;
    b = temp;
}

// ----------------------------------------------------------------
// Sorting Algorithms
// ----------------------------------------------------------------
void merge(News *articles, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    News* L = new News[n1];
    News* R = new News[n2];
    
    for (int i = 0; i < n1; i++)
        L[i] = articles[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = articles[mid + 1 + j];
    
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i].year <= R[j].year) {
            articles[k] = L[i];
            i++;
        } else {
            articles[k] = R[j];
            j++;
        }
        k++;
    }
    
    while (i < n1) {
        articles[k] = L[i];
        i++;
        k++;
    }
    while (j < n2) {
        articles[k] = R[j];
        j++;
        k++;
    }
    
    delete[] L;
    delete[] R;
}

void mergeSort(News *articles, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(articles, left, mid);
        mergeSort(articles, mid + 1, right);
        merge(articles, left, mid, right);
    }
}

int partition(News *articles, int left, int right) {
    int pivot = articles[right].year;
    int i = left - 1;
    for (int j = left; j < right; j++) {
        if (articles[j].year < pivot) {
            i++;
            swap(articles[i], articles[j]);
        }
    }
    swap(articles[i+1], articles[right]);
    return i + 1;
}

void quickSort(News *articles, int left, int right) {
    recursionDepth++;
    if (left < right) {
        int pi = partition(articles, left, right);
        quickSort(articles, left, pi - 1);
        quickSort(articles, pi + 1, right);
    }
}

void insertionSort(News *articles, int count) {
    for (int i = 1; i < count; i++) {
        News key = articles[i];
        int j = i - 1;
        while (j >= 0 && articles[j].year > key.year) {
            articles[j + 1] = articles[j];
            j--;
        }
        articles[j + 1] = key;
    }
}

// ----------------------------------------------------------------
// Utility Function: Display articles and count totals.
// ----------------------------------------------------------------
void traverseAndCountArticles(News *articles, int count) {
    int fakeCount = 0;
    int trueCount = 0;
    for (int i = 0; i < count; i++) {
        cout << "Title: " << articles[i].title << ", Year: " << articles[i].year << endl;
        if (articles[i].isTrue)
            trueCount++;
        else
            fakeCount++;
    }
    cout << "Total articles: " << count << endl;
    cout << "Total FAKE articles: " << fakeCount << endl;
    cout << "Total TRUE articles: " << trueCount << endl;
}

// ----------------------------------------------------------------
// Option 2: Counting Articles (Iterative Only)
// ----------------------------------------------------------------
void countArticlesIterative(News *articles, int count, int &fakeCount, int &trueCount) {
    fakeCount = 0;
    trueCount = 0;
    for (int i = 0; i < count; i++) {
        if (articles[i].isTrue)
            trueCount++;
        else
            fakeCount++;
    }
}

// ----------------------------------------------------------------
// Option 3: Searching (using dynamic arrays, no vectors)
// ----------------------------------------------------------------
int* linearSearchByYear(News *articles, int count, int targetYear, int &matchCount) {
    matchCount = 0;
    for (int i = 0; i < count; i++) {
        if (articles[i].year == targetYear)
            matchCount++;
    }
    if (matchCount == 0)
        return nullptr;
    int* indices = new int[matchCount];
    int j = 0;
    for (int i = 0; i < count; i++) {
        if (articles[i].year == targetYear)
            indices[j++] = i;
    }
    return indices;
}

int binarySearchByYear(News *articles, int left, int right, int targetYear) {
    if (left > right)
        return -1;
    int mid = left + (right - left) / 2;
    if (articles[mid].year == targetYear)
        return mid;
    else if (articles[mid].year < targetYear)
        return binarySearchByYear(articles, mid + 1, right, targetYear);
    else
        return binarySearchByYear(articles, left, mid - 1, targetYear);
}

// ----------------------------------------------------------------
// Option 4: Percentage for Fake Political News by Month (Linear Only)
// ----------------------------------------------------------------
void percentageByMonthLinear(News *articles, int count) {
    int total[13] = {0};  // months 1..12
    int fake[13] = {0};

    // Count total/fake for political news in 2016
    for (int i = 0; i < count; i++) {
        if (articles[i].year == 2016 && articles[i].subject.find("politics") != string::npos) {
            string monthStr = articles[i].date.substr(3, 2);
            int month = stoi(monthStr);
            total[month]++;
            if (!articles[i].isTrue)
                fake[month]++;
        }
    }

    cout << "=== Percentage for Fake Political News by Month (Linear Scan) ===" << endl;
    for (int m = 1; m <= 12; m++) {
        if (total[m] > 0) {
            double percentage = (static_cast<double>(fake[m]) / total[m]) * 100.0;
            int starCount = static_cast<int>(percentage);
            cout << "Month " << m << ": ";
            for (int s = 0; s < starCount; s++) {
                cout << "*";
            }
            cout << " " << fixed << setprecision(4) << percentage << "%" << endl;
        } else {
            cout << "Month " << m << ": No data" << endl;
        }
    }
}

// ----------------------------------------------------------------
// Main Menu Loop
// ----------------------------------------------------------------
int main() {
    News* articles = nullptr;
    int count = loadArticles("DataCleaned.txt", articles);
    if (count == 0) {
        cerr << "No articles loaded." << endl;
        return 1;
    }
    
    int mainChoice = 0;
    do {
        cout << "\n==================== MAIN MENU ====================" << endl;
        cout << "1. Sort the news articles by year" << endl;
        cout << "2. Calculate total articles (fake and true) (Iterative Count Only)" << endl;
        cout << "3. Search article by year" << endl;
        cout << "4. Display percentage for fake political news by month in 2016 (Linear Scan)" << endl;
        cout << "5. Exit" << endl;
        cout << "Enter your option: ";
        cin >> mainChoice;
        
        if (mainChoice == 1) {
            int sortChoice = 0;
            cout << "\n--- Choose Sorting Algorithm ---" << endl;
            cout << "1. Merge Sort" << endl;
            cout << "2. Quick Sort" << endl;
            cout << "3. Insertion Sort" << endl;
            cout << "Enter your choice: ";
            cin >> sortChoice;
            
            // Create a temporary copy for sorting
            News *articlesCopy = new News[count];
            string sortName;
            for (int i = 0; i < count; i++)
                articlesCopy[i] = articles[i];
                
            pair<double, size_t> result; // {time in seconds, memory used in bytes}
            
            switch (sortChoice) {
                case 1:
                    result = measurePerformance("Merge Sort", mergeSort, articlesCopy, 0, count - 1);
                    cout << "\n=== Sorted Articles using Merge Sort ===" << endl;
                    sortName = "Merge Sort";
                    break;
                case 2:
                    recursionDepth = 0; // reset recursion depth
                    result = measurePerformance("Quick Sort", quickSort, articlesCopy, 0, count - 1);
                    cout << "\n=== Sorted Articles using Quick Sort ===" << endl;
                    cout << "Quick Sort Recursion Depth: " << recursionDepth << endl;
                    sortName = "Quick Sort";
                    break;
                case 3:
                    result = measurePerformance("Insertion Sort", insertionSort, articlesCopy, count);
                    cout << "\n=== Sorted Articles using Insertion Sort ===" << endl;
                    sortName = "Insertion Sort";
                    break;
                default:
                    cout << "Invalid sorting option." << endl;
                    delete[] articlesCopy;
                    continue;
            }
            
            traverseAndCountArticles(articlesCopy, count);
            cout << "==================== " << sortName << " ====================" << endl;
            cout << "Time Taken: " << result.first << " s" << endl;
            cout << "Memory Used: " << result.second << " bytes" << endl;
            delete[] articlesCopy;
            
        } else if (mainChoice == 2) {
            cout << "Starting Option 2 (Iterative Count)..." << endl;
            int fakeCountIter = 0, trueCountIter = 0;
            auto iterResult = measurePerformance("Iterative Count", countArticlesIterative, articles, count, std::ref(fakeCountIter), std::ref(trueCountIter));
            cout << "\n=== Total Articles Count (Iterative) ===" << endl;
            cout << "Total Articles: " << count 
                 << ", TRUE: " << trueCountIter << ", FAKE: " << fakeCountIter 
                 << ", Time: " << iterResult.first << " s, Memory Diff: " << iterResult.second << " bytes" << endl;
            
        } else if (mainChoice == 3) {
            int targetYear;
            cout << "Enter the year to search for: ";
            cin >> targetYear;
            
            // Linear Search
            cout << "\n=== Linear Search Results ===" << endl;
            int matchCountLinear = 0;
            auto startLinear = high_resolution_clock::now();
            int* linearIndices = linearSearchByYear(articles, count, targetYear, matchCountLinear);
            auto endLinear = high_resolution_clock::now();
            double timeLinear = duration<double>(endLinear - startLinear).count();
            if (matchCountLinear == 0) {
                cout << "No articles found for year " << targetYear << endl;
            } else {
                cout << "Found " << matchCountLinear << " articles:" << endl;
                for (int i = 0; i < matchCountLinear; i++) {
                    int idx = linearIndices[i];
                    cout << "Title: " << articles[idx].title << ", Year: " << articles[idx].year << endl;
                }
            }
            cout << "Linear Search Time: " << timeLinear << " s" << endl;
            delete[] linearIndices;
            
            // Binary Search (requires sorted copy)
            News *articlesSorted = new News[count];
            for (int i = 0; i < count; i++)
                articlesSorted[i] = articles[i];
            recursionDepth = 0;
            quickSort(articlesSorted, 0, count - 1);
            auto startBinary = high_resolution_clock::now();
            int foundIndex = binarySearchByYear(articlesSorted, 0, count - 1, targetYear);
            auto endBinary = high_resolution_clock::now();
            double timeBinary = duration<double>(endBinary - startBinary).count();
            if (foundIndex == -1) {
                cout << "\n=== Binary Search Results (on sorted array) ===" << endl;
                cout << "No articles found for year " << targetYear << endl;
            } else {
                int left = foundIndex;
                while (left >= 0 && articlesSorted[left].year == targetYear)
                    left--;
                left++;
                int right = foundIndex;
                while (right < count && articlesSorted[right].year == targetYear)
                    right++;
                int matchCountBinary = right - left;
                int* binaryIndices = new int[matchCountBinary];
                for (int i = 0; i < matchCountBinary; i++) {
                    binaryIndices[i] = left + i;
                }
                cout << "\n=== Binary Search Results (on sorted array) ===" << endl;
                cout << "Found " << matchCountBinary << " articles:" << endl;
                for (int i = 0; i < matchCountBinary; i++) {
                    int idx = binaryIndices[i];
                    cout << "Title: " << articlesSorted[idx].title << ", Year: " << articlesSorted[idx].year << endl;
                }
                cout << "Binary Search Time: " << timeBinary << " s" << endl;
                cout << "Linear Search Time: " << timeLinear << " s" << endl;
                delete[] binaryIndices;
            }
            delete[] articlesSorted;
            
        } else if (mainChoice == 4) {
            auto linearRes = measurePerformance("Percentage by Month (Linear)", percentageByMonthLinear, articles, count);
            cout << "\n=== Fake Political News Percentage by Month (Linear Scan) ===" << endl;
            cout << "Time Taken: " << linearRes.first << " s, Memory Diff: " << linearRes.second << " bytes" << endl;
            
        } else if (mainChoice == 5) {
            cout << "Exiting program." << endl;
        } else {
            cout << "Invalid option. Please try again." << endl;
        }
        
    } while (mainChoice != 5);
    
    delete[] articles;
    return 0;
}
