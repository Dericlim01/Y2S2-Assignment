#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

using namespace std;

struct News {
    string title, text, subject, date, identify;
    News *next, *head;
    News() : next(nullptr) {}
    News(string title, string text, string subject, string date, string identify)
    : title(title), text(text), subject(subject), date(date), identify(identify), next(nullptr) {}
};

/**
 * Insert at Beginning
 */
void insertAtBeginning(News** head, string title, string text, string subject, string date, string identify) {
    News* newNews = new News; // Create a new Contact
    newNews -> title = title;
    newNews -> text = text;
    newNews -> subject = subject;
    newNews -> date = date;
    newNews -> identify = identify;
    newNews -> next = *head;
    *head = newNews;
}

/**
 * Insert at End
 */
void insertAtEnd(News** head, string title, string text, string subject, string date, string identify) {
    News* newNews = new News; // Create a new Contact
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
 * Insert at Position
 */
void insertAtPosition(News** head, string title, string text, string subject, string date, string identify, int position) {
    if (position == 0) {
        insertAtBeginning(head, title, text, subject, date, identify);
    } else {
        News* newNews = new News; // Create a new Contact
        newNews -> title = title;
        newNews -> text = text;
        newNews -> subject = subject;
        newNews -> date = date;
        newNews -> identify = identify;

        News* currentNews = *head;
        for (int i = 0; i < position - 1; i++) {
            if (currentNews == nullptr) {
                cout << "Position out of bounds" << endl;
                return;
            }
            currentNews = currentNews -> next;
        }
        newNews -> next = currentNews -> next;
        currentNews -> next = newNews;
    }
}

void sortByYear(News** head) {
    if (*head == nullptr || (*head)->next == nullptr) {
        return; // List is empty or has only one element
    }

    bool swapped;
    News* ptr1;
    News* lptr = nullptr;

    // Bubble sort algorithm
    do {
        swapped = false;
        ptr1 = *head;

        while (ptr1 -> next != lptr) {
            if (ptr1 -> title > ptr1 -> next -> title) {
                // Swap the data of the nodes
                swap(ptr1 -> title, ptr1 -> next -> title);
                swap(ptr1 -> text, ptr1 -> next -> text);
                swap(ptr1 -> subject, ptr1 -> next -> subject);
                swap(ptr1 -> date, ptr1 -> next -> date);
                swap(ptr1 -> identify, ptr1 -> next -> identify);
                swapped = true;
            }
            ptr1 = ptr1 -> next;
        }
        lptr = ptr1;
    } while (swapped);
}

/**
 * Calculate Total Number of News
 */
void countNews(News** head) {
    News* currentNews = *head;
    int count = 0;

    while (currentNews != nullptr) {
        currentNews = currentNews -> next;
        count++;
    }
    cout << "News count: " << count << endl;
}

/**
 * Calculate Total Number of Political News
 */
void countPoliticNews(News** news) {
    News* currentNews = *news;
    // Initialize variables
    int totalNews = 0;
    int news2016 = 0;
    
    // Read through each line
    while (currentNews != nullptr) {
        stringstream ss(currentNews -> date);
        string day, month, year;
        getline(ss, day, '-');
        getline(ss, month, '-');
        getline(ss, year);
        if (currentNews -> subject == "politics" && year == "2016") {
            if (currentNews -> identify == "TRUE") {
                news2016++;
            }
            totalNews++;
        }
        currentNews = currentNews -> next;
    }
    cout << "Total number of political news articles: " << totalNews << endl;
    cout << "Total number of political news articles in 2016: " << news2016 << endl;
    if (totalNews > 0) {
        double percentage = static_cast<double>(news2016 / totalNews) * 100;
        cout << "Percentage of fake news articles in 2016: " << percentage << "%" << endl;
    } else {
        cout << "No political news articles found" << endl;
    }
}

void printList(News* head) {
    while (head) {
        cout << "Title: " << head -> title << "Text: " << head -> text << "Subject: " << head -> subject << "Date: " << head -> date << "Identify: " << head -> identify << endl;
        head = head -> next;
    }
}

// void def_delimiter(string line) {
//     // Declaration of delimiter
//     char* str = new char[line.length() + 1]; // Allocate memory
//     strcpy(str, line.c_str());

//     bool inQuotes = false;
//     char* token = str;

//     for (int column = 0; column < line.length(); column++) {
//         if (str[column] == '"') {
//             inQuotes = !inQuotes;
//             continue;
//         } else if (str[column] == ',' && !inQuotes) {
//             str[column] = '\0';

//             if (*token == '"' && *(token + strlen(token) - 1) == '"') {
//                 token[strlen(token) - 1] = '\0';
//                 token++;
//             }
//             cout << token << "\n";
//             token = str + column + 1;
//         }
//     }

//     if (*token == '"' && *token + strlen(token) - 1 == '"') {
//         token[strlen(token) - 1] = '\0';
//         token++;
//     }

//     cout << token << "\n";

//     delete[] str;
// }

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
                // Toggle the inQuotes flag
                inQuotes = !inQuotes;
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
        }
    }
}

int main(int argc, char const *argv[]) {
    News* newsBook = nullptr;
    News* news = new News;

    ifstream infile("DataCleaned.csv"); // Open file for reading

    // Check the file open successfully
    if (!infile) {
        cerr << "Error opening file" << endl;
        return 1;
    }

/**
 * 1. How can you efficiently sort the news articles by year and display the total number of articles in both datasets?
 * 2. What percentage of political news articles (including fake and true news) from the year of 2016 are fake? 
    (Hint: To answer this, you will need to search through both datasets and analyse the date and category of each article.)
 * 3. Which words are most frequently used in fake news articles related to government topics?
    (Hint: You are required to extract the most common words from these articles, sort them by frequency, and present the results.)
 */

    // User Menu
    cout << "User Menu" << endl;
    cout << "1. Sort By Year and display the total number of articles in both datasets" << endl;
    cout << "2. Percentage of political news article (including fake and true news) from year 2016 are fake" << endl;
    cout << "3. Most frequently word used in fake news article related to government topics" << endl;

    // User Input
    int choice;
    cout << "Enter your choice: ";
    cin >> choice;
    switch (choice) {
        // Search News
        case 1: {
            string line;
            cout << news -> title << endl;
            cout << "Sorting by title" << endl;
            while (getline(infile, line)) {
                parseCSVLine(line, news -> title, news -> text, news -> subject, news -> date, news -> identify);
                insertAtEnd(&newsBook, news -> title, news -> text, news -> subject, news -> date, news -> identify);
            }
            sortByYear(&newsBook);
            printList(newsBook);
            countNews(&newsBook);
            break;
        }
        // Sort By Title
        case 2: {
            string line;
            while (getline(infile, line)) {
                parseCSVLine(line, news -> title, news -> text, news -> subject, news -> date, news -> identify);
                insertAtEnd(&newsBook, news -> title, news -> text, news -> subject, news -> date, news -> identify);
            }
            countPoliticNews(&newsBook);
        }
        // Invalid Choice
        default: cout << "Invalid choice" << endl;
    }

    // Close file
    infile.close();
    
    return 0;
}
