#include "Chunk.hh"
#include <Helpers/Collision/BoundingBox/AABB.hh>
void Chunk::add_wall(Wall const &wall) {
    // The stationary tree's leaf payload is the wall's index in `walls`, packed
    // into EntityID's id field. Built once at map init; never moved after.
    uint16_t index = (uint16_t)walls.size();
    walls.push_back(wall);
    stationary_tree.create_proxy_wall(AABB::from_wall(wall), EntityID(index, 0));
}
