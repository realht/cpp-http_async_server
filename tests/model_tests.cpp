#include <catch2/catch_test_macros.hpp>
#include "../src/loot_generator.h"
#include "../src/app.h"
#include "../src/json_loader.h"


using namespace std::literals;

constexpr const char DB_URL[]{ "GAME_DB_URL" };

SCENARIO("Trophy generation") {
    using namespace std::literals;
    extra_data::TrophyList trophies;

    GIVEN("Input game data") {

        const char* db_url = std::getenv(DB_URL);

        // for local base test
        //const char* db_url = "postgres://postgres:1231234@localhost:5432/book_db"; 

        if (!db_url) {
            throw std::runtime_error("DB URL is not specified");
        }

        model::Game game = json_loader::LoadGame("../data/config.json", trophies);
        app::Players players;
        app::AppConfig aconf{ true, false, "", 0, db_url };

        app::Application apl(game, players, trophies, aconf);

        WHEN("add 1 dog") {
            auto pi = apl.JoinGame("map1", "John");
            auto sess = apl.GetSession(*pi.token);

            THEN("Count trophy on start") {
                REQUIRE(sess->GetCountTrophyOnMap() == 0);
            }

            REQUIRE(game.GetCountNewTrophy(*sess, 0) == 0);
            REQUIRE(game.GetCountNewTrophy(*sess, 10) == 1);

            THEN("Add trophy on map") {
                apl.UpdateWorldState(30);
                REQUIRE(sess->GetCountTrophyOnMap() == 1);
            }
            

            WHEN("add 2 more dog") {
                apl.JoinGame("map1", "Mike Lasa");
                apl.JoinGame("map1", "Nike Tyson");
                REQUIRE(sess->GetCountDogOnMap() == 3);
                REQUIRE(game.GetCountNewTrophy(*sess, 20) == 3);

                THEN("Add trophy on map") {
                    apl.UpdateWorldState(30);
                    REQUIRE(sess->GetCountTrophyOnMap() == 3);
                }
            }
        }
    }
}