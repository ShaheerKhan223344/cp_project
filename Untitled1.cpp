#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

using namespace std;

const string DB_FILE = "parking_db.csv";
const string EXPECTED_KEY = "ACCESS_123";

bool authenticateWithESP32() {
    HANDLE hSerial = CreateFile("\\\\.\\COM9", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hSerial == INVALID_HANDLE_VALUE) {
        cerr << "Error: Cannot open serial port." << endl;
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
    
    cout << "Checking hardware key via ESP32..." << endl;
    Sleep(500);

    if (ReadFile(hSerial, szBuff, 10, &bytesRead, NULL)) {
        string receivedKey(szBuff);
        CloseHandle(hSerial);
        
        if(receivedKey == EXPECTED_KEY) {
            cout << "Authentication Successful! Key: " << receivedKey << endl;
            return true;
        } else {
            cout << "Authentication Failed. Invalid Key: " << receivedKey << endl;
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
        newFile << "SpotID,LicensePlate,Status\n";
        newFile.close();
        cout << "New database created with headers." << endl;
    } else {
        cout << "Database loaded successfully." << endl;
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
        cout << "Computer is currently looking at: [" << currentID << "]" << endl;
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
    cout << "Record added successfully!" << endl;
}

void searchByID(string id) {
    ifstream file(DB_FILE);
    string line, currentID;
    bool found = false;

    while (getline(file, line)) {
        stringstream ss(line);
        getline(ss, currentID, ',');
        
        if (currentID == id) {
            cout << "Data Retrieved: " << line << endl;
            found = true;
            break;
        }
    }
    
    if (!found) {
        cout << "Error: Record with ID " << id << " not found." << endl;
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
        cout << "Record updated successfully!" << endl;
    } else {
        cout << "Update failed: ID not found." << endl;
    }
}

int main() {
    initializeDatabase();
    
    int choice;
    while(true) {
        cout << "\nSMART PARKING DBMS\n";
        cout << "1. Add New Parking Spot\n";
        cout << "2. Search Spot by ID\n";
        cout << "3. Update Spot Data\n";
        cout << "4. Exit\n";
        cout << "Enter choice: ";
        cin >> choice;

        if(choice == 4) break;

        if(!authenticateWithESP32()) {
            cout << "Action Denied. Hardware key required.\n";
            continue;
        }

        string id, plate, status, data;
        
        switch(choice) {
            case 1:
                cout << "Enter Spot ID (e.g., P01): "; cin >> id;
                if(!isUnique(id)) {
                    cout << "Error: Spot ID already exists!\n";
                    break;
                }
                cout << "Enter License Plate: "; cin >> plate;
                cout << "Enter Status (Occupied/Free): "; cin >> status;
                data = id + "," + plate + "," + status;
                appendRecord(data);
                break;
                
            case 2:
                cout << "Enter Spot ID to Search: "; cin >> id;
                searchByID(id);
                break;
                
            case 3:
                cout << "Enter Spot ID to Update: "; cin >> id;
                cout << "Enter New License Plate: "; cin >> plate;
                cout << "Enter New Status (Occupied/Free): "; cin >> status;
                data = id + "," + plate + "," + status;
                updateRecord(id, data);
                break;
                
            default:
                cout << "Invalid choice.\n";
        }
    }
    return 0;
}
