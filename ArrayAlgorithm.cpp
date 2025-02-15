#include <iostream>
#include <fstream>
#include <string>
#include <cctype>  // for toupper()
using namespace std;

struct News {
    string title;
    string text;
    string subject;
    string date;
    bool isFake;
    int year;
};

int main() {
    const int MAX_ARTICLES = 50000;
    // Dynamically allocate an array of News objects on the heap.
    News* articles = new News[MAX_ARTICLES];
    int articleCount = 0;

    ifstream file("DataCleaned.csv");
    if (!file) {
        cerr << "Error opening file." << endl;
        delete[] articles;
        return 1;
    }

    // Skip the header line.
    string header;
    getline(file, header);

    string line;
    int lineNumber = 1; // Header is line 1.
    while (getline(file, line) && articleCount < MAX_ARTICLES) {
        lineNumber++;
        // --- Multi-line record handling ---
        int quoteCount = 0;
        for (char c : line)
            if (c == '"')
                quoteCount++;
        // If quote count is odd, append lines until quotes balance.
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

        // --- Parse the record into fields using a fixed array ---
        // We assume at most 20 fields can be split.
        string tempFields[20];
        int fieldCount = 0;
        bool inQuotes = false;
        string currentField = "";

        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];
            if (c == '"') {
                // Handle escaped quotes: if inside quotes and next char is also a quote.
                if (inQuotes && i + 1 < line.size() && line[i + 1] == '"') {
                    currentField.push_back('"');
                    i++; // Skip the escaped quote.
                } else {
                    inQuotes = !inQuotes; // Toggle the inQuotes flag.
                }
            } else if (c == ',' && !inQuotes) {
                // Comma outside quotes signals end of current field.
                tempFields[fieldCount++] = currentField;
                currentField = "";
            } else {
                currentField.push_back(c);
            }
        }
        // Add the last field.
        tempFields[fieldCount++] = currentField;

        // --- Collapse extra fields if there are more than 5 ---
        // We expect exactly 5 columns:
        // 0: title, 1: text, 2: subject, 3: date, 4: T/F field.
        if (fieldCount < 5) {
            cerr << "Warning (line " << lineNumber << "): Expected at least 5 fields, got " << fieldCount 
                 << ". Skipping this record." << endl;
            continue; // Skip malformed record.
        }
        if (fieldCount > 5) {
            string combined = tempFields[4];
            for (int i = 5; i < fieldCount; i++) {
                combined += "," + tempFields[i];
            }
            tempFields[4] = combined;
            fieldCount = 5;
        }

        // Replace empty fields with "NA".
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

        // Extract the year from the date.
        // Assuming the date is in "DD-MM-YYYY" format, extract the last 4 characters.
        if (article.date != "NA" && article.date.size() >= 10) {
            try {
                article.year = stoi(article.date.substr(article.date.size() - 4, 4));
            } catch (...) {
                article.year = 0;
            }
        } else {
            article.year = 0;
        }

        // Process the T/F field (column 5). Convert to uppercase.
        string tfField = tempFields[4];
        for (size_t i = 0; i < tfField.size(); i++) {
            tfField[i] = toupper(tfField[i]);
        }
        // Original logic: article.isFake = (tfField == "FAKE");
        // Changed logic: Invert the boolean value so that if it was false it becomes true.
        // For example, if tfField equals "FAKE", we now want isFake to be false, otherwise true.
        article.isFake = !(tfField == "FAKE");

        articles[articleCount++] = article;
    }
    file.close();

    // --- Output verification for every article ---
    // Print date, subject, and the isFake (true/false) for each article.
    for (int i = 0; i < articleCount; i++) {
        cout << "Article " << i + 1 << ": ";
        cout << "Date: " << articles[i].date << ", ";
        cout << "Subject: " << articles[i].subject << ", ";
        cout << "isFake: " << (articles[i].isFake ? "true" : "false") << endl;
    }

    // Optionally, print details for the first article.
    if (articleCount > 0) {
        cout << "\nFirst article details:" << endl;
        cout << "  Title:   " << articles[0].title << endl;
        cout << "  Text:    " << articles[0].text << endl;
        cout << "  Subject: " << articles[0].subject << endl;
        cout << "  Date:    " << articles[0].date << " (Year: " << articles[0].year << ")" << endl;
        cout << "  isFake:  " << (articles[0].isFake ? "true" : "false") << endl;
    }

    // Free the dynamically allocated memory.
    delete[] articles;
    return 0;
}
