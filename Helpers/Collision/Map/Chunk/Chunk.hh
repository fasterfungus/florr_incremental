#pragma once


#include <Server/BVH.hh>
#include <Helpers/Collision/Map/Wall.hh>
#include <vector>

class Chunk {
public:
    BVH stationary_tree;
    std::vector<Wall> walls;
    void add_wall(Wall const &);
};

