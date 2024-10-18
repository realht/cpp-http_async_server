#pragma once
#include <algorithm>
#include <vector>
#include "geom.h"
#include "player_tokens.h"


namespace collision_detector {

    const double DOG_COLLIDER_SIZE = 0.3;
    const double OFFICE_COLLIDER_SIZE = 0.25;
    const double ITEM_COLLIDER_SIZE = 0.0;

struct CollectionResult {
    bool IsCollected(double collect_radius) const {
        return proj_ratio >= 0 && proj_ratio <= 1 && sq_distance <= collect_radius * collect_radius;
    }

    // квадрат расстояния до точки
    double sq_distance;

    // доля пройденного отрезка
    double proj_ratio;
};

CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c);

struct Item {
    size_t id;
    geom::Point2D position;
    double width;
};

struct Gatherer {
    Token token;
    geom::Point2D start_pos;
    geom::Point2D end_pos;
    double width;
};

struct Office {
    std::string id;
    geom::Point2D position;
    double width;
};

class ItemGathererProvider {
protected:
    ~ItemGathererProvider() = default;

public:
    virtual size_t ItemsCount() const = 0;
    virtual Item GetItem(size_t idx) const = 0;
    virtual size_t GatherersCount() const = 0;
    virtual Gatherer GetGatherer(size_t idx) const = 0;
    virtual size_t OfficeCount() const = 0;
    virtual Office GetOffice(size_t idx) const = 0;
};

struct GatheringEvent {
    bool is_office;
    size_t item_id;
    Token gatherer_token;
    double sq_distance;
    double time;
};

static auto IsPointEquals = [](geom::Point2D p1, geom::Point2D p2) {
    return (p1.x == p2.x) && (p1.y == p2.y);
};

static auto SortByTime = [](const GatheringEvent& l, const GatheringEvent& r) {
    return l.time < r.time;
};

std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider);

class TrophyProvider : public collision_detector::ItemGathererProvider {
public:
    TrophyProvider(const std::vector<collision_detector::Item>& items,
        const std::vector<collision_detector::Gatherer>& gaths,
        const std::vector < collision_detector::Office>& office) :
        items_(items),
        gaths_(gaths),
        office_(office) {}

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

}  //collision_detector