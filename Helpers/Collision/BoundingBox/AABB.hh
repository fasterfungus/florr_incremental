#pragma once
#include <Shared/Entity.hh>
#include <Helpers/Collision/Map/Wall.hh>
#include <algorithm>
struct AABB {
    float min_x, min_y, max_x, max_y;

    AABB() : min_x(0), min_y(0), max_x(0), max_y(0) {}
    AABB(float _min_x, float _min_y, float _max_x, float _max_y)
        : min_x(_min_x), min_y(_min_y), max_x(_max_x), max_y(_max_y) {}

    static AABB from_entity(Entity const &e) {
        float min_x = e.get_x() + e.get_minx();
        float min_y = e.get_y() + e.get_miny();
        float max_x = e.get_x() + e.get_maxx();
        float max_y = e.get_y() + e.get_maxy();
        return AABB(min_x, min_y, max_x, max_y);
    }

    static AABB from_half_extents(float x, float y, float w, float h) {
        return AABB(x - w, y - h, x + w, y + h);
    }

    static AABB from_circle(float x, float y, float r) {
        return AABB(x - r, y - r, x + r, y + r);
    }

    static AABB from_wall(Wall const &w) {
        return AABB(w.minx, w.miny, w.maxx, w.maxy);
    }

    void expand(AABB const &o) {
        min_x = std::min(min_x, o.min_x);
        min_y = std::min(min_y, o.min_y);
        max_x = std::max(max_x, o.max_x);
        max_y = std::max(max_y, o.max_y);
    }

    bool overlaps(AABB const &o) const {
        return !(max_x < o.min_x || min_x > o.max_x ||
                 max_y < o.min_y || min_y > o.max_y);
    }

    // 2D SAH cost proxy (analogue of Box2D's b2Perimeter surface area).
    float perimeter() const {
        return (max_x - min_x) + (max_y - min_y);
    }

    static AABB combine(AABB const &a, AABB const &b) {
        return AABB(std::min(a.min_x, b.min_x), std::min(a.min_y, b.min_y),
                    std::max(a.max_x, b.max_x), std::max(a.max_y, b.max_y));
    }

    // Does *this* fully enclose o?
    bool contains(AABB const &o) const {
        return min_x <= o.min_x && min_y <= o.min_y &&
               max_x >= o.max_x && max_y >= o.max_y;
    }

    float center_x() const { return (min_x + max_x) * 0.5f; }
    float center_y() const { return (min_y + max_y) * 0.5f; }
};