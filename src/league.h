// league.h - FootballLeague: owns all data, enforces every business rule, makes reports, saves/loads files.
#pragma once
#include <fstream>
#include "models.h"
#include "recordfile.h"
#include "stadiumShop.h"

namespace kpfl {

class FootballLeague 
{
    map<string, shared_ptr<FootballTeam>> teams;       // key = registration number
    map<string, shared_ptr<Stadium>> stadiums;         // key = stadium ID
    map<string, shared_ptr<Fixture>> fixtures;         // key = fixture ID
    map<string, shared_ptr<MatchOfficial>> officials;
    map<string, shared_ptr<Spectator>> spectators;
    map<string, int> idCounters;
    StadiumShop shop;
    shared_ptr<LeagueCommissioner> commissioner;

public:
    FootballLeague() : commissioner(make_shared<LeagueCommissioner>("LC001", "League Commissioner", 40, "Female")) {}

    // Makes IDs like P001, F002.
    string nextId(const string& prefix)
     {
        int n = ++idCounters[prefix];
        ostringstream o;
        o << prefix << setfill('0') << setw(3) << n;
        return o.str();
    }

    // =============== TEAMS ===============
    shared_ptr<FootballTeam> registerTeam(const string& name, const string& parish, const string& reg) 
    {
        auto t = make_shared<FootballTeam>(name, parish, reg);   // validates
        if (teams.count(t->getRegistrationNumber()))
            throw InvalidDataException("Registration number " + t->getRegistrationNumber() + " is already used.");
        teams[t->getRegistrationNumber()] = t;
        return t;
    }
    void approveTeam(const string& reg) { commissioner->approveTeam(*requireTeam(reg)); }
    void suspendTeam(const string& reg) { commissioner->suspendTeam(*requireTeam(reg)); }
    void updateTeam(const string& reg, const string& newName, const string& newParish) {
        auto t = requireTeam(reg);
        t->setName(newName);
        t->setParish(newParish);
    }
    shared_ptr<FootballTeam> requireTeam(const string& reg) const {
        auto it = teams.find(toUpper(trim(reg)));
        if (it == teams.end()) throw InvalidDataException("No team with registration number " + reg);
        return it->second;
    }
    vector<shared_ptr<FootballTeam>> searchTeams(const string& keyword) const {
        vector<shared_ptr<FootballTeam>> found;
        string k = toLower(trim(keyword));
        for (const auto& kv : teams) {
            const auto& t = kv.second;
            if (toLower(t->getName()).find(k) != string::npos || toLower(t->getParish()).find(k) != string::npos ||
                toLower(t->getRegistrationNumber()).find(k) != string::npos) found.push_back(t);
        }
        return found;
    }
    vector<shared_ptr<FootballTeam>> getTeams() const {
        vector<shared_ptr<FootballTeam>> v;
        for (const auto& kv : teams) v.push_back(kv.second);
        return v;
    }

    // =============== PLAYERS & COACHES ===============
    shared_ptr<Player> addPlayer(const string& teamReg, const string& name, int age,
                                      const string& gender, const string& position, int jersey) {
        auto team = requireTeam(teamReg);
        auto p = make_shared<Player>(nextId("P"), name, age, gender, position, jersey);  // enforces min age
        team->addPlayer(p);
        return p;
    }
    void removePlayer(const string& teamReg, const string& playerId) { requireTeam(teamReg)->removePlayer(playerId); }
    shared_ptr<Coach> addCoach(const string& teamReg, const string& name, int age,
                                    const string& gender, const string& qualification, const string& role) {
        auto team = requireTeam(teamReg);
        auto c = make_shared<Coach>(nextId("C"), name, age, gender, qualification, role);
        team->addCoach(c);
        return c;
    }
    void removeCoach(const string& teamReg, const string& coachId) { requireTeam(teamReg)->removeCoach(coachId); }
    void recordGoals(const string& teamReg, const string& playerId, int goals) {
        auto p = requireTeam(teamReg)->findPlayer(playerId);
        if (!p) throw InvalidDataException("No player " + playerId + " in that team.");
        p->addGoals(goals);
    }

    // =============== STADIUMS ===============
    shared_ptr<Stadium> registerStadium(const string& name, const string& location, int capacity,
                                             FieldCondition cond, double ticketPrice) {
        auto s = make_shared<Stadium>(nextId("S"), name, location, capacity, cond, ticketPrice);
        stadiums[s->getId()] = s;
        return s;
    }
    shared_ptr<Stadium> requireStadium(const string& id) const {
        auto it = stadiums.find(toUpper(trim(id)));
        if (it == stadiums.end()) throw InvalidDataException("No stadium with ID " + id);
        return it->second;
    }
    vector<shared_ptr<Stadium>> getStadiums() const {
        vector<shared_ptr<Stadium>> v;
        for (const auto& kv : stadiums) v.push_back(kv.second);
        return v;
    }

    // =============== MATCH OFFICIALS ===============
    shared_ptr<MatchOfficial> registerOfficial(const string& name, int age, const string& gender,
                                                    const string& role, const string& rank) {
        auto o = make_shared<MatchOfficial>(nextId("O"), name, age, gender, role, rank);
        officials[o->getId()] = o;
        return o;
    }
    vector<shared_ptr<MatchOfficial>> getOfficials() const {
        vector<shared_ptr<MatchOfficial>> v;
        for (const auto& kv : officials) v.push_back(kv.second);
        return v;
    }

    // =============== FIXTURES ===============
    // Rules: eligible teams, no team twice a day, no stadium double-booking, playable field.
    shared_ptr<Fixture> scheduleFixture(const string& homeReg, const string& awayReg,
                                             const string& stadiumId, const string& dateText,
                                             const string& timeText) {
        string date = validateDate(dateText);
        int time = parseTime(timeText);
        auto home = requireTeam(homeReg);
        auto away = requireTeam(awayReg);
        if (home == away) throw InvalidDataException("A team cannot play against itself.");
        for (const auto& t : {home, away})
            if (!t->isEligibleForFixture())
                throw BusinessRuleException(t->getName() + " is " + toString(t->getStatus()) + " and cannot be assigned to a fixture.");
        auto stadium = requireStadium(stadiumId);
        if (stadium->getFieldCondition() == FieldCondition::Unplayable)
            throw BusinessRuleException(stadium->getName() + " has an unplayable field.");

        for (const auto& kv : fixtures) {
            const auto& f = kv.second;
            if (f->getStatus() == FixtureStatus::Cancelled || f->getDate() != date) continue;
            if (f->involves(home.get()) || f->involves(away.get()))
                throw SchedulingConflictException("A team is already playing on " + date + " (fixture " + f->getId() + ").");
            if (f->getStadium() == stadium && f->overlaps(time))
                throw SchedulingConflictException(stadium->getName() + " is already booked at that time (fixture " + f->getId() + ").");
        }
        auto fx = make_shared<Fixture>(nextId("F"), home, away, stadium, date, time);
        fixtures[fx->getId()] = fx;
        stadium->setCurrentMatchup(home->getName() + " vs " + away->getName());
        return fx;
    }
    shared_ptr<Fixture> requireFixture(const string& id) const {
        auto it = fixtures.find(toUpper(trim(id)));
        if (it == fixtures.end()) throw InvalidDataException("No fixture with ID " + id);
        return it->second;
    }
    vector<shared_ptr<Fixture>> getFixtures() const {
        vector<shared_ptr<Fixture>> v;
        for (const auto& kv : fixtures) v.push_back(kv.second);
        return v;
    }

    // Rule: an official cannot be assigned to conflicting fixtures.
    void assignOfficial(const string& fixtureId, const string& officialId) {
        auto fx = requireFixture(fixtureId);
        auto it = officials.find(toUpper(trim(officialId)));
        if (it == officials.end()) throw InvalidDataException("No official with ID " + officialId);
        auto o = it->second;
        if (fx->getStatus() != FixtureStatus::Scheduled)
            throw BusinessRuleException("Officials can only be assigned to scheduled fixtures.");
        if (!o->isAvailable()) throw BusinessRuleException(o->getName() + " is marked unavailable.");
        if (fx->hasOfficial(o.get())) throw InvalidDataException(o->getName() + " is already assigned to this fixture.");
        for (const auto& kv : fixtures) {
            const auto& other = kv.second;
            if (other == fx || other->getStatus() == FixtureStatus::Cancelled) continue;
            if (other->hasOfficial(o.get()) && other->getDate() == fx->getDate() && other->overlaps(fx->getTimeMinutes()))
                throw SchedulingConflictException(o->getName() + " is already assigned to fixture " + other->getId() + " at a clashing time.");
        }
        fx->addOfficial(o);
    }

    // Records the score, completes the fixture and updates both teams' tables.
    void recordResult(const string& fixtureId, int homeScore, int awayScore, int yellow, int red) {
        auto fx = requireFixture(fixtureId);
        if (fx->getStatus() != FixtureStatus::Scheduled)
            throw BusinessRuleException("Fixture " + fx->getId() + " is already " + toString(fx->getStatus()) + ".");
        MatchResult r(fx->getHome()->getName(), fx->getAway()->getName(), homeScore, awayScore, yellow, red);
        fx->setResult(r);
        fx->markCompleted();
        fx->getHome()->recordMatch(homeScore, awayScore);
        fx->getAway()->recordMatch(awayScore, homeScore);
    }

    // =============== TICKETS ===============
    shared_ptr<Spectator> registerSpectator(const string& name, int age, const string& gender) {
        auto s = make_shared<Spectator>(nextId("SP"), name, age, gender);
        spectators[s->getId()] = s;
        return s;
    }
    vector<shared_ptr<Spectator>> getSpectators() const {
        vector<shared_ptr<Spectator>> v;
        for (const auto& kv : spectators) v.push_back(kv.second);
        return v;
    }
    // Rules: capacity limit and sufficient payment. VIP tickets cost double.
    Receipt sellTickets(const string& fixtureId, const string& spectatorId, TicketCategory category,
                        int quantity, double payment) {
        auto fx = requireFixture(fixtureId);
        auto it = spectators.find(toUpper(trim(spectatorId)));
        if (it == spectators.end()) throw InvalidDataException("No spectator with ID " + spectatorId);
        auto sp = it->second;
        if (fx->getStatus() != FixtureStatus::Scheduled)
            throw BusinessRuleException("Tickets can only be sold for scheduled fixtures.");
        if (quantity <= 0) throw InvalidDataException("Ticket quantity must be at least 1.");
        auto st = fx->getStadium();
        if (quantity > st->getRemainingSeats(fx->getId()))
            throw CapacityExceededException("Only " + to_string(st->getRemainingSeats(fx->getId())) + " seats left for this fixture.");

        double unit = st->getTicketPrice() * (category == TicketCategory::Vip ? 2 : 1);
        Receipt receipt;
        receipt.addLine(toString(category) + " ticket " + fx->getId(), quantity, unit);
        receipt.finish(payment);                       // throws if payment too small -> nothing is sold

        st->reserveSeats(fx->getId(), quantity);       // commit only after every check passed
        for (int i = 0; i < quantity; i++) sp->addTicket(Ticket(nextId("T"), fx->getId(), sp->getId(), category, unit));
        return receipt;
    }

    // =============== SHOP ===============
    StadiumShop& getShop() { return shop; }

    // =============== REPORTS ===============
    string standingsReport() const {
        auto table = getTeams();
        sort(table.begin(), table.end(), [](const shared_ptr<FootballTeam>& a, const shared_ptr<FootballTeam>& b) {
            if (a->getPoints() != b->getPoints()) return a->getPoints() > b->getPoints();
            if (a->getGoalDifference() != b->getGoalDifference()) return a->getGoalDifference() > b->getGoalDifference();
            if (a->getGoalsFor() != b->getGoalsFor()) return a->getGoalsFor() > b->getGoalsFor();
            return toLower(a->getName()) < toLower(b->getName());
        });
        string s = "=== LEAGUE STANDINGS ===\n";
        s += padRight("#", 4) + padRight("Team", 24) + padLeft("P", 4) + padLeft("W", 4) + padLeft("D", 4) + padLeft("L", 4) +
             padLeft("GF", 5) + padLeft("GA", 5) + padLeft("GD", 5) + padLeft("Pts", 5) + "\n";
        int pos = 1;
        for (const auto& t : table) {
            s += padRight(to_string(pos++), 4) + padRight(t->getName(), 24) + padLeft(to_string(t->getPlayed()), 4) +
                 padLeft(to_string(t->getWins()), 4) + padLeft(to_string(t->getDraws()), 4) +
                 padLeft(to_string(t->getLosses()), 4) + padLeft(to_string(t->getGoalsFor()), 5) +
                 padLeft(to_string(t->getGoalsAgainst()), 5) + padLeft(to_string(t->getGoalDifference()), 5) +
                 padLeft(to_string(t->getPoints()), 5) + "\n";
        }
        return s;
    }
    string fixtureReport() const {
        string s = "=== FIXTURES & RESULTS ===\n";
        if (fixtures.empty()) s += "(none)\n";
        for (const auto& kv : fixtures) s += kv.second->describe() + "\n";
        return s;
    }
    string stadiumOccupancyReport() const {
        string s = "=== STADIUM OCCUPANCY ===\n";
        for (const auto& kv : fixtures) {
            const auto& f = kv.second;
            int sold = f->getStadium()->getOccupancy(f->getId());
            int cap = f->getStadium()->getSeatingCapacity();
            ostringstream pct;
            pct << fixed << setprecision(1) << 100.0 * sold / cap;
            s += f->getId() + " | " + f->getStadium()->getName() + " | " + to_string(sold) + "/" +
                 to_string(cap) + " seats (" + pct.str() + "%)\n";
        }
        return s;
    }
    string topScorersReport() const {
        vector<shared_ptr<Player>> all;
        for (const auto& kv : teams) for (const auto& p : kv.second->getPlayers()) all.push_back(p);
        stable_sort(all.begin(), all.end(), [](const shared_ptr<Player>& a, const shared_ptr<Player>& b) {
            return a->getGoals() > b->getGoals();
        });
        string s = "=== TOP SCORERS ===\n";
        int shown = 0;
        for (const auto& p : all) {
            if (p->getGoals() == 0 || shown == 5) break;
            s += p->getName() + " [" + p->getId() + "] - " + to_string(p->getGoals()) + " goals\n";
            shown++;
        }
        if (shown == 0) s += "(no goals recorded yet)\n";
        return s;
    }
    string salesReport() const {
        return "=== SALES SUMMARY ===\nShop transactions: " + to_string(shop.getTransactionCount()) +
               "\nShop sales (incl. GCT): $" + money(shop.getTotalSales()) + "\n";
    }

    // =============== FILES ===============
    // RANDOM ACCESS FILE (see recordfile.h). Record 0 is a header holding a format name and the number of
    // data records. Records 1..N are the data, each stored at byte position record_number * RECORD_SIZE.
    static constexpr const char* FILE_FORMAT = "KPFL_RA_V1";

    // Describes everything we must store as a list of records: {TAG, field1, field2, ...}.
    vector<vector<string>> buildRecords() const {
        vector<vector<string>> r;
        for (const auto& kv : idCounters) r.push_back({"COUNTER", kv.first, to_string(kv.second)});
        r.push_back({"COMMISSIONER", commissioner->getId(), commissioner->getName(),
                     to_string(commissioner->getAge()), commissioner->getGender()});
        for (const auto& kv : teams) {
            const auto& t = *kv.second;
            const string& reg = t.getRegistrationNumber();
            r.push_back({"TEAM", reg, t.getName(), t.getParish(), toString(t.getStatus()), to_string(t.getPlayed()),
                         to_string(t.getWins()), to_string(t.getDraws()), to_string(t.getLosses()),
                         to_string(t.getGoalsFor()), to_string(t.getGoalsAgainst()), to_string(t.getPoints())});
            for (const auto& p : t.getPlayers())
                r.push_back({"PLAYER", reg, p->getId(), p->getName(), to_string(p->getAge()), p->getGender(),
                             p->getPosition(), to_string(p->getJerseyNumber()), to_string(p->getGoals()),
                             toString(p->getPlayingStatus())});
            for (const auto& c : t.getCoaches())
                r.push_back({"COACH", reg, c->getId(), c->getName(), to_string(c->getAge()), c->getGender(),
                             c->getQualification(), c->getCoachingRole()});
        }
        for (const auto& kv : stadiums) {
            const auto& s = *kv.second;
            r.push_back({"STADIUM", s.getId(), s.getName(), s.getLocation(), to_string(s.getSeatingCapacity()),
                         toString(s.getFieldCondition()), money(s.getTicketPrice()), s.getCurrentMatchup()});
            for (const auto& occ : s.getOccupancyMap()) r.push_back({"OCC", s.getId(), occ.first, to_string(occ.second)});
        }
        for (const auto& kv : officials) {
            const auto& o = *kv.second;
            r.push_back({"OFFICIAL", o.getId(), o.getName(), to_string(o.getAge()), o.getGender(),
                         o.getOfficiatingRole(), o.getRank(), o.isAvailable() ? "1" : "0"});
        }
        for (const auto& kv : spectators) {
            const auto& s = *kv.second;
            r.push_back({"SPECTATOR", s.getId(), s.getName(), to_string(s.getAge()), s.getGender()});
        }
        for (const auto& kv : fixtures) {
            const auto& f = *kv.second;
            r.push_back({"FIXTURE", f.getId(), f.getHome()->getRegistrationNumber(), f.getAway()->getRegistrationNumber(),
                         f.getStadium()->getId(), f.getDate(), formatTime(f.getTimeMinutes()), toString(f.getStatus())});
            for (const auto& o : f.getOfficials()) r.push_back({"FOFFICIAL", f.getId(), o->getId()});
            if (f.getResult())
                r.push_back({"RESULT", f.getId(), to_string(f.getResult()->getHomeScore()),
                             to_string(f.getResult()->getAwayScore()), to_string(f.getResult()->getYellowCards()),
                             to_string(f.getResult()->getRedCards())});
        }
        for (const auto& kv : spectators)
            for (const auto& t : kv.second->getTickets())
                r.push_back({"TICKET", t.getTicketId(), t.getFixtureId(), t.getSpectatorId(), toString(t.getCategory()),
                             money(t.getPrice())});
        for (const auto& p : shop.getProducts())
            r.push_back({"PRODUCT", string(1, p->typeCode()), p->getId(), p->getName(), money(p->getPrice()),
                         to_string(p->getStock()), p->saveExtra()});
        r.push_back({"SHOP", money(shop.getTotalSales()), to_string(shop.getTransactionCount())});
        return r;
    }

    // Writes record 0 (header) and then every data record at its own position in the file.
    void save(const string& path) const {
        vector<vector<string>> records = buildRecords();
        RecordFile file(path, RecordFile::CREATE);
        file.write(0, makeRecord("HEADER", {FILE_FORMAT, to_string(records.size())}));
        for (size_t i = 0; i < records.size(); i++) {
            vector<string> fields(records[i].begin() + 1, records[i].end());      // everything after the tag
            file.write(static_cast<long>(i) + 1, makeRecord(records[i][0], fields));
        }
        file.flush();
    }

    static FootballLeague loadFromFile(const string& path) {
        RecordFile file(path, RecordFile::READ);            // throws if the file does not exist
        long count = 0;
        try {
            if (file.recordCount() < 1) throw KpflException("empty file");
            vector<string> header = recordToStrings(file.read(0));
            if (header.size() < 3 || header[0] != "HEADER" || header[1] != FILE_FORMAT) throw KpflException("bad header");
            count = stol(header[2]);
            if (file.recordCount() != count + 1) throw KpflException("wrong number of records");
        } catch (const exception&) {
            throw KpflException("Not a valid (or a damaged) KPFL data file: " + path);
        }
        FootballLeague lg;
        for (long i = 1; i <= count; i++) {                 // read record 1, 2, 3 ... in dependency order
            try { lg.loadLine(recordToStrings(file.read(i))); }
            catch (const exception& e) {
                throw KpflException("Problem in data file at record " + to_string(i) + ": " + e.what());
            }
        }
        return lg;
    }

    // Number of records in a data file (header included).
    static long recordCountInFile(const string& path) {
        RecordFile file(path, RecordFile::READ);
        return file.recordCount();
    }
    // RANDOM READ demo: fetch ONE record by number without reading any of the others.
    static string describeRecordAt(const string& path, long n) {
        RecordFile file(path, RecordFile::READ);
        long total = file.recordCount();
        if (n < 0 || n >= total) throw InvalidDataException("Record number must be between 0 and " + to_string(total - 1) + ".");
        vector<string> parts = recordToStrings(file.read(n));
        string out = "Record " + to_string(n) + ": ";
        for (size_t i = 0; i < parts.size(); i++) out += (i ? " | " : "") + parts[i];
        return out;
    }

private:
    // Rebuilds one record from a saved line. Throws if the line is damaged.
    void loadLine(const vector<string>& p) {
        const string& tag = p[0];
        auto need = [&](size_t n) { if (p.size() < n) throw InvalidDataException("Too few fields for " + tag); };
        if (tag == "COUNTER") { need(3); idCounters[p[1]] = stoi(p[2]); }
        else if (tag == "COMMISSIONER") {
            need(5);
            commissioner = make_shared<LeagueCommissioner>(p[1], p[2], stoi(p[3]), p[4]);
        } else if (tag == "TEAM") {
            need(12);
            auto t = make_shared<FootballTeam>(p[2], p[3], p[1]);
            t->setStatus(teamStatusFromString(p[4]));
            t->restoreStats(stoi(p[5]), stoi(p[6]), stoi(p[7]), stoi(p[8]), stoi(p[9]), stoi(p[10]), stoi(p[11]));
            teams[t->getRegistrationNumber()] = t;
        } else if (tag == "PLAYER") {
            need(10);
            auto pl = make_shared<Player>(p[2], p[3], stoi(p[4]), p[5], p[6], stoi(p[7]));
            pl->addGoals(stoi(p[8]));
            pl->setPlayingStatus(playingStatusFromString(p[9]));
            requireTeam(p[1])->addPlayer(pl);
        } else if (tag == "COACH") {
            need(8);
            requireTeam(p[1])->addCoach(make_shared<Coach>(p[2], p[3], stoi(p[4]), p[5], p[6], p[7]));
        } else if (tag == "STADIUM") {
            need(8);
            auto s = make_shared<Stadium>(p[1], p[2], p[3], stoi(p[4]), fieldConditionFromString(p[5]), stod(p[6]));
            s->setCurrentMatchup(p[7]);
            stadiums[s->getId()] = s;
        } else if (tag == "OCC") { need(4); requireStadium(p[1])->restoreOccupancy(p[2], stoi(p[3])); }
        else if (tag == "OFFICIAL") {
            need(8);
            auto o = make_shared<MatchOfficial>(p[1], p[2], stoi(p[3]), p[4], p[5], p[6]);
            o->setAvailable(p[7] == "1");
            officials[o->getId()] = o;
        } else if (tag == "SPECTATOR") {
            need(5);
            auto s = make_shared<Spectator>(p[1], p[2], stoi(p[3]), p[4]);
            spectators[s->getId()] = s;
        } else if (tag == "FIXTURE") {
            need(8);
            auto fx = make_shared<Fixture>(p[1], requireTeam(p[2]), requireTeam(p[3]), requireStadium(p[4]),
                                                validateDate(p[5]), parseTime(p[6]));
            fx->restoreStatus(fixtureStatusFromString(p[7]));
            fixtures[fx->getId()] = fx;
        } else if (tag == "FOFFICIAL") {
            need(3);
            auto it = officials.find(p[2]);
            if (it == officials.end()) throw InvalidDataException("Unknown official " + p[2]);
            requireFixture(p[1])->addOfficial(it->second);
        } else if (tag == "RESULT") {
            need(6);
            auto fx = requireFixture(p[1]);
            fx->setResult(MatchResult(fx->getHome()->getName(), fx->getAway()->getName(), stoi(p[2]), stoi(p[3]),
                                      stoi(p[4]), stoi(p[5])));
        } else if (tag == "TICKET") {
            need(6);
            auto it = spectators.find(p[3]);
            if (it == spectators.end()) throw InvalidDataException("Unknown spectator " + p[3]);
            it->second->addTicket(Ticket(p[1], p[2], p[3], ticketCategoryFromString(p[4]), stod(p[5])));
        } else if (tag == "PRODUCT") {
            need(7);
            if (p[1] == "M") shop.addProduct(make_shared<Merchandise>(p[2], p[3], stod(p[4]), stoi(p[5]), p[6]));
            else shop.addProduct(make_shared<FoodItem>(p[2], p[3], stod(p[4]), stoi(p[5]), p[6] == "1"));
        } else if (tag == "SHOP") { need(3); shop.restoreTotals(stod(p[1]), stoi(p[2])); }
        else throw InvalidDataException("Unknown record type " + tag);
    }
};

}  // namespace kpfl
