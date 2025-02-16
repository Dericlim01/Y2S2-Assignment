#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
using namespace std;

// Structure to hold individual news article information.
struct News {
    string title;    // Title of the article
    string text;     // Full text content of the article
    string subject;  // Subject/category (e.g., politics, government news)
    string date;     // Publication date as a string ("DD-MM-YYYY")
    bool isTrue;     // Boolean flag: true if article is true, false if fake (inverted logic)
    int year;        // Year extracted from the date
};

// Structure to store a word and its frequency for word frequency analysis.
struct WordFrequency {
    string word;  // The word/token
    int count;    // Frequency count of the word
};

// ---------------------------------------------------------
// Function: loadArticles
// Purpose: Load articles from a CSV file into a dynamically allocated array.
// Returns: The number of articles loaded.
// ---------------------------------------------------------
int loadArticles(const string &filename, News *&articles) {
    const int MAX_ARTICLES = 50000;
    articles = new News[MAX_ARTICLES];  // Allocate articles array on the heap.
    int articleCount = 0;
    
    ifstream file(filename);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return 0;
    }
    
    // Skip the header line.
    string header;
    getline(file, header);
    
    string line;
    // Read each line until end-of-file or maximum articles reached.
    while (getline(file, line) && articleCount < MAX_ARTICLES) {
        // --- Multi-line record handling ---
        // Count quotes in the current line.
        int quoteCount = 0;
        for (char c : line)
            if (c == '"')
                quoteCount++;
        // If quotes are unbalanced (odd number), append additional lines.
        while ((quoteCount % 2) != 0 && !file.eof()) {
            string extra;
            if (!getline(file, extra))
                break;
            line += "\n" + extra;
            quoteCount = 0;
            for (char c : line)
                if (c == '"')
                    quoteCount++;
        }
        
        // --- Parse the record into fields ---
        // Use a dynamically allocated array for temporary fields.
        const int TEMP_FIELD_SIZE = 20;
        string* tempFields = new string[TEMP_FIELD_SIZE];
        int fieldCount = 0;
        bool inQuotes = false;
        string currentField = "";
        
        // Loop over each character in the line to split it by commas,
        // while handling quotes correctly.
        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];
            if (c == '"') {
                // If inside quotes and next character is also a quote, it's an escaped quote.
                if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    currentField.push_back('"');
                    i++; // Skip the escaped quote.
                } else {
                    // Toggle the inQuotes flag.
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                // A comma outside of quotes signals the end of a field.
                tempFields[fieldCount++] = currentField;
                currentField = "";
            } else {
                // Otherwise, add the character to the current field.
                currentField.push_back(c);
            }
        }
        // Add the final field.
        tempFields[fieldCount++] = currentField;
        
        // --- Collapse extra fields if there are more than 5 ---
        // The expected CSV has 5 columns: title, text, subject, date, and T/F field.
        if (fieldCount < 5) {
            delete[] tempFields;  // Clean up temporary array.
            continue; // Skip this malformed record.
        }
        if (fieldCount > 5) {
            // Combine fields beyond the 4th index into one field.
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
        
        // --- Populate a News object from the parsed fields ---
        News article;
        article.title   = tempFields[0];
        article.text    = tempFields[1];
        article.subject = tempFields[2];
        article.date    = tempFields[3];
        
        // Extract the year from the date string (assuming format "DD-MM-YYYY").
        if (article.date != "NA" && article.date.size() >= 10) {
            try {
                article.year = stoi(article.date.substr(article.date.size() - 4, 4));
            } catch (...) {
                article.year = 0;
            }
        } else {
            article.year = 0;
        }
        
        // Process the T/F field (5th column).
        // Convert the field to uppercase to standardize the comparison.
        string tfField = tempFields[4];
        for (size_t i = 0; i < tfField.size(); i++) {
            tfField[i] = toupper(tfField[i]);
        }
        // Invert the logic: if tfField equals "FAKE", then isTrue is false; otherwise true.
        article.isTrue = !(tfField == "FAKE");
        
        // Store the article in the articles array.
        articles[articleCount++] = article;
        
        // Free the temporary fields array.
        delete[] tempFields;
    }
    file.close();
    return articleCount;  // Return the total number of articles loaded.
}

// ---------------------------------------------------------
// swap: Swap two News objects.
// ---------------------------------------------------------
void swap(News &a, News &b) {
    News temp = a;
    a = b;
    b = temp;
}

// ---------------------------------------------------------
// partition: Partition function used by Quick Sort.
// Uses the year as the pivot value.
// ---------------------------------------------------------
int partition(News *articles, int left, int right) {
    int pivot = articles[right].year; // Choose pivot as the rightmost element's year.
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

// ---------------------------------------------------------
// quickSort: Recursive Quick Sort algorithm on News array.
// ---------------------------------------------------------
void quickSort(News *articles, int left, int right) {
    if (left < right) {
        int pi = partition(articles, left, right); // Partition index
        quickSort(articles, left, pi - 1);           // Recursively sort left subarray
        quickSort(articles, pi + 1, right);          // Recursively sort right subarray
    }
}

// ---------------------------------------------------------
// sortArticlesByYear: Sort the News array by publication year.
// ---------------------------------------------------------
void sortArticlesByYear(News *articles, int count) {
    quickSort(articles, 0, count - 1);
}

// ---------------------------------------------------------
// traverseAndCountArticles: Traverse the News array,
// print each article's title and year, and count fake/true articles.
// ---------------------------------------------------------
void traverseAndCountArticles(News *articles, int count) {
    int fakeCount = 0;
    int trueCount = 0;

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

// ---------------------------------------------------------
// calculateFakePoliticalNewsPercentage:
// Calculate and display the percentage of political news articles
// from 2016 that are fake.
// ---------------------------------------------------------
void calculateFakePoliticalNewsPercentage(News *articles, int count) {
    int totalPolitics2016 = 0;
    int fakePolitics2016 = 0;

    for (int i = 0; i < count; i++) {
        // Check if the article is from 2016 and its subject contains "politics".
        if (articles[i].year == 2016 && articles[i].subject.find("politics") != string::npos) {
            totalPolitics2016++;
            if (!articles[i].isTrue) {
                fakePolitics2016++;
            }
        }
    }

    if (totalPolitics2016 > 0) {
        double percentage = (static_cast<double>(fakePolitics2016) / totalPolitics2016) * 100;
        cout << "Percentage of fake political news articles in 2016: " << percentage << "%" << endl;
    } else {
        cout << "No political news articles found for the year 2016." << endl;
    }
}

// ---------------------------------------------------------
// tokenize: Split a text string into words (tokens).
// Removes punctuation and converts words to lowercase.
// The tokens are stored in a dynamically allocated array.
// ---------------------------------------------------------
void tokenize(const string &text, string* tokens, int &tokenCount) {
    stringstream ss(text);
    string word;
    tokenCount = 0;
    while (ss >> word) {
        // Remove punctuation.
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        // Convert to lowercase.
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        tokens[tokenCount++] = word;
    }
}

// ---------------------------------------------------------
// linearSearch: Search for a word in the WordFrequency array.
// Returns the index if found, or -1 if not found.
// ---------------------------------------------------------
int linearSearch(WordFrequency* wordFreq, int wordCount, const string &word) {
    for (int i = 0; i < wordCount; i++) {
        if (wordFreq[i].word == word) {
            return i;
        }
    }
    return -1;
}

// ---------------------------------------------------------
// insertAtLastPosition: Insert a word into the WordFrequency array
// at the last position with an initial count of 1.
// ---------------------------------------------------------
void insertAtLastPosition(WordFrequency* wordFreq, int &wordCount, const string &word) {
    wordFreq[wordCount].word = word;
    wordFreq[wordCount].count = 1;
    wordCount++;
}

// ---------------------------------------------------------
// selectionSort: Sort the WordFrequency array in descending order
// based on the count using selection sort.
// ---------------------------------------------------------
void selectionSort(WordFrequency* wordFreq, int wordCount) {
    for (int i = 0; i < wordCount - 1; i++) {
        int maxIdx = i;
        for (int j = i + 1; j < wordCount; j++) {
            if (wordFreq[j].count > wordFreq[maxIdx].count) {
                maxIdx = j;
            }
        }
        swap(wordFreq[i], wordFreq[maxIdx]);
    }
}

// ---------------------------------------------------------
// findMostFrequentWords:
// Filters fake articles with "government" in the subject,
// tokenizes their text, counts word frequencies, sorts them,
// and displays the top N most frequent words.
// ---------------------------------------------------------
void findMostFrequentWords(News *articles, int count, int topN) {
    const int MAX_WORDS = 10000;
    // Dynamically allocate the word frequency array.
    WordFrequency* wordFreq = new WordFrequency[MAX_WORDS];
    int wordCount = 0;

    for (int i = 0; i < count; i++) {
        // Use a case-insensitive check for "government" in the subject.
        // Modify this condition if necessary based on your dataset.
        if (!articles[i].isTrue && articles[i].subject.find("Government News") != string::npos) {
            const int MAX_TOKENS = 10000;
            // Dynamically allocate tokens array.
            string* tokens = new string[MAX_TOKENS];
            int tokenCount;
            tokenize(articles[i].text, tokens, tokenCount);
            for (int j = 0; j < tokenCount; j++) {
                int index = linearSearch(wordFreq, wordCount, tokens[j]);
                if (index != -1) {
                    wordFreq[index].count++;
                } else {
                    insertAtLastPosition(wordFreq, wordCount, tokens[j]);
                }
            }
            delete[] tokens;  // Free tokens array.
        }
    }

    // Sort word frequencies in descending order.
    selectionSort(wordFreq, wordCount);

    cout << "Top " << topN << " most frequent words in fake government news articles:" << endl;
    // Display the top N words.
    for (int i = 0; i < min(topN, wordCount); i++) {
        cout << wordFreq[i].word << ": " << wordFreq[i].count << endl;
    }
    delete[] wordFreq;  // Free word frequency array.
}

// ---------------------------------------------------------
// main: Entry point of the program.
// Prompts the user for an option and calls the corresponding function.
// ---------------------------------------------------------
int main() {
    News* articles = nullptr;
    // Load articles from the CSV file.
    int count = loadArticles("DataCleaned.csv", articles);
    if (count == 0) {
        cerr << "No articles loaded." << endl;
        return 1;
    }

    string option;
    
    // Display options to the user.
    cout << "Enter Your Option:" << endl
         << "1. Sort the news articles by year and display total " << endl
         << "2. Percentage of political news articles (including fake and true news) from the year of 2016" << endl
         << "3. Most frequently words used in fake governments news?" << endl;
    cin >> option;
    
    if (option == "1") {
        sortArticlesByYear(articles, count);
        traverseAndCountArticles(articles, count);
    } else if (option == "2") {
        calculateFakePoliticalNewsPercentage(articles, count);
    } else if (option == "3") {
        int topN;
        cout << "Enter the number of top frequent words to display: ";
        cin >> topN;
        findMostFrequentWords(articles, count, topN);
    } else {
        cout << "Invalid option." << endl;
    }
    
    // Free the dynamically allocated memory for articles.
    delete[] articles;
    return 0;
}
