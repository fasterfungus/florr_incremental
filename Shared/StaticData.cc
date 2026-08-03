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
std::array<struct MobData, MobID::kNumMobs> const MOB_DATA = {
    {
        {
            .name = "Baby Ant",
            .description = "Weak and defenseless, but big dreams.",
            .rarity = RarityID::kCommon,
            .health = 10.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 14.0,
            .shape = CollisionShape::kCircle,
            .xp = 1,
            .drops = {
                PetalID::kLeaf, PetalID::kTwin, PetalID::kRice, PetalID::kTriplet, PetalID::kRiceStinger
            },
            .attributes = {}
        },
        {
            .name = "Worker Ant",
            .description = "It's temperamental, probably from working all the time.",
            .rarity = RarityID::kCommon,
            .health = 25.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 14.0,
            .shape = CollisionShape::kCircle,
            .xp = 3,
            .drops = {
                PetalID::kLight, PetalID::kLeaf, PetalID::kTwin, PetalID::kCorn, PetalID::kBone
            },
            .attributes = {}
        },
        {
            .name = "Soldier Ant",
            .description = "It's got wings and it's ready to use them.",
            .rarity = RarityID::kUnusual,
            .health = 40.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 14.0,
            .shape = CollisionShape::kCircle,
            .xp = 5,
            .drops = {
                PetalID::kTwin, PetalID::kIris, PetalID::kWing, PetalID::kFaster, PetalID::kTriplet
            },
            .attributes = {}
        },
        {
            .name = "Bee",
            .description = "It stings. Don't touch it.",
            .rarity = RarityID::kCommon,
            .health = 15.0,
            .damage = 50.0,
            .scale = 3.0,
            .radius = 20.0,
            .width = 63.75,
            .height = 45,
            .shape = CollisionShape::kEllipse,
            .xp = 4,
            .drops = {
                PetalID::kLight, PetalID::kStinger, PetalID::kTwin, PetalID::kWing
            },
            .attributes = {}
        },
        {
            .name = "Ladybug",
            .description = "Cute and harmless.",
            .rarity = RarityID::kCommon,
            .health = 25.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 32.5,
            .shape = CollisionShape::kCircle,
            .xp = 3,
            .drops = {
                PetalID::kLight, PetalID::kRose, PetalID::kTwin, PetalID::kBubble
            },
            .attributes = {}
        },
        {
            .name = "Beetle",
            .description = "It's hungry and flowers are its favorite meal.",
            .rarity = RarityID::kUnusual,
            .health = 40.0,
            .damage = 35.0,
            .scale = 3.0,
            .radius = 32.5,
            .length = 25.0,
            .shape = CollisionShape::kCapsule,
            .xp = 10,
            .drops = {
                PetalID::kIris, PetalID::kSalt, PetalID::kWing, PetalID::kTriplet
            },
            .attributes = {}
        },
        {
            .name = "Massive Ladybug",
            .description = "Much larger, but still cute.",
            .rarity = RarityID::kEpic,
            .health = 1000.0,
            .damage = 10.0,
            .scale = 7.0,
            .radius = 32.5,
            .shape = CollisionShape::kCircle,
            .xp = 400,
            .drops = {
                PetalID::kRose, PetalID::kDahlia, PetalID::kBubble, PetalID::kAzalea, PetalID::kObserver
            },
            .attributes = {}
        },
        {
            .name = "Massive Beetle",
            .description = "Someone overfed this one, you might be next.",
            .rarity = RarityID::kRare,
            .health = 600.0,
            .damage = 35.0,
            .scale = 3.0,
            .radius = 30.0,
            .length = 25.0,
            .shape = CollisionShape::kCapsule,
            .xp = 50,
            .drops = {
                PetalID::kIris, PetalID::kWing, PetalID::kBlueIris, PetalID::kTriplet, PetalID::kBeetleEgg,
                PetalID::kThirdEye
            },
            .attributes = {
                .aggro_radius = 750
            }
        },
        {
            .name = "Ladybug",
            .description = "Cute and harmless... if left unprovoked.",
            .rarity = RarityID::kUnusual,
            .health = 35.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 30.0,
            .shape = CollisionShape::kCircle,
            .xp = 5,
            .drops = {
                PetalID::kDahlia, PetalID::kWing, PetalID::kYinYang, PetalID::kAzalea
            },
            .attributes = {}
        },
        {
            .name = "Hornet",
            .description = "These aren't quite as nice as the little bees.",
            .rarity = RarityID::kUnusual,
            .health = 40.0,
            .damage = 40.0,
            .scale = 1.0,
            .radius = 20.0,
            .width = 60.0,
            .height = 40.0,
            .shape = CollisionShape::kEllipse,
            .xp = 12,
            .drops = {
                PetalID::kDandelion, PetalID::kMissile, PetalID::kWing, PetalID::kBubble, PetalID::kAntennae
            },
            .attributes = {
                .aggro_radius = 600
            }
        },
        {
            .name = "Cactus",
            .description = "This one's prickly, don't touch it either.",
            .rarity = RarityID::kCommon,
            .health = 25.0,
            .damage = 30.0,
            .scale = 2.0,
            .radius = 30.0,
            .shape = CollisionShape::kCircle,
            .xp = 2,
            .drops = {
                PetalID::kStinger, PetalID::kYucca, PetalID::kCactus, PetalID::kPoisonCactus, PetalID::kTricac
            },
            .attributes = {
                .stationary = 1
            }
        },
        {
            .name = "Rock",
            .description = "A rock. It doesn't do much.",
            .rarity = RarityID::kCommon,
            .health = 5.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 10.0,
            .shape = CollisionShape::kCircle,
            .xp = 1,
            .drops = {
                PetalID::kHeavy, PetalID::kLight, PetalID::kRock
            },
            .attributes = {
                .stationary = 1
            }
        },
        {
            .name = "Boulder",
            .description = "A bigger rock. It also doesn't do much.",
            .rarity = RarityID::kUnusual,
            .health = 40.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 50.0,
            .shape = CollisionShape::kCircle,
            .xp = 10,
            .drops = {
                PetalID::kHeavy, PetalID::kRock, PetalID::kHeaviest, PetalID::kMoon
            },
            .attributes = {
                .stationary = 1
            }
        },
        {
            .name = "Centipede",
            .description = "It's just there doing its thing.",
            .rarity = RarityID::kUnusual,
            .health = 50.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 35.0,
            .shape = CollisionShape::kCircle,
            .xp = 2,
            .drops = {
                PetalID::kLight, PetalID::kTwin, PetalID::kLeaf, PetalID::kPeas, PetalID::kTriplet, PetalID::kGoldenLeaf
            },
            .attributes = {
                .segments = 10
            }
        },
        {
            .name = "Evil Centipede",
            .description = "This one loves flowers.",
            .rarity = RarityID::kRare,
            .health = 50.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 35.0,
            .shape = CollisionShape::kCircle,
            .xp = 3,
            .drops = {
                PetalID::kIris, PetalID::kPoisonPeas, PetalID::kBlueIris
            },
            .attributes = {
                .segments = 10,
                .poison_damage = {
                    .damage = 5.0,
                    .time = 2.0
                }
            }
        },
        {
            .name = "Desert Centipede",
            .description = "It doesn't like it when you interrupt its run.",
            .rarity = RarityID::kRare,
            .health = 50.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 35.0,
            .shape = CollisionShape::kCircle,
            .xp = 4,
            .drops = {
                PetalID::kSand, PetalID::kFaster, PetalID::kSalt, PetalID::kStick
            },
            .attributes = {
                .segments = 6
            }
        },
        {
            .name = "Sandstorm",
            .description = "Quite unpredictable.",
            .rarity = RarityID::kUnusual,
            .health = 30.0,
            .damage = 40.0,
            .scale = 1.0,
            .radius = 32.0,
            .shape = CollisionShape::kCircle,
            .xp = 5,
            .drops = {
                PetalID::kSand, PetalID::kFaster, PetalID::kStick
            },
            .attributes = {}
        },
        {
            .name = "Scorpion",
            .description = "This one stings, now with poison.",
            .rarity = RarityID::kUnusual,
            .health = 35.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 35.0,
            .shape = CollisionShape::kCircle,
            .xp = 10,
            .drops = {
                PetalID::kIris, PetalID::kPincer, PetalID::kTriplet, PetalID::kLotus
            },
            .attributes = {
                .poison_damage = {
                    .damage = 10.0,
                    .time = 1.0
                }
            }
        },
        {
            .name = "Spider",
            .description = "Spooky.",
            .rarity = RarityID::kUnusual,
            .health = 35.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 15.0,
            .shape = CollisionShape::kCircle,
            .xp = 8,
            .drops = {
                PetalID::kStinger, PetalID::kWeb, PetalID::kFaster, PetalID::kTriweb
            },
            .attributes = {
                .poison_damage = {
                    .damage = 5.0,
                    .time = 3.0
                }
            }
        },
        {
            .name = "Ant Hole",
            .description = "Ants go in, and come out. Can't explain that.",
            .rarity = RarityID::kRare,
            .health = 500.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 45.0,
            .shape = CollisionShape::kCircle,
            .xp = 25,
            .drops = {
                PetalID::kIris, PetalID::kWing, PetalID::kAntEgg, PetalID::kTriplet
            },
            .attributes = {
                .stationary = 1
            }
        },
        {
            .name = "Queen Ant",
            .description = "You must have done something really bad if she's chasing you.",
            .rarity = RarityID::kRare,
            .health = 350.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 25.0,
            .vertics = {{-60,0},{-55,14.5},{-37.5,29},{37.5,23},{50,0},{37.5,-23},{-37.5,-29},{-55,-14.5}},
            .shape = CollisionShape::kPolygon,
            .xp = 15,
            .drops = {
                PetalID::kTwin, PetalID::kIris, PetalID::kWing, PetalID::kAntEgg, PetalID::kTringer
            },
            .attributes = {
                .aggro_radius = 750
            }
        },
        {
            .name = "Ladybug",
            .description = "This one is shiny... I wonder what it could mean...",
            .rarity = RarityID::kEpic,
            .health = 25.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 30.0,
            .shape = CollisionShape::kCircle,
            .xp = 30,
            .drops = {
                PetalID::kDahlia, PetalID::kWing, PetalID::kBubble, PetalID::kYggdrasil
            },
            .attributes = {}
        },
        {
            .name = "Square",
            .description = "???",
            .rarity = RarityID::kUnique,
            .health = 20.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 40.0,
            .width = 60,
            .height = 60,
            .shape = CollisionShape::kRectangle,
            .xp = 1,
            .drops = {
                PetalID::kSquare
            },
            .attributes = {
                .stationary = 1
            }
        },
        {
            .name = "Ellipse",
            .description = "???",
            .rarity = RarityID::kUnique,
            .health = 20.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 20.0,
            .width = 60.0,
            .height = 40.0,
            .shape = CollisionShape::kEllipse,
            .xp = 1,
            .drops = {
                PetalID::kEllipse
            },
            .attributes = {
                .stationary = 1
            }
        },
        {
            .name = "Segment",
            .description = "???",
            .rarity = RarityID::kUnique,
            .health = 20000.0,
            .damage = 1.0,
            .scale = 1.0,
            .radius = 40.0,
            .length = 800,
            .shape = CollisionShape::kSegment,
            .xp = 1,
            .drops = {
            },
            .attributes = {
                .stationary = 1
            },
        },
        {
            .name = "Digger",
            .description = "Friend or foe? You'll never know...",
            .rarity = RarityID::kEpic,
            .health = 250.0,
            .damage = 25.0,
            .scale = 1.0,
            .radius = 40.0,
            .shape = CollisionShape::kCircle,
            .xp = 0,
            .drops = {
                PetalID::kCutter
            },
            .attributes = {}
        },
    }
};

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
