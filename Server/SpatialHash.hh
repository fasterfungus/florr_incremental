#pragma once

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>

#include <Server/BVH.hh>

#include <cstdint>
#include <functional>

class Simulation;
class Entity;

// Broad-phase spatial index over active physics entities. Backed by a dynamic
// AABB BVH (see BVH.hh). The public interface is unchanged from the previous
// grid-based implementation, so callers (Simulation, Detection, Culling, Game)
// need no modification.
class SpatialHash {
    Simulation *simulation;
    BVH tree;
    uint32_t width;
    uint32_t height;
public:
    SpatialHash(Simulation *);
    void refresh(uint32_t, uint32_t);
    void reset();
    void sync(Entity &);
    void remove(Entity &);
    void collide(std::function<void(Simulation *, Entity &, Entity &)>);
    void query(float, float, float, float, std::function<void(Simulation *, Entity &)>);
};
