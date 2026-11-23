// main.cpp - console menu. It only talks to the user; all rules live in FootballLeague.
// Every action is wrapped in try/catch so a bad entry prints a message instead of crashing.
#include <iostream>
//#include "createLeague.h"
#include "league.h"

using namespace kpfl;

static const char* DATA_FILE = "kpfl_data.dat";
static FootballLeague league;

struct QuitProgram {};   // thrown when the input stream ends (e.g. Ctrl+D) so we can save and leave

// ---------------- input helpers (keep asking until the input is valid) ----------------
static string readText(const string& prompt) {
    cout << prompt << flush;
    string line;
    if (!getline(cin, line)) throw QuitProgram();
    return trim(line);
}
static int readInt(const string& prompt) {
    while (true) {
        string s = readText(prompt);
        try {
            size_t used = 0;
            int v = stoi(s, &used);
            if (used == s.size()) return v;
        } catch (const exception&) {}
        cout << "Please type a whole number.\n";
    }
}
static double readDouble(const string& prompt) {
    while (true) {
        string s = readText(prompt);
        try {
            size_t used = 0;
            double v = stod(s, &used);
            if (used == s.size()) return v;
        } catch (const exception&) {}
        cout << "Please type a number (e.g. 1250.50).\n";
    }
}

// ---------------- sub-menus ----------------
static void teamMenu() 
{
    cout << "\n\n======== Kingston Premier Football League (KPFL) Management System ================\n";
    cout << "\n(1) Register Team\t\t(2) Approve Team\n(3) Suspend Team\t\t(4) Update Team\n(5) Search Team\t\t\t(6) Display Teams\n\n";
    switch (readInt("Choose: ")) 
    {
        case 1: {
            string name = readText("Team name: "), parish = readText("Parish: "), reg = readText("Registration number: ");
            auto t = league.registerTeam(name, parish, reg);
            cout << "Registered (UNREGISTERED until approved): " << t->describe() << "\n";
            break;
        }
        case 2: league.approveTeam(readText("Registration number: ")); cout << "Team approved.\n"; break;
        case 3: league.suspendTeam(readText("Registration number: ")); cout << "Team suspended.\n"; break;
        case 4: {
            string reg = readText("Registration number: "), n = readText("New name: "), p = readText("New parish: ");
            league.updateTeam(reg, n, p);
            cout << "Updated.\n";
            break;
        }
        case 5: for (const auto& t : league.searchTeams(readText("Search text: "))) cout << t->describe() << "\n"; break;
        case 6: for (const auto& t : league.getTeams()) cout << t->describe() << "\n"; break;
        default: cout << "Invalid choice.\n";
    }
}

static void peopleMenu() 
{
    cout << "================================ Kingston Premier Football League (KPFL) Management System ======================================\n";
    cout << "\n(1) Add Player\t\t(2) Remove player\n(3) Add coach\t\t(4) Remove coach\n(5) Show squad\t\t(6) Record goals\n";
    switch (readInt("Choose: ")) {
        case 1: {
            string team = readText("Team reg no: "), name = readText("Name: ");
            int age = readInt("Age: ");
            string gender = readText("Gender: "), pos = readText("Position: ");
            int jersey = readInt("Jersey number: ");
            cout << "Added: " << league.addPlayer(team, name, age, gender, pos, jersey)->describe() << "\n";
            break;
        }
        case 2: {
            string team = readText("Team reg no: "), id = readText("Player ID: ");
            league.removePlayer(team, id);
            cout << "Removed.\n";
            break;
        }
        case 3: {
            string team = readText("Team reg no: "), name = readText("Name: ");
            int age = readInt("Age: ");
            string gender = readText("Gender: "), qual = readText("Qualification: "), role = readText("Coaching role: ");
            cout << "Added: " << league.addCoach(team, name, age, gender, qual, role)->describe() << "\n";
            break;
        }
        case 4: {
            string team = readText("Team reg no: "), id = readText("Coach ID: ");
            league.removeCoach(team, id);
            cout << "Removed.\n";
            break;
        }
        case 5: {
            auto team = league.requireTeam(readText("Team reg no: "));
            cout << team->describe() << "\n";
            // POLYMORPHISM in action: a vector of Person pointers; describe() runs the Player or Coach version.
            vector<shared_ptr<Person>> everyone;
            for (const auto& p : team->getPlayers()) everyone.push_back(p);
            for (const auto& c : team->getCoaches()) everyone.push_back(c);
            for (const auto& person : everyone) cout << "  " << person->describe() << "\n";
            break;
        }
        case 6: {
            string team = readText("Team reg no: "), id = readText("Player ID: ");
            league.recordGoals(team, id, readInt("Goals: "));
            cout << "Recorded.\n";
            break;
        }
        default: cout << "Invalid choice.\n";
    }
}

static void stadiumMenu() 
{
    cout << "================================ Kingston Premier Football League (KPFL) Management System ======================================\n";
    cout << "\n(1) Register stadium\t\t(2) List stadiums\n";
    switch (readInt("Choose: ")) {
        case 1: {
            string name = readText("Name: "), loc = readText("Location: ");
            int cap = readInt("Seating capacity: ");
            cout << "Field conditions: EXCELLENT, GOOD, FAIR, POOR, UNPLAYABLE\n";
            FieldCondition fc = fieldConditionFromString(readText("Field condition: "));
            double price = readDouble("Ticket price (incl. GCT): ");
            cout << "Registered: " << league.registerStadium(name, loc, cap, fc, price)->describe() << "\n";
            break;
        }
        case 2: for (const auto& s : league.getStadiums()) cout << s->describe() << "\n"; break;
        default: cout << "Invalid choice.\n";
    }
}

static void fixtureMenu() 
{
    cout << "================================ Kingston Premier Football League (KPFL) Management System ======================================\n";
    cout << "\n(1) Schedule\t\t\t(2) Assign official\n(3) Record result\t\t(4) List fixtures\n";
    switch (readInt("Choose: ")) 
    {
        case 1: {
            string h = readText("Home team reg no: "), a = readText("Away team reg no: "), s = readText("Stadium ID: ");
            string d = readText("Date (YYYY-MM-DD): "), t = readText("Time (HH:MM, 24h): ");
            cout << "Scheduled: " << league.scheduleFixture(h, a, s, d, t)->describe() << "\n";
            break;
        }
        case 2: {
            string f = readText("Fixture ID: "), o = readText("Official ID: ");
            league.assignOfficial(f, o);
            cout << "Assigned.\n";
            break;
        }
        case 3: {
            string f = readText("Fixture ID: ");
            int hs = readInt("Home score: "), as = readInt("Away score: ");
            int y = readInt("Yellow cards: "), r = readInt("Red cards: ");
            league.recordResult(f, hs, as, y, r);
            cout << "Result recorded; standings updated.\n";
            break;
        }
        case 4: for (const auto& f : league.getFixtures()) cout << f->describe() << "\n"; break;
        default: cout << "Invalid choice.\n";
    }
}

static void officialMenu() 
{
    cout << "================================ Kingston Premier Football League (KPFL) Management System ======================================\n";
    cout << "\n(1) Register Match Official\t\t(2) List Officials\n";
    switch (readInt("Choose: ")) {
        case 1: {
            string name = readText("Name: ");
            int age = readInt("Age: ");
            string gender = readText("Gender: "), role = readText("Role (Referee/Assistant Referee/Fourth Official): "), rank = readText("Rank: ");
            cout << "Registered: " << league.registerOfficial(name, age, gender, role, rank)->describe() << "\n";
            break;
        }
        case 2: for (const auto& o : league.getOfficials()) cout << o->describe() << "\n"; break;
        default: cout << "Invalid choice.\n";
    }
}

static void ticketMenu() 
{
    cout << "================================ Kingston Premier Football League (KPFL) Management System ======================================\n";
    cout << "\n(1) Register spectator\t\t(2) Sell tickets\t\t(3) List spectators\n";
    switch (readInt("Choose: ")) {
        case 1: {
            string name = readText("Name: ");
            int age = readInt("Age: ");
            string gender = readText("Gender: ");
            cout << "Registered: " << league.registerSpectator(name, age, gender)->describe() << "\n";
            break;
        }
        case 2: {
            string fid = readText("Fixture ID: "), sid = readText("Spectator ID: ");
            TicketCategory cat = ticketCategoryFromString(readText("Category (STANDARD/VIP): "));
            int qty = readInt("Quantity: ");
            double pay = readDouble("Payment amount: ");
            cout << league.sellTickets(fid, sid, cat, qty, pay).print();
            break;
        }
        case 3: for (const auto& s : league.getSpectators()) cout << s->describe() << "\n"; break;
        default: cout << "Invalid choice.\n";
    }
}

static void shopMenu()
 {
    cout << "================================ Kingston Premier Football League (KPFL) Management System ======================================\n";
    cout << "\n(1) List products\t\t(2) Add merchandise\n(3) Add food/drink\t\t(4) Restock\t(5) Sell\n";
    StadiumShop& shop = league.getShop();

    switch (readInt("Choose: ")) {
        case 1: for (const auto& p : shop.getProducts()) cout << p->describe() << "\n"; break;
        case 2: {
            string id = readText("Product ID: "), name = readText("Name: ");
            double price = readDouble("Price (incl. GCT): ");
            int stock = readInt("Stock: ");
            shop.addProduct(make_shared<Merchandise>(id, name, price, stock, readText("Size: ")));
            cout << "Added.\n";
            break;
        }
        case 3: {
            string id = readText("Product ID: "), name = readText("Name: ");
            double price = readDouble("Price (incl. GCT): ");
            int stock = readInt("Stock: ");
            bool bev = toLower(readText("Is it a beverage? (yes/no): ")) == "yes";
            shop.addProduct(make_shared<FoodItem>(id, name, price, stock, bev));
            cout << "Added.\n";
            break;
        }
        case 4: {
            string id = readText("Product ID: ");
            shop.restock(id, readInt("Quantity to add: "));
            cout << "Restocked.\n";
            break;
        }
        case 5: {
            map<string, int> cart;
            while (true) {
                string id = readText("Product ID to add (or DONE): ");
                if (toUpper(id) == "DONE") break;
                cart[toUpper(id)] = readInt("Quantity: ");
            }
            double pay = readDouble("Payment amount: ");
            cout << shop.purchase(cart, pay).print();
            break;
        }
        default: cout << "Invalid choice.\n";
    }
}

static void reportMenu() 
{
    cout << "================================ Kingston Premier Football League (KPFL) Management System ======================================\n";
    cout << "\n(1) Standings\t\t\t(2) Fixtures & results\n(3) Stadium occupancy\t\t(4) Top scorers\n(5) Sales summary\t\t(6) Export standings CSV\n\n";
    switch (readInt("Choose: ")) {
        case 1: cout << league.standingsReport() << "\n"; break;
        case 2: cout << league.fixtureReport() << "\n"; break;
        case 3: cout << league.stadiumOccupancyReport() << "\n"; break;
        case 4: cout << league.topScorersReport() << "\n"; break;
        case 5: cout << league.salesReport() << "\n"; break;
        case 6: {
            ofstream csv("standings.csv");
            if (!csv) throw KpflException("Could not write standings.csv");
            csv << "Team,Played,Wins,Draws,Losses,GF,GA,GD,Points\n";
            for (const auto& t : league.getTeams())
                csv << t->getName() << "," << t->getPlayed() << "," << t->getWins() << "," << t->getDraws() << "," << t->getLosses()
                    << "," << t->getGoalsFor() << "," << t->getGoalsAgainst() << "," << t->getGoalDifference() << "," << t->getPoints() << "\n";
            cout << "Written to standings.csv\n";
            break;
        }
        default: cout << "Invalid choice.\n";
    }
}

int main() {
    cout << "================================ Kingston Premier Football League (KPFL) Management System ======================================\n";
    try {
        league = FootballLeague::loadFromFile(DATA_FILE);
        //cout << "Loaded saved data from " << DATA_FILE << "\n";
    } catch (const KpflException& e) {
        cout << "No usable saved data (" << e.what() << ") - starting a new league.\n";
    }

    bool running = true;
    while (running) 
    {
        try {
            cout << "\n************************** MAIN MENU ***************************\n"
                      << "(1) TEAM MANAGEMENT\t\t\t(2) SQUAD MANAGEMENT\n(3) STADIUM & VENUES\t\t\t(4) FIXTURES & MATCH RESULTS\n(5) REFEREE & OFFICIALS\t\t"
                      << "(6) BOOKING\n(7) STADIUM SHOP\t\t\t(8) LEAGUE REPORTS\n(9) SAVE\t\t\t\t(10) LOAD\n"
                      << "(12) SEARCH RECORDS\t\t\t"
                      << "(0) SAVE & EXIT\n";
            cout << "*********************************************************************\n";
            switch (readInt("Menu Option: ")) 
            {
                case 1: teamMenu(); break;
                case 2: peopleMenu(); break;
                case 3: stadiumMenu(); break;
                case 4: fixtureMenu(); break;
                case 5: officialMenu(); break;
                case 6: ticketMenu(); break;
                case 7: shopMenu(); break;
                case 8: reportMenu(); break;
                case 9: league.save(DATA_FILE); cout << "Saved.\n"; break;
                case 10: league = FootballLeague::loadFromFile(DATA_FILE); cout << "Loaded.\n"; break;
                case 12: {
                    // RANDOM ACCESS: jump straight to one record in the data file.
                    league.save(DATA_FILE);   // make sure the file matches what is in memory
                    long total = FootballLeague::recordCountInFile(DATA_FILE);
                    cout << "The data file has " << total << " records (0 = header).\n";
                    long n = readInt("Record number to read: ");
                    cout << FootballLeague::describeRecordAt(DATA_FILE, n) << "\n";
                    break;
                }
                case 0: running = false; break;
                default: cout << "Please choose a number from the menu.\n";
            }
        } catch (const KpflException& e) {
            cout << "ERROR: " << e.what() << "\n";           // our own rule / validation messages
        } catch (const QuitProgram&) {
            running = false;
        } catch (const exception& e) {
            cout << "Unexpected error: " << e.what() << "\n";
        }
    }
    try { league.save(DATA_FILE); cout << "Data saved. GOODBYE!\n"; }
    catch (const KpflException& e) { cout << "ERROR saving: " << e.what() << "\n"; }
    return 0;
}
