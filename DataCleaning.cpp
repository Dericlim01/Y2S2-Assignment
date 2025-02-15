#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

/**
 * Function to remove leading and trailing spaces, tabs, and newlines from a string.
 * This helps clean up unwanted whitespace from CSV fields.
 */
string trim(const string &str) {                                // Const prevents modification of str in the function, & avoids making a copy of str, improving efficiency
    size_t first_char = str.find_first_not_of(" \t");           // Find first non-space/tab character
    if (first_char == string::npos) return "";                  // Empty or white space only, if line is whitespace, return empty string
    size_t last_char = str.find_last_not_of(" \t");             // Find last non-space/tab character
    return str.substr(first_char, last_char - first_char + 1);  // Extract the trimmed string
}

/**
 * Function to handle missing values in a CSV row.
 * If a field is empty (e.g., ",,"), it replaces it with "NA".
 */
string handleMissingValue(const string &line) {
    stringstream ss(line);      // Entire row line is passed in as a single string, convert it into a stream so can extract each column (separate by ",")
    string token, cleanedLine;  // Variable to store each extracted column and the final cleaned line
    bool firstColumn = true;    // To handle comma placement

    while (getline(ss, token, ',')) {           // Read each field separated by a comma
        token = trim(token);                    // Trim spaces
        if (token.empty()) token = "NA";        // Replace missing values with "NA"

        if (!firstColumn) cleanedLine += ",";   // Add a comma before appending the next value
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
        if ((ch >= 32 && ch <= 126) || ch == '\'') {    // Allow only printable ASCII and apostrophes
            if (ch == '\n' || ch == '\r') {             // If a newline or carriage return is found
                if (!lastWasNewline) {                  // Avoid adding multiple spaces for consecutive newlines
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
 * @param firstPart The first line
 */
string readQuotedField(ifstream &infile, string firstPart) {
    string line, fullField = firstPart;

    while (getline(infile, line)) {                 // Read additional lines if needed
        fullField += "\n" + line;                   // Append the new line to the existing field
        if (line.find('"') != string::npos) break;  // Stop if a closing quote is found
    }

    return fullField;
}

/**
 * Function to convert month name (full or abbreviated) to its two-digit number representation.
 * Example: "December" or "Dec" becomes "12".
 */
string getMonthNumber(const string &month) {
    // This function uses a series of if-else statements to match the month.
    if (month == "January" || month == "Jan")   return "01";
    if (month == "February" || month == "Feb")  return "02";
    if (month == "March" || month == "Mar")     return "03";
    if (month == "April" || month == "Apr")     return "04";
    if (month == "May")                         return "05";
    if (month == "June" || month == "Jun")       return "06";
    if (month == "July" || month == "Jul")       return "07";
    if (month == "August" || month == "Aug")     return "08";
    if (month == "September" || month == "Sep")   return "09";
    if (month == "October" || month == "Oct")     return "10";
    if (month == "November" || month == "Nov")    return "11";
    if (month == "December" || month == "Dec")    return "12";
    return "00"; // Default error case
}

/**
 * Function to reformat a date string from various formats to "DD-MM-YYYY".
 * Handles formats such as:
 * - "December 23, 2017" or "Dec 20, 2017"
 * - "19-Feb-18" or "19-Feb-2018"
 */
string formatDate(const string &dateStr) {
    string ds = trim(dateStr);
    if(ds.empty()) return "NA";
    
    // Check if the format contains a comma, indicating "Month DD, YYYY" format (full or abbreviated month)
    if(ds.find(',') != string::npos) {
        stringstream ss(ds);
        string month, day, year;
        ss >> month >> day >> year; // Extract month, day, and year
        if(!day.empty() && day.back() == ',') {       // Remove the comma from the day if present
            day.pop_back();
        }
        if(year.size() == 2) {                          // Convert short year to four-digit year
            year = "20" + year;
        }
        return day + "-" + getMonthNumber(month) + "-" + year;
    }
    // Check if the format contains '-' indicating a "DD-MMM-YY" or "DD-MMM-YYYY" format
    else if(ds.find('-') != string::npos) {
        size_t firstDash = ds.find('-');
        size_t secondDash = ds.find('-', firstDash + 1);
        if(firstDash == string::npos || secondDash == string::npos) {
            return ds; // Return original if not as expected
        }
        string day = ds.substr(0, firstDash);
        string month = ds.substr(firstDash + 1, secondDash - firstDash - 1);
        string year = ds.substr(secondDash + 1);
        if(year.size() == 2) {                          // Convert short year to four-digit year
            year = "20" + year;
        }
        return day + "-" + getMonthNumber(month) + "-" + year;
    }
    // If no known delimiter is found, return the original string
    else {
        return ds;
    }
}

/**
 * Function to parse a CSV line into its individual columns while correctly handling quoted fields.
 * This function extracts the first four columns: title, text, subject, and date.
 * It manually parses the CSV line without using vector or list.
 */
void parseCSVLine(const string &line, string &title, string &text, string &subject, string &date) {
    string field;          // To accumulate characters for a field
    int col = 0;           // Column counter
    bool inQuotes = false; // Flag to track if we're inside quotes

    // Iterate over each character in the line
    for (size_t i = 0; i < line.size(); i++) {
        char c = line[i];

        if (c == '"') {             // If we encounter a quote
            inQuotes = !inQuotes;   // Toggle the inQuotes flag
        } else if (c == ',' && !inQuotes) { // Comma delimiter outside quotes
            // Assign the accumulated field to the appropriate column
            switch(col) {
                case 0: title = trim(field); break;
                case 1: text = trim(field); break;
                case 2: subject = trim(field); break;
                case 3: date = trim(field); break;
                // We ignore any extra columns here
            }
            col++;         // Move to next column
            field.clear(); // Clear the field accumulator
        } else {
            field.push_back(c); // Append character to the field
        }
    }
    // After the loop, assign the last field (if any)
    if (!field.empty() || col == 3) {
        switch(col) {
            case 0: title = trim(field); break;
            case 1: text = trim(field); break;
            case 2: subject = trim(field); break;
            case 3: date = trim(field); break;
        }
    }
}

/**
 * Function to process a CSV file, clean the data, reformat the date, and add a new "source" column.
 * The source column helps identify whether the data comes from "fake.csv" or "true.csv".
 * @param filename The name of the CSV file to process (e.g., "fake.csv" or "true.csv").
 * @param outfile The output file stream where the cleaned and merged data is written.
 * @param source The source label (either "fake" or "true") that gets added as a new column.
 */
void processCSV(const string &filename, ofstream &outfile, const string &source, bool skipHeader) {
    ifstream infile(filename); // Open the input CSV file

    // Check if the file was opened successfully
    if (!infile) {
        cerr << "Error opening file: " << filename << endl;
        return;
    }

    string line;

    // Skip the first row (header) if needed
    if (skipHeader && getline(infile, line)) {
        // Simply read and discard the first line
    }

    while (getline(infile, line)) {     // Read the CSV file line by line
        line = trim(line);              // Remove unnecessary spaces around the line

        // Handle multi-line quoted fields
        if (!line.empty() && line.front() == '"' && line.find('"', 1) == string::npos) {
            line = readQuotedField(infile, line);
        }

        line = handleSymbols(line);     // Clean symbols from the text

        if (!line.empty()) {            // Ensure the cleaned line isn't empty
            // Instead of handling missing values by splitting on commas,
            // we now parse the line to correctly handle commas inside quoted fields.
            string title, text, subject, date;
            parseCSVLine(line, title, text, subject, date);

            // Handle missing values for each field individually
            if (title.empty())   title = "NA";
            if (text.empty())    text = "NA";
            if (subject.empty()) subject = "NA";
            if (date.empty())    date = "NA";
            else                date = formatDate(date); // Reformat the date

            // Write the cleaned data with the source column added (enclosed in quotes to prevent Excel issues)
            outfile << "\"" << title << "\","
                    << "\"" << text << "\","
                    << "\"" << subject << "\","
                    << "\"" << date << "\","
                    << "\"" << source << "\"\n";
        }
    }

    // Close the input file
    infile.close(); 
}

/**
 * Main function to merge two CSV files ("fake.csv" and "true.csv").
 * Creates a new output file "DataCleaned.csv" with an additional "source" column.
 */
int main() {
    ofstream outfile("DataCleaned.csv"); // Open the output file for writing

    // Check if the file was successfully created
    if (!outfile) {
        cerr << "Error opening DataCleaned.csv for writing!" << endl;
        return 1;
    }

    // Write the header row with an additional "t/f" column
    outfile << "title,text,subject,date,T/F\n";

    // Process both CSV files, skipping headers in them
    processCSV("fake.csv", outfile, "FAKE", true);
    processCSV("true.csv", outfile, "TRUE", true);

    outfile.close(); // Close the output file
    cout << "Cleaning complete: DataCleaned.csv" << endl;

    return 0;
}
