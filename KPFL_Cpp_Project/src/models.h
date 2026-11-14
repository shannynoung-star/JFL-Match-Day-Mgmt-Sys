// models.h - FootballTeam, LeagueCommissioner, Stadium, MatchResult, Fixture.
#pragma once
#include <map>
#include "people.h"

namespace kpfl {

// ===================== FOOTBALL TEAM =====================
enum class TeamStatus { Unregistered, Registered, Suspended };
inline string toString(TeamStatus s) {
    switch (s) { case TeamStatus::Unregistered: return "UNREGISTERED"; case TeamStatus::Registered: return "REGISTERED"; default: return "SUSPENDED"; }
}
inline TeamStatus teamStatusFromString(const string& s) {
    string u = toUpper(s);
    if (u == "UNREGISTERED") return TeamStatus::Unregistered;
    if (u == "REGISTERED") return TeamStatus::Registered;
    if (u == "SUSPENDED") return TeamStatus::Suspended;
    throw InvalidDataException("Unknown team status " + s);
}

// COMPOSITION/AGGREGATION: a team holds shared_ptrs to its players and coaches.
// shared_ptr = smart pointer that frees the object automatically when nobody uses it any more.
class FootballTeam {
    string name, parish, registrationNumber;
    TeamStatus status = TeamStatus::Unregistered;
    vector<shared_ptr<Player>> players;
    vector<shared_ptr<Coach>> coaches;
    int played = 0, wins = 0, draws = 0, losses = 0, goalsFor = 0, goalsAgainst = 0, points = 0;
public:
    static constexpr size_t MAX_SQUAD = 30;

    FootballTeam(const string& name_, const string& parish_, const string& reg) {
        registrationNumber = toUpper(cleanText(reg, "Registration number"));
        setName(name_);
        setParish(parish_);
    }
    void setName(const string& n) { name = cleanText(n, "Team name"); }
    void setParish(const string& p) { parish = cleanText(p, "Parish"); }
    void setStatus(TeamStatus s) { status = s; }

    // ---- players ----
    void addPlayer(const shared_ptr<Player>& p) {
        if (!p) throw InvalidDataException("Player cannot be null.");
        if (players.size() >= MAX_SQUAD)
            throw BusinessRuleException("Squad is full (maximum " + to_string(MAX_SQUAD) + " players).");
        for (const auto& existing : players) {
            if (existing->getId() == p->getId()) throw InvalidDataException("Player ID already in this squad.");
            if (existing->getJerseyNumber() == p->getJerseyNumber())
                throw BusinessRuleException("Jersey number " + to_string(p->getJerseyNumber()) + " is already taken.");
        }
        players.push_back(p);
    }
    void removePlayer(const string& playerId) {
        for (size_t i = 0; i < players.size(); i++) {
            if (toUpper(players[i]->getId()) == toUpper(trim(playerId))) {
                players.erase(players.begin() + static_cast<long>(i));
                return;
            }
        }
        throw InvalidDataException("No player with ID " + playerId + " in " + name);
    }
    shared_ptr<Player> findPlayer(const string& playerId) const {
        for (const auto& p : players) if (toUpper(p->getId()) == toUpper(trim(playerId))) return p;
        return nullptr;
    }

    // ---- coaches ----
    void addCoach(const shared_ptr<Coach>& c) {
        if (!c) throw InvalidDataException("Coach cannot be null.");
        for (const auto& existing : coaches)
            if (existing->getId() == c->getId()) throw InvalidDataException("Coach ID already on this team.");
        c->setTeamRegNumber(registrationNumber);
        coaches.push_back(c);
    }
    void removeCoach(const string& coachId) {
        for (size_t i = 0; i < coaches.size(); i++) {
            if (toUpper(coaches[i]->getId()) == toUpper(trim(coachId))) {
                coaches[i]->setTeamRegNumber("");
                coaches.erase(coaches.begin() + static_cast<long>(i));
                return;
            }
        }
        throw InvalidDataException("No coach with ID " + coachId + " in " + name);
    }

    // ---- results: win = 3 points, draw = 1, loss = 0 ----
    void recordMatch(int scored, int conceded) {
        if (scored < 0 || conceded < 0) throw InvalidDataException("Scores cannot be negative.");
        played++; goalsFor += scored; goalsAgainst += conceded;
        if (scored > conceded) { wins++; points += 3; }
        else if (scored == conceded) { draws++; points += 1; }
        else { losses++; }
    }
    // Used only when loading a saved file.
    void restoreStats(int p, int w, int d, int l, int gf, int ga, int pts) {
        played = p; wins = w; draws = d; losses = l; goalsFor = gf; goalsAgainst = ga; points = pts;
    }

    bool isEligibleForFixture() const { return status == TeamStatus::Registered; }

    const string& getName() const { return name; }
    const string& getParish() const { return parish; }
    const string& getRegistrationNumber() const { return registrationNumber; }
    TeamStatus getStatus() const { return status; }
    size_t getSquadSize() const { return players.size(); }
    const vector<shared_ptr<Player>>& getPlayers() const { return players; }
    const vector<shared_ptr<Coach>>& getCoaches() const { return coaches; }
    int getPlayed() const { return played; }
    int getWins() const { return wins; }
    int getDraws() const { return draws; }
    int getLosses() const { return losses; }
    int getGoalsFor() const { return goalsFor; }
    int getGoalsAgainst() const { return goalsAgainst; }
    int getGoalDifference() const { return goalsFor - goalsAgainst; }
    int getPoints() const { return points; }

    string describe() const {
        return name + " (" + registrationNumber + ") | Parish: " + parish + " | " + toString(status) +
               " | squad " + to_string(players.size()) + " | coaches " + to_string(coaches.size());
    }
};

// ===================== LEAGUE COMMISSIONER =====================
// Only the commissioner changes a team's registration status.
class LeagueCommissioner : public Person {
public:
    using Person::Person;
    void approveTeam(FootballTeam& t) const { t.setStatus(TeamStatus::Registered); }
    void suspendTeam(FootballTeam& t) const { t.setStatus(TeamStatus::Suspended); }
    string getRole() const override { return "League Commissioner"; }
};

// ===================== STADIUM =====================
enum class FieldCondition { Excellent, Good, Fair, Poor, Unplayable };
inline string toString(FieldCondition c) {
    switch (c) {
        case FieldCondition::Excellent: return "EXCELLENT"; case FieldCondition::Good: return "GOOD";
        case FieldCondition::Fair: return "FAIR"; case FieldCondition::Poor: return "POOR";
        default: return "UNPLAYABLE";
    }
}
inline FieldCondition fieldConditionFromString(const string& s) {
    string u = toUpper(trim(s));
    if (u == "EXCELLENT") return FieldCondition::Excellent;
    if (u == "GOOD") return FieldCondition::Good;
    if (u == "FAIR") return FieldCondition::Fair;
    if (u == "POOR") return FieldCondition::Poor;
    if (u == "UNPLAYABLE") return FieldCondition::Unplayable;
    throw InvalidDataException("'" + s + "' is not a valid field condition.");
}

class Stadium {
    string id, name, location, currentMatchup = "None";
    int seatingCapacity = 0;
    FieldCondition fieldCondition = FieldCondition::Good;
    double ticketPrice = 0;                          // includes 15% GCT
    map<string, int> occupancyByFixture;   // fixtureId -> seats sold
public:
    Stadium(const string& id_, const string& name_, const string& loc, int capacity,
            FieldCondition cond, double price) {
        id = toUpper(cleanText(id_, "Stadium ID"));
        setName(name_); setLocation(loc); setSeatingCapacity(capacity); setFieldCondition(cond); setTicketPrice(price);
    }
    void setName(const string& n) { name = cleanText(n, "Stadium name"); }
    void setLocation(const string& l) { location = cleanText(l, "Location"); }
    void setSeatingCapacity(int c) {
        if (c <= 0) throw InvalidDataException("Seating capacity must be greater than zero.");
        seatingCapacity = c;
    }
    void setFieldCondition(FieldCondition f) { fieldCondition = f; }
    void setTicketPrice(double p) {
        if (p < 0) throw InvalidDataException("Ticket price cannot be negative.");
        ticketPrice = p;
    }
    // Trimmed to the maximum stored text length so it always fits in a fixed-size file record.
    void setCurrentMatchup(const string& t) { currentMatchup = t.substr(0, MAX_TEXT_LENGTH); }

    int getOccupancy(const string& fixtureId) const {
        auto it = occupancyByFixture.find(fixtureId);
        return it == occupancyByFixture.end() ? 0 : it->second;
    }
    int getRemainingSeats(const string& fixtureId) const { return seatingCapacity - getOccupancy(fixtureId); }

    // RULE: ticket sales cannot exceed the seating capacity.
    void reserveSeats(const string& fixtureId, int quantity) {
        if (quantity <= 0) throw InvalidDataException("Ticket quantity must be at least 1.");
        if (quantity > getRemainingSeats(fixtureId))
            throw CapacityExceededException("Only " + to_string(getRemainingSeats(fixtureId)) +
                                            " seats left at " + name + " for this fixture.");
        occupancyByFixture[fixtureId] = getOccupancy(fixtureId) + quantity;
    }
    void restoreOccupancy(const string& fixtureId, int sold) { occupancyByFixture[fixtureId] = sold; }
    const map<string, int>& getOccupancyMap() const { return occupancyByFixture; }

    const string& getId() const { return id; }
    const string& getName() const { return name; }
    const string& getLocation() const { return location; }
    int getSeatingCapacity() const { return seatingCapacity; }
    FieldCondition getFieldCondition() const { return fieldCondition; }
    double getTicketPrice() const { return ticketPrice; }
    const string& getCurrentMatchup() const { return currentMatchup; }

    string describe() const {
        return name + " [" + id + "] " + location + " | capacity " + to_string(seatingCapacity) +
               " | field " + toString(fieldCondition) + " | ticket $" + money(ticketPrice) + " | current: " + currentMatchup;
    }
};

// ===================== MATCH RESULT =====================
class MatchResult {
    int homeScore, awayScore, yellowCards, redCards;
    string winner;   // team name or "Draw"
public:
    MatchResult(const string& homeName, const string& awayName, int hs, int as, int yellow, int red)
        : homeScore(hs), awayScore(as), yellowCards(yellow), redCards(red) {
        if (hs < 0 || as < 0) throw InvalidDataException("Match scores cannot be negative.");
        if (yellow < 0 || red < 0) throw InvalidDataException("Card counts cannot be negative.");
        winner = hs > as ? homeName : (as > hs ? awayName : "Draw");
    }
    int getHomeScore() const { return homeScore; }
    int getAwayScore() const { return awayScore; }
    int getYellowCards() const { return yellowCards; }
    int getRedCards() const { return redCards; }
    const string& getWinner() const { return winner; }
    string describe() const {
        return to_string(homeScore) + " - " + to_string(awayScore) + " (winner: " + winner +
               ", yellow " + to_string(yellowCards) + ", red " + to_string(redCards) + ")";
    }
};

// ===================== FIXTURE =====================
enum class FixtureStatus { Scheduled, Completed, Cancelled };
inline string toString(FixtureStatus s) {
    switch (s) { case FixtureStatus::Scheduled: return "SCHEDULED"; case FixtureStatus::Completed: return "COMPLETED"; default: return "CANCELLED"; }
}
inline FixtureStatus fixtureStatusFromString(const string& s) {
    string u = toUpper(s);
    if (u == "SCHEDULED") return FixtureStatus::Scheduled;
    if (u == "COMPLETED") return FixtureStatus::Completed;
    if (u == "CANCELLED") return FixtureStatus::Cancelled;
    throw InvalidDataException("Unknown fixture status " + s);
}

// ASSOCIATION: a fixture points to teams/stadium that exist independently of it.
// COMPOSITION: it owns its result (optional - empty until a result is recorded).
class Fixture {
    string id, date;
    shared_ptr<FootballTeam> home, away;
    shared_ptr<Stadium> stadium;
    int timeMinutes;
    FixtureStatus status = FixtureStatus::Scheduled;
    unique_ptr<MatchResult> result;   // unique_ptr: only the fixture owns it; empty = no result yet
    vector<shared_ptr<MatchOfficial>> officials;
public:
    static constexpr int MATCH_MINUTES = 120;
    static constexpr size_t MAX_OFFICIALS = 4;

    Fixture(string id_, shared_ptr<FootballTeam> h, shared_ptr<FootballTeam> a,
            shared_ptr<Stadium> s, string date_, int time)
        : id(move(id_)), date(move(date_)), home(move(h)), away(move(a)),
          stadium(move(s)), timeMinutes(time) {}

    bool involves(const FootballTeam* t) const { return home.get() == t || away.get() == t; }
    bool hasOfficial(const MatchOfficial* o) const {
        for (const auto& x : officials) if (x.get() == o) return true;
        return false;
    }
    // True when a match starting at otherMinutes would clash with this fixture's 120-minute slot.
    bool overlaps(int otherMinutes) const { return abs(timeMinutes - otherMinutes) < MATCH_MINUTES; }

    void addOfficial(const shared_ptr<MatchOfficial>& o) {
        if (officials.size() >= MAX_OFFICIALS)
            throw BusinessRuleException("A fixture can have at most " + to_string(MAX_OFFICIALS) + " officials.");
        officials.push_back(o);
    }
    void setResult(const MatchResult& r) { result = make_unique<MatchResult>(r); }

    // RULE: a fixture cannot be completed until a valid result has been recorded.
    void markCompleted() {
        if (!result) throw BusinessRuleException("Cannot complete fixture " + id + ": no result recorded.");
        status = FixtureStatus::Completed;
    }
    void restoreStatus(FixtureStatus s) { status = s; }
    void cancel() { status = FixtureStatus::Cancelled; }

    const string& getId() const { return id; }
    const string& getDate() const { return date; }
    int getTimeMinutes() const { return timeMinutes; }
    const shared_ptr<FootballTeam>& getHome() const { return home; }
    const shared_ptr<FootballTeam>& getAway() const { return away; }
    const shared_ptr<Stadium>& getStadium() const { return stadium; }
    FixtureStatus getStatus() const { return status; }
    const MatchResult* getResult() const { return result.get(); }
    const vector<shared_ptr<MatchOfficial>>& getOfficials() const { return officials; }

    string describe() const {
        string s = id + " | " + home->getName() + " vs " + away->getName() + " | " + date + " " +
                        formatTime(timeMinutes) + " @ " + stadium->getName() + " | " + toString(status);
        if (result) s += " " + result->describe();
        return s + " | officials: " + to_string(officials.size());
    }
};

}  // namespace kpfl
