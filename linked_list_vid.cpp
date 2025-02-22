#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <chrono>
#include <iomanip>
#include <tuple>

using namespace std;

struct News {
    string title, text, subject, date, identify;
    int year, data;
    News *next, *head;
    News() : next(nullptr) {}
    News(int x) { data = x, next = nullptr; }
    News(string t, string tx, string sub, string d, string id)
        : title(t), text(tx), subject(sub), date(d), identify(id), next(nullptr) {}
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
 * @param news The news object
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
 * Function to trim date
 * @param date The date string to trim
 * @return Tuple containing day, month, and year
 */
tuple<string, string, string> trimDate(const string& date) {
    stringstream ss(date);
    // Trim the date
    string day, month, year;
    getline(ss, day, '-');
    getline(ss, month, '-');
    getline(ss, year);
    return {day, month, year};
}

/**
 * Insertion Sort Function for Linked List
 * @param head The head of the linked list
 */
void insertionSort(News*& head) {
    if (!head || !head -> next) return;
    
    News* sorted = nullptr;     // Initialize sorted linked list
    News* current = head;       // Initialize current node
    
    while (current != nullptr) {
        News* next = current -> next;  // Store next for next iteration

        // Special case for insertion at head
        if (sorted == nullptr || compareDate(current -> date, sorted -> date)) {
            current -> next = sorted;
            sorted = current;
        } else {
            News* temp = sorted;
            // Locate node before insertion point
            while (temp -> next != nullptr && !compareDate(current -> date, temp -> next -> date)) {
                temp = temp -> next;
            }
            current -> next = temp -> next;
            temp -> next = current;
        }
        current = next;  // Move to next node
    }
    head = sorted;       // Update head to point to sorted list
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
 * Print all News
 * @param head The head of the linked list
 */
void printList(News* head) {
    cout << "News sorted successfully!" << endl;
    cout << "\nPrint sorted news?" << "\n1. Yes" << "\n2. No" << endl;
    int printChoice;
    cout << "\nEnter Choice: ";
    cin >> printChoice;
    if (printChoice == 1) {// Display sorted news
        while (head) {
            cout << "Title: " << head -> title
                << "\nText: " << head -> text
                << "\nSubject: " << head -> subject
                << "\nDate: " << head -> date
                << "\nIdentify: " << head -> identify
                << "\n======================================" << endl;
            head = head -> next;
        }
    }
}

/**
 * Linear Search Function for Linked List
 * @param head The head of the linked list
 * @param target The target year to search for
 * @return total article found
 */
int linearSearch(News* head, int target) {
    News* current = head;
    int totalCount = 0;
    int invalidDates = 0;
    
    while (current != nullptr) {
        if (!isValidDate(current -> date)) {
            invalidDates++;
            current = current -> next;
            continue;
        }
        
        try {
            auto [day, month, year] = trimDate(current -> date);
            if (stoi(year) == target) {
                totalCount++;
            }
        } catch (const exception& e) {
            invalidDates++;
        }
        current = current -> next;
    }
    
    return totalCount;
}

/**
 * Helper function to find the middle node of a linked list
 * This uses the fast and slow pointer technique
 * @param start The start node of the linked list
 * @param end The end node of the linked list
 * @return the middle node of the linked list
 */
News* getMiddle(News* start, News* end) {
    if (start == nullptr) return nullptr;

    News* slow = start;
    News* fast = start;

    while (fast != end && fast -> next != end) {
        slow = slow -> next;
        fast = fast -> next -> next;
    }
    return slow;
}

/**
 * Binary Search Function for Linked List
 * @param head The head of the linked list
 * @param target The target year to search for
 * @return total article found
 */
int binarySearch(News* head, int target) {
    News* start = head;
    News* end = nullptr;
    int totalCount = 0;

    while (start != end) {
        News* mid = getMiddle(start, end);
        if (!mid) break;
        if(!isValidDate(mid -> date)) {
            start = mid -> next;
            continue;
        }
        try {
            auto [day, month, year] = trimDate(mid -> date);
            int yearValue = stoi(year);
            
            if (yearValue == target) {
                // Found target year, count all occurrences
                News* temp = start;
                while (temp != end) {
                    if (isValidDate(temp -> date)) {
                        auto [d, m, y] = trimDate(temp -> date);
                        if (stoi(y) == target) {
                            totalCount++;
                        }
                    }
                    temp = temp -> next;
                }
                return totalCount;
            } 
            else if (yearValue < target) {
                start = mid -> next;
            } 
            else {
                end = mid;
            }
        } catch (const exception& e) {
            cerr << "Error parsing date: " << mid -> date << endl;
            start = mid -> next;
        }
    }
    
    return totalCount;
}

/**
 * Display the plot of fake news over time
 * @param fakeNewsCount - The count of fake news articles by month
 * @param totalNewsCount - The total count of news articles by month
 */
void displayNewsPlot(const unordered_map<int, int>& fakeNewsCount, const unordered_map<int, int>& totalNewsCount) {
    // Display the news plot here
    for (int month = 1; month <= fakeNewsCount.size(); month++) {
        int fakeCount = fakeNewsCount.count(month) ? fakeNewsCount.at(month) : 0;
        int totalCount = totalNewsCount.count(month) ? totalNewsCount.at(month) : 0;
        // Calculate the percentage of fake news articles
        double percentage = (totalCount > 0) ? (static_cast<double>(fakeCount) / totalCount) * 100 : 0;
        string monthName;
        switch (month) {
            case 1: monthName = "January"; break;
            case 2: monthName = "February"; break;
            case 3: monthName = "March"; break;
            case 4: monthName = "April"; break;
            case 5: monthName = "May"; break;
            case 6: monthName = "June"; break;
            case 7: monthName = "July"; break;
            case 8: monthName = "August"; break;
            case 9: monthName = "September"; break;
            case 10: monthName = "October"; break;
            case 11: monthName = "November"; break;
            case 12: monthName = "December"; break;
            default:
                break;
        }
        // Display the news plot
        cout << left << setw(10) << monthName << "\t | " << string(static_cast<int>(percentage), '*')
            << " " << fixed << setprecision(1) << percentage << "%" << endl;
    }
}

/**
 * Calculate Total Number of Political News
 * @param news The head of the linked list
 */
void countPoliticNews(News** news) {
    News* currentNews = *news;
    // Initialize variables
    unordered_map<int, int> fakeNewsCount, totalNewsCount;
    double totalPoliticalNews = 0;
    double fakePoliticalNews2016 = 0;

    // Read through each line
    while (currentNews != nullptr) {
        stringstream ss(currentNews -> date);
        // Trim the date
        auto [day, month, year] = trimDate(currentNews -> date);
        
        // If subject is politics and year is 2016
        if ((currentNews -> subject == "politics" || currentNews -> subject == "politicsNews") && year == "2016") {
            if (currentNews -> identify == "FAKE") {    // If identify is FAKE
                fakePoliticalNews2016++;                // Increment fake news count
                fakeNewsCount[stoi(month)]++;           // Increment fake news count by month
            }
            totalPoliticalNews++;           // Increment total news count
            totalNewsCount[stoi(month)]++;  // Increment total news count by month
        }
        currentNews = currentNews -> next;
    }

    // Display the news plot
    cout << "\nPercentage of fake Political News Article by every month in 2016" << endl;
    displayNewsPlot(fakeNewsCount, totalNewsCount);

    // Print the results
    cout << "\nTotal number of political news articles: " << totalPoliticalNews << endl;
    cout << "Total number of political news articles in 2016: " << fakePoliticalNews2016 << endl;
    // If total news is greater than 0
    if (totalPoliticalNews > 0) {
        // Calculate the percentage of fake news articles in 2016
        double percentage = (totalPoliticalNews > 0) ? (fakePoliticalNews2016 / totalPoliticalNews) * 100 : 0;
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

int main(int argc, char const *argv[]) {
    News* newsBook = nullptr;
    News* news = new News;

    ifstream infile("DataCleaned.csv"); // Open file for reading

    // Check the file open successfully
    if (!infile) { cerr << "Error opening file" << endl; return 1; }

    cout << "Loading news into Linked List..." << endl;
    preloadNews(infile, newsBook); // Preload news into linked list
    cout << "News loaded successfully!\n" << endl;
    infile.close(); // Close file

/**
 * 1. Sort the articles by year, display whole list in ascending order
 * 2. Calculate the total articles (fake and true csv)
 * 3. Search article by specific criteria (can define by ourselves in anyway), display the results
 * 4. Functions that display all percentage for fake political news by every month in 2016 (minimum requirements) (MOST IMPORTANT!!)
 */

    bool running = true;
    while (running) {
        // User Menu
        cout << "<<< User Menu >>>" << endl;
        cout << "1. Sort By Year and display in ascending order" << endl;
        cout << "2. Calculate the total articles (fake and true csv)" << endl;
        cout << "3. Search article by specific criteria" << endl;
        cout << "4. Display percentage for fake political news by every month in 2016" << endl;

        // User Input
        int choice;
        cout << "\nEnter Choice: ";
        cin >> choice;
        switch (choice) {
            // Sort by year
            case 1: {
                // Quick Sort, insertion sort
                cout << "\nSorting Menu" << endl;
                cout << "1. Quick Sort" << endl;
                cout << "2. Insertion Sort" << endl;

                // User Input
                int sortChoice;
                cout << "\nSelect a sorting algorithm: ";
                cin >> sortChoice;
                switch (sortChoice) {
                    case 1: { // Quick Sort
                        cout << "\nUsing Quick Sort to sort..." << endl;
                        auto startMem_quicksort = calculateDetailedMemory(newsBook);
                        auto timeStart_quicksort = chrono::high_resolution_clock::now();
                        quickSort(&newsBook);
                        auto timeEnd_quicksort = chrono::high_resolution_clock::now();
                        auto endMem_quicksort = calculateDetailedMemory(newsBook);
                        endMem_quicksort.timeElapsed = chrono::duration<double>(timeEnd_quicksort - timeStart_quicksort).count();
                        displayMemoryStats(endMem_quicksort, "Quick Sort");
                        printList(newsBook);
                        break;
                    }

                    case 2: { // Insertion Sort
                        cout << "\nUsing Insertion Sort to sort..." << endl;
                        auto startMem_insertsort = calculateDetailedMemory(newsBook);
                        auto timeStart_insertsort = chrono::high_resolution_clock::now();
                        insertionSort(newsBook);
                        auto timeEnd_insertsort = chrono::high_resolution_clock::now();
                        auto endMem_insertsort = calculateDetailedMemory(newsBook);
                        endMem_insertsort.timeElapsed = chrono::duration<double>(timeEnd_insertsort - timeStart_insertsort).count();
                        displayMemoryStats(endMem_insertsort, "Insertion Sort");
                        printList(newsBook);
                        break;
                    }

                    default: { cout << "\nInvalid choice. Please try again." << endl; break; }
                }
                break;
            }

            // Calculate total articles using Iterative methods
            case 2: {
                if (!newsBook) { cout << "No news articles found." << endl; break; }
                cout << "\nCalculating using iterative methods..." << endl;
                auto start_calcNews = chrono::high_resolution_clock::now();
                iterativeCount(newsBook);
                MemoryStats stats_calcNews = calculateDetailedMemory(newsBook);
                auto end_calcNews = chrono::high_resolution_clock::now();
                stats_calcNews.timeElapsed = chrono::duration<double>(end_calcNews - start_calcNews).count();
                displayMemoryStats(stats_calcNews, "Total Articles Calculation");
                break;
            }

            // Search article by specific criteria
            case 3: { // Linear Search and Binary Search for ques 3
                cout << "\nEnter search criteria (year): ";
                int year;
                cin >> year;
                cout << "Searching for year in " << year << " ..." << endl;

                // Linear Search
                auto startMem_linSearch = calculateDetailedMemory(newsBook);
                auto start_linSearch = chrono::high_resolution_clock::now();
                cout << "\nTotal articles found: " << linearSearch(newsBook, year) << endl;
                MemoryStats stats_linSearch = calculateDetailedMemory(newsBook);
                auto end_linSearch = chrono::high_resolution_clock::now();
                stats_linSearch.timeElapsed = chrono::duration<double>(end_linSearch - start_linSearch).count();
                displayMemoryStats(stats_linSearch, "Linear Search");

                // Binary Search
                auto startMem_binSearch = calculateDetailedMemory(newsBook);
                auto start_binSearch = chrono::high_resolution_clock::now();
                cout << "\nArticle found: " << binarySearch(newsBook, year) << endl;
                MemoryStats stats_binSearch = calculateDetailedMemory(newsBook);
                auto end_binSearch = chrono::high_resolution_clock::now();
                stats_binSearch.timeElapsed = chrono::duration<double>(end_binSearch - start_binSearch).count();
                displayMemoryStats(stats_binSearch, "Binary Search");
                break;
            }

            // Display percentage for fake political news by every month in 2016
            case 4: {
                cout << "\nSearching..." << endl;
                auto start_disPercent = chrono::high_resolution_clock::now();
                countPoliticNews(&newsBook);
                MemoryStats stats_disPercent = calculateDetailedMemory(newsBook);
                auto end_disPercent = chrono::high_resolution_clock::now();
                stats_disPercent.timeElapsed = chrono::duration<double>(end_disPercent - start_disPercent).count();
                displayMemoryStats(stats_disPercent, "Political News Analysis");
                break;
            }

            default: { cout << "Invalid choice, please try again." << endl; }
        }

        if (running && choice >= 1 && choice <= 4) {
            cout << "\nPress Enter to return to menu...\n";
            cin.ignore();   // Ignore newline character
            cin.get();      // Wait for user input
        }
    }
    // Cleanup
    // Free memory allocated for the linked list
    delete news;
    return 0;
}
