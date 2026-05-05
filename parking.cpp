#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <ctime>
 #include <cstdlib>
 #include <string>
#include <regex>
#include <algorithm>
#include <cstdio>

using namespace std;

// ---------------- VEHICLE ----------------
class Vehicle {
protected:
    string vehicleNumber;
    time_t entryTime;

public: 
    Vehicle(string num, time_t entry = time(0)) {
        vehicleNumber = num;
        entryTime = entry;
    }

    virtual float calculateCharge(int hours) = 0;
    string getVehicleNumber() { return vehicleNumber; }
    time_t getEntryTime() { return entryTime; }
    virtual int getType() = 0;
    virtual ~Vehicle() {}
};

class Car : public Vehicle {
    bool isVIP;
    bool isEmergency;
//constructor calls parent constructor
public:
    Car(string num, bool vip=false, bool emergency=false, time_t entry = time(0))
        : Vehicle(num, entry)
    {
        isVIP = vip;
        isEmergency = emergency;
    }
//function overriding->polymorphism
    float calculateCharge(int hours) {

        if(isVIP)
            return hours * 100;     // VIP Price

        if(isEmergency)
            return hours * 80;      // Emergency Price

        return hours * 50;          // Normal Price
    }

    int getType() { return 1; }
};

class Bike : public Vehicle {
public:
    Bike(string num, time_t entry = time(0)) : Vehicle(num, entry) {}
    float calculateCharge(int hours) { return hours * 20; }
    int getType() { return 2; }
};

class ParkingSlot {
    string slotID;
    int type;       // 1 = Car, 2 = Bike
    bool occupied;
    bool vipSlot;
    bool emergencySlot;
//constructor initialises slot properties
public:
    ParkingSlot(string id, int t, bool vip=false, bool emergency=false) {
        slotID = id;
        type = t;
        occupied = false;
        vipSlot = vip;
        emergencySlot = emergency;
    }

    bool isOccupied() { return occupied; }
    int getType() { return type; }
    string getSlotID() { return slotID; }
    bool isVIP() { return vipSlot; }
    bool isEmergency() { return emergencySlot; }

    void occupy() { occupied = true; }
    void release() { occupied = false; }
};     

class Ticket {
private:
    string slotID;
    Vehicle* vehicle;//pointer to vehicle object polymorphism
    string mobile;
    int otp;
    time_t entryTime;   // store entry time in ticket

public:
//ticket constructor
    Ticket(string slot, Vehicle* v, string mob, time_t entry, int existingOtp = -1) {
    slotID = slot;
    vehicle = v;
    mobile = mob;
    entryTime = entry;   // now entry is properly defined

    if (existingOtp == -1) {//generate otp if ticket is new
        otp = 1000 + rand() % 9000;//random 4 digit otp
        cout << "\nOTP for Exit: " << otp << endl;
    } else {
        otp = existingOtp;//used when loading from file
    }
}

    string getMobile() { return mobile; }
    string getVehicleNumber() { return vehicle->getVehicleNumber(); }
    string getSlotID() { return slotID; }
    int getType() { return vehicle->getType(); }
    time_t getEntryTime() { return entryTime; }
    int getOTP() { return otp; }

    bool verifyOTP(int entered) {
        return entered == otp;
    }

    float closeTicket(int paymentChoice) {

        time_t exitTime = time(0);

        double seconds = difftime(exitTime, entryTime);
        double totalHours = seconds / 3600.0;

        int chargeableHours = ceil(totalHours);
        if (chargeableHours <= 0) chargeableHours = 1;
        //dynamic binding
        float charge = vehicle->calculateCharge(chargeableHours);

        if(chargeableHours > 12) {
            charge += 100;
            cout << "Overstay Fine Applied: Rs 100\n";
        }

        string payment = (paymentChoice == 1) ? "UPI" :
                         (paymentChoice == 2) ? "Card" : "Cash";

        cout << "\nPayment Successful via " << payment << endl;

        int totalMinutes = seconds / 60;
        int hrs = totalMinutes / 60;
        int mins = totalMinutes % 60;

        cout << "\n------ BILL RECEIPT ------\n";
        cout << "Vehicle: " << vehicle->getVehicleNumber() << endl;
        cout << "Slot: " << slotID << endl;
        cout << "Parked Time: " << hrs << " hr " << mins << " min\n";
        cout << "Chargeable Hours: " << chargeableHours << endl;
        cout << "Amount: Rs " << charge << endl;
        cout << "--------------------------\n";

        return charge;
    }

    ~Ticket() { delete vehicle; }
};

// ---------------- SYSTEM ----------------
class ParkingSystem {

    vector<ParkingSlot> slots;
    vector<Ticket*> activeTickets;
    string adminPassword;
    float totalRevenue = 0;

public:
    string getAdminPassword() {
        return adminPassword;
    }

    ParkingSystem() {

        srand(time(0));
        loadAdminPassword(); 
        //load previous revenue from file
        ifstream revFile("revenue.txt");
        if(revFile)
            revFile >> totalRevenue;
        revFile.close();

        // CAR SLOTS
        for(int i=1;i<=10;i++) {

            string slot = "A" + to_string(i);

            if(i==1)
                slots.push_back(ParkingSlot(slot,1,true,false));   // VIP
            else if(i==2)
                slots.push_back(ParkingSlot(slot,1,false,true));   // Emergency
            else
                slots.push_back(ParkingSlot(slot,1));
        }

        // BIKE SLOTS
        for(int i=1;i<=10;i++) {

            string slot = "B" + to_string(i);
            slots.push_back(ParkingSlot(slot,2));
        }

        loadFromFile();
    }
    void loadAdminPassword() {
        ifstream file("adminPassword.txt");
        if(file)
            file >> adminPassword;
        // else
        //     adminPassword = "admin123"; // fallback
        file.close();
    }
    float getTodayRevenue() {
        ifstream file("daily_revenue.txt");
        string date;
        float amount;

        while(file >> date >> amount) {
            if(date == getTodayDate())
                return amount;
        }
        return 0;
    }

    float getMonthRevenue() {
        ifstream file("daily_revenue.txt");
        string date;
        float amount;
        float total = 0;

        while(file >> date >> amount) {
            if(date.substr(0,7) == getMonthPrefix())
                total += amount;
        }
        return total;
    }

    float getOverallRevenue() {
        return totalRevenue;
    }

    void showTodayHistory() {
        ifstream file("history.txt");
        if (!file) {
            cout << "No History Found\n";
            return;
        }

        string number, slot;
        time_t exitTime;

        cout << "\n----- TODAY HISTORY -----\n";

        while (file >> number >> slot >> exitTime) {
            tm *ltm = localtime(&exitTime);

            char date[11];
            sprintf(date,"%04d-%02d-%02d",
                1900 + ltm->tm_year,
                ltm->tm_mon + 1,
                ltm->tm_mday);

            if(string(date) == getTodayDate()) {
                cout << "Vehicle: " << number
                     << " | Slot: " << slot
                     << " | Exit Time: "
                     << ctime(&exitTime);
            }
        }

        file.close();
    }

    //  MONTH HISTORY
    void showMonthHistory() {
        ifstream file("history.txt");
        if (!file) {
            cout << "No History Found\n";
            return;
        }

        string number, slot;
        time_t exitTime;

        cout << "\n----- MONTH HISTORY -----\n";

        while (file >> number >> slot >> exitTime) {
            tm *ltm = localtime(&exitTime);

            char date[11];
            sprintf(date,"%04d-%02d-%02d",
                1900 + ltm->tm_year,
                ltm->tm_mon + 1,
                ltm->tm_mday);
            if(string(date).substr(0,7) == getMonthPrefix()) {
                cout << "Vehicle: " << number
                     << " | Slot: " << slot
                     << " | Exit Time: "
                     << ctime(&exitTime);
            }
        }

        file.close();
    }

    void showFilteredMonthHistory(string targetMonth) {
    ifstream file("history.txt");
    if (!file) {
        cout << "No History Found\n";
        return;
    }

    string number, slot;
    time_t exitTime;
    bool found = false;

    cout << "\n----- HISTORY FOR " << targetMonth << " -----\n";

    while (file >> number >> slot >> exitTime) {
        tm *ltm = localtime(&exitTime);
        char dateBuf[8]; 
        sprintf(dateBuf, "%04d-%02d", 1900 + ltm->tm_year, 1 + ltm->tm_mon);
        
        if (string(dateBuf) == targetMonth) {
            cout << "Vehicle: " << number << " | Slot: " << slot 
                 << " | Exit Time: " << ctime(&exitTime);
            found = true;
        }
    }
    if(!found) cout << "No records found for this month.\n";
    file.close();
}

    // ✅ EXISTING FUNCTIONS (UNCHANGED BELOW)

    void saveToFile() {
        ofstream file("data.txt");
        for (auto t : activeTickets) {
            file << t->getVehicleNumber() << " "
                 << t->getSlotID() << " "
                 << t->getType() << " "
                 << t->getEntryTime() << " "
                 << t->getOTP() << " "
                 << t->getMobile() << "\n";
        }
        file.close();
    }
//load active tickets from file
void loadFromFile() {

    ifstream file("data.txt");
    if (!file) return;

    string number, slotID, mobile;
    int type, otp;
    time_t entry;

    while (file >> number >> slotID >> type >> entry >> otp>> mobile) {
        Vehicle* v;
        bool vip = false;
        bool emergency = false;

        for (auto &slot : slots) {
            if (slot.getSlotID() == slotID) {
                if (slot.isVIP()) vip = true;
                if (slot.isEmergency()) emergency = true;
                slot.occupy();
            }
        }
        if(type == 1)
             v = new Car(number, vip, emergency, entry);
        else
            v = new Bike(number, entry);
        activeTickets.push_back(new Ticket(slotID, v, mobile, entry, otp));
    }

    file.close();
}

    bool isValidIndianNumber(string number) {
        regex pattern("^[A-Z]{2}[0-9]{2}[A-Z]{2}[0-9]{4}$");
        return regex_match(number, pattern);
    }

    bool isValidMobile(string mobile) {
    regex pattern("^[6-9][0-9]{9}$");
    return regex_match(mobile, pattern);
}

    bool isDuplicate(string number) {
        for (auto t : activeTickets)
            if (t->getVehicleNumber() == number)
                return true;
        return false;
    }

    void parkVehicle(string number, int type, string mobile) {
        number.erase(remove(number.begin(), number.end(), ' '), number.end());
        number.erase(remove(number.begin(), number.end(), '-'), number.end());
        transform(number.begin(), number.end(), number.begin(), ::toupper);
        if(!isValidMobile(mobile)) {
            cout << "Invalid Indian Mobile Number!\n";
            return;
        }

        if (!isValidIndianNumber(number)) {
            cout << "Invalid Indian Vehicle Number Format!\n";
            return;
        }

        if (isDuplicate(number)) {
            cout << "Duplicate Vehicle Entry!\n";
            return;
        }

        for (auto &slot : slots) {

             if (!slot.isOccupied() &&
                 slot.getType() == type &&
                !slot.isVIP() &&
                !slot.isEmergency()) {

                    Vehicle* v;
             if(type == 1)
                v = new Car(number, false, false);
             else
                v = new Bike(number);

                slot.occupy();
                activeTickets.push_back(new Ticket(slot.getSlotID(), v, mobile, time(0)));
                saveToFile();

                cout << "SUCCESS: Vehicle Parked at Slot "
                     << slot.getSlotID() << endl;

                return;
            }
        }
        cout << "Parking Full!\n";
    }

void showRevenue() {
    ifstream revFile("revenue.txt");
    if(revFile) revFile >> totalRevenue;
    revFile.close();

    cout << "\n========== 💰 REVENUE REPORT ==========\n";
    cout << "TOTAL REVENUE       : Rs " << totalRevenue << endl;
    cout << "MONTH REVENUE       : Rs " << getMonthRevenue() << endl;
    cout << "TODAY REVENUE       : Rs " << getTodayRevenue() << endl;
    cout << "--------------------------------------\n";
    
    // Show the daily list
    showDailyRevenue(); 
    
    cout << "======================================\n";
}
    void saveRevenue() {
        ofstream file("revenue.txt");
        file << totalRevenue;
        file.close();
    }

    void exitVehicle(string number, int enteredOTP, int paymentChoice) {

        transform(number.begin(), number.end(), number.begin(), ::toupper);

        for (int i = 0; i < activeTickets.size(); i++) {

            if (activeTickets[i]->getVehicleNumber() == number) {

                if (!activeTickets[i]->verifyOTP(enteredOTP)) {
                    cout << "Wrong OTP!\n";
                    return;
                }

                // ✅ STORE DATA BEFORE DELETE
                string vehicleNumber = activeTickets[i]->getVehicleNumber();
                string slotID = activeTickets[i]->getSlotID();
                time_t exitTime = time(0);

                float amount = activeTickets[i]->closeTicket(paymentChoice);
                totalRevenue += amount;
                saveRevenue();
                updateDailyRevenue(amount);

                // ✅ SAVE TO HISTORY FILE
                ofstream history("history.txt", ios::app);
                history << vehicleNumber << " "
                        << slotID << " "
                        << exitTime << "\n";
                history.close();

                // release slot
                for (auto &slot : slots)
                    if (slot.getSlotID() == slotID)
                        slot.release();

                // delete ticket safely
                delete activeTickets[i];
                activeTickets.erase(activeTickets.begin() + i);

                saveToFile();

                cout << "Vehicle Exited Successfully!\n";
                return;
            }
        }

        cout << "Vehicle Not Found!\n";
    }

 void adminForceExit(string number) {

    number.erase(remove(number.begin(), number.end(), ' '), number.end());
    number.erase(remove(number.begin(), number.end(), '-'), number.end());
    transform(number.begin(), number.end(), number.begin(), ::toupper);

    for (int i = 0; i < activeTickets.size(); i++) {

        if (activeTickets[i]->getVehicleNumber() == number) {

            cout << "\n⚠ ADMIN FORCE EXIT INITIATED ⚠\n";

            // Store details before deleting
            string vehicleNumber = activeTickets[i]->getVehicleNumber();
            string slotID = activeTickets[i]->getSlotID();
            time_t exitTime = time(0);

            // Close ticket with Cash by default (3)
            float amount = activeTickets[i]->closeTicket(3); 
            totalRevenue += amount;
            saveRevenue();
            updateDailyRevenue(amount);

            // Save history
            ofstream history("history.txt", ios::app);
            history << vehicleNumber << " "
                    << slotID << " "
                    << exitTime << "\n";
            history.close();

            // Release slot
            for (auto &slot : slots)
                if (slot.getSlotID() == slotID)
                    slot.release();

            delete activeTickets[i];
            activeTickets.erase(activeTickets.begin() + i);

            saveToFile();

            cout << "Vehicle Force Removed Successfully!\n";
            return;
        }
    }

    cout << "Vehicle Not Found!\n";
}


string getTodayDate() {
    time_t now = time(0);
    tm *ltm = localtime(&now);

    char today[11];
    // Corrected: 1900 + year and 1 + month
    sprintf(today, "%04d-%02d-%02d",
            1900 + ltm->tm_year,
            1 + ltm->tm_mon, 
            ltm->tm_mday);

    return string(today);
}

string getMonthPrefix() {
    time_t now = time(0);
    tm *ltm = localtime(&now);

    char month[8];
    // Corrected: 1900 + year and 1 + month
    sprintf(month, "%04d-%02d",
            1900 + ltm->tm_year,
            1 + ltm->tm_mon);

    return string(month);
}

    void showHistory() {

    ifstream file("history.txt");

    if (!file) {
        cout << "No History Found\n";
        return;
    }

    string number;
    string slot;
    time_t exitTime;

    cout << "\n----- PARKING HISTORY -----\n";

    while (file >> number >> slot >> exitTime) {
        cout << "Vehicle: " << number
             << " | Slot: " << slot
             << " | Exit Time: "
             << ctime(&exitTime);
    }

    file.close();
}

  void adminLogin(string password) {

    if(password != adminPassword) {
        cout << "ERROR: Wrong Password\n";
        return;
    }
    else {
        cout << "SUCCESS: Login Successful\n";
    }

    cout << "\n========= ADMIN PANEL =========\n";

    showSlots();
    showHistory();
    showDailyRevenue();

    cout << "\nActive Vehicles:\n";

    for(auto t : activeTickets) {
        cout << "Vehicle: "
             << t->getVehicleNumber()
             << " | Slot: "
             << t->getSlotID()
             << endl;
    }
            cout << "Total Revenue: Rs " << totalRevenue << endl;
            cout << "Today Revenue: Rs " << getTodayRevenue() << endl;
            cout << "Month Revenue: Rs " << getMonthRevenue() << endl;
            }

    void showSlots() {

        int freeCar = 0, freeBike = 0;

        cout << "=====================================\n";
        cout << "         PARKING SLOT STATUS         \n";
        cout << "=====================================\n\n";

        cout << "--------------- CAR SLOTS ---------------\n";

        for (auto &slot : slots)
        {
            if (slot.getType() == 1)
            {
                cout << "Slot " << slot.getSlotID();

                if (slot.isVIP())
                    cout << " (VIP)";
                else if (slot.isEmergency())
                    cout << " (EMERGENCY)";

                if (slot.isOccupied())
                {
                    cout << " : [OCCUPIED]";

                    for (auto t : activeTickets)
                    {
                        if (t->getSlotID() == slot.getSlotID())
                        {
                          time_t now = time(0);
                            double seconds = difftime(now, t->getEntryTime());

                            int totalMinutes = seconds / 60;
                            int hrs = totalMinutes / 60;
                            int mins = totalMinutes % 60;
                            int secs = (int)seconds % 60;

                            cout << " | Vehicle: " << t->getVehicleNumber();
                            cout << " | Mobile: " << t->getMobile();
                            cout << " | Parked: " << hrs << " hr " << mins << " min " << secs << " sec";                        }
                    }
                }
                else
                {
                    cout << " : [FREE]";
                    freeCar++;
                }

                cout << endl;
            }
        }

        cout << "\n--------------- BIKE SLOTS --------------\n";

        for (auto &slot : slots)
        {
            if (slot.getType() == 2)
            {
                cout << "Slot " << slot.getSlotID();

                if (slot.isOccupied())
                {
                    cout << " : [OCCUPIED]";

                    for (auto t : activeTickets)
                    {
                        if (t->getSlotID() == slot.getSlotID())
                        {
                            time_t now = time(0);
                            double seconds = difftime(now, t->getEntryTime());
                            int mins = seconds / 60;
                            int secs = (int)seconds % 60;

                            cout << " | Vehicle: " << t->getVehicleNumber();
                            cout << " | Mobile: " << t->getMobile();
                            cout << " | Parked: " << mins << " mins " << secs << " secs";
                        }
                    }
                }
                else
                {
                    cout << " : [FREE]";
                    freeBike++;
                }

                cout << endl;
            }
        }

        cout << "\n=====================================\n";
        cout << "          CAPACITY SUMMARY           \n";
        cout << "=====================================\n";
        int totalCar = 0, totalBike = 0;

        for (auto &slot : slots) {
            if (slot.getType() == 1) totalCar++;
            if (slot.getType() == 2) totalBike++;
        }

        cout << "Total Car Parked          : " << totalCar << endl;
        cout << "Free Car Slots           : " << freeCar << endl;
        cout << "Total Bike Slots         : " << totalBike << endl;
        cout << "Free Bike Slots          : " << freeBike << endl;
        cout << "=====================================\n";
    }

    void searchVehicle(string number) {

    transform(number.begin(), number.end(), number.begin(), ::toupper);

    for (auto t : activeTickets)
        if (t->getVehicleNumber() == number) {
            cout << "Vehicle Found in Slot "
                 << t->getSlotID() << endl;
            return;
        }

    cout << "Vehicle Not Found!\n";
}   

void emergencyPark(string number, string mobile) {

    transform(number.begin(), number.end(), number.begin(), ::toupper);

    for(auto &slot : slots) {

        if (isDuplicate(number)) {
            cout << "Duplicate Vehicle Entry!\n";
            return;
        }
        if(!isValidMobile(mobile)) {
            cout << "Invalid Indian Mobile Number!\n";
            return;
        }

        if(slot.isEmergency() && !slot.isOccupied()) {

           Vehicle* v = new Car(number, false, true);
            slot.occupy();

            activeTickets.push_back(new Ticket(slot.getSlotID(), v, mobile, time(0)));

            saveToFile();

            cout << "Emergency Vehicle Parked at Emergency Slot "
                 << slot.getSlotID() << endl;

            return;
        }
    }

    cout << "Emergency Slot Occupied!\n";
}

void vipPark(string number, string mobile) {

    transform(number.begin(), number.end(), number.begin(), ::toupper);

    for(auto &slot : slots) {

        if(!isValidMobile(mobile)) {
            cout << "Invalid Indian Mobile Number!\n";
            return;
        }
        if (isDuplicate(number)) {
            cout << "Duplicate Vehicle Entry!\n";
            return;
        }

        if(slot.isVIP() && !slot.isOccupied()) {

            Vehicle* v = new Car(number, true, false); // VIP true
            slot.occupy();

            activeTickets.push_back(new Ticket(slot.getSlotID(), v, mobile, time(0)));

            saveToFile();

            cout << "VIP Vehicle Parked at VIP Slot "
                 << slot.getSlotID() << endl;

            return;
        }
    }

    cout << "VIP Slot Occupied!\n";
}
void showDashboardStats() {

    int totalCar = 0, totalBike = 0;
    int freeCar = 0, occCar = 0, freeBike = 0, occBike = 0;

    cout << "{\n";
    cout << "\"slots\": [\n";

    bool first = true;

    for (auto &slot : slots) {

        if (!first) cout << ",\n";
        first = false;

        cout << "{";
        cout << "\"id\":\"" << slot.getSlotID() << "\",";
        cout << "\"status\":\"" << (slot.isOccupied() ? "occupied" : "free") << "\"";
        cout << "}";

        if (slot.getType() == 1) {
            totalCar++;
            if (slot.isOccupied()) occCar++; else freeCar++;
        } else {
            totalBike++;
            if (slot.isOccupied()) occBike++; else freeBike++;
        }
    }

    cout << "\n],\n";

    cout << "\"total\":" << (occCar + occBike) << ",\n";
    cout << "\"free\":" << (freeCar + freeBike) << "\n";

    cout << "}";
}

 void updateDailyRevenue(float amount) {
    string today = getTodayDate();
    vector<pair<string, float>> records;
    ifstream file("daily_revenue.txt");
    string date;
    float revenue;
    bool found = false;

    if (file) {
        while (file >> date >> revenue) {
            if (date == today) {
                revenue += amount;
                found = true;
            }
            records.push_back({date, revenue});
        }
        file.close();
    }

    if (!found) records.push_back({today, amount});

    ofstream out("daily_revenue.txt");
    for (auto const& r : records) {
        out << r.first << " " << r.second << endl;
    }
    out.close();
    cout << "DEBUG: Saving " << amount << " to file" << endl;
}

void showDailyRevenue() {

    ifstream file("daily_revenue.txt");

    if(!file) {
        cout << "No Revenue Data\n";
        return;
    }

    string date;
    float amount;

    cout << "\n------ DAILY REVENUE HISTORY ------\n";

    while(file >> date >> amount) {
        cout << "Date: " << date
             << " | Revenue: Rs " << amount << endl;
    }

    file.close();
}

};

int safeStoi(string s) {
    try {
        if (s.empty()) return 0;
        // Remove hidden carriage returns (\r) or spaces
        s.erase(remove(s.begin(), s.end(), '\r'), s.end());
        s.erase(remove(s.begin(), s.end(), '\n'), s.end());
        if (s.empty()) return 0;
        return stoi(s);
    } catch (...) {
        return 0; 
    }
}

int main() {
    ParkingSystem parking;
    string choiceStr;

    // Use getline for EVERYTHING to keep the buffer clean
    if (!getline(cin, choiceStr)) return 0;
    int choice = safeStoi(choiceStr);

    if (choice == 1) { // PARK
        string number, typeStr, mobile;
        getline(cin, number);
        getline(cin, typeStr);
        getline(cin, mobile);
        parking.parkVehicle(number, safeStoi(typeStr), mobile);
    }
    else if (choice == 2) { // EXIT
        string number, otpStr, payStr;
        getline(cin, number);
        getline(cin, otpStr);
        getline(cin, payStr);
        parking.exitVehicle(number, safeStoi(otpStr), safeStoi(payStr));
    }
    else if (choice == 3) { parking.showSlots(); }
    else if (choice == 4) { 
        string number; getline(cin, number); 
        parking.searchVehicle(number); 
    }
    else if (choice == 5) { // LOGIN
        string pass; getline(cin, pass); 
        parking.adminLogin(pass); 
    }
    else if (choice == 6) { 
        string number, mobile; 
        getline(cin, number); getline(cin, mobile); 
        parking.emergencyPark(number, mobile); 
    }
    else if (choice == 7) { 
        string pass, number;
        getline(cin, pass); getline(cin, number);
        if (pass == parking.getAdminPassword()) parking.adminForceExit(number);
        else cout << "Wrong Password" << endl;
    }
    else if (choice == 8) { 
        string number, mobile; 
        getline(cin, number); getline(cin, mobile); 
        parking.vipPark(number, mobile); 
    }
    else if (choice == 9) { parking.showDashboardStats(); }
    else if (choice == 10) { parking.showRevenue(); }
    else if (choice == 11) { parking.showHistory(); }
    else if (choice == 12) { parking.showTodayHistory(); }
    else if (choice == 13) { parking.showMonthHistory(); }
    else if (choice == 14) {
        string targetMonth;
        getline(cin, targetMonth); 
        parking.showFilteredMonthHistory(targetMonth);
    }
    else {
        cout << "Invalid Choice: " << choice << endl;
    }

    return 0;
}