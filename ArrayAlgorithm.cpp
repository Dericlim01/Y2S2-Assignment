#include <iostream>
#include <fstream>
#include <sstream>
#include <cctype> // Required for isdigit()
#include <chrono> // For execution time measurement
using namespace std;
using namespace std::chrono;

// Structure to store news articles
struct NewsArticle {
    string title;
    string text;
    string subject;
    string date;
    string label;
};

// Function to extract year from date
int extractYear(const string& date) {
    return stoi(date.substr(0, 4));
}

// Function to validate date format (YYYY-MM-DD)
bool isValidDateFormat(const string& date) {
    if (date.length() != 10) return false;
    cout << date;
    if (date[4] != '-' || date[7] != '-') return false;
    for (int i = 0; i < 4; ++i) if (!isdigit(date[i])) return false;
    for (int i = 5; i < 7; ++i) if (!isdigit(date[i])) return false;
    for (int i = 8; i < 10; ++i) if (!isdigit(date[i])) return false;
    return true;
}

// Function to read CSV file and store only valid rows
int readCSV(const string &filename, NewsArticle articles[], int maxSize) {
    ifstream file(filename);
    string line;
    int count = 0;
    
    if (!file.is_open()) {
        cerr << "Error opening file!" << endl;
        return 0;
    }
    
    getline(file, line); // Skip header
    while (getline(file, line) && count < maxSize) {
        stringstream ss(line);
        string temp[5];
        int colCount = 0;
        
        while (colCount < 5 && getline(ss, temp[colCount], ',')) {
            colCount++;
        }
        if (colCount != 5 || !isValidDateFormat(temp[3])) {
            continue;
        }
        articles[count++] = {temp[0], temp[1], temp[2], temp[3], temp[4]};
    }
    
    file.close();
    return count;
}

// Bubble Sort by Year
void bubbleSortByYear(NewsArticle arr[], int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (extractYear(arr[j].date) > extractYear(arr[j + 1].date)) {
                swap(arr[j], arr[j + 1]);
            }
        }
    }
}

// Insertion Sort by Title
void insertionSortByTitle(NewsArticle arr[], int size) {
    for (int i = 1; i < size; i++) {
        NewsArticle key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j].title > key.title) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

// Binary Search for Title
int binarySearchByTitle(NewsArticle arr[], int size, string targetTitle) {
    int left = 0, right = size - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (arr[mid].title == targetTitle) {
            return mid; // Found
        } else if (arr[mid].title < targetTitle) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return -1; // Not found
}

// Linear Search by Year
void searchByYear(NewsArticle arr[], int size, int year) {
    cout << "\nArticles from the year " << year << ":\n";
    bool found = false;
    for (int i = 0; i < size; i++) {
        if (extractYear(arr[i].date) == year) {
            cout << "Title: " << arr[i].title << " | Date: " << arr[i].date << endl;
            found = true;
        }
    }
    if (!found) {
        cout << "No articles found for the year " << year << ".\n";
    }
}

// Function to print sorted articles
void printArticles(NewsArticle arr[], int size) {
    for (int i = 0; i < size; ++i) {
        cout << "Title: " << arr[i].title << " | Date: " << arr[i].date << endl;
    }
}

int main() {
    const int MAX_SIZE = 5000;
    NewsArticle articles[MAX_SIZE];
    
    string filename = "DataCleaned.csv";
    int size = readCSV(filename, articles, MAX_SIZE);
    
    if (size == 0) {
        cout << "No valid data found in CSV!" << endl;
        return 1;
    }
    
    // Sorting by Year (Bubble Sort)
    cout << "\nSorting by Year (Chronological Order using Bubble Sort):" << endl;
    auto start = high_resolution_clock::now();
    bubbleSortByYear(articles, size);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "Execution Time (Bubble Sort by Year): " << duration.count() << " ms\n";
    printArticles(articles, size);
    
    // Sorting by Title (Insertion Sort)
    cout << "\nSorting by Title (Alphabetical Order using Insertion Sort):" << endl;
    start = high_resolution_clock::now();
    insertionSortByTitle(articles, size);
    stop = high_resolution_clock::now();
    duration = duration_cast<milliseconds>(stop - start);
    cout << "Execution Time (Insertion Sort by Title): " << duration.count() << " ms\n";
    printArticles(articles, size);
    
    // Search Articles by Year
    int searchYear = 2016; // Replace with the desired year
    searchByYear(articles, size, searchYear);
    
    // Binary Search by Title
    string targetTitle = "Example Title"; // Replace with actual title to search
    int result = binarySearchByTitle(articles, size, targetTitle);
    if (result != -1) {
        cout << "\nArticle Found: " << articles[result].title << " | Date: " << articles[result].date << endl;
    } else {
        cout << "\nArticle not found." << endl;
    }
    
    return 0;
}
