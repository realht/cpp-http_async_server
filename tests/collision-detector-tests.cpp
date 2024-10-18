#define _USE_MATH_DEFINES
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>
#include "../src/collision_detector.h"

#include <sstream>

static const double EPSILON = 1e-10;

namespace Catch {
    template<>
    struct StringMaker<collision_detector::GatheringEvent> {
        static std::string convert(collision_detector::GatheringEvent const& value) {
            std::ostringstream tmp;
            tmp << "(" << *value.gatherer_token << "," << value.item_id << "," << value.sq_distance << "," << value.time << ")";

            return tmp.str();
        }
    };
}

class TestItemGathererProvider : public collision_detector::ItemGathererProvider {
public:
    TestItemGathererProvider(std::vector<collision_detector::Item> items,
        std::vector<collision_detector::Gatherer> gaths,
        std::vector < collision_detector::Office> office) :
        items_(items),
        gaths_(gaths),
        office_(office){}

    size_t ItemsCount() const override {
        return items_.size();
    }
    collision_detector::Item GetItem(size_t idx) const override {
        return items_.at(idx);
    }
    size_t GatherersCount() const override {
        return gaths_.size();
    }
    collision_detector::Gatherer GetGatherer(size_t idx) const override {
        return gaths_.at(idx);
    }
    size_t OfficeCount() const override {
        return office_.size();
    }
    collision_detector::Office GetOffice(size_t idx) const override {
        return office_.at(idx);
    }

private:
    std::vector<collision_detector::Item> items_;
    std::vector<collision_detector::Gatherer> gaths_;
    std::vector<collision_detector::Office> office_;
};

namespace compare {
    class CompareResults {
    public:
        bool operator()(const collision_detector::GatheringEvent& l,
            const collision_detector::GatheringEvent& r) {
            if (l.is_office != r.is_office) {
                return false;
            }
            if (*l.gatherer_token != *r.gatherer_token || l.item_id != r.item_id) {
                return false;
            }
            if (std::abs(l.sq_distance - r.sq_distance) > EPSILON) {
                return false;
            }
            if (std::abs(l.time - r.time) > EPSILON) {
                return false;
            }
            return true;
        }
    };

    template <typename Range, typename Predicate>
    struct IsEqualResultMatcher : Catch::Matchers::MatcherGenericBase {
        IsEqualResultMatcher(Range const& range, Predicate predicate) :
            range_(std::move(range)),
            predicate_(std::move(predicate)) {}

        template <typename OtherRange>
        bool match(OtherRange other) const {
            using std::begin;
            using std::end;
            return std::equal(begin(range_), end(range_), begin(other), end(other), predicate_);
        }

        std::string describe() const override {
            return "Is equal of: " + Catch::rangeToString(range_);
        }

    private:
        Range range_;
        Predicate predicate_;
    };

    template <typename Range, typename Predicate>
    auto IsEqualResult(const Range& range, Predicate predicate) {
        return IsEqualResultMatcher<Range, Predicate>{range, predicate};
    }
}




SCENARIO("Check collision method \"FindGatherEvents\"") {
    using namespace std::literals;
    WHEN("item list empty") {
        Token t1("A");
        Token t2("B");
        TestItemGathererProvider no_item_list{
            {},
            { {t1,{0,0},{0,2}, 0.6}, {t2, {4,2},{8,2}, 0.6}},
            {}
        };
        THEN("result list must be empty") {
            auto result_no_items = collision_detector::FindGatherEvents(no_item_list);
            CHECK(result_no_items.empty());
        }
    }
    WHEN("garther list empty") {
        TestItemGathererProvider no_garther_list{
            { {0, {0,0},0.1}, {1, {1,1},0.1}, {2, {2,2},0.1} },
            {},
            {}
        };
        THEN("result list must be empty") {
            auto result_no_gather = collision_detector::FindGatherEvents(no_garther_list);
            CHECK(result_no_gather.empty());
        }
    }
    WHEN("several items and one gather") {
        Token t1("A");
        TestItemGathererProvider several_items_one_gather{
            { {0, {1,2},0.1}, {1, {2,2.4},0.1}, {2,{3,1.7},0.1}, {3,{4,3},0.1}, {4,{5,2.1},0.1}, {5,{6,1.3},0.1}, {6,{7,2},0.1}, {7,{8,2},0.1} },
            { {t1, {2,2},{7,2},0.6} },
            {}
        };
        THEN("correct collision detected") {
            auto result_several_gather_one_gather = collision_detector::FindGatherEvents(several_items_one_gather);
            std::vector<collision_detector::GatheringEvent> currect_result = {
                {false, 1, t1, 0.4 * 0.4, 0},
                {false, 2, t1, 0.3 * 0.3, 0.2},
                {false, 4, t1, 0.1 * 0.1, 0.6},
                {false, 5, t1, 0.7 * 0.7, 0.8},
                {false, 6, t1, 0.0 * 0.0, 1} };
            CHECK_THAT(result_several_gather_one_gather, compare::IsEqualResult(currect_result, compare::CompareResults()));
            
        }
    }
    WHEN("one item several gather") {
        Token t1("A");
        Token t2("B");
        Token t3("C");
        TestItemGathererProvider one_item_several_gather{
            { {0, {0,0.1}, 0.1} },
            { {t1, {-2,0}, {3,0}, 0.6}, {t2, {0,1}, {0,-7}, 0.6}, {t3, {0.5,0.5}, {-0.5,-0.5}, 0.6} },
            {}
        };
        THEN("correct data order") {
            auto result_one_item_several_gather = collision_detector::FindGatherEvents(one_item_several_gather);
            std::vector<collision_detector::GatheringEvent> currect_result = {
                {false, 0, t2, 0.0 * 0.0, 0.1125},
                {false, 0, t1, 0.1 * 0.1, 0.4},
                {false, 0, t3, 0.005, 0.45} };
            CHECK_THAT(result_one_item_several_gather, compare::IsEqualResult(currect_result, compare::CompareResults()));
        }
    }
    WHEN("gather no move") {
        Token t1("A");
        Token t2("B");
        TestItemGathererProvider gather_no_move{
            { {0, {1,1}, 1} },
            { {t1, {0.5,0.5},{0.5,0.5},2}, {t2, {2,2},{2,2},3}},
            {}
        };
        THEN("result list must be empty") {
            auto result_gather_no_move = collision_detector::FindGatherEvents(gather_no_move);
            CHECK(result_gather_no_move.empty());
        }
    }
}