// common.h - exceptions and small helper functions used everywhere.
#pragma once
#include <algorithm>
#include <cctype>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

// One line that lets us write string, vector, cout... instead of std::string, std::vector, std::cout...
using namespace std;

namespace kpfl {

// ---------------------------------------------------------------------------
// CUSTOM EXCEPTIONS
// Every exception is a class. Inheriting from runtime_error gives each one
// a message we can read with e.what(). "using ...::...;" re-uses the parent's constructor.
// Tree:  KpflException
//          |- InvalidDataException        (bad input: empty, negative, unknown ID...)
//          |- BusinessRuleException       (a league rule was broken)
//               |- SchedulingConflictException   (double-booking)
//               |- CapacityExceededException     (too many tickets)
//               |- InsufficientPaymentException  (not enough money)
// ---------------------------------------------------------------------------
class KpflException : public runtime_error 
{
public: using runtime_error::runtime_error;
};
class InvalidDataException : public KpflException 
{
public: using KpflException::KpflException;
};
class BusinessRuleException : public KpflException 
{
public: using KpflException::KpflException;
};
class SchedulingConflictException : public BusinessRuleException 
{
public: using BusinessRuleException::BusinessRuleException;
};
class CapacityExceededException : public BusinessRuleException 
{
public: using BusinessRuleException::BusinessRuleException;
};
class InsufficientPaymentException : public BusinessRuleException 
{
public: using BusinessRuleException::BusinessRuleException;
};

// ---------------------------------------------------------------------------
// TEXT HELPERS
// ---------------------------------------------------------------------------
inline string trim(const string& s) 
{
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}
inline string toUpper(string s)
{
    for (char& c : s) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    return s;
}
inline string toLower(string s) 
{
    for (char& c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s;
}
// Random access files use fixed-size records, so every text field has a maximum length.
const size_t MAX_TEXT_LENGTH = 39;

// Validates a text field: not empty, not too long, and no '|' character.
inline string cleanText(const string& raw, const string& field) 
{
    string s = trim(raw);
    if (s.empty()) throw InvalidDataException(field + " cannot be empty.");
    if (s.size() > MAX_TEXT_LENGTH)
        throw InvalidDataException(field + " cannot be longer than " + to_string(MAX_TEXT_LENGTH) + " characters.");
    if (s.find('|') != string::npos || s.find('\n') != string::npos)
        throw InvalidDataException(field + " cannot contain the '|' character.");
    return s;
}
inline vector<string> split(const string& line, char sep) 
{
    vector<string> parts;
    string current;
    istringstream in(line);
    while (getline(in, current, sep)) parts.push_back(current);
    if (!line.empty() && line.back() == sep) parts.push_back("");
    return parts;
}
inline string padRight(string s, size_t width)
{
    if (s.size() < width) s.append(width - s.size(), ' ');
    return s;
}
inline string padLeft(const string& s, size_t width) 
{
    return s.size() >= width ? s : string(width - s.size(), ' ') + s;
}

// ---------------------------------------------------------------------------
// MONEY HELPERS
// ---------------------------------------------------------------------------
inline double roundMoney(double v) { return round(v * 100.0) / 100.0; }
inline string money(double v) {
    ostringstream o;
    o << fixed << setprecision(2) << v;
    return o.str();
}

// ---------------------------------------------------------------------------
// DATE / TIME HELPERS
// Dates are kept as text "YYYY-MM-DD" (they sort and compare correctly as text).
// Times are kept as minutes after midnight so we can do maths on them.
// ---------------------------------------------------------------------------
inline string validateDate(const string& raw) 
{
    string s = trim(raw);
    if (s.size() != 10 || s[4] != '-' || s[7] != '-')
        throw InvalidDataException("Date must look like YYYY-MM-DD.");
    for (size_t i = 0; i < s.size(); i++)
        if (i != 4 && i != 7 && !isdigit(static_cast<unsigned char>(s[i])))
            throw InvalidDataException("Date must look like YYYY-MM-DD.");
    int y = stoi(s.substr(0, 4)), m = stoi(s.substr(5, 2)), d = stoi(s.substr(8, 2));
    static const int daysIn[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
    int maxDay = (m >= 1 && m <= 12) ? daysIn[m - 1] + ((m == 2 && leap) ? 1 : 0) : 0;
    if (m < 1 || m > 12 || d < 1 || d > maxDay) throw InvalidDataException("That date does not exist.");
    return s;
}
inline int parseTime(const string& raw) 
{
    string s = trim(raw);
    if (s.size() != 5 || s[2] != ':' || !isdigit(static_cast<unsigned char>(s[0])) ||
        !isdigit(static_cast<unsigned char>(s[1])) || !isdigit(static_cast<unsigned char>(s[3])) ||
        !isdigit(static_cast<unsigned char>(s[4])))
        throw InvalidDataException("Time must look like HH:MM (24-hour).");
    int h = stoi(s.substr(0, 2)), m = stoi(s.substr(3, 2));
    if (h > 23 || m > 59) throw InvalidDataException("That time does not exist.");
    return h * 60 + m;
}
inline string formatTime(int minutes) 
{
    ostringstream o;
    o << setfill('0') << setw(2) << minutes / 60 << ':' << setw(2) << minutes % 60;
    return o.str();
}
inline string todayPlusDays(int n) 
{
    time_t t = time(nullptr) + static_cast<time_t>(n) * 86400;
    tm local = {};
#ifdef _WIN32
    localtime_s(&local, &t);    // Windows (Visual Studio / MinGW): the "safe" version
#else
    localtime_r(&t, &local);    // Mac / Linux
#endif
    char buf[16];
    strftime(buf, sizeof buf, "%Y-%m-%d", &local);
    return buf;
}

}  // namespace kpfl
