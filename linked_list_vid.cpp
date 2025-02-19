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
 * Returns true if date1 <= date2
 */
bool compareDate(const string& date1, const string& date2) {
    //cout << "Comparing dates: " << date1 << " and " << date2 << endl;
    int day1, month1, year1, day2, month2, year2;
    char dash;  // for the '-' separator
    stringstream ss1(date1), ss2(date2);
    cout << ss1.str() << " " << ss2.str() << endl;
    
    ss1 >> day1 >> dash >> month1 >> dash >> year1;
    ss2 >> day2 >> dash >> month2 >> dash >> year2;
    
    if (year1 != year2) return year1 < year2;
    if (month1 != month2) return month1 < month2;
    return day1 <= day2;
}

/**
 * Partition function for Quick Sort
 */
News* partition(News* head, News* end, News** newHead, News** newEnd) {
    News* pivot = end;
    News* prev = nullptr, *cur = head, *tail = pivot;

    while (cur != pivot) {
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
    }
    if ((*newHead) == nullptr) (*newHead) = pivot;
    (*newEnd) = tail;
    return pivot;
}

/**
 * Recursive Quick Sort Function
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

// Function to split the singly linked list into two halves
News *split(News *head) {
    News *fast = head;
    News *slow = head;

    // Move fast pointer two steps and slow pointer
    // one step until fast reaches the end
    while (fast != nullptr && fast->next != nullptr) {
        fast = fast->next->next;
        if (fast != nullptr) {
            slow = slow->next;
        }
    }

    // Split the list into two halves
    News *temp = slow->next;
    slow->next = nullptr;
    return temp;
}

// Function to merge two sorted singly linked lists
News *merge(News *first, News *second) {
    // If either list is empty, return the other list
    if (first == nullptr) return second;
    if (second == nullptr) return first;

    int day_first, day_second, month_first, month_second, year_first, year_second;
    try {
        cout << "Comparing dates: " << first->date << " and " << second->date << endl;
        auto [day1, month1, year1] = trimDate(first->date);
        auto [day2, month2, year2] = trimDate(second->date);
        year_first = stoi(year1);
        year_second = stoi(year2);
        month_first = stoi(month1);
        month_second = stoi(month2);
        day_first = stoi(day1);
        day_second = stoi(day2);
    } catch (const exception& e) {
        cerr << e.what() << '\n';
    }
    
    if (year_first < year_second) {
        // Recursively merge the rest of the lists and
        // link the result to the current node
        first->next = merge(first->next, second);
        return first;
    } else if (year_first == year_second) {
        if (month_first < month_second) {
            // Recursively merge the rest of the lists and
            // link the result to the current node
            first->next = merge(first->next, second);
            return first;
        } else if (month_first == month_second) {
            if (day_first < day_second) {
                // Recursively merge the rest of the lists and
                // link the result to the current node
                first->next = merge(first->next, second);
                return first;
            } else {
                // Recursively merge the rest of the lists
                // and link the result to the current node
                second->next = merge(first, second->next);
                return second;
            }
        } else {
            // Recursively merge the rest of the lists
            // and link the result to the current node
            second->next = merge(first, second->next);
            return second;
        }
    } else {
        // Recursively merge the rest of the lists
        // and link the result to the current node
        second->next = merge(first, second->next);
        return second;
    }

    // Pick the smaller value between first and second nodes
    if (first->data < second->data) {
        // Recursively merge the rest of the lists and
        // link the result to the current node
        first->next = merge(first->next, second);
        return first;
    }
    else {
        // Recursively merge the rest of the lists
        // and link the result to the current node
        second->next = merge(first, second->next);
        return second;
    }
}

// Function to perform merge sort on a singly linked list
News *MergeSort(News *head) {
    // Base case: if the list is empty or has only one node, 
    // it's already sorted
    if (head == nullptr || head->next == nullptr)
        return head;

    // Split the list into two halves
    News *second = split(head);

    // Recursively sort each half
    head = MergeSort(head);
    second = MergeSort(second);

    // Merge the two sorted halves
    return merge(head, second);
}

/**
 * Insertion Sort Function for Linked List
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
 */
int iterativeCount(News* head) {
    int count = 0;
    News* current = head;
    while (current != nullptr) {
        count++;
        current = current -> next;
    }
    return count;
}

void calcNews(News** head) {
    if (!head || !(*head)) {
        cout << "Empty list - nothing to count" << endl;
        return;
    }

    try {
        // Iterative count
        int iterCount = iterativeCount(*head);
        cout << "Iterative count: " << iterCount << " articles" << endl;
    } catch (const exception& e) {
        cerr << "Error during counting: " << e.what() << endl;
    }
}

/**
 * Print all News
 */
void printList(News* head) {
    while (head) {
        cout << "Title: " << head -> title
            << "Text: " << head -> text
            << "Subject: " << head -> subject
            << "Date: " << head -> date
            << "Identify: " << head -> identify << endl;
        head = head -> next;
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
    
    if (invalidDates > 0) {
        cerr << "Warning: Found " << invalidDates << " invalid date(s) during search" << endl;
    }
    
    return totalCount;
}

/**
 * Helper function to find the middle node of a linked list
 * This uses the fast and slow pointer technique
 * @param start The start node of the linked list
 * @param end The end node of the linked list
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
    // Print the results
    cout << "Total number of political news articles: " << totalPoliticalNews << endl;
    cout << "Total number of political news articles in 2016: " << fakePoliticalNews2016 << endl;
    // If total news is greater than 0
    if (totalPoliticalNews > 0) {
        // Calculate the percentage of fake news articles in 2016
        double percentage = (totalPoliticalNews > 0) ? (fakePoliticalNews2016 / totalPoliticalNews) * 100 : 0;
        cout << "Percentage of fake news articles in 2016: " << percentage << "%" << endl;
    } else { // No political news articles found
        cout << "No political news articles found" << endl;
    }

    // Display the news plot
    cout << "Percentage of fake Political News Article by every month in 2016" << endl;
    displayNewsPlot(fakeNewsCount, totalNewsCount);
}

/**
 * Write the linked list to a CSV file
 * @param filename The name of the CSV file
 * @param newsBook The linked list of news
 */
void writeCSV(const string& filename, News* newsBook) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    // Write the data
    while (newsBook) {
        file << "\"" << newsBook->title << "\","
            << "\"" << newsBook->text << "\","
            << "\"" << newsBook->subject << "\","
            << "\"" << newsBook->date << "\","
            << "\"" << newsBook->identify << "\"\n";
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
        cout << "\nUser[~] $ ";
        cin >> choice;
        switch (choice) {
            // Sort by year
            case 1: {
                // Compare Quick Sort, merge sort, insertion sort
                cout << "\nSorting Menu" << endl;
                cout << "1. Quick Sort" << endl;
                cout << "2. Merge Sort" << endl;
                cout << "3. Insertion Sort" << endl;

                // User Input
                int sortChoice;
                cout << "\nUser[~] $ ";
                cin >> sortChoice;
                auto start = chrono::high_resolution_clock::now();
                switch (sortChoice) {
                    case 1:
                        // Quick Sort
                        cout << "\nUsing Quick Sort to sort..." << endl;
                        quickSort(&newsBook); // Quick Sort
                        writeCSV("QuickSort.csv", newsBook);
                        break;

                    case 2:
                        // Merge Sort
                        cout << "\nUsing Merge Sort to sort..." << endl;
                        MergeSort(newsBook); // Merge Sort
                        writeCSV("MergeSort.csv", newsBook);
                        break;

                    case 3:
                        // Insertion Sort
                        cout << "\nUsing Insertion Sort to sort..." << endl;
                        insertionSort(newsBook); // Insertion Sort
                        writeCSV("InsertionSort.csv", newsBook);
                        break;

                    default:
                        cout << "Invalid choice. Please try again." << endl; break;
                }
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double> elapsed = end - start;
                cout << "News sorted successfully!" << endl;
                cout << "Time taken for sorting: " << elapsed.count() << " seconds" << endl;
                cout << "\nPrint sorted news?" << "\n1. Yes" << "\n2. No" << endl;
                int printChoice;
                cout << "\nUser[~] $ ";
                cin >> printChoice;
                if (printChoice == 1) printList(newsBook); // Display sorted news
                break;
            }

            // Calculate total articles using Iterative and recursive methods
            case 2: {
                if (!newsBook) { cout << "No news articles found." << endl; break; }
                cout << "Calculating using iterative and recursive methods..." << endl;
                auto start = chrono::high_resolution_clock::now();
                calcNews(&newsBook);
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double> elapsed = end - start;
                cout << "Time taken: " << elapsed.count() << " seconds" << endl;
                break;
            }

            // Search article by specific criteria
            case 3: { // Linear Search and Binary Search for ques 3
                cout << "Enter search criteria (year): ";
                int year;
                cin >> year;
                cout << "Searching for year in " << year << " ..." << endl;

                // Linear Search
                auto start = chrono::high_resolution_clock::now();
                int result = linearSearch(newsBook, year);
                auto end = chrono::high_resolution_clock::now();
                cout << "Total articles found: " << result << endl;
                chrono::duration<double> elapsed = end - start;
                cout << "Time taken for linear search: " << elapsed.count() << " seconds" << endl;

                // Binary Search
                auto start1 = chrono::high_resolution_clock::now();
                int result1 = binarySearch(newsBook, year);
                auto end1 = chrono::high_resolution_clock::now();
                cout << "Article found: " << result1 << endl;
                chrono::duration<double> elapsed1 = end1 - start1;
                cout << "Time taken for binary search: " << elapsed1.count() << " seconds" << endl;
                break;
            }

            // Display percentage for fake political news by every month in 2016
            case 4: {
                cout << "Searching..." << endl;
                auto start = chrono::high_resolution_clock::now();
                countPoliticNews(&newsBook);
                auto end = chrono::high_resolution_clock::now();
                chrono::duration<double> elapsed = end - start;
                cout << "Time taken: " << elapsed.count() << " seconds" << endl;
                break;
            }
            default: { cout << "Invalid choice, please try again." << endl; return 0; }
        }

        if (running && choice >= 1 && choice <= 4) {
            cout << "\nPres Enter to return to menu...";
            cin.ignore();   // Ignore newline character
            cin.get();      // Wait for user input
        }
    }
    // Cleanup
    // Free memory allocated for the linked list
    delete news;
    return 0;
}
