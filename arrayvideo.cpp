#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <chrono>
#include <functional>   // for std::ref
#include <utility>      // for std::forward
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
// measureEfficiency: Measures the time taken by a function.
// ----------------------------------------------------------------
template<typename Func, typename... Args>
long long measureEfficiency(const string& operationName, Func func, Args&&... args) {
    cout << "Starting " << operationName << endl; // Debug statement
    auto start = high_resolution_clock::now();
    func(forward<Args>(args)...);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    cout << "Completed " << operationName << endl; // Debug statement
    return duration.count();
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
    
    // Dynamically allocate temporary arrays for left and right halves
    News* L = new News[n1];
    News* R = new News[n2];
    
    // Copy data to temporary arrays
    for (int i = 0; i < n1; i++)
        L[i] = articles[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = articles[mid + 1 + j];
    
    // Merge the temporary arrays back into articles[left...right]
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
    
    // Copy the remaining elements of L[], if any
    while (i < n1) {
        articles[k] = L[i];
        i++;
        k++;
    }
    
    // Copy the remaining elements of R[], if any
    while (j < n2) {
        articles[k] = R[j];
        j++;
        k++;
    }
    
    // Free the temporary arrays
    delete[] L;
    delete[] R;
}

// ----------------------------------------------------------------
// Merge sort function for News array (by year)
// ----------------------------------------------------------------
void mergeSort(News *articles, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        // Sort first and second halves recursively
        mergeSort(articles, left, mid);
        mergeSort(articles, mid + 1, right);
        // Merge the sorted halves
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
// Option 2: Counting Articles
// ----------------------------------------------------------------
void countArticlesRecursive(News *articles, int count, int index, int &fakeCount, int &trueCount) {
    cout << "Recursive count at index: " << index << endl; // Debug statement
    if (index >= count) return;
    if (articles[index].isTrue)
        trueCount++;
    else
        fakeCount++;
    countArticlesRecursive(articles, count, index + 1, fakeCount, trueCount);
}

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
// Option 4: Fake Political News Percentage by Month (2016)
// ----------------------------------------------------------------
void percentageByMonthLinear(News *articles, int count) {
    int total[13] = {0};  // months 1..12
    int fake[13] = {0};
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
            double percentage = (static_cast<double>(fake[m]) / total[m]) * 100;
            cout << "Month " << m << ": " << percentage << "%" << endl;
        } else {
            cout << "Month " << m << ": No data" << endl;
        }
    }
}

void percentageByMonthSorting(News *articles, int count) {
    int filteredCount = 0;
    News* filtered = new News[count]; // worst-case size
    for (int i = 0; i < count; i++) {
        if (articles[i].year == 2016 && articles[i].subject.find("politics") != string::npos) {
            filtered[filteredCount++] = articles[i];
        }
    }
    auto getMonth = [](const News &article) -> int {
        if (article.date.size() >= 5) {
            try {
                return stoi(article.date.substr(3, 2));
            } catch(...) {
                return 0;
            }
        }
        return 0;
    };
    for (int i = 0; i < filteredCount - 1; i++) {
        for (int j = 0; j < filteredCount - i - 1; j++) {
            if (getMonth(filtered[j]) > getMonth(filtered[j+1])) {
                swap(filtered[j], filtered[j+1]);
            }
        }
    }
    cout << "=== Percentage for Fake Political News by Month (Sorting & Grouping) ===" << endl;
    int i = 0;
    while (i < filteredCount) {
        int month = getMonth(filtered[i]);
        int total = 0;
        int fakeCount = 0;
        while (i < filteredCount && getMonth(filtered[i]) == month) {
            total++;
            if (!filtered[i].isTrue)
                fakeCount++;
            i++;
        }
        double percentage = (static_cast<double>(fakeCount) / total) * 100;
        cout << "Month " << month << ": " << percentage << "%" << endl;
    }
    delete[] filtered;
}

// ----------------------------------------------------------------
// Main Menu Loop
// ----------------------------------------------------------------
int main() {
    News* articles = nullptr;
    int count = loadArticles("DataCleaned.csv", articles);
    if (count == 0) {
        cerr << "No articles loaded." << endl;
        return 1;
    }
    
    int mainChoice = 0;
    do {
        cout << "\n==================== MAIN MENU ====================" << endl;
        cout << "1. Sort the news articles by year" << endl;
        cout << "2. Calculate total articles (fake and true)" << endl;
        cout << "3. Search article by year" << endl;
        cout << "4. Display percentage for fake political news by month in 2016" << endl;
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
            for (int i = 0; i < count; i++)
                articlesCopy[i] = articles[i];
            long long timeTaken = 0;
            size_t memoryUsed = count * sizeof(News);
            
            switch (sortChoice) {
                case 1:
                    timeTaken = measureEfficiency("Merge Sort", mergeSort, articlesCopy, 0, count - 1);
                    cout << "\n=== Sorted Articles using Merge Sort ===" << endl;
                    break;
                case 2:
                    recursionDepth = 0; // reset recursion depth
                    timeTaken = measureEfficiency("Quick Sort", quickSort, articlesCopy, 0, count - 1);
                    cout << "\n=== Sorted Articles using Quick Sort ===" << endl;
                    cout << "Quick Sort Recursion Depth: " << recursionDepth << endl;
                    break;
                case 3:
                    timeTaken = measureEfficiency("Insertion Sort", insertionSort, articlesCopy, count);
                    cout << "\n=== Sorted Articles using Insertion Sort ===" << endl;
                    break;
                default:
                    cout << "Invalid sorting option." << endl;
                    delete[] articlesCopy;
                    continue;
            }
            
            traverseAndCountArticles(articlesCopy, count);
            cout << "Time Taken: " << timeTaken << " µs" << endl;
            cout << "Memory Used: " << memoryUsed << " bytes" << endl;
            delete[] articlesCopy;
            

        } else if (mainChoice == 2) {
            cout << "Starting option 2..." << endl;
        
            int fakeCountRec = 0, trueCountRec = 0;
            cout << "Before Recursive Count" << endl;
            long long timeRec = measureEfficiency("Recursive Count", countArticlesIterative, articles, count, 
                                                    std::ref(fakeCountRec), std::ref(trueCountRec));
            cout << "After Recursive Count" << endl;
            cout << "Recursive count completed." << endl;
        
            int fakeCountIter = 0, trueCountIter = 0;
            cout << "Before Iterative Count" << endl;
            long long timeIter = measureEfficiency("Iterative Count", countArticlesIterative, articles, count, 
                                                     std::ref(fakeCountIter), std::ref(trueCountIter));
            cout << "After Iterative Count" << endl;
            cout << "Iterative count completed." << endl;
        
            cout << "\n=== Total Articles Count ===" << endl;
            cout << "Recursive Count: Total Articles: " << count 
                 << ", TRUE: " << trueCountRec << ", FAKE: " << fakeCountRec 
                 << ", Time: " << timeRec << " µs" << endl;
            cout << "Iterative Count: Total Articles: " << count 
                 << ", TRUE: " << trueCountIter << ", FAKE: " << fakeCountIter 
                 << ", Time: " << timeIter << " µs" << endl;
            
            
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
            long long timeLinear = duration_cast<microseconds>(endLinear - startLinear).count();
            if (matchCountLinear == 0) {
                cout << "No articles found for year " << targetYear << endl;
            } else {
                cout << "Found " << matchCountLinear << " articles:" << endl;
                for (int i = 0; i < matchCountLinear; i++) {
                    int idx = linearIndices[i];
                    cout << "Title: " << articles[idx].title << ", Year: " << articles[idx].year << endl;
                }
            }
            cout << "Linear Search Time: " << timeLinear << " µs" << endl;
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
            long long timeBinary = duration_cast<microseconds>(endBinary - startBinary).count();
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
                cout << "Binary Search Time: " << timeBinary << " µs" << endl;
                cout << "Linear Search Time: " << timeLinear << " µs" << endl;
                delete[] binaryIndices;
            }
            delete[] articlesSorted;
            
        } else if (mainChoice == 4) {
            long long timeLinear = measureEfficiency("Percentage by Month (Linear)", percentageByMonthLinear, articles, count);
            long long timeSorting = measureEfficiency("Percentage by Month (Sorting)", percentageByMonthSorting, articles, count);
            cout << "\nTime Taken (Linear Scan): " << timeLinear << " µs" << endl;
            cout << "Time Taken (Sorting & Grouping): " << timeSorting << " µs" << endl;
            
        } else if (mainChoice == 5) {
            cout << "Exiting program." << endl;
        } else {
            cout << "Invalid option. Please try again." << endl;
        }
        
    } while (mainChoice != 5);
    
    delete[] articles;
    return 0;
}
