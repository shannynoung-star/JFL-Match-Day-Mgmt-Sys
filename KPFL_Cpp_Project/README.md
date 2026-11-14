# KPFL Management System (C++17) – Setup & Run Instructions

A console application for the Kingston Premier Football League. Plain standard C++17, no external libraries.

## 1. What you need
A C++17 compiler:

| System | Install |
|--------|---------|
| Windows | MSYS2 / MinGW-w64 (https://www.msys2.org), then `pacman -S mingw-w64-ucrt-x86_64-gcc`; add its `bin` folder to PATH. Or install Visual Studio with "Desktop development with C++". |
| macOS | `xcode-select --install` |
| Linux | `sudo apt install build-essential` (Debian/Ubuntu) |

Check with: `g++ --version`

## 2. Folder contents
| Item | Purpose |
|------|---------|
| `src/common.h` | `using namespace std;`, custom exceptions, text/money/date helpers |
| `src/recordfile.h` | **Random access file**: fixed-size binary records read/written by record number |
| `src/people.h` | `Ticket`, abstract `Person`, `Player`, `Coach`, `MatchOfficial`, `Spectator` |
| `src/models.h` | `FootballTeam`, `LeagueCommissioner`, `Stadium`, `MatchResult`, `Fixture` |
| `src/shop.h` | Abstract `Product`, `Merchandise`, `FoodItem`, `Receipt`, `StadiumShop` |
| `src/league.h` | `FootballLeague` – all business rules, reports, random-access save/load |
| `src/demo.h` | Sample data |
| `src/main.cpp` | Console menu (the program) |
| `src/tests.cpp` | 50 automated tests |
| `Makefile`, `CMakeLists.txt`, `build.bat` | Build helpers |
| `kpfl`, `kpfl_tests` | Pre-built Linux (x86-64) programs |
| `KPFL_UML_Class_Diagram.mermaid` | UML class diagram source |

## 3. Build and run
**Mac / Linux (or Windows with `make`):**
```
make            # builds kpfl and kpfl_tests
./kpfl          # run the application
./kpfl_tests    # run the automated tests
```
**Windows without make:** double-click `build.bat` (builds, runs tests, then starts the app), or run
```
g++ -std=c++17 -o kpfl.exe src\main.cpp
g++ -std=c++17 -o kpfl_tests.exe src\tests.cpp
```
**Visual Studio (Windows):** File > Open > Folder, choose this folder (it reads `CMakeLists.txt`). Pick `kpfl.exe` in the green Start dropdown and press **Ctrl+F5**. Tests: pick `kpfl_tests.exe` instead. Needs the "Desktop development with C++" workload.

## 4. First steps in the program
1. Choose **11** to load demo data (4 teams, 2 stadiums, officials, fixtures, shop items), or build your own:
   Teams -> Register -> Approve (only REGISTERED teams can play).
2. **Fixtures -> Schedule**: date like `2026-12-05`, time like `15:00`.
3. **Tickets**: register a spectator, then sell tickets (prices include 15% GCT).
4. **Stadium Shop -> Sell**: enter product IDs and quantities, type `DONE`, then the payment.
5. **Reports**: standings, fixtures, occupancy, top scorers, sales; option 6 writes `standings.csv`.
6. **12** demonstrates random access: it tells you how many records the data file holds and lets you read any one of them by number, jumping straight to it.
7. **0** saves and exits. Data is stored in `kpfl_data.dat` (a binary random-access file) in the folder you ran from, and reloads automatically. Do not open it in a text editor; use the program (option 12) to look inside.

## 5. How the random access file works
* Every record is exactly `sizeof(Record)` bytes (500 bytes), so record number **N** starts at byte **N x 500**.
* `RecordFile::read(n)` uses `seekg` to jump there and reads one record; `write(n, rec)` uses `seekp` and overwrites one record.
* Record 0 is a header (format name + number of records). Records 1..N hold teams, players, coaches, stadiums, fixtures, tickets, products and so on, stored in dependency order.
* Because records are fixed-size, every text field is limited to **39 characters**.

## 6. Troubleshooting
* *g++ not recognized* -> install a compiler and reopen the terminal.
* *Program says "No usable saved data"* -> first run, or the file was edited incorrectly; it starts a fresh league. Delete `kpfl_data.dat` to reset.
* Text fields are limited to 39 characters and cannot contain the `|` character.
* The data file is not portable between very different systems (it stores raw numbers); create it on the machine that will read it.
