#include <Helpers/Collision/CCD.hh>

#include <algorithm>
#include <cmath>

namespace CCD {

// Swept circle vs AABB via the slab method on the radius-expanded box.
// Expanding the box by the circle radius reduces the problem to a ray (the
// circle center path) vs the enlarged box. This is exact for axis-aligned
// boxes; the rounded Minkowski corners are treated as square, which only makes
// the test slightly conservative (may report contact a hair early at corners),
// which is the safe direction for anti-tunneling.
SweepHit swept_circle_aabb(Vector start, Vector dir, float radius, AABB box) {
    SweepHit result{false, 1.0f, Vector(0, 0)};

    AABB e(box.min_x - radius, box.min_y - radius,
           box.max_x + radius, box.max_y + radius);

    // Already inside the expanded box at t=0 → immediate contact.
    if (start.x >= e.min_x && start.x <= e.max_x &&
        start.y >= e.min_y && start.y <= e.max_y) {
        result.hit = true;
        result.t = 0.0f;
        // Push out along the axis of least penetration.
        float dl = start.x - e.min_x, dr = e.max_x - start.x;
        float db = start.y - e.min_y, dt = e.max_y - start.y;
        float m = std::min(std::min(dl, dr), std::min(db, dt));
        if (m == dl)      result.normal = Vector(-1, 0);
        else if (m == dr) result.normal = Vector(1, 0);
        else if (m == db) result.normal = Vector(0, -1);
        else              result.normal = Vector(0, 1);
        return result;
    }

    float tmin = 0.0f, tmax = 1.0f;
    Vector hit_normal(0, 0);

    // X slab.
    if (std::fabs(dir.x) < 1e-8f) {
        if (start.x < e.min_x || start.x > e.max_x) return result;  // parallel & outside
    } else {
        float inv = 1.0f / dir.x;
        float t1 = (e.min_x - start.x) * inv;
        float t2 = (e.max_x - start.x) * inv;
        float nx = -1.0f;
        if (t1 > t2) { std::swap(t1, t2); nx = 1.0f; }
        if (t1 > tmin) { tmin = t1; hit_normal = Vector(nx, 0); }
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return result;
    }

    // Y slab.
    if (std::fabs(dir.y) < 1e-8f) {
        if (start.y < e.min_y || start.y > e.max_y) return result;
    } else {
        float inv = 1.0f / dir.y;
        float t1 = (e.min_y - start.y) * inv;
        float t2 = (e.max_y - start.y) * inv;
        float ny = -1.0f;
        if (t1 > t2) { std::swap(t1, t2); ny = 1.0f; }
        if (t1 > tmin) { tmin = t1; hit_normal = Vector(0, ny); }
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return result;
    }

    // tmin is the first contact fraction; valid only if within [0,1].
    if (tmin >= 0.0f && tmin <= 1.0f) {
        result.hit = true;
        result.t = tmin;
        result.normal = hit_normal;
    }
    return result;
}

}  // namespace CCD
