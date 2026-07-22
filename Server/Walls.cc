// Static wall layout for the map. Call init_walls() once after Simulation
// is constructed (from GameInstance::init) to register all walls into the
// BVHCollisionManager's stationary tree.

#include <Server/Walls.hh>
#include <Shared/Simulation.hh>
#include <Server/Wall.hh>
#include <Helpers/Math.hh>

#include <cmath>

// Convenience: add a wall by center, length, angle-in-degrees.
static void add_from_length(Simulation *sim, float x, float y, float length, float angle) {
    sim->bvh_collision_manager.add_wall(Wall(x, y, length, angle * (float)M_PI / 180.0f));
}
static void add(Simulation *sim, float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);
    if (length < EPSILON) return;
    float rotation = atan2f(dy, dx);
    sim->bvh_collision_manager.add_wall(Wall((x1+x2)/2, (y1+y2)/2, length, rotation));
}
void init_walls(Simulation *sim) {
    /*
    // -----------------------------------------------------------------------
    // Map: 40000 x 4000. Walls are line segments (center x,y, length, angle).
    // angle=0 → horizontal, angle=90° → vertical.
    // -----------------------------------------------------------------------

    // --- Central corridor obstacles (x ≈ 20000, spread vertically) ----------
    // Two vertical pillars flanking the mid-line, leaving a narrow gap.
    add(sim, 20000, 1000, 600, 90);   // upper pillar
    add(sim, 20000, 3000, 600, 90);   // lower pillar

    // --- Left-third choke point (x ≈ 13000) --------------------------------
    // An angled wall creates a deflection hazard.
    add(sim, 13000, 2000, 800, 45);   // 45° diagonal
    add(sim, 13000, 2000, 800, -45);  // mirror: X shape
    // --- Right-third choke point (x ≈ 27000) --------------------------------
    add(sim, 27000, 2000, 800, 45);
    add(sim, 27000, 2000, 800, -45);

    // --- Horizontal barriers near top and bottom edges ----------------------
    // Narrow gaps (300 units) on each side so players can squeeze through.
    //
    // Left section, top wall (gap on the right end).
    add(sim,  6000,  400, 2000,  0);  // y=400, x=5000..7000
    // Left section, bottom wall (gap on the left end).
    add(sim,  6000, 3600, 2000,  0);

    // Right section, top wall.
    add(sim, 34000,  400, 2000,  0);
    // Right section, bottom wall.
    add(sim, 34000, 3600, 2000,  0);

    // --- Mid-map horizontal dividers with gaps ------------------------------
    // Upper horizontal, gap in the middle (two segments).
    add(sim, 17500, 1200, 3000,  0);  // left half
    add(sim, 22500, 1200, 3000,  0);  // right half  — 1000-unit gap at x=20000
    // Lower horizontal, gap in the middle.
    add(sim, 17500, 2800, 3000,  0);
    add(sim, 22500, 2800, 3000,  0);
    */
}
