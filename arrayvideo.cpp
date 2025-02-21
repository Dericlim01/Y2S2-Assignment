#include <iostream>         // For cout, cin, cerr, etc.
#include <fstream>          // For file input/output operations
#include <string>           // To use the std::string class
#include <algorithm>        // For transform(), sort(), etc.
#include <cctype>           // For character functions such as isdigit() and ::toupper
#include <sstream>          // For stringstream to parse strings
#include <iomanip>          // For formatted output (e.g., setprecision, fixed)
#include <chrono>           // For high-resolution clock to measure execution time
#include <functional>       // For std::function if needed
#include <utility>          // For std::pair and std::move
#include <windows.h>        // For Windows API functions (e.g., GetCurrentProcess())
#include <psapi.h>          // For process memory usage functions

using namespace std;
using namespace std::chrono;

// ----------------------------------------------------------------
// Structure Definitions
// ----------------------------------------------------------------

// The News structure holds information for each news article.
// Each article has a title, text content, subject category,
// a publication date (in "DD-MM-YYYY" format), a boolean indicating
// whether the article is true (if false then it is fake),
// and an integer representing the publication year extracted from the date.
struct News {
    string title;    // The headline or title of the news article
    string text;     // The full body text of the article
    string subject;  // The category or subject (e.g., politics, government news)
    string date;     // Publication date as a string in the format "DD-MM-YYYY"
    bool isTrue;     // Boolean flag: true if the article is true; false if fake
    int year;        // Year extracted from the date string
};

// The WordFrequency structure is designed to store a word and its frequency count.
// This can be used when analyzing word occurrences in news articles.
struct WordFrequency {
    string word;   // The word or token extracted from text
    int count;     // The number of times the word appears
};

// Global variable used to track recursion depth during Quick Sort.
// This is useful for analyzing performance and understanding the recursion tree.
int recursionDepth = 0;

// ----------------------------------------------------------------
// getCurrentMemoryUsage Function
// ----------------------------------------------------------------
// This function uses Windows PSAPI to get the current memory usage of the process.
// It returns the working set size (physical memory usage) in bytes.
size_t getCurrentMemoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc; 
    // Get current process memory info and check if successful
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;  // Return the working set size in bytes.
    }
    return 0; // In case of failure, return 0.
}

// ----------------------------------------------------------------
// measurePerformance Template Function
// ----------------------------------------------------------------
// This template function measures the performance (time taken and memory usage)
// of any function passed to it along with its arguments.
// It prints the start and completion messages, and then returns a pair:
// {timeTaken in seconds, memoryUsed in bytes}.
template<typename Func, typename... Args>
pair<double, size_t> measurePerformance(const string& operationName, Func func, Args&&... args) {
    cout << "Starting " << operationName << endl;
    // Capture memory usage before executing the function
    size_t memoryBefore = getCurrentMemoryUsage();
    // Record the starting time using a high resolution clock
    auto start = high_resolution_clock::now();
    
    // Call the function with the forwarded arguments
    func(forward<Args>(args)...);
    
    // Record the ending time
    auto end = high_resolution_clock::now();
    // Capture memory usage after function execution
    size_t memoryAfter = getCurrentMemoryUsage();
    // Calculate the time difference in seconds
    double timeTaken = duration<double>(end - start).count();
    cout << "Completed " << operationName << endl;
    
    // Calculate the memory difference (only if memory increased)
    size_t memoryUsed = (memoryAfter > memoryBefore) ? (memoryAfter - memoryBefore) : 0;
    return {timeTaken, memoryUsed}; // Return the performance metrics.
}

// ----------------------------------------------------------------
// loadArticles Function
// ----------------------------------------------------------------
// This function loads news articles from a CSV file into a dynamically allocated array.
// It returns the number of articles loaded and assigns the allocated array to 'articles'.
// It handles multi-line records and quoted fields.
int loadArticles(const string &filename, News *&articles) {
    const int MAX_ARTICLES = 50000;   // Maximum articles to load (limit to prevent excessive memory use)
    articles = new News[MAX_ARTICLES]; // Allocate an array of News objects on the heap
    int articleCount = 0;
    
    ifstream file(filename);  // Open the CSV file for reading
    if (!file) {              // Check if file opening failed
        cerr << "Error opening file: " << filename << endl;
        return 0;
    }
    
    // Skip the CSV header line
    string header;
    getline(file, header);
    
    string line;
    // Read file line by line until end-of-file or MAX_ARTICLES is reached.
    while (getline(file, line) && articleCount < MAX_ARTICLES) {
        // --- Handle multi-line records ---
        // Count the number of quotes in the line.
        int quoteCount = 0;
        for (char c : line)
            if (c == '"') 
                quoteCount++;
        // If the number of quotes is odd, the record is not complete.
        // Append additional lines until the quotes are balanced.
        while ((quoteCount % 2) != 0 && !file.eof()) {
            string extra;
            if (!getline(file, extra))
                break;
            line += "\n" + extra;  // Append the extra line to the current line
            quoteCount = 0;        // Reset quote count and re-calculate
            for (char c : line)
                if (c == '"')
                    quoteCount++;
        }
        
        // --- Parse CSV fields from the line ---
        // Create a temporary dynamic array to hold fields.
        const int TEMP_FIELD_SIZE = 20;
        string* tempFields = new string[TEMP_FIELD_SIZE];
        int fieldCount = 0;
        bool inQuotes = false;  // Flag to indicate if we're inside a quoted field
        string currentField = "";
        
        // Iterate over each character in the line
        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];
            // Check if character is a quote
            if (c == '"') {
                // If we are already in quotes and the next character is also a quote,
                // then this is an escaped quote.
                if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    currentField.push_back('"'); // Append one quote
                    i++; // Skip the next quote
                } else {
                    // Toggle the inQuotes flag if a quote is encountered.
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                // A comma outside quotes means the current field is complete.
                tempFields[fieldCount++] = currentField;
                currentField = "";  // Reset field accumulator
            } else {
                // Otherwise, add the character to the current field.
                currentField.push_back(c);
            }
        }
        // Add the last field (after the loop completes).
        tempFields[fieldCount++] = currentField;
        
        // --- Handle records with extra fields ---
        // We expect exactly 5 fields: title, text, subject, date, and a true/false field.
        if (fieldCount < 5) {
            delete[] tempFields; // Free temporary memory if record is malformed
            continue; // Skip this record
        }
        if (fieldCount > 5) {
            // If there are more than 5 fields, combine the extra fields into field 5.
            string combined = tempFields[4];
            for (int i = 5; i < fieldCount; i++) {
                combined += "," + tempFields[i];
            }
            tempFields[4] = combined;
            fieldCount = 5;
        }
        
        // Replace any empty field with "NA" to ensure data consistency.
        for (int i = 0; i < 5; i++) {
            if (tempFields[i].empty())
                tempFields[i] = "NA";
        }
        
        // --- Create and populate a News object ---
        News article;
        article.title   = tempFields[0]; // Article title
        article.text    = tempFields[1]; // Article text
        article.subject = tempFields[2]; // Article subject/category
        article.date    = tempFields[3]; // Publication date
        // Extract the publication year from the date string.
        if (article.date != "NA" && article.date.size() >= 10) {
            try {
                // Assumes the last 4 characters of the date string represent the year.
                article.year = stoi(article.date.substr(article.date.size() - 4, 4));
            } catch (...) {
                article.year = 0; // Fallback if conversion fails
            }
        } else {
            article.year = 0; // Default to 0 if date is "NA" or too short.
        }
        // Process the True/False field (fifth field)
        string tfField = tempFields[4];
        // Convert the field to uppercase to standardize comparison.
        transform(tfField.begin(), tfField.end(), tfField.begin(), ::toupper);
        // If the field equals "FAKE", then isTrue is false; otherwise, it is true.
        article.isTrue = !(tfField == "FAKE");
        
        // Store the article in the dynamic array.
        articles[articleCount++] = article;
        // Free the temporary array used for parsing this record.
        delete[] tempFields;
    }
    file.close(); // Close the CSV file
    return articleCount; // Return the total number of articles loaded.
}

// ----------------------------------------------------------------
// swap Function
// ----------------------------------------------------------------
// This function swaps two News objects.
// It is used by the sorting algorithms.
void swap(News &a, News &b) {
    News temp = a;
    a = b;
    b = temp;
}

// ----------------------------------------------------------------
// Sorting Algorithms: Quick Sort and Insertion Sort
// ----------------------------------------------------------------

// partition function used by Quick Sort.
// It partitions the subarray (from index 'left' to 'right') based on the pivot (article year).
// All articles with a year less than the pivot are moved to the left.
int partition(News *articles, int left, int right) {
    int pivot = articles[right].year;  // Use the rightmost element as the pivot
    int i = left - 1;                  // Index of the smaller element
    for (int j = left; j < right; j++) {
        // If current element's year is less than the pivot's year, swap it to the left partition.
        if (articles[j].year < pivot) {
            i++;
            swap(articles[i], articles[j]);
        }
    }
    // Place the pivot element in its correct position.
    swap(articles[i+1], articles[right]);
    return i + 1;  // Return the partitioning index
}

// quickSort recursively sorts the articles array using Quick Sort algorithm.
void quickSort(News *articles, int left, int right) {
    recursionDepth++;  // Increment global recursion depth counter
    if (left < right) {
        int pi = partition(articles, left, right); // Partition the array
        // Recursively sort the subarray before and after the partition index.
        quickSort(articles, left, pi - 1);
        quickSort(articles, pi + 1, right);
    }
}

// insertionSort sorts the articles array by year using the Insertion Sort algorithm.
void insertionSort(News *articles, int count) {
    for (int i = 1; i < count; i++) {
        News key = articles[i];  // The element to be inserted
        int j = i - 1;
        // Shift elements of the sorted segment forward if they are greater than the key.
        while (j >= 0 && articles[j].year > key.year) {
            articles[j + 1] = articles[j];
            j--;
        }
        // Insert the key element at the correct position.
        articles[j + 1] = key;
    }
}

// ----------------------------------------------------------------
// traverseAndCountArticles Function
// ----------------------------------------------------------------
// This function iterates through the articles array,
// prints each article's title and publication year,
// and counts the total number of fake and true articles.
void traverseAndCountArticles(News *articles, int count) {
    int fakeCount = 0;
    int trueCount = 0;
    for (int i = 0; i < count; i++) {
        // Display the title and year of the article.
        cout << "Title: " << articles[i].title << ", Year: " << articles[i].year << endl;
        // Increment counts based on the truthfulness flag.
        if (articles[i].isTrue)
            trueCount++;
        else
            fakeCount++;
    }
    // Output the totals.
    cout << "Total articles: " << count << endl;
    cout << "Total FAKE articles: " << fakeCount << endl;
    cout << "Total TRUE articles: " << trueCount << endl;
}

// ----------------------------------------------------------------
// countArticlesIterative Function
// ----------------------------------------------------------------
// This function counts the number of fake and true articles iteratively.
// The counts are returned through reference parameters.
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
// linearSearchByYear Function
// ----------------------------------------------------------------
// Searches through the articles array for articles that match the target year.
// It first counts the number of matches and then creates an array of matching indices.
// The function returns a dynamically allocated array of indices (caller must delete[] it)
// and sets the matchCount reference to the number of found articles.
int* linearSearchByYear(News *articles, int count, int targetYear, int &matchCount) {
    matchCount = 0;
    // First pass: count the number of articles matching targetYear.
    for (int i = 0; i < count; i++) {
        if (articles[i].year == targetYear)
            matchCount++;
    }
    if (matchCount == 0)
        return nullptr;
    // Allocate an array to hold indices of matching articles.
    int* indices = new int[matchCount];
    int j = 0;
    // Second pass: store the indices of matching articles.
    for (int i = 0; i < count; i++) {
        if (articles[i].year == targetYear)
            indices[j++] = i;
    }
    return indices;
}

// ----------------------------------------------------------------
// binarySearchByYear Function
// ----------------------------------------------------------------
// Recursively performs a binary search on a sorted articles array for an article with targetYear.
// Parameters 'left' and 'right' define the current search bounds.
// Returns the index of a matching article if found, or -1 if not found.
int binarySearchByYear(News *articles, int left, int right, int targetYear) {
    if (left > right)
        return -1;  // Base case: not found
    int mid = left + (right - left) / 2;  // Compute mid index
    if (articles[mid].year == targetYear)
        return mid;  // Match found at mid
    else if (articles[mid].year < targetYear)
        return binarySearchByYear(articles, mid + 1, right, targetYear);  // Search in right half
    else
        return binarySearchByYear(articles, left, mid - 1, targetYear);   // Search in left half
}

// ----------------------------------------------------------------
// percentageByMonthLinear Function
// ----------------------------------------------------------------
// Calculates the percentage of fake political news articles by month for the year 2016.
// It assumes that the article's date is in the format "DD-MM-YYYY".
// A simple histogram (using stars) is printed for each month.
void percentageByMonthLinear(News *articles, int count) {
    int total[13] = {0};  // Array to count total articles per month (index 1 to 12)
    int fake[13] = {0};   // Array to count fake articles per month

    // Iterate over each article.
    for (int i = 0; i < count; i++) {
        // Process only articles from 2016 whose subject contains "politics"
        if (articles[i].year == 2016 && articles[i].subject.find("politics") != string::npos) {
            // Extract the month from the date string (characters 3-4, as "DD-MM-YYYY")
            string monthStr = articles[i].date.substr(3, 2);
            int month = stoi(monthStr);  // Convert month string to integer
            total[month]++;              // Increment total count for this month
            if (!articles[i].isTrue)
                fake[month]++;           // Increment fake count if article is fake
        }
    }

    cout << "=== Percentage for Fake Political News by Month (Linear Scan) ===" << endl;
    // Loop over months 1 to 12 to display percentages.
    for (int m = 1; m <= 12; m++) {
        if (total[m] > 0) {
            // Calculate percentage of fake articles for this month.
            double percentage = (static_cast<double>(fake[m]) / total[m]) * 100.0;
            // Determine number of stars to print (one per percentage point, roughly).
            int starCount = static_cast<int>(percentage);
            cout << "Month " << m << ": ";
            // Print stars as a histogram.
            for (int s = 0; s < starCount; s++) {
                cout << "*";
            }
            // Print the percentage with 4 decimal places.
            cout << " " << fixed << setprecision(4) << percentage << "%" << endl;
        } else {
            cout << "Month " << m << ": No data" << endl;
        }
    }
}

// ----------------------------------------------------------------
// Main Function
// ----------------------------------------------------------------
// Provides a main menu loop to interact with the user. Options include:
// 1. Sorting the articles by year (using Quick Sort or Insertion Sort)
// 2. Counting the total number of articles (iteratively)
// 3. Searching for articles by a specific year (using linear and binary search)
// 4. Displaying the percentage of fake political news by month for 2016
// 5. Exiting the program
int main() {
    News* articles = nullptr;
    // Load articles from the CSV file "DataCleaned.csv" into a dynamic array.
    int count = loadArticles("DataCleaned.csv", articles);
    if (count == 0) {  // If no articles were loaded, print error and exit.
        cerr << "No articles loaded." << endl;
        return 1;
    }
    
    int mainChoice = 0;
    // Main loop: continue until the user chooses to exit (option 5).
    do {
        // Display main menu options.
        cout << "\n==================== MAIN MENU ====================" << endl;
        cout << "1. Sort the news articles by year" << endl;
        cout << "2. Calculate total articles (fake and true) (Iterative Count Only)" << endl;
        cout << "3. Search article by year" << endl;
        cout << "4. Display percentage for fake political news by month in 2016 (Linear Scan)" << endl;
        cout << "5. Exit" << endl;
        cout << "Enter your option: ";
        cin >> mainChoice;
        
        if (mainChoice == 1) {  // Sorting option
            int sortChoice = 0;
            cout << "\n--- Choose Sorting Algorithm ---" << endl;
            cout << "1. Quick Sort" << endl;
            cout << "2. Insertion Sort" << endl;
            cout << "Enter your choice: ";
            cin >> sortChoice;
            
            // Create a temporary copy of the articles array so the original is preserved.
            News *articlesCopy = new News[count];
            string sortName;
            for (int i = 0; i < count; i++)
                articlesCopy[i] = articles[i];
                
            // Measure performance of the chosen sorting algorithm.
            pair<double, size_t> result; // result.first = time in seconds, result.second = memory used in bytes
            
            switch (sortChoice) {
                case 1:
                    recursionDepth = 0;  // Reset recursion depth counter before sorting.
                    result = measurePerformance("Quick Sort", quickSort, articlesCopy, 0, count - 1);
                    cout << "\n=== Sorted Articles using Quick Sort ===" << endl;
                    cout << "Quick Sort Recursion Depth: " << recursionDepth << endl;
                    sortName = "Quick Sort";
                    break;
                case 2:
                    result = measurePerformance("Insertion Sort", insertionSort, articlesCopy, count);
                    cout << "\n=== Sorted Articles using Insertion Sort ===" << endl;
                    sortName = "Insertion Sort";
                    break;
                default:
                    cout << "Invalid sorting option." << endl;
                    delete[] articlesCopy;
                    continue;  // Return to main menu if invalid option is selected.
            }
            
            // After sorting, display each article's title and year, and overall counts.
            traverseAndCountArticles(articlesCopy, count);
            // Display performance metrics.
            cout << "==================== " << sortName << " ====================" << endl;
            cout << "Time Taken: " << result.first << " s" << endl;
            cout << "Memory Used: " << result.second << " bytes" << endl;
            // Free the temporary sorted copy.
            delete[] articlesCopy;
            
        } else if (mainChoice == 2) {  // Iterative count option
            cout << "Starting Option 2 (Iterative Count)..." << endl;
            int fakeCountIter = 0, trueCountIter = 0;
            // Measure performance of the iterative count operation.
            auto iterResult = measurePerformance("Iterative Count", countArticlesIterative, articles, count, std::ref(fakeCountIter), std::ref(trueCountIter));
            cout << "\n=== Total Articles Count (Iterative) ===" << endl;
            cout << "Total Articles: " << count 
                 << ", TRUE: " << trueCountIter << ", FAKE: " << fakeCountIter 
                 << ", Time: " << iterResult.first << " s, Memory Diff: " << iterResult.second << " bytes" << endl;
            
        } else if (mainChoice == 3) {  // Search articles by year option
            int targetYear;
            cout << "Enter the year to search for: ";
            cin >> targetYear;
            
            // --- Perform Linear Search ---
            cout << "\n=== Linear Search Results ===" << endl;
            int matchCountLinear = 0;
            auto startLinear = high_resolution_clock::now();
            // Get an array of indices matching the target year.
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
            delete[] linearIndices; // Free the indices array.
            
            // --- Perform Binary Search ---
            // For binary search, we need a sorted copy of the articles array.
            News *articlesSorted = new News[count];
            for (int i = 0; i < count; i++)
                articlesSorted[i] = articles[i];
            recursionDepth = 0;  // Reset recursion depth for sorting.
            quickSort(articlesSorted, 0, count - 1);  // Sort the array using Quick Sort.
            auto startBinary = high_resolution_clock::now();
            // Search for the target year in the sorted array.
            int foundIndex = binarySearchByYear(articlesSorted, 0, count - 1, targetYear);
            auto endBinary = high_resolution_clock::now();
            double timeBinary = duration<double>(endBinary - startBinary).count();
            if (foundIndex == -1) {
                cout << "\n=== Binary Search Results (on sorted array) ===" << endl;
                cout << "No articles found for year " << targetYear << endl;
            } else {
                // To find all articles with the target year, scan left and right from foundIndex.
                int left = foundIndex;
                while (left >= 0 && articlesSorted[left].year == targetYear)
                    left--;
                left++; // Adjust left boundary
                int right = foundIndex;
                while (right < count && articlesSorted[right].year == targetYear)
                    right++;
                int matchCountBinary = right - left;
                // Build an array of indices for binary search results.
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
                delete[] binaryIndices; // Free the binary indices array.
            }
            delete[] articlesSorted; // Free the sorted copy.
            
        } else if (mainChoice == 4) {  // Option 4: Display percentage by month for fake political news in 2016
            // Measure performance for this operation
            auto linearRes = measurePerformance("Percentage by Month (Linear)", percentageByMonthLinear, articles, count);
            cout << "\n=== Fake Political News Percentage by Month (Linear Scan) ===" << endl;
            cout << "Time Taken: " << linearRes.first << " s, Memory Diff: " << linearRes.second << " bytes" << endl;
            
        } else if (mainChoice == 5) {  // Option 5: Exit the program
            cout << "Exiting program." << endl;
        } else {  // Handle invalid input
            cout << "Invalid option. Please try again." << endl;
        }
        
    } while (mainChoice != 5);
    
    // Clean up: Free the dynamically allocated articles array.
    delete[] articles;
    return 0;
}
