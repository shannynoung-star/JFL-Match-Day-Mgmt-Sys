// people.h - Ticket, Person and its subclasses (Player, Coach, MatchOfficial, Spectator).
#pragma once
#include <memory>
#include "common.h"

namespace kpfl {

// ===================== TICKET =====================
enum class TicketCategory { Standard, Vip };
inline string toString(TicketCategory c) { return c == TicketCategory::Vip ? "VIP" : "STANDARD"; }
inline TicketCategory ticketCategoryFromString(const string& s) {
    string u = toUpper(trim(s));
    if (u == "STANDARD") return TicketCategory::Standard;
    if (u == "VIP") return TicketCategory::Vip;
    throw InvalidDataException("'" + s + "' is not a valid ticket category (STANDARD or VIP).");
}

class Ticket {
    string ticketId, fixtureId, spectatorId;
    TicketCategory category;
    double price;
public:
    Ticket(string id, string fixture, string spectator, TicketCategory cat, double p)
        : ticketId(move(id)), fixtureId(move(fixture)), spectatorId(move(spectator)),
          category(cat), price(p) {}
    const string& getTicketId() const { return ticketId; }
    const string& getFixtureId() const { return fixtureId; }
    const string& getSpectatorId() const { return spectatorId; }
    TicketCategory getCategory() const { return category; }
    double getPrice() const { return price; }
    string describe() const {
        return "Ticket " + ticketId + " | fixture " + fixtureId + " | " + toString(category) + " | $" + money(price);
    }
};

// ===================== PERSON (ABSTRACT) =====================
// ABSTRACTION: Person has a pure virtual function (= 0) so you cannot create a plain Person.
// ENCAPSULATION: data members are private; setters validate before storing.
class Person {
private:
    string id, name, gender;
    int age = 0;
public:
    Person(const string& id_, const string& name_, int age_, const string& gender_) {
        setId(id_); setName(name_); setAge(age_); setGender(gender_);
    }
    // A virtual destructor is REQUIRED in a base class so derived objects are cleaned up correctly.
    virtual ~Person() = default;

    void setId(const string& v) { id = cleanText(v, "ID"); }
    void setName(const string& v) { name = cleanText(v, "Name"); }
    void setAge(int v) {
        if (v < 1 || v > 120) throw InvalidDataException("Age must be between 1 and 120.");
        age = v;
    }
    void setGender(const string& v) { gender = cleanText(v, "Gender"); }

    const string& getId() const { return id; }
    const string& getName() const { return name; }
    int getAge() const { return age; }
    const string& getGender() const { return gender; }

    // POLYMORPHISM: each subclass overrides these; calls through a Person pointer pick the right version.
    virtual string getRole() const = 0;
    virtual string describe() const {
        return getRole() + " [" + id + "] " + name + ", age " + to_string(age) + ", " + gender;
    }
};

// ===================== PLAYER =====================
enum class PlayingStatus { Active, Injured, Suspended };
inline string toString(PlayingStatus s) {
    switch (s) { case PlayingStatus::Active: return "ACTIVE"; case PlayingStatus::Injured: return "INJURED"; default: return "SUSPENDED"; }
}
inline PlayingStatus playingStatusFromString(const string& s) {
    string u = toUpper(s);
    if (u == "ACTIVE") return PlayingStatus::Active;
    if (u == "INJURED") return PlayingStatus::Injured;
    if (u == "SUSPENDED") return PlayingStatus::Suspended;
    throw InvalidDataException("Unknown playing status " + s);
}

class Player : public Person {
    string position;
    int jerseyNumber = 0;
    int goals = 0;
    PlayingStatus status = PlayingStatus::Active;
public:
    static constexpr int MINIMUM_AGE = 16;   // league rule

    Player(const string& id, const string& name, int age, const string& gender,
           const string& pos, int jersey)
        : Person(id, name, age, gender) {
        if (age < MINIMUM_AGE)
            throw BusinessRuleException("Players must be at least " + to_string(MINIMUM_AGE) + " years old.");
        setPosition(pos);
        setJerseyNumber(jersey);
    }
    void setPosition(const string& p) { position = cleanText(p, "Position"); }
    void setJerseyNumber(int n) {
        if (n < 1 || n > 99) throw InvalidDataException("Jersey number must be 1-99.");
        jerseyNumber = n;
    }
    void addGoals(int n) {
        if (n < 0) throw InvalidDataException("Goals cannot be negative.");
        goals += n;
    }
    void setPlayingStatus(PlayingStatus s) { status = s; }

    const string& getPosition() const { return position; }
    int getJerseyNumber() const { return jerseyNumber; }
    int getGoals() const { return goals; }
    PlayingStatus getPlayingStatus() const { return status; }

    string getRole() const override { return "Player"; }
    string describe() const override {
        return Person::describe() + " | #" + to_string(jerseyNumber) + " " + position +
               " | goals " + to_string(goals) + " | " + toString(status);
    }
};

// ===================== COACH =====================
class Coach : public Person {
    string qualification, coachingRole, teamRegNumber;
public:
    Coach(const string& id, const string& name, int age, const string& gender,
          const string& qual, const string& role)
        : Person(id, name, age, gender) {
        if (age < 18) throw InvalidDataException("Coaches must be at least 18.");
        qualification = cleanText(qual, "Qualification");
        coachingRole = cleanText(role, "Coaching role");
    }
    const string& getQualification() const { return qualification; }
    const string& getCoachingRole() const { return coachingRole; }
    const string& getTeamRegNumber() const { return teamRegNumber; }
    void setTeamRegNumber(const string& reg) { teamRegNumber = reg; }

    string getRole() const override { return "Coach"; }
    string describe() const override { return Person::describe() + " | " + coachingRole + " | " + qualification; }
};

// ===================== MATCH OFFICIAL =====================
class MatchOfficial : public Person {
    string officiatingRole, rank;
    bool available = true;
public:
    MatchOfficial(const string& id, const string& name, int age, const string& gender,
                  const string& role, const string& rank_)
        : Person(id, name, age, gender) {
        if (age < 18) throw InvalidDataException("Match officials must be at least 18.");
        officiatingRole = cleanText(role, "Officiating role");
        rank = cleanText(rank_, "Rank");
    }
    const string& getOfficiatingRole() const { return officiatingRole; }
    const string& getRank() const { return rank; }
    bool isAvailable() const { return available; }
    void setAvailable(bool a) { available = a; }

    string getRole() const override { return "Match Official"; }
    string describe() const override {
        return Person::describe() + " | " + officiatingRole + " (" + rank + ") | " + (available ? "available" : "unavailable");
    }
};

// ===================== SPECTATOR =====================
class Spectator : public Person {
    vector<Ticket> tickets;   // ASSOCIATION: a spectator owns tickets
public:
    using Person::Person;          // re-use Person's constructor
    void addTicket(const Ticket& t) { tickets.push_back(t); }
    const vector<Ticket>& getTickets() const { return tickets; }

    string getRole() const override { return "Spectator"; }
    string describe() const override { return Person::describe() + " | tickets: " + to_string(tickets.size()); }
};

}  // namespace kpfl
