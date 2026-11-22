// demo.h - builds a small sample league so the menus can be tried straight away.
#pragma once
#include "league.h"

namespace kpfl {

inline FootballLeague createLeague() 
{
    FootballLeague league;
    const char* teams[4][3] = 
                            {
                            {"Kingston Lions", "Kingston", "KPFL01"},
                               {"Papine Rangers", "Saint Andrew", "KPFL02"},
                               {"Harbour View Stars", "Kingston", "KPFL03"},
                               {"Cross Roads United", "Saint Andrew", "KPFL04"}
                            };
    for (auto& t : teams) 
    {
        league.registerTeam(t[0], t[1], t[2]);
        league.approveTeam(t[2]);
        string shortName = string(t[0]).substr(0, string(t[0]).find(' '));
        for (int i = 1; i <= 11; i++)
            league.addPlayer(t[2], shortName + " Player" + to_string(i), 18 + i, i % 2 == 0 ? "Male" : "Female",
                             i == 1 ? "Goalkeeper" : (i < 6 ? "Defender" : (i < 9 ? "Midfielder" : "Forward")), i);
        league.addCoach(t[2], shortName + " Coach", 40, "Male", "CONCACAF B Licence", "Head Coach");
    }
    
    league.registerStadium("National Stadium", "Kingston", 35000, FieldCondition::Excellent, 1150.00);
    league.registerStadium("Harbour View Arena", "Kingston", 7000, FieldCondition::Good, 575.00);
    league.registerOfficial("Peter Clarke", 38, "Male", "Referee", "National");
    league.registerOfficial("Andre Reid", 33, "Male", "Assistant Referee", "National");
    league.registerOfficial("Sasha Grant", 29, "Female", "Fourth Official", "Regional");

    string day = todayPlusDays(7);
    league.scheduleFixture("KPFL01", "KPFL02", "S001", day, "15:00");
    league.scheduleFixture("KPFL03", "KPFL04", "S002", day, "15:00");
    league.registerSpectator("Devon Walker", 27, "Male");

    StadiumShop& shop = league.getShop();
    shop.addProduct(make_shared<Merchandise>("M001", "KPFL Jersey", 6900.00, 40, "M"));
    shop.addProduct(make_shared<Merchandise>("M002", "Team Flag", 1500.00, 60, "One size"));
    shop.addProduct(make_shared<FoodItem>("F001", "Jerk Chicken Box", 1200.00, 100, false));
    shop.addProduct(make_shared<FoodItem>("B001", "Bottled Water", 250.00, 200, true));
    return league;
}

}  // namespace kpfl
