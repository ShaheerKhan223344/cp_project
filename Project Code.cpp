#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

const string CSV_FILE = "Blood_Director_database.csv";

// Struct representing the Blood Bank Data Model
struct Donor {
    string donorID;
    string bloodGroup;
    string lastDonationDate;
};

//------------------------------------------------------------------
// 1. Mandatory Function: initializeDatabase()
//------------------------------------------------------------------
// Checks for the CSV file and creates it with headers if missing [cite: 24]
void initializeDatabase() {
    ifstream file(CSV_FILE);
    if (!file.good()) {
        ofstream outFile(CSV_FILE);
        // Write standard CSV headers matching Blood Bank theme [cite: 18, 24]
        outFile << "DonorID,BloodGroup,LastDonationDate\n";
        outFile.close();
        cout << "[SYSTEM] database.csv not found. Created new file with headers.\n";
    } else {
        cout << "[SYSTEM] Existing database.csv detected successfully.\n";
        file.close();
    }
}

//------------------------------------------------------------------
// 2. Mandatory Function: isUnique(string id)
//------------------------------------------------------------------
// Scans the CSV to prevent duplicate IDs during the "Write" process [cite: 25]
bool isUnique(string id) {
    ifstream file(CSV_FILE);
    string line;
    
    // Skip the header row
    if (getline(file, line)) {
        while (getline(file, line)) {
            stringstream ss(line);
            string currentID;
            getline(ss, currentID, ','); // Read up to first comma
            
            if (currentID == id) {
                file.close();
                return false; // Found a duplicate!
            }
        }
    }
    file.close();
    return true; // ID is completely unique
}

//------------------------------------------------------------------
// 3. Mandatory Function: appendRecord(string data)
//------------------------------------------------------------------
// Adds a new comma-separated record to the end of the CSV file [cite: 26]
void appendRecord(string data) {
    ofstream file(CSV_FILE, ios::app); // Open in append mode
    if (file.is_open()) {
        file << data << "\n";
        file.close();
        cout << "Record appended successfully.\n";
    } else {
        cout << "Error: Unable to open database file for writing.\n";
    }
}

//------------------------------------------------------------------
// 4. Mandatory Function: searchByID(string id)
//------------------------------------------------------------------
// Performs a sequential search to find and return a specific record [cite: 27]
string searchByID(string id) {
    ifstream file(CSV_FILE);
    string line;
    
    // Skip header line
    if (getline(file, line)) {
        while (getline(file, line)) {
            // Match structural criteria from Project Spec Example [cite: 59]
            if (line.find(id) == 0) { 
                file.close();
                return line; // Return full comma-separated line matching id
            }
        }
    }
    file.close();
    return ""; // Return empty if not found
}

//------------------------------------------------------------------
// 5. Mandatory Function: updateRecord(string id, string newData)
//------------------------------------------------------------------
// Modifies or deletes an entry using a temporary file for integrity [cite: 28]
void updateRecord(string id, string newData) {
    ifstream file(CSV_FILE);
    ofstream tempFile("temp.csv"); // Temporary file architecture [cite: 28]
    
    string line;
    bool updated = false;
    
    while (getline(file, line)) {
        // Look sequentially for the ID target row
        if (line.find(id) == 0) {
            if (!newData.empty()) {
                // If newData is passed, modify the line [cite: 28]
                tempFile << newData << "\n";
                updated = true;
            } else {
                // If newData is empty string, skip writing it (Deletes the record) [cite: 28]
                updated = true;
            }
        } else {
            // Write unaffected data directly to temp file
            tempFile << line << "\n";
        }
    }
    
    file.close();
    tempFile.close();
    
    // Delete old database and replace with clean temp copy [cite: 28]
    remove(CSV_FILE.c_str());
    rename("temp.csv", CSV_FILE.c_str());
    
    if (updated) {
        cout << "Database updated successfully.\n";
    } else {
        cout << "Record ID not found. No modifications executed.\n";
    }
}

//------------------------------------------------------------------
// UI / Application Logic Connection Layer
//------------------------------------------------------------------
void displayMenu() {
    cout << "\n=========================================\n";
    cout << " BLOOD BANK HYBRID MANAGEMENT INTERFACE  \n";
    cout << "=========================================\n";
    cout << "1. Create/Verify Local Database file\n";
    cout << "2. Register New Donor (Requires Unique Key validation)\n";
    cout << "3. Search Donor Records via Unique Key ID\n";
    cout << "4. Modify Existing Donor Record Entries\n";
    cout << "5. Delete Donor Record from Directory\n";
    cout << "6. Exit Application\n";
    cout << "Enter selection option (1-6): ";
}

int main() {
    initializeDatabase(); // Always run standard initial checks on boot [cite: 24]
    
    int UserSelection = 0;
    while (UserSelection != 6) {
        displayMenu();
        if (!(cin >> UserSelection)) {
            cout << "Invalid alphanumeric entry received. Resetting input parser.\n";
            cin.clear();
            cin.ignore(10000, '\n');
            continue;
        }
        
        switch (UserSelection) {
            case 1: {
                initializeDatabase();
                break;
            }
            case 2: {
                string id, bg, date;
                cout << "\n[ESP32 SIMULATED PORT INTERFACE Active]\n";
                cout << "Enter Unique Donor ID Key: ";
                cin >> id;
                
                // Invoke mandatory unique code check guard [cite: 25]
                if (!isUnique(id)) {
                    cout << "Execution Halted: Key sequence matches an existing entry!\n";
                    break;
                }
                
                cout << "Enter Blood Group (e.g., O+, AB-, A+): ";
                cin >> bg;
                cout << "Enter Last Donation Date (DD/MM/YYYY): ";
                cin >> date;
                
                // Construct standard comma-separated format string package
                string rawCsvLine = id + "," + bg + "," + date;
                appendRecord(rawCsvLine); // Invoke mandatory append call [cite: 26]
                break;
            }
            case 3: {
                string searchKey;
                cout << "Provide Target Donor ID Search Sequence: ";
                cin >> searchKey;
                
                string result = searchByID(searchKey); // Invoke sequential search call [cite: 27]
                if (!result.empty()) {
                    cout << "\n[DATA RECORD LOCATED]: " << result << "\n";
                } else {
                    cout << "\n[NOT FOUND]: Identifier sequence does not match directory.\n";
                }
                break;
            }
            case 4: {
                string id, newBg, newDate;
                cout << "Enter Target Donor ID Sequence to Edit: ";
                cin >> id;
                
                string checkedRecord = searchByID(id);
                if (checkedRecord.empty()) {
                    cout << "Target identifier doesn't exist.\n";
                    break;
                }
                
                cout << "Enter Updated Blood Group: ";
                cin >> newBg;
                cout << "Enter Updated Last Donation Date (DD/MM/YYYY): ";
                cin >> newDate;
                
                string modernPackedLine = id + "," + newBg + "," + newDate;
                updateRecord(id, modernPackedLine); // Execute mandatory file sweep modify [cite: 28]
                break;
            }
            case 5: {
                string id;
                cout << "Enter Target Donor ID Sequence to Remove permanently: ";
                cin >> id;
                
                // Passing empty string inside modern parameter triggers code deletion block [cite: 28]
                updateRecord(id, ""); 
                break;
            }
            case 6:
                cout << "\nTerminating Local Control session safely. File streams saved.\n";
                break;
            default:
                cout << "Selection index outside expected criteria options.\n";
        }
    }
    return 0;
}
