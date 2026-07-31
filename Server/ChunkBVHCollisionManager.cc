//
// Created by administrator on 2026/7/29.
//
#include <cmath>
#include "ChunkBVHCollisionManager.hh"

#include <filesystem>
#include <fstream>
#include <string>
#include "Shared/Simulation.hh"
constexpr float CHUNK_QUERY_PADDING = 2.0f;

void ChunkBVHCollisionManager::reset()
{
    for (uint8_t i = 0; i < 121; i++)
    {
        chunks[i].stationary_tree.clear();
        chunks[i].walls.clear();
    }
}

ChunkBVHCollisionManager::ChunkBVHCollisionManager(Simulation* sim)
{
    this->sim = sim;
}

std::vector<uint8_t> ChunkBVHCollisionManager::query_overlapping_chunks(AABB const& region) //返回index
{
    std::vector<uint8_t> indexs;
    int sx0 = static_cast<int>(std::floor((region.min_x - CHUNK_QUERY_PADDING) / 5632));
    int sy0 = static_cast<int>(std::floor((region.min_y - CHUNK_QUERY_PADDING) / 5632));

    int sx1 = static_cast<int>(std::floor((region.max_x + CHUNK_QUERY_PADDING) / 5632));
    int sy1 = static_cast<int>(std::floor((region.max_y + CHUNK_QUERY_PADDING) / 5632));

    sx0 = std::clamp(sx0, 0, 10);
    sy0 = std::clamp(sy0, 0, 10);

    sx1 = std::clamp(sx1, 0, 10);
    sy1 = std::clamp(sy1, 0, 10);
    for (uint8_t y = sy0; y <= sy1; y++)
    {
        for (uint8_t x = sx0; x <= sx1; x++)
        {
            indexs.push_back(y * 11 + x);
        }
    }
    return indexs;
}

Chunk& ChunkBVHCollisionManager::get_chunk(uint8_t x, uint8_t y)
{
    return chunks[y * 11 + x];
}

void ChunkBVHCollisionManager::add_wall(Wall const& wall, uint8_t x, uint8_t y)
{
    // The stationary tree's leaf payload is the wall's index in `walls`, packed
    // into EntityID's id field. Built once at map init; never moved after.
    Chunk& chunk = get_chunk(x, y);
    uint16_t index = (uint16_t)chunk.walls.size();
    chunk.walls.push_back(wall);
    chunk.stationary_tree.create_proxy_wall(AABB::from_wall(wall), EntityID(index, 0));
}

void ChunkBVHCollisionManager::collide_stationary(
    std::function<void(Simulation*, Entity&, Wall const&)> on_wall_collide)
{
    sim->for_each_entity([&](Simulation* sim, Entity& ent)
    {
        auto region = AABB::from_entity(ent);
        auto indexs = query_overlapping_chunks(region);
        for (uint8_t i = 0; i < indexs.size(); i++)
        {
            uint8_t index = indexs[i];
            chunks[index].stationary_tree.query(region, [&](EntityID wall_id)
            {
                on_wall_collide(sim, ent, chunks[index].walls[wall_id.id]);
            });
        }
    });
}

void ChunkBVHCollisionManager::query_walls_in_aabb(AABB const& region,
                                                   std::function<void(Wall const&)> cb)
{
    auto indexs = query_overlapping_chunks(region);
    for (uint8_t i = 0; i < indexs.size(); i++)
    {
        uint8_t index = indexs[i];
        chunks[index].stationary_tree.query(region, [&](EntityID wall_id)
        {
            cb(chunks[index].walls[wall_id.id]);
        });
    }
}

void ChunkBVHCollisionManager::add(float x1, float y1, float x2, float y2, uint8_t x, uint8_t y)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    float length = sqrtf(dx * dx + dy * dy);
    if (length < EPSILON) return;
    float rotation = atan2f(dy, dx);
    add_wall(Wall((x1 + x2) / 2, (y1 + y2) / 2, length, rotation), x, y);
}

void ChunkBVHCollisionManager::load_chunks()
{
    std::string path = "map/chunks";


    for (uint32_t x = 0; x < 11; x++)
    {
        for (uint32_t y = 0; y < 11; y++)
        {
            std::string filename =
                path +
                "/chunk_" +
                std::to_string(x) +
                "_" +
                std::to_string(y) +
                ".txt";


            if (!std::filesystem::exists(filename))
            {
                continue;
            }


            load_chunk_file(
                filename,
                static_cast<uint8_t>(x),
                static_cast<uint8_t>(y)
            );
        }
    }
}

void ChunkBVHCollisionManager::load_chunk_file(
    std::string const& filename,
    uint8_t x,
    uint8_t y)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        return;
    }


    std::string line;


    while (std::getline(file, line))
    {
        float x1;
        float y1;
        float x2;
        float y2;


        if (sscanf(
            line.c_str(),
            "(%f,%f,%f,%f)",
            &x1,
            &y1,
            &x2,
            &y2
        ) != 4)
        {
            continue;
        }


        add(
            x1,
            y1,
            x2,
            y2,
            x,
            y
        );
    }
}
