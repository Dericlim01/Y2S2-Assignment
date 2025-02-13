#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>

using namespace std;

struct News {
    string title, text, subject, date, identify;
    News *next, *head;
};

void def_delimiter(string line) {
    // Declaration of delimiter
    char* str = new char[line.length() + 1]; // Allocate memory
    strcpy(str, line.c_str());

    bool inQuotes = false;
    char* token = str;

    for (int column = 0; column < line.length(); column++) {
        if (str[column] == '"') {
            inQuotes = !inQuotes;
            continue;
        } else if (str[column] == ',' && !inQuotes) {
            str[column] = '\0';

            if (*token == '"' && *(token + strlen(token) - 1) == '"') {
                token[strlen(token) - 1] = '\0';
                token++;
            }
            cout << token << "\n";
            token = str + column + 1;
        }
    }

    if (*token == '"' && *token + strlen(token) - 1 == '"') {
        token[strlen(token) - 1] = '\0';
        token++;
    }

    cout << token << "\n";

    delete[] str;
}

int main(int argc, char const *argv[]) {
    News* news = new News;

    // Open file for reading
    ifstream infile("DataCleaned.csv");

    // Check the file open successfully
    if (!infile) {
        cerr << "Error opening file" << endl;
        return 1;
    }

    string line;
    while (getline(infile, line)) {
        def_delimiter(line);
    }

    cout << "Done" << endl;

    // Close file
    infile.close();
    
    return 0;
}
