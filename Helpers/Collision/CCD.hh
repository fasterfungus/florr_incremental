#pragma once

#include <Helpers/Vector.hh>
#include <Server/BVH.hh>  // for AABB

// Continuous collision detection helpers. Sweep a moving circle (radius r)
// along a segment and find the first fraction t in [0,1] where it touches a
// static AABB, so fast entities can be clamped to the contact point instead of
// tunneling through the obstacle in a single tick.
namespace CCD {

// Result of a swept-circle-vs-AABB test.
struct SweepHit {
    bool hit;         // did the swept circle touch the box within [0,1]?
    float t;          // fraction of the motion at first contact (1.0 if no hit)
    Vector normal;    // surface normal at the contact (unit, points away from box)
};

// Sweep a circle of radius `radius` from `start` along `dir` against a line
// segment from `seg_a` to `seg_b`. Uses exact analytic geometry (side-plane
// test + endpoint capsule tests) instead of the AABB approximation, so it
// never fires early for diagonal segments.
SweepHit swept_circle_segment(Vector start, Vector dir, float radius,
                               Vector seg_a, Vector seg_b);

// Sweep a circle of radius `radius` from `start` along `dir` (dir = end-start,
// NOT normalized) against `box`. Uses the analytic slab method on the box
// Minkowski-expanded by `radius`. Returns the first contact in [0,1].
SweepHit swept_circle_aabb(Vector start, Vector dir, float radius, AABB box);

}  // namespace CCD
