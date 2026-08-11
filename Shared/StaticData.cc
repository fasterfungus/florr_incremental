#include <Shared/StaticData.hh>
#include <cmath>
#include <array>
#include <string_view>
#include <cstdint>
#include <type_traits>
uint32_t const MAX_LEVEL = 99;
uint32_t const TPS = 20;

//these two are based on the base game's 20 TPS
float const PLAYER_ACCELERATION = 5.0f;
float const DEFAULT_FRICTION = 1.0f / 3.0f;

float const PETAL_DISABLE_DELAY = 45.0f; //seconds
float const SUMMON_RETREAT_RADIUS = 600.0f;
float const DIGGER_SPAWN_CHANCE = 0.25f;

float const BASE_FLOWER_RADIUS = 25.0f;
float const BASE_PETAL_ROTATION_SPEED = 2.5f;
float const BASE_FOV = 0.9f;
float const BASE_HEALTH = 100.0f;
float const BASE_BODY_DAMAGE = 25.0f;
/*
std::array<StaticArray<float, MAX_DROPS_PER_MOB>, MobID::kNumMobs> const MOB_DROP_CHANCES = []()
{
    std::array<StaticArray<float, MAX_DROPS_PER_MOB>, MobID::kNumMobs> ret;
    double const RARITY_MULT[RarityID::kNumRarities] = {50000, 15000, 2500, 100, 10, 2.5, 1};
    double MOB_SPAWN_RATES[MobID::kNumMobs] = {0};
    double PETAL_AGGREGATE_DROPS[PetalID::kNumPetals] = {0};
    for (struct ZoneDefinition const& zone : MAP_DATA)
    {
        double total = 0;
        for (SpawnChance const& s : zone.spawns) total += s.chance;
        for (SpawnChance const& s : zone.spawns)
        {
            double base_chance = (s.chance * zone.drop_multiplier / total);
            MOB_SPAWN_RATES[s.id] += base_chance;
            if (s.id == MobID::kAntHole)
            {
                MOB_SPAWN_RATES[MobID::kDigger] += DIGGER_SPAWN_CHANCE * base_chance;
                for (auto const& spawn_wave : ANTHOLE_SPAWNS)
                    for (MobID::T spawn : spawn_wave)
                        MOB_SPAWN_RATES[spawn] += base_chance;
            }
        }
    }

    for (MobID::T id = 0; id < MobID::kNumMobs; ++id)
        for (PetalID::T const drop_id : MOB_DATA[id].drops) PETAL_AGGREGATE_DROPS[drop_id]++;

    double const BASE_NUM = MOB_SPAWN_RATES[MobID::kSquare];
    if (BASE_NUM <= 0)
        assert(!"Square mob must spawn in at least one zone");

    for (MobID::T id = 0; id < MobID::kNumMobs; ++id)
    {
        for (PetalID::T const drop_id : MOB_DATA[id].drops)
        {
            float chance = fclamp(

                (BASE_NUM * RARITY_MULT[RarityID::kCommon]) / (PETAL_AGGREGATE_DROPS[drop_id] * MOB_SPAWN_RATES //(BASE_NUM * RARITY_MULT[PETAL_DATA[drop_id].rarity]) / (PETAL_AGGREGATE_DROPS[drop_id] * MOB_SPAWN_RATES
                    [id] * MOB_DATA[id].attributes.segments), 0, 1);
            ret[id].push(chance);
        }
    }
    return ret;
}();
*/
uint32_t score_to_pass_level(uint32_t level)
{
    return (uint32_t)(pow(1.06, level - 1) * level) + 3;
}

uint32_t score_to_level(uint32_t score)
{
    uint32_t level = 1;
    while (level < MAX_LEVEL)
    {
        uint32_t level_score = score_to_pass_level(level);
        if (score < level_score) break;
        score -= level_score;
        ++level;
    }
    return level;
}

uint32_t level_to_score(uint32_t level)
{
    uint32_t score = 0;
    for (uint32_t i = 1; i < level; ++i)
        score += score_to_pass_level(i);
    return score;
}

uint32_t loadout_slots_at_level(uint32_t level)
{
    if (level > MAX_LEVEL) level = MAX_LEVEL;
    uint32_t ret = 5 + level / LEVELS_PER_EXTRA_SLOT;
    if (ret > MAX_SLOT_COUNT) return MAX_SLOT_COUNT;
    return ret;
}

float hp_at_level(uint32_t level)
{
    if (level > MAX_LEVEL) level = MAX_LEVEL;
    return BASE_HEALTH + level;
}
