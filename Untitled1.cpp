#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

const string DB_FILE = "blood_bank_db.csv";
const string EXPECTED_KEY = "ACCESS_123";

bool authenticateWithESP32() {
    HANDLE hSerial = CreateFile("\\\\.\\COM6", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hSerial == INVALID_HANDLE_VALUE) {
        cerr << "\n[Error] Cannot open serial port COM6. Ensure ESP32 is plugged in." << endl;
        return false;
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    GetCommState(hSerial, &dcbSerialParams);
    dcbSerialParams.BaudRate = CBR_115200;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;
    SetCommState(hSerial, &dcbSerialParams);

    DWORD bytesWritten;
    char request = 'R';
    WriteFile(hSerial, &request, 1, &bytesWritten, NULL);

    char szBuff[11] = { 0 };
    DWORD bytesRead;
    
    cout << "Checking hardware key via ESP32 on COM6..." << endl;
    Sleep(500);

    if (ReadFile(hSerial, szBuff, 10, &bytesRead, NULL)) {
        string receivedKey(szBuff);
        CloseHandle(hSerial);
        
        if(receivedKey == EXPECTED_KEY) {
            cout << "Authentication Successful! Key Verified: " << receivedKey << endl;
            return true;
        } else {
            cout << "Authentication Failed. Invalid Token: " << receivedKey << endl;
            return false;
        }
    }
    
    CloseHandle(hSerial);
    return false;
}

void initializeDatabase() {
    ifstream file(DB_FILE);
    if (!file.is_open()) {
        ofstream newFile(DB_FILE);
        newFile << "DonorID,BloodType,Units,DonorName\n";
        newFile.close();
        cout << "New blood bank database created with system headers." << endl;
    } else {
        cout << "Blood bank database loaded successfully." << endl;
        file.close();
    }
}

bool isUnique(string id) {
    ifstream file(DB_FILE);
    string line, currentID;
    
    getline(file, line);
    
    while (getline(file, line)) {
        stringstream ss(line);
        getline(ss, currentID, ',');
        
        cout << "Computer is currently searching row index: [" << currentID << "]" << endl;
        if (currentID == id) {
            return false;
        }
    }
    return true;
}

void appendRecord(string data) {
    ofstream file;
    file.open(DB_FILE, ios_base::app); 
    file << data << "\n";
    file.close();
    cout << "Blood record added to database successfully!" << endl;
}

void searchByID(string id) {
    ifstream file(DB_FILE);
    string line, currentID;
    bool found = false;

    while (getline(file, line)) {
        stringstream ss(line);
        getline(ss, currentID, ',');
        
        if (currentID == id) {
            cout << "\n>>> Data Retrieved: " << line << endl;
            found = true;
            break;
        }
    }
    
    if (!found) {
        cout << "Error: Record with ID " << id << " not found in system." << endl;
    }
    file.close();
}

void updateRecord(string id, string newData) {
    ifstream fileIn(DB_FILE);
    ofstream fileOut("temp.csv");
    string line, currentID;
    bool updated = false;

    while (getline(fileIn, line)) {
        stringstream ss(line);
        getline(ss, currentID, ',');
        
        if (currentID == id) {
            fileOut << newData << "\n"; 
            updated = true;
        } else {
            fileOut << line << "\n"; 
        }
    }
    
    fileIn.close();
    fileOut.close();

    remove(DB_FILE.c_str());
    rename("temp.csv", DB_FILE.c_str());

    if(updated) {
        cout << "Blood inventory database updated successfully!" << endl;
    } else {
        cout << "Update failed: Provided ID not found." << endl;
    }
}

int main() {
    initializeDatabase();
    
    int choice;
    while(true) {
        cout << "\n=========================================" << endl;
        cout << "      BLOOD BANK SECURE ENGINE SYSTEM    " << endl;
        cout << "=========================================" << endl;
        cout << "1. Add New Blood Record / Batch\n";
        cout << "2. Search Blood Record by ID\n";
        cout << "3. Update Blood Stock Data\n";
        cout << "4. Exit System\n";
        cout << "Enter choices (1-4): ";
        cin >> choice;

        if(choice == 4) break;

        if(!authenticateWithESP32()) {
            cout << "Action Denied. Valid hardware authorization required.\n";
            continue;
        }

        string id, bloodType, units, donorName, data;
        
        switch(choice) {
            case 1:
                cout << "Enter Donor/Batch ID (e.g., B01): "; cin >> id;
                if(!isUnique(id)) {
                    cout << "Error: Entry with this ID already exists!\n";
                    break;
                }
                cout << "Enter Blood Type (e.g., A+, O-, AB+): "; cin >> bloodType;
                cout << "Enter Volume Units: "; cin >> units;
                cout << "Enter Donor Name: "; 
                cin.ignore();
                getline(cin, donorName);
                
                data = id + "," + bloodType + "," + units + "," + donorName;
                appendRecord(data);
                break;
                
            case 2:
                cout << "Enter Record ID to Search: "; cin >> id;
                searchByID(id);
                break;
                
            case 3:
                cout << "Enter Record ID to Update: "; cin >> id;
                cout << "Enter New Blood Type: "; cin >> bloodType;
                cout << "Enter New Volume Units: "; cin >> units;
                cout << "Enter New Donor Name: "; 
                cin.ignore();
                getline(cin, donorName);
                
                data = id + "," + bloodType + "," + units + "," + donorName;
                updateRecord(id, data);
                break;
                
            default:
                cout << "Invalid runtime choice selected.\n";
        }
    }
    return 0;
}
