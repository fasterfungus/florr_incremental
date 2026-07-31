#pragma once
#include <Shared/Entity.hh>
#include <Helpers/Collision/Map/Chunk/Chunk.hh>
#include <Server/BVH.hh>
#include <Helpers/Collision/Map/Wall.hh>
#include <cstdint>
#include <functional>
#include <vector>


class ChunkBVHCollisionManager {
    Simulation *sim;
    Chunk chunks[121];
public:

    ChunkBVHCollisionManager(Simulation *);
    Chunk& get_chunk(uint8_t x,uint8_t y);
    void reset();
    void add_wall(Wall const &,uint8_t,uint8_t);
    std::vector<uint8_t> query_overlapping_chunks(AABB const &region);
    void collide_stationary(std::function<void(Simulation *, Entity &, Wall const &)>);
    void query_walls_in_aabb(AABB const &region, std::function<void(Wall const &)> cb);
    void load_chunks();
    void add(float x1, float y1, float x2, float y2 , uint8_t x , uint8_t y);
    void load_chunk_file(
        std::string const& filename,
        uint8_t x,
        uint8_t y);

};


