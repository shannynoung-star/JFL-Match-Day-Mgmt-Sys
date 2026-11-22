// tests.cpp - plain C++ test program (no libraries needed).
// Each test is labelled NORMAL, BOUNDARY or INVALID so it matches the test plan.
#include <cstdio>
#include <functional>
#include <iostream>
#include "league.h"

using namespace kpfl;

static int passed = 0, failed = 0;

static void check(const string& type, const string& name, bool ok) {
    (ok ? passed : failed)++;
    cout << padRight(type, 9) << padRight(name, 64) << (ok ? "PASS" : "FAIL") << "\n";
}

// Passes only if the action throws exactly the expected exception type (or a child of it).
template <typename ExpectedException>
static void expectThrow(const string& type, const string& name, const function<void()>& action) {
    bool ok = false;
    try { action(); }
    catch (const ExpectedException&) { ok = true; }
    catch (...) { ok = false; }
    check(type, name, ok);
}

static FootballLeague fresh() {
    FootballLeague l;
    l.registerTeam("Alpha FC", "Kingston", "A1");       l.approveTeam("A1");
    l.registerTeam("Beta FC", "Saint Andrew", "B1");    l.approveTeam("B1");
    l.registerTeam("Gamma FC", "Kingston", "G1");       l.approveTeam("G1");
    l.registerStadium("Tiny Park", "Kingston", 10, FieldCondition::Good, 100.00);
    l.registerOfficial("Ref One", 30, "Male", "Referee", "National");
    l.registerSpectator("Fan One", 25, "Female");
    return l;
}
static void addExtraTeams(FootballLeague& x) {
    x.registerTeam("D FC", "K", "D1"); x.approveTeam("D1");
    x.registerTeam("E FC", "K", "E1"); x.approveTeam("E1");
}

int main() {
    const string day = "2030-01-10", t3 = "15:00";
    cout << padRight("TYPE", 9) << padRight("TEST", 64) << "RESULT\n";

    // ---------- NORMAL ----------
    {
        FootballLeague l = fresh();
        l.addPlayer("A1", "Sam Smith", 22, "Male", "Forward", 9);
        check("NORMAL", "Add player to a team", l.requireTeam("A1")->getSquadSize() == 1);
        l.removePlayer("A1", "P001");
        check("NORMAL", "Remove player from a team", l.requireTeam("A1")->getSquadSize() == 0);
        auto f = l.scheduleFixture("A1", "B1", "S001", day, t3);
        check("NORMAL", "Schedule a valid fixture", f->getStatus() == FixtureStatus::Scheduled);
        l.assignOfficial(f->getId(), "O001");
        check("NORMAL", "Assign an official", f->getOfficials().size() == 1);
        l.recordResult(f->getId(), 2, 1, 1, 0);
        check("NORMAL", "Win gives 3 points, loss gives 0", l.requireTeam("A1")->getPoints() == 3 && l.requireTeam("B1")->getPoints() == 0);
        check("NORMAL", "Fixture marked COMPLETED", f->getStatus() == FixtureStatus::Completed);
        auto f2 = l.scheduleFixture("A1", "G1", "S001", "2030-01-11", t3);
        l.recordResult(f2->getId(), 1, 1, 0, 0);
        check("NORMAL", "Draw gives 1 point each", l.requireTeam("G1")->getPoints() == 1);

        Receipt r;                                   // 115.00 includes GCT -> subtotal 100.00, tax 15.00
        r.addLine("Item", 1, 115.00);
        r.finish(200.00);
        check("NORMAL", "Receipt subtotal is 100.00", r.getSubtotal() == 100.00);
        check("NORMAL", "Receipt tax is 15.00", r.getTax() == 15.00);
        check("NORMAL", "Receipt change is 85.00", r.getChange() == 85.00);

        FootballLeague saved = fresh();
        saved.addPlayer("A1", "Saved Player", 21, "Male", "Forward", 10);
        auto fx = saved.scheduleFixture("A1", "B1", "S001", day, t3);
        saved.assignOfficial(fx->getId(), "O001");
        saved.sellTickets(fx->getId(), "SP001", TicketCategory::Vip, 2, 1000.00);
        saved.getShop().addProduct(make_shared<FoodItem>("F1", "Patty", 300, 5, false));
        saved.save("test_data.dat");
        FootballLeague back = FootballLeague::loadFromFile("test_data.dat");
        check("NORMAL", "Save and reload keeps teams and players",
              back.getTeams().size() == 3 && back.requireTeam("A1")->getSquadSize() == 1);
        check("NORMAL", "Save and reload keeps fixture, officials and sold seats",
              back.getFixtures().size() == 1 && back.getFixtures()[0]->getOfficials().size() == 1 &&
              back.requireStadium("S001")->getOccupancy("F001") == 2);
        check("NORMAL", "Save and reload keeps tickets and shop stock",
              back.getSpectators()[0]->getTickets().size() == 2 && back.getShop().findProduct("F1")->getStock() == 5);
        // long team names make a long "current matchup" text; saving must still work
        FootballLeague longNames = fresh();
        longNames.registerTeam("Harbour View Stars Football Club", "K", "H1"); longNames.approveTeam("H1");
        longNames.registerTeam("Cross Roads United Football Club", "K", "H2"); longNames.approveTeam("H2");
        longNames.scheduleFixture("H1", "H2", "S001", day, t3);
        bool longSaved = true;
        try { longNames.save("long_test.dat"); } catch (const KpflException&) { longSaved = false; }
        check("NORMAL", "Save works even with very long team names", longSaved);
        remove("long_test.dat");
        // random access: read ONE record directly by its number
        check("NORMAL", "Random access: record 1 is read directly by number",
              FootballLeague::describeRecordAt("test_data.dat", 1).find("Record 1: ") == 0);
        check("NORMAL", "Random access: file size = records x RECORD_SIZE",
              FootballLeague::recordCountInFile("test_data.dat") > 10);
        remove("test_data.dat");
    }

    // ---------- BOUNDARY ----------
    {
        FootballLeague l = fresh();
        l.addPlayer("A1", "Exactly Sixteen", Player::MINIMUM_AGE, "Male", "Forward", 7);
        check("BOUNDARY", "Player aged exactly 16 is accepted", l.requireTeam("A1")->getSquadSize() == 1);
        expectThrow<BusinessRuleException>("BOUNDARY", "Player aged 15 is rejected",
            [] { fresh().addPlayer("A1", "Too Young", 15, "Male", "Forward", 8); });
        l.addPlayer("A1", "Jersey One", 20, "Male", "Defender", 1);
        l.addPlayer("A1", "Jersey 99", 20, "Male", "Defender", 99);
        check("BOUNDARY", "Jersey numbers 1 and 99 accepted", l.requireTeam("A1")->getSquadSize() == 3);
        expectThrow<InvalidDataException>("BOUNDARY", "Jersey number 100 rejected",
            [] { fresh().addPlayer("A1", "Big Number", 20, "Male", "Defender", 100); });
        expectThrow<InvalidDataException>("BOUNDARY", "Jersey number 0 rejected",
            [] { fresh().addPlayer("A1", "Zero", 20, "Male", "Defender", 0); });
        auto bf = l.scheduleFixture("A1", "B1", "S001", day, t3);
        l.sellTickets(bf->getId(), "SP001", TicketCategory::Standard, 10, 1150.00);   // exactly the capacity (10)
        check("BOUNDARY", "Selling exactly the capacity (10) works", l.requireStadium("S001")->getRemainingSeats(bf->getId()) == 0);
        expectThrow<CapacityExceededException>("BOUNDARY", "Selling one seat more than capacity fails",
            [&] { l.sellTickets(bf->getId(), "SP001", TicketCategory::Standard, 1, 115.00); });
        Receipt exact;
        exact.addLine("Item", 1, 50.00);
        exact.finish(50.00);
        check("BOUNDARY", "Payment exactly equal to total gives 0 change", exact.getChange() == 0.0);
        check("BOUNDARY", "Match with 0-0 score is a draw", MatchResult("H", "A", 0, 0, 0, 0).getWinner() == "Draw");
        expectThrow<SchedulingConflictException>("BOUNDARY", "Stadium booked 119 min later clashes", [&] {
            FootballLeague x = fresh(); addExtraTeams(x);
            x.scheduleFixture("A1", "B1", "S001", day, t3);
            x.scheduleFixture("D1", "E1", "S001", day, "16:59");
        });
        FootballLeague y = fresh(); addExtraTeams(y);
        y.scheduleFixture("A1", "B1", "S001", day, t3);
        y.scheduleFixture("D1", "E1", "S001", day, "17:00");
        check("BOUNDARY", "Stadium booked exactly 120 min later is allowed", y.getFixtures().size() == 2);
        check("BOUNDARY", "Team name of exactly 39 characters accepted",
              fresh().registerTeam(string(39, 'x'), "K", "L1")->getName().size() == 39);
        expectThrow<InvalidDataException>("BOUNDARY", "Team name of 40 characters rejected",
            [] { fresh().registerTeam(string(40, 'x'), "K", "L2"); });
        expectThrow<InvalidDataException>("BOUNDARY", "Impossible date 2030-02-30 rejected",
            [&] { fresh().scheduleFixture("A1", "B1", "S001", "2030-02-30", t3); });
    }

    // ---------- INVALID ----------
    expectThrow<BusinessRuleException>("INVALID", "Unregistered team cannot be scheduled", [&] {
        FootballLeague x = fresh();
        x.registerTeam("New FC", "K", "N1");
        x.scheduleFixture("N1", "A1", "S001", day, t3);
    });
    expectThrow<BusinessRuleException>("INVALID", "Suspended team cannot be scheduled", [&] {
        FootballLeague x = fresh();
        x.suspendTeam("A1");
        x.scheduleFixture("A1", "B1", "S001", day, t3);
    });
    expectThrow<SchedulingConflictException>("INVALID", "Team cannot play twice on the same day", [&] {
        FootballLeague x = fresh();
        x.registerStadium("Second Park", "K", 50, FieldCondition::Good, 100);
        x.scheduleFixture("A1", "B1", "S001", day, t3);
        x.scheduleFixture("A1", "G1", "S002", day, "20:00");
    });
    expectThrow<SchedulingConflictException>("INVALID", "Official cannot take two clashing fixtures", [&] {
        FootballLeague x = fresh(); addExtraTeams(x);
        x.registerStadium("Second Park", "K", 50, FieldCondition::Good, 100);
        auto a = x.scheduleFixture("A1", "B1", "S001", day, t3);
        auto b = x.scheduleFixture("D1", "E1", "S002", day, t3);
        x.assignOfficial(a->getId(), "O001");
        x.assignOfficial(b->getId(), "O001");
    });
    expectThrow<InvalidDataException>("INVALID", "Negative match score rejected", [&] {
        FootballLeague x = fresh();
        auto a = x.scheduleFixture("A1", "B1", "S001", day, t3);
        x.recordResult(a->getId(), -1, 0, 0, 0);
    });
    expectThrow<BusinessRuleException>("INVALID", "Fixture cannot complete without a result", [&] {
        Fixture f("F9", nullptr, nullptr, nullptr, day, 900);
        f.markCompleted();
    });
    expectThrow<BusinessRuleException>("INVALID", "Result cannot be recorded twice", [&] {
        FootballLeague x = fresh();
        auto a = x.scheduleFixture("A1", "B1", "S001", day, t3);
        x.recordResult(a->getId(), 1, 0, 0, 0);
        x.recordResult(a->getId(), 2, 0, 0, 0);
    });
    expectThrow<InsufficientPaymentException>("INVALID", "Insufficient payment for tickets rejected", [&] {
        FootballLeague x = fresh();
        auto a = x.scheduleFixture("A1", "B1", "S001", day, t3);
        x.sellTickets(a->getId(), "SP001", TicketCategory::Standard, 2, 50.00);
    });
    expectThrow<InvalidDataException>("INVALID", "Negative ticket quantity rejected", [&] {
        FootballLeague x = fresh();
        auto a = x.scheduleFixture("A1", "B1", "S001", day, t3);
        x.sellTickets(a->getId(), "SP001", TicketCategory::Standard, -2, 500.00);
    });
    expectThrow<BusinessRuleException>("INVALID", "Duplicate jersey number rejected", [&] {
        FootballLeague x = fresh();
        x.addPlayer("A1", "One", 20, "Male", "Forward", 9);
        x.addPlayer("A1", "Two", 20, "Male", "Forward", 9);
    });
    expectThrow<InvalidDataException>("INVALID", "Duplicate team registration number rejected",
        [] { fresh().registerTeam("Copy FC", "K", "a1"); });
    expectThrow<InvalidDataException>("INVALID", "Empty team name rejected", [] { fresh().registerTeam("  ", "K", "Z9"); });
    expectThrow<InvalidDataException>("INVALID", "Unknown team ID rejected", [] { fresh().requireTeam("NOPE"); });
    expectThrow<InvalidDataException>("INVALID", "Text containing '|' rejected", [] { fresh().registerTeam("Bad|Name", "K", "Z8"); });
    expectThrow<InsufficientPaymentException>("INVALID", "Shop sale with insufficient payment rejected", [] {
        StadiumShop s;
        s.addProduct(make_shared<FoodItem>("F1", "Patty", 300, 5, false));
        s.purchase({{"F1", 2}}, 100);
    });
    {
        StadiumShop shop;
        shop.addProduct(make_shared<FoodItem>("F1", "Patty", 300, 5, false));
        bool threw = false;
        try { shop.purchase({{"F1", 6}}, 5000); } catch (const InvalidDataException&) { threw = true; }
        check("INVALID", "Buying more than stock rejected (stock unchanged)", threw && shop.findProduct("F1")->getStock() == 5);
    }
    expectThrow<InvalidDataException>("INVALID", "Negative stock rejected", [] { FoodItem("X", "Bad", 10, -1, false); });
    expectThrow<KpflException>("INVALID", "Loading a missing file fails cleanly", [] { FootballLeague::loadFromFile("no_such_file.dat"); });

    {
        ofstream bad("bad_data.dat", ios::binary);
        bad << "this is not a KPFL data file";
        bad.close();
        expectThrow<KpflException>("INVALID", "Corrupt data file is rejected cleanly", [] { FootballLeague::loadFromFile("bad_data.dat"); });
        remove("bad_data.dat");
    }
    expectThrow<InvalidDataException>("INVALID", "Record number outside the file rejected", [&] {
        fresh().save("range_test.dat");
        try { FootballLeague::describeRecordAt("range_test.dat", 9999); } catch (...) { remove("range_test.dat"); throw; }
    });
    cout << "\nTOTAL: " << passed << " passed, " << failed << " failed\n";
    return failed > 0 ? 1 : 0;
}
