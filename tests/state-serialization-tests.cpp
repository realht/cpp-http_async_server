#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <catch2/catch_test_macros.hpp>
#include <sstream>
#include "../src/model.h"
#include "../src/model_serialization.h"

using namespace model;
using namespace std::literals;
namespace {

using InputArchive = boost::archive::text_iarchive;
using OutputArchive = boost::archive::text_oarchive;

using InputPoliArchive = boost::archive::polymorphic_text_iarchive;
using OutputPoliArchive = boost::archive::polymorphic_text_oarchive;

struct Fixture {
    std::stringstream strm;
    OutputArchive output_archive{strm};
};

struct FixturePoli {
    std::stringstream strm;
    OutputPoliArchive output_archive{ strm };
};

}  // namespace

SCENARIO_METHOD(Fixture, "Point serialization") {
    GIVEN("A point") {
        const geom::Point2D p{10, 20};
        WHEN("point is serialized") {
            output_archive << p;

            THEN("it is equal to point after serialization") {
                InputArchive input_archive{strm};
                geom::Point2D restored_point;
                input_archive >> restored_point;
                CHECK(p == restored_point);
            }
        }
    }
}

SCENARIO_METHOD(Fixture, "Dog Serialization") {
    GIVEN("a dog") {
        const auto dog = [] {
            Dog dog{"Pluto"s, {42.2, 12.5}, 3};
            dog.AddScore(42);
            dog.AddItemToBag({ 0,1,{1,2} });
            dog.SetDirection(Direction::East);
            dog.SetSpeed({2.3, -1.2});
            return dog;
        }();

        WHEN("dog is serialized") {
            {
                serialization::DogRepr repr{dog};
                output_archive << repr;
            }

            THEN("it can be deserialized") {
                InputArchive input_archive{strm};
                serialization::DogRepr repr;
                input_archive >> repr;
                const auto restored = repr.Restore();

                CHECK(dog.GetName() == restored.GetName());
                CHECK(dog.GetPosition() == restored.GetPosition());
                CHECK(dog.GetSpeed() == restored.GetSpeed());
                CHECK(dog.GetBagCapacity() == restored.GetBagCapacity());
                CHECK(dog.GetScore() == restored.GetScore());
                CHECK(dog.GetDirection() == restored.GetDirection());
                CHECK(dog.GetItemFromBag().size() == restored.GetItemFromBag().size());
            }
        }
    }
}

SCENARIO_METHOD(FixturePoli, "Trophy Serialization") {
    GIVEN("trophy") {
        model::Trophy trophy{ 11, 15, {17,19} };
        std::string map_id = "map17";

        WHEN("Trophy is serialized") {
            {
                serialization::TrophyPrep trophy_prep{ map_id, trophy };
                output_archive << trophy_prep;
            }

            THEN("Trophy can be deserialized") {
                InputPoliArchive input_archive{ strm };
                serialization::TrophyPrep trophyp;
                input_archive >> trophyp;

                CHECK(trophy.GetId() == trophyp.GetId());
                CHECK(trophy.GetType() == trophyp.GetType());
                CHECK(trophy.GetPosition() == trophyp.GetPos());
                CHECK(trophyp.GetMapID() == map_id);
            }
        }  
    }
}

SCENARIO_METHOD(FixturePoli, "Player Serialization") {
    GIVEN("Player") {
        model::Map::Id map_id{ "map17" };
        model::Map map{ map_id, "map 17" };
        model::Dog dog{ "Bond"s, {4, 5}, 3 };
        model::GameSession session(dog, map);
        const model::GameSession* session_ptr = &session;
        const model::Dog* dog_ptr = &dog;
        Token token("thisCurrectTokenForTest");
        app::Player player(session_ptr, dog_ptr, token, 777);

        WHEN("Player is serialized") {
            {
                serialization::PlayerPrep p_p{ player };
                output_archive << p_p;
            }

            THEN("Player can be deserialized") {
                InputPoliArchive input_archive{ strm };
                serialization::PlayerPrep pp;
                input_archive >> pp;

                CHECK(player.GetId() == pp.GetID());
                CHECK(player.GetToken() == pp.GetToken());
                CHECK(pp.GetMapId() == *map_id);

                model::Dog rdog(pp.GetDogRepr().Restore());
                CHECK(rdog.GetName() == "Bond");
                CHECK(rdog.GetPosition() == dog.GetPosition());
                CHECK(rdog.GetBagCapacity() == dog.GetBagCapacity());
                CHECK(rdog.GetItemFromBag().size() == dog.GetItemFromBag().size());
                CHECK(rdog.GetSpeed() == dog.GetSpeed());
                CHECK(rdog.GetScore() == dog.GetScore());
            }
        }
    }
}