#include <cassert>
#include "collision_detector.h"
#include <iostream>

namespace collision_detector {

CollectionResult TryCollectPoint(geom::Point2D a, geom::Point2D b, geom::Point2D c) {

    assert(b.x != a.x || b.y != a.y);
    const double u_x = c.x - a.x;
    const double u_y = c.y - a.y;
    const double v_x = b.x - a.x;
    const double v_y = b.y - a.y;
    const double u_dot_v = u_x * v_x + u_y * v_y;
    const double u_len2 = u_x * u_x + u_y * u_y;
    const double v_len2 = v_x * v_x + v_y * v_y;
    const double proj_ratio = u_dot_v / v_len2;
    const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

    return CollectionResult(sq_distance, proj_ratio);
}

std::vector<GatheringEvent> FindGatherEvents(const ItemGathererProvider& provider) {
    std::vector<GatheringEvent> result;

    for (size_t i = 0; i < provider.GatherersCount(); ++i) {
        Gatherer gath = provider.GetGatherer(i);

        if (IsPointEquals(gath.start_pos, gath.end_pos)) {
            continue;
        }

        for (size_t j = 0; j < provider.ItemsCount(); ++j) {
            Item item = provider.GetItem(j);
            auto try_collision = TryCollectPoint(gath.start_pos, gath.end_pos, item.position);

            if (try_collision.IsCollected(gath.width + item.width)) {
                GatheringEvent collision{ false,  item.id, gath.token, try_collision.sq_distance, try_collision.proj_ratio };
                result.push_back(collision);
            }
        }

        for (size_t k = 0; k < provider.OfficeCount(); ++k) {
            Office office = provider.GetOffice(k);
            auto try_collision = TryCollectPoint(gath.start_pos, gath.end_pos, office.position);

            if (try_collision.IsCollected(gath.width + office.width)) {
                GatheringEvent collision{ true,  777777, gath.token, try_collision.sq_distance, try_collision.proj_ratio };
                result.push_back(collision);
            }
        }
    }

    std::sort(result.begin(), result.end(), SortByTime);
    return result;
}

}  // collision_detector