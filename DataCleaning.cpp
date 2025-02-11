#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

/**
 * Function to remove leading and trailing spaces, tabs, and newlines from a string.
 * This helps clean up unwanted whitespace from CSV fields.
 */
string trim(const string &str) {
    size_t first = str.find_first_not_of(" \t\n");
    if (first == string::npos) return ""; // If the string is empty or contains only spaces
    size_t last = str.find_last_not_of(" \t\n");
    return str.substr(first, last - first + 1);
}

/**
 * Function to handle missing values in a CSV row.
 * If a field is empty (e.g., ",,"), it replaces it with "NA".
 */
string handleMissingValue(const string &line) {
    stringstream ss(line);  // Convert the line into a stream for easy parsing
    string token, cleanedLine;
    bool firstColumn = true;

    while (getline(ss, token, ',')) { // Read each field separated by a comma
        token = trim(token); // Trim spaces
        if (token.empty()) token = "NA"; // Replace missing values with "NA"

        if (!firstColumn) cleanedLine += ","; // Add a comma before appending the next value
        cleanedLine += token;
        firstColumn = false;
    }

    return cleanedLine;
}

/**
 * Function to remove unwanted symbols and non-printable ASCII characters from a string.
 * Keeps necessary symbols like apostrophes ('), letters, numbers, and standard punctuation.
 */
string handleSymbols(const string &str) {
    string cleaned;
    bool lastWasNewline = false; // Track if the last character was a newline to avoid excessive spaces

    for (char ch : str) {
        if ((ch >= 32 && ch <= 126) || ch == '\'') { // Allow only printable ASCII and apostrophes
            if (ch == '\n' || ch == '\r') { // If a newline or carriage return is found
                if (!lastWasNewline) { // Avoid adding multiple spaces for consecutive newlines
                    cleaned += " ";
                    lastWasNewline = true;
                }
            } else {
                cleaned += ch;
                lastWasNewline = false;
            }
        }
    }

    return trim(cleaned); // Trim spaces at the start and end before returning
}

/**
 * Function to handle multi-line quoted fields in a CSV file.
 * Some text fields in CSVs can have multiple lines enclosed in quotes.
 * This function reads until the closing quote is found.
 */
string readQuotedField(ifstream &infile, string firstPart) {
    string line, fullField = firstPart;

    while (getline(infile, line)) { // Read additional lines if needed
        fullField += "\n" + line; // Append the new line to the existing field
        if (line.find('"') != string::npos) break; // Stop if a closing quote is found
    }

    return fullField;
}

/**
 * Function to process a CSV file, clean the data, and add a new "source" column.
 * The source column helps identify whether the data comes from "fake.csv" or "true.csv".
 * 
 * @param filename The name of the CSV file to process (e.g., "fake.csv" or "true.csv").
 * @param outfile The output file stream where the cleaned and merged data is written.
 * @param source The source label (either "fake" or "true") that gets added as a new column.
 */
void processCSV(const string &filename, ofstream &outfile, const string &source, bool skipHeader) {
    ifstream infile(filename); // Open the input CSV file

    if (!infile) { // Check if the file was opened successfully
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string line;

    // Skip the first row (header) if needed
    if (skipHeader && getline(infile, line)) {
        // Simply read and discard the first line
    }

    while (getline(infile, line)) { // Read the CSV file line by line
        line = trim(line); // Remove unnecessary spaces around the line

        // Handle multi-line quoted fields
        if (!line.empty() && line.front() == '"' && line.find('"', 1) == string::npos) {
            line = readQuotedField(infile, line);
        }

        line = handleSymbols(line); // Clean symbols from the text
        if (!line.empty()) { // Ensure the cleaned line isn't empty
            line = handleMissingValue(line); // Handle missing values (empty fields)
            line += ",\"" + source + "\""; // Add the "source" column (enclosed in double quotes to prevent Excel issues)
            outfile << line << "\n"; // Write to the output file
        }
    }

    infile.close(); // Close the input file
}


/**
 * Main function to merge two CSV files ("fake.csv" and "true.csv").
 * Creates a new output file "DataCleaned.csv" with an additional "source" column.
 */
int main() {
    ofstream outfile("DataCleaned.csv"); // Open the output file for writing

    if (!outfile) { // Check if the file was successfully created
        cerr << "Error opening DataCleaned.csv for writing!" << endl;
        return 1;
    }

    // Write the header row with an additional "t/f" column
    outfile << "title,text,subject,date,t/f\n";

    // Process both CSV files, skipping headers in them
    processCSV("fake.csv", outfile, "FAKE", true);
    processCSV("true.csv", outfile, "TRUE", true);

    outfile.close(); // Close the output file
    cout << "Cleaning complete: DataCleaned.csv" << endl;

    return 0;
}
