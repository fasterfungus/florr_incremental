#pragma once

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <Server/BVH.hh>
#include <Server/Wall.hh>

#include <cstdint>
#include <functional>
#include <vector>

class Simulation;
class Entity;

// Broad-phase spatial index over active physics entities. Backed by a dynamic
// AABB BVH (see BVH.hh). The public interface is unchanged from the previous
// grid-based implementation, so callers (Simulation, Detection, Culling, Game)
// need no modification.
//
// In addition to the per-tick `dynamic_tree`, the manager owns a
// `stationary_tree` of immovable Wall obstacles. It is built once at map init
// (via add_wall) and never mutated again, so it pays no per-tick rebuild cost.
// collide_stationary() cross-collides the two trees so moving entities react to
// walls.
class BVHCollisionManager {
    Simulation *simulation;
    BVH dynamic_tree;
    BVH stationary_tree;
    std::vector<Wall> walls;      // stationary_tree leaf payload = index into this
    uint32_t width;
    uint32_t height;
public:
    BVHCollisionManager(Simulation *);
    void refresh(uint32_t, uint32_t);
    void reset();
    void sync(Entity &);
    void remove(Entity &);
    void collide(std::function<void(Simulation *, Entity &, Entity &)>);
    void query(float, float, float, float, std::function<void(Simulation *, Entity &)>);

    // Map init: register a static wall obstacle into the stationary tree.
    void add_wall(Wall const &);
    // Cross-collide every dynamic entity against every overlapping wall.
    void collide_stationary(std::function<void(Simulation *, Entity &, Wall const &)>);
};

