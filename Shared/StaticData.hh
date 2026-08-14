#pragma once

#include <Shared/StaticDefinitions.hh>
#include <Helpers/Math.hh>
#include <algorithm>
#include <array>
#include <cstdint>

constexpr std::size_t NUM_PETALS = static_cast<std::size_t>(PetalID::kNumPetals);
constexpr std::size_t NUM_RARITIES = static_cast<std::size_t>(RarityID::kNumRarities);
constexpr std::size_t NUM_MOBS = static_cast<std::size_t>(MobID::kNumMobs);
static constexpr int MAX_POINTS = 8;
struct PetalStats;
extern uint32_t const MAX_LEVEL;
extern uint32_t const TPS;

extern float const PETAL_DISABLE_DELAY;
extern float const PLAYER_ACCELERATION;
extern float const DEFAULT_FRICTION;
extern float const SUMMON_RETREAT_RADIUS;
extern float const DIGGER_SPAWN_CHANCE;

extern float const BASE_FLOWER_RADIUS;
extern float const BASE_PETAL_ROTATION_SPEED;
extern float const BASE_FOV;
extern float const BASE_HEALTH;
extern float const BASE_BODY_DAMAGE;
constexpr double RARITY_TABLE[10] = {
    0.0,
    0.8589559816476333,
    0.9963889387112547,
    0.9998247626378451,
    0.9999965538341747,
    0.9999999896581012,
    0.9999999999655730,
    0.9999999999999313,
};

constexpr double RELATIVE_RARITY[10] = {
    60000.0,
    15000.0,
    1500.0,
    100.0,
    5.0,
    0.1,
    0.000675,
    0.0000025,
};

template <typename T>
struct RarityValue
{
    std::array<T, NUM_RARITIES> values;

    constexpr RarityValue(const T& val) : values{}
    {
        for (std::size_t i = 0; i < NUM_RARITIES; ++i) values[i] = val;
    }

    template <typename F, typename = std::enable_if_t<std::is_invocable_v<F, int>>>
    constexpr RarityValue(F f) : values{}
    {
        for (std::size_t i = 0; i < NUM_RARITIES; ++i)
        {
            values[i] = f(static_cast<int>(i));
        }
    }

    constexpr RarityValue() : values{}
    {
    }
};

constexpr float ScaleMobHealth(float base, int rarity)
{
    constexpr float HEALTH_GROWTH[] = {
        1.0f, // 普通
        3.75f, // 罕见 / 普通
        3.6f, // 稀有 / 罕见
        4.0f, // 史诗 / 稀有
        6.0f, // 传奇 / 史诗
        9.75f, // 神话 / 传奇
        46.154f, // 究极 / 神话
        27.942f, // 超级 / 究极
    };

    float result = base;
    for (int i = 1; i <= rarity; ++i)
    {
        result *= HEALTH_GROWTH[i];
    }
    return result;
}

constexpr float ScaleBy3(float base, int rarity)
{
    float result = base;
    for (int i = 0; i < static_cast<int>(rarity); ++i) result *= 3.0;
    return result;
}

constexpr float ScaleBy14(float base, int rarity)
{
    float result = base;
    for (int i = 0; i < static_cast<int>(rarity); ++i) result *= 1.4;
    return result;
}

struct PetalConfig
{
    char const* name;
    char const* description;
    RarityValue<float> health;
    RarityValue<float> damage;
    RarityValue<float> scale;
    RarityValue<float> radius;
    RarityValue<float> reload;
    RarityValue<int> count;
    RarityValue<PetalAttributes> attributes;
    RarityID::T min_drop_rarity;
};

struct MobConfig
{
    char const* name;
    char const* description;
    RarityValue<float> health;
    RarityValue<float> damage;
    RarityValue<float> scale;
    float radius;
    float width;
    float height;
    float length;
    StaticArray<std::array<float, 2>, 8> vertics;
    RarityValue<uint8_t> shape;
    RarityValue<uint32_t> xp;
    RarityValue<StaticArray<std::pair<PetalID::T, double>, 10>> drops; //basechance+drop
    RarityValue<MobAttributes> attributes;
};

constexpr std::array<MobConfig, NUM_MOBS> MOB_CONFIGS = {
    {
        {
            .name = "Baby Ant",
            .description = "Weak and defenseless, but big dreams.",
            .health = [](int r)
            {
                return ScaleMobHealth(10.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 14.0,
            .shape = CollisionShape::kCircle,
            .xp = 1,
            .drops = {{{PetalID::kLight, 0.25}, {PetalID::kRice, 0.5}, {PetalID::kLeaf, 0.25}}},
            .attributes = MobAttributes{}
        },
        {
            .name = "Worker Ant",
            .description = "It's temperamental, probably from working all the time.",
            .health = [](int r)
            {
                return ScaleMobHealth(25.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 14.0,
            .width = 40.0,
            .height = 30.0,
            .shape = CollisionShape::kEllipse,
            .xp = 3,
            .drops = {{{PetalID::kLeaf, 0.14999999999999999}, {PetalID::kCorn, 0.14999999999999999}}},
            .attributes = MobAttributes{}
        },
        {
            .name = "Soldier Ant",
            .description = "It's got wings and it's ready to use them.",
            .health = [](int r)
            {
                return ScaleMobHealth(40.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 14.0,
            .width = 40.0,
            .height = 30.0,
            .shape = CollisionShape::kEllipse,
            .xp = 5,
            .drops = {{{PetalID::kSand, 0.29999999999999999}, {PetalID::kYucca, 0.25}}},
            .attributes = MobAttributes{}
        },
        {
            .name = "Bee",
            .description = "It stings. Don't touch it.",
            .health = [](int r)
            {
                return ScaleMobHealth(15.0, r);
            },
            .damage = 50.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 20.0,
            .width = 63.75,
            .height = 45,
            .shape = CollisionShape::kEllipse,
            .xp = 4,
            .drops = {{{PetalID::kStinger, 0.25}, {PetalID::kPollen, 0.10000000000000001}}},
            .attributes = MobAttributes{}
        },
        {
            .name = "Ladybug",
            .description = "Cute and harmless.",
            .health = [](int r)
            {
                return ScaleMobHealth(25.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 32.5,
            .shape = CollisionShape::kCircle,
            .xp = 3,
            .drops = {{{PetalID::kRose, 0.25}, {PetalID::kLight, 0.25}}},
            .attributes = MobAttributes{}
        },
        {
            .name = "Beetle",
            .description = "It's hungry and flowers are its favorite meal.",
            .health = [](int r)
            {
                return ScaleMobHealth(40.0, r);
            },
            .damage = 35.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 32.5,
            .length = 25.0,
            .shape = CollisionShape::kCapsule,
            .xp = 10,
            .drops = {{{PetalID::kBeetleEgg, 0.5}}},
            .attributes = MobAttributes{}
        },
        {
            .name = "Ladybug",
            .description = "Cute and harmless... if left unprovoked.",
            .health = [](int r)
            {
                return ScaleMobHealth(35.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 30.0,
            .shape = CollisionShape::kCircle,
            .xp = 5,
            .attributes = MobAttributes{}
        },
        {
            .name = "Hornet",
            .description = "These aren't quite as nice as the little bees.",
            .health = [](int r)
            {
                return ScaleMobHealth(40, r);
            },
            .damage = 40.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 30.0,
            .width = 60.0,
            .height = 40.0,
            .shape = CollisionShape::kEllipse,
            .xp = 12,
            .drops = {{{PetalID::kAntennae, 0.10000000000000001}, {PetalID::kMissile, 0.25}}},
            .attributes = MobAttributes{
                .aggro_radius = 600
            }
        },
        {
            .name = "Cactus",
            .description = "This one's prickly, don't touch it either.",
            .health = [](int r)
            {
                return ScaleMobHealth(25.0, r);
            },
            .damage = 30.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 30.0,
            .shape = CollisionShape::kCircle,
            .xp = 2,
            .attributes = MobAttributes{
                .stationary = 1
            }
        },
        {
            .name = "Rock",
            .description = "A rock. It doesn't do much.",
            .health = [](int r)
            {
                return ScaleMobHealth(5.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 10.0,
            .shape = CollisionShape::kCircle,
            .xp = 1,
            .drops = {
                {
                    {PetalID::kRock, 0.14999999999999999}, {PetalID::kHeaviest, 0.050000000000000003},
                    {PetalID::kMoon, 0.0050000000000000001}
                }
            },
            .attributes = MobAttributes{
                .stationary = 1
            }
        },
        /*
        {
            .name = "Boulder",
            .health = [](int r)
            {
                return ScaleMobHealth(40.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 50.0,
            .shape = CollisionShape::kCircle,
            .xp = 10,
            .attributes = MobAttributes{
                .stationary = 1
            }
        },
        */
        {
            .name = "Centipede",
            .description = "It's just there doing its thing.",
            .health = [](int r)
            {
                return ScaleMobHealth(50.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 35.0,
            .shape = CollisionShape::kCircle,
            .xp = 2,
            .drops = {{{PetalID::kPeas, 0.75}, {PetalID::kLeaf, 0.25}}},
            .attributes = MobAttributes{
                .segments = 10
            }
        },
        {
            .name = "Evil Centipede",
            .description = "This one loves flowers.",
            .health = [](int r)
            {
                return ScaleMobHealth(50.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 35.0,
            .shape = CollisionShape::kCircle,
            .xp = 3,
            .attributes = MobAttributes{
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
            .health = [](int r)
            {
                return ScaleMobHealth(50.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 35.0,
            .shape = CollisionShape::kCircle,
            .xp = 4,
            .attributes = MobAttributes{
                .segments = 6
            }
        },
        {
            .name = "Sandstorm",
            .description = "Quite unpredictable.",
            .health = [](int r)
            {
                return ScaleMobHealth(30.0, r);
            },
            .damage = 40.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 32.0,
            .shape = CollisionShape::kCircle,
            .xp = 5,
            .attributes = MobAttributes{}
        },
        {
            .name = "Scorpion",
            .description = "This one stings, now with poison.",
            .health = [](int r)
            {
                return ScaleMobHealth(35.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 35.0,
            .shape = CollisionShape::kCircle,
            .xp = 10,
            .attributes = MobAttributes{
                .poison_damage = {
                    .damage = 10.0,
                    .time = 1.0
                }
            }
        },
        {
            .name = "Spider",
            .description = "Spooky.",
            .health = [](int r)
            {
                return ScaleMobHealth(35.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 15.0,
            .shape = CollisionShape::kCircle,
            .xp = 8,
            .drops = {{{PetalID::kWeb, 0.25}, {PetalID::kFaster, 0.25}, {PetalID::kThirdEye, 0.01}}},
            .attributes = MobAttributes{
                .poison_damage = {
                    .damage = 5.0,
                    .time = 3.0
                }
            }
        },
        {
            .name = "Ant Hole",
            .description = "Ants go in, and come out. Can't explain that.",
            .health = [](int r)
            {
                return ScaleMobHealth(500.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 45.0,
            .shape = CollisionShape::kCircle,
            .xp = 25,
            .attributes = MobAttributes{
                .stationary = 1
            }
        },
        {
            .name = "Queen Ant",
            .description = "You must have done something really bad if she's chasing you.",
            .health = [](int r)
            {
                return ScaleMobHealth(350.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 25.0,
            .vertics = {
                {-58, 0}, {-55, 14.5}, {-37.5, 29}, {37.5, 20}, {48, 0}, {37.5, -20}, {-37.5, -29}, {-55, -14.5}
            },
            .shape = CollisionShape::kPolygon,
            .xp = 15,
            .drops = {{{PetalID::kAntEgg, 0.5}}},
            .attributes = MobAttributes{
                .aggro_radius = 750
            }
        },
        {
            .name = "Ladybug",
            .description = "This one is shiny... I wonder what it could mean...",
            .health = [](int r)
            {
                return ScaleMobHealth(25.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 30.0,
            .shape = CollisionShape::kCircle,
            .xp = 30,
            .attributes = MobAttributes{}
        },
        {
            .name = "Square",
            .description = "???",
            .health = [](int r)
            {
                return ScaleMobHealth(20.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 40.0,
            .width = 60,
            .height = 60,
            .shape = CollisionShape::kRectangle,
            .xp = 1,
            .attributes = MobAttributes{
                .stationary = 1
            }
        },
        {
            .name = "Ellipse",
            .description = "???",
            .health = [](int r)
            {
                return ScaleMobHealth(20.0, r);
            },
            .damage = 10.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 20.0,
            .width = 60.0,
            .height = 40.0,
            .shape = CollisionShape::kEllipse,
            .xp = 1,
            .attributes = MobAttributes{
                .stationary = 1
            }
        },
        {
            .name = "Segment",
            .description = "???",
            .health = [](int r)
            {
                return ScaleMobHealth(20.0, r);
            },
            .damage = 1.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 40.0,
            .length = 80,
            .shape = CollisionShape::kSegment,
            .xp = 1,
            .attributes = MobAttributes{
                .stationary = 1
            },
        },
        {
            .name = "Digger",
            .description = "Friend or foe? You'll never know...",
            .health = [](int r)
            {
                return ScaleMobHealth(250.0, r);
            },
            .damage = 25.0,
            .scale = [](int r)
            {
                return ScaleBy14(1, r);
            },
            .radius = 25.0,
            .shape = CollisionShape::kCircle,
            .xp = 0,
            .attributes = MobAttributes{}
        },
    }
};

constexpr std::array<std::array<MobData, NUM_RARITIES>, NUM_MOBS> BakeMobData()
{
    std::array<std::array<MobData, NUM_RARITIES>, NUM_MOBS> result{};

    for (std::size_t p = 0; p < NUM_MOBS; ++p)
    {
        const auto& config = MOB_CONFIGS[p];
        for (std::size_t r = 0; r < NUM_RARITIES; ++r)
        {
            // 直接从提前算好的 RarityValue 的 .values 数组中提取对应的数据
            result[p][r].name = config.name;
            result[p][r].description = config.description;
            result[p][r].health = config.health.values[r];
            result[p][r].damage = config.damage.values[r];
            result[p][r].scale = config.scale.values[r];
            result[p][r].radius = config.radius;
            result[p][r].width = config.width;
            result[p][r].height = config.height;
            result[p][r].length = config.length;
            result[p][r].vertics = config.vertics;
            result[p][r].shape = config.shape.values[r];
            result[p][r].xp = config.xp.values[r];
            result[p][r].drops = config.drops.values[r];
            result[p][r].attributes = config.attributes.values[r];
        }
    }
    return result;
}

inline constexpr auto MOB_DATA = BakeMobData();
constexpr std::array<PetalConfig, NUM_PETALS> PETAL_CONFIGS = {
    {
        // [0] None
        {
            .name = "None",
            .description = "How can you see this?",
            .health = 1.0,
            .damage = 1.0,
            .scale = 1.0,
            .radius = 1.0,
            .reload = 1.0,
            .count = 1,
            .attributes = PetalAttributes{},
            .min_drop_rarity = RarityID::kCommon

        },
        // [1] Basic
        {
            .name = "Basic",
            .description = "A nice petal, not too strong but not too weak",
            .health = 10.0,
            .damage = [](int r)
            {
                return ScaleBy3(10.0, r);
            },
            .scale = 1.0,
            .radius = 10.0,
            .reload = 2.5,
            .count = 1,
            .attributes = {},
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Light",
            .description = "Weaker than most petals, but reloads very quickly",
            .health = 5.0,
            .damage = 8.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 1.0,
            .count = [](int r) {
                if (r == RarityID::kCommon) return 1;
                if (r<RarityID::kLegendary) return 2;
                if (r == RarityID::kLegendary) return 3;
                if (r >= RarityID::kMythic) return 5;
                return 1;
            },
            .attributes = PetalAttributes{},
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Heavy",
            .description = "Very resilient and deals more damage, but reloads very slowly",
            .health = 20.0,
            .damage = 20.0,
            .scale = 1.0,
            .radius = 12.0,
            .reload = 4.5,
            .count = 1,
            .attributes = PetalAttributes{},
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Stinger",
            .description = "It really hurts, but it's really fragile",
            .health = 5.0,
            .damage = 35.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 3.5,
            .count = [](int r) {
                if (r == RarityID::kMythic) return 3;
                if (r > RarityID::kMythic) return 5;
                return 1;
            },
            .attributes = [](int r)
            {
                if (r >= RarityID::kMythic) return PetalAttributes{.clump_radius = 10};
                return PetalAttributes{.clump_radius = 0};
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Leaf",
            .description = "Gathers energy from the sun to passively heal your flower",
            .health = 10.0,
            .damage = 8.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 1.0,
            .count = 1,
            .attributes = PetalAttributes{
                .constant_heal = 1,
                .icon_angle = -1
            },
            .min_drop_rarity = RarityID::kCommon
        },
        /*
        {
            .name = "Twin",
            .description = "Why stop at one? Why not TWO?!",
            .health = 5.0,
            .damage = 8.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 1.0,
            .count = 2,
            .attributes = PetalAttributes{}
        },
        */
        {
            .name = "Rose",
            .description = "Its healing properties are amazing. Not so good at combat though",
            .health = 5.0,
            .damage = 5.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 3.5,
            .count = 1,
            .attributes = PetalAttributes{
                .secondary_reload = 1.0,
                .burst_heal = 10,
                .defend_only = 1
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Iris",
            .description = "Very poisonous, but takes a while to do its work",
            .health = 5.0,
            .damage = 5.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 5.0,
            .count = 1,
            .attributes = PetalAttributes{
                .poison_damage = {
                    .damage = 10.0,
                    .time = 6.0
                }
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Missile",
            .description = "You can actually shoot this one",
            .health = 10.0,
            .damage = 35.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 3.5,
            .count = 1,
            .attributes = PetalAttributes{
                .secondary_reload = 1.0,
                .defend_only = 1,
                .icon_angle = 1,
                .rotation_style = PetalAttributes::kFollowRot
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Dandelion",
            .description = "Its interesting properties prevent healing effects on affected units",
            .health = 10.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 1.0,
            .count = 1,
            .attributes = PetalAttributes{
                .icon_angle = 1,
                .rotation_style = PetalAttributes::kFollowRot
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Bubble",
            .description = "You can right click to pop it and propel your flower",
            .health = 1.0,
            .damage = 0.0,
            .scale = 1.0,
            .radius = 12.0,
            .reload = 0.01,
            .count = 1,
            .attributes = PetalAttributes{
                .secondary_reload = 0.5,
                .defend_only = 1,
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Faster",
            .description = "It's so light it makes your other petals spin faster",
            .health = 5.0,
            .damage = 7.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 0.5,
            .count = 1,
            .attributes = PetalAttributes{
                .extra_rotation_speed = 1.0
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Rock",
            .description = "Even more durable, but slower to recharge",
            .health = 100.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 12.0,
            .reload = 7.5,
            .count = 1,
            .attributes = PetalAttributes{},
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Cactus",
            .description = "Not very strong, but somehow increases your maximum health",
            .health = 15.0,
            .damage = 5.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 1.0,
            .count = 1,
            .attributes = PetalAttributes{
                .extra_health = 20
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Web",
            .description = "It's really sticky",
            .health = 10.0,
            .damage = 5.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 3.0,
            .count = 1,
            .attributes = PetalAttributes{
                .secondary_reload = 0.5,
                .defend_only = 1,
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Wing",
            .description = "It comes and goes",
            .health = 15.0,
            .damage = 15.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 2.5,
            .count = 1,
            .attributes = PetalAttributes{
                .icon_angle = 1,
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Peas",
            .description = "4 in 1 deal",
            .health = 5.0,
            .damage = 8.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 1.5,
            .count = 4,
            .attributes = PetalAttributes{
                .clump_radius = 8,
                .secondary_reload = 0.5,
                .defend_only = 1,
                .split_projectile = 1
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Sand",
            .description = "It's coarse, rough, and gets everywhere",
            .health = 10.0,
            .damage = 4.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 1.5,
            .count = 4,
            .attributes = PetalAttributes{
                .clump_radius = 10,
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Pincer",
            .description = "Stuns and poisons targets for a short duration",
            .health = 10.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 2.5,
            .count = 1,
            .attributes = PetalAttributes{
                .slow_inflict_seconds = 2,
                .poison_damage = {
                    .damage = 5.0,
                    .time = 1.0
                },
                .icon_angle = 0.7
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Dahlia",
            .description = "Its healing properties are amazing. Not so good at combat though",
            .health = 5.0,
            .damage = 5.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 3.5,
            .count = 3,
            .attributes = PetalAttributes{
                .clump_radius = 10,
                .secondary_reload = 1.0,
                .burst_heal = 3.5,
                .defend_only = 1
            },
            .min_drop_rarity = RarityID::kCommon
        },
        /*
        {
            .name = "Triplet",
            .description = "How about THREE?!",
            .health = 5.0,
            .damage = 8.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 1.0,
            .count = 3,
            .attributes = PetalAttributes{}
        },
        */
        {
            .name = "Egg",
            .description = "Something interesting might pop out of this",
            .health = 50.0,
            .damage = 1.0,
            .scale = 1.0,
            .radius = 12.5,
            .reload = 1.0,
            .count = 2,
            .attributes = PetalAttributes{
                .secondary_reload = 2.5,
                .defend_only = 1,
                .rotation_style = PetalAttributes::kNoRot,
                .spawns = MobID::kSoldierAnt
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Iris",
            .description = "Deals its effects quicker than traditional irises",
            .health = 10.0,
            .damage = 5.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 5.0,
            .count = 1,
            .attributes = PetalAttributes{
                .poison_damage = {
                    .damage = 15.0,
                    .time = 4.0
                }
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Pollen",
            .description = "Asthmatics beware",
            .health = 7.0,
            .damage = 8.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 1.5,
            .count = 3,
            .attributes = PetalAttributes{
                .secondary_reload = 0.5,
                .defend_only = 1
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Peas",
            .description = "4 in 1 deal, now with a secret ingredient: poison",
            .health = 5.0,
            .damage = 2.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 1.5,
            .count = 4,
            .attributes = PetalAttributes{
                .clump_radius = 8,
                .secondary_reload = 0.5,
                .poison_damage = {
                    .damage = 20.0,
                    .time = 0.5
                },
                .defend_only = 1,
                .split_projectile = 1,
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Egg",
            .description = "Something interesting might pop out of this",
            .health = 50.0,
            .damage = 1.0,
            .scale = 1.0,
            .radius = 15.0,
            .reload = 1.0,
            .count = 1,
            .attributes = PetalAttributes{
                .secondary_reload = 2.5,
                .defend_only = 1,
                .rotation_style = PetalAttributes::kNoRot,
                .spawns = MobID::kBeetle
            },
            .min_drop_rarity = RarityID::kCommon
        },
        /*
        {
            .name = "Rose",
            .description = "Extremely powerful rose, almost unheard of",
            .health = 5.0,
            .damage = 5.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 3.5,
            .count = 1,
            .attributes = PetalAttributes{
                .secondary_reload = 1.0,
                .burst_heal = 22,
                .defend_only = 1
            }
        },
        */
        {
            .name = "Stick",
            .description = "Harnesses the power of the wind",
            .health = 10.0,
            .damage = 1.0,
            .scale = 1.0,
            .radius = 15.0,
            .reload = 3.0,
            .count = 1,
            .attributes = PetalAttributes{
                .secondary_reload = 4.0,
                .defend_only = 1,
                .icon_angle = 1,
                .spawns = MobID::kSandstorm,
                .spawn_count = 2
            },
            .min_drop_rarity = RarityID::kEpic
        },
        /*
        {
            .name = "Stinger",
            .description = "It really hurts, but it's really fragile",
            .health = 5.0,
            .damage = 35.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 4.5,
            .count = 3,
            .attributes = PetalAttributes{
                .clump_radius = 10
            }
        },
        */
        /*
        {
            .name = "Web",
            .description = "It's really sticky",
            .health = 10.0,
            .damage = 5.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 3.0,
            .count = 3,
            .attributes = PetalAttributes{
                .clump_radius = 10,
                .secondary_reload = 0.5,
                .defend_only = 1,
            }
        },
        */
        {
            .name = "Antennae",
            .description = "Allows your flower to sense foes from farther away",
            .health = 0.0,
            .damage = 0.0,
            .scale = 1.0,
            .radius = 12.5,
            .reload = 0.0,
            .count = 0,
            .attributes = PetalAttributes{
                .vision_factor = 0.6,
                .equipment = EquipmentFlags::kAntennae
            },
            .min_drop_rarity = RarityID::kCommon
        },
        /*
        {
            .name = "Cactus",
            .description = "Not very strong, but somehow increases your maximum health",
            .health = 15.0,
            .damage = 5.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 1.0,
            .count = 3,
            .attributes = PetalAttributes{
                .clump_radius = 10,
                .extra_health = 40,
            }
        },
        */
        {
            .name = "Heaviest",
            .description = "This thing is so heavy that nothing gets in the way",
            .health = 200.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 12.0,
            .reload = 15.0,
            .count = 1,
            .attributes = PetalAttributes{},
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Third Eye",
            .description = "Allows your flower to extend petals further out",
            .health = 0.0,
            .damage = 0.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 0.0,
            .count = 0,
            .attributes = PetalAttributes{
                .extra_range = 75,
                .equipment = EquipmentFlags::kThirdEye
            },
            .min_drop_rarity = RarityID::kMythic
        },
        {
            .name = "Observer",
            .description = "The one who sees all",
            .health = 0.0,
            .damage = 0.0,
            .scale = 1.0,
            .radius = 12.5,
            .reload = 0.0,
            .count = 0,
            .attributes = PetalAttributes{
                .vision_factor = 0.25,
                .equipment = EquipmentFlags::kObserver
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Cactus",
            .description = "Turns your flower poisonous. Enemies will take poison damage on contact",
            .health = 15.0,
            .damage = 5.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 1.0,
            .count = 1,
            .attributes = PetalAttributes{
                .extra_health = 20,
                .poison_damage = {
                    .damage = 1.0,
                    .time = 5.0
                }
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Salt",
            .description = "Reflects some damage dealt to the flower. Does not stack with itself",
            .health = 10.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 2.5,
            .count = 1,
            .attributes = PetalAttributes{
                .damage_reflection = 0.25
            },
            .min_drop_rarity = RarityID::kCommon
        },
        /*
        {
            .name = "Basic",
            .description = "Something incredibly rare and useless",
            .health = 10.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 2.5,
            .count = 1,
            .attributes = PetalAttributes{}
        },
        */
        {
            .name = "Square",
            .description = "This shape... it looks familiar...",
            .health = 10.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 15.0,
            .reload = 2.5,
            .count = 1,
            .attributes = PetalAttributes{
                .icon_angle = M_PI / 4 + 1
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Ellipse",
            .description = "This shape... it looks familiar...",
            .health = 10.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 15.0,
            .reload = 2.5,
            .count = 1,
            .attributes = PetalAttributes{
                .icon_angle = M_PI / 4 + 1
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Moon",
            .description = "Where did this come from?",
            .health = 1000.0,
            .damage = 3.0,
            .scale = 1.0,
            .radius = 50.0,
            .reload = 10.0,
            .count = 1,
            .attributes = PetalAttributes{
                .mass = 200
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Lotus",
            .description = "Absorbs some poison damage taken by the flower",
            .health = 5.0,
            .damage = 5.0,
            .scale = 1.0,
            .radius = 12.0,
            .reload = 2.0,
            .count = 1,
            .attributes = PetalAttributes{
                .poison_armor = 5,
                .icon_angle = 0.1
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Cutter",
            .description = "Increases the flower's body damage",
            .health = 0.0,
            .damage = 0.0,
            .scale = 1.0,
            .radius = 40.0,
            .reload = 0.0,
            .count = 0,
            .attributes = PetalAttributes{
                .extra_body_damage = 15,
                .equipment = EquipmentFlags::kCutter
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Yin Yang",
            .description = "Alters the flower's petal rotation in interesting ways",
            .health = 15.0,
            .damage = 15.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 2.5,
            .count = 1,
            .attributes = PetalAttributes{},
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Yggdrasil",
            .description = "Unfortunately, its powers are useless here",
            .health = 1.0,
            .damage = 1.0,
            .scale = 1.0,
            .radius = 12.0,
            .reload = 10.0,
            .count = 1,
            .attributes = PetalAttributes{
                .defend_only = 1,
                .icon_angle = M_PI
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Rice",
            .description = "Spawns instantly, but not very strong",
            .health = 1.0,
            .damage = 4.0,
            .scale = 1.0,
            .radius = [](int r)
            {
                if (r >= RarityID::kMythic)
                {
                    return 26.0;
                }
                return 13.0;
            },

            .reload = 0.05,
            .count = 1,
            .attributes = PetalAttributes{
                .icon_angle = 0.7
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Bone",
            .description = "Sturdy",
            .health = 12.0,
            .damage = 15.0,
            .scale = 1.0,
            .radius = 12.0,
            .reload = 2.5,
            .count = 1,
            .attributes = PetalAttributes{
                .armor = 8,
                .icon_angle = 1
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Yucca",
            .description = "Heals the flower, but only while in the defensive position",
            .health = 10.0,
            .damage = 5.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 1.0,
            .count = 1,
            .attributes = PetalAttributes{
                .constant_heal = 1.5,
                .icon_angle = -1
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Corn",
            .description = "Takes a long time to spawn, but has a lot of health",
            .health = 500.0,
            .damage = 2.5,
            .scale = 1.0,
            .radius = 16.0,
            .reload = 10.0,
            .count = 1,
            .attributes = PetalAttributes{
                .icon_angle = 0.5
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Leaf",
            .description = "Allows your petals to recharge slower but do more damage",
            .health = 10.0,
            .damage = 8.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 1.0,
            .count = 1,
            .attributes = PetalAttributes{
                .extra_damage_factor = 1.2,
                .extra_reload_factor = 1.2,
                .icon_angle = -1
            },
            .min_drop_rarity = RarityID::kCommon
        },
        {
            .name = "Stinger",
            .description = "TOO OP",
            .health = 1.0,
            .damage = [](int r)
            {
                return ScaleBy3(70.0, r);
            },
            .scale = 1.0,
            .radius = 10.0,
            .reload = 0.1,
            .count = 1,
            .attributes = PetalAttributes{},
            .min_drop_rarity = RarityID::kMythic
        },
    }
};

constexpr std::array<std::array<PetalData, NUM_RARITIES>, NUM_PETALS> BakePetalData()
{
    std::array<std::array<PetalData, NUM_RARITIES>, NUM_PETALS> result{};

    for (std::size_t p = 0; p < NUM_PETALS; ++p)
    {
        const auto& config = PETAL_CONFIGS[p];
        for (std::size_t r = 0; r < NUM_RARITIES; ++r)
        {
            // 直接从提前算好的 RarityValue 的 .values 数组中提取对应的数据
            result[p][r].name = config.name;
            result[p][r].description = config.description;
            result[p][r].health = config.health.values[r];
            result[p][r].damage = config.damage.values[r];
            result[p][r].scale = config.scale.values[r];
            result[p][r].radius = config.radius.values[r];
            result[p][r].reload = config.reload.values[r];
            result[p][r].count = config.count.values[r];
            result[p][r].attributes = config.attributes.values[r];
            result[p][r].min_drop_rarity = config.min_drop_rarity;
        }
    }
    return result;
}

inline constexpr auto PETAL_DATA = BakePetalData();

constexpr int getDropCap(int mobRarity)
{
    if (mobRarity <= 0)
        return 1;

    if (mobRarity <= 6)
        return mobRarity;

    return 6;
}

constexpr double CalculateDropChance(
    double baseChance,
    int mobRarity,
    int dropRarity
)
{
    if (mobRarity < 0)
        mobRarity = 0;

    if (mobRarity >= NUM_RARITIES)
        mobRarity = NUM_RARITIES - 1;

    if (dropRarity < 0)
        dropRarity = 0;

    if (dropRarity >= NUM_RARITIES)
        dropRarity = NUM_RARITIES - 1;

    const int cap = getDropCap(mobRarity);

    if (dropRarity > cap)
        return 0.0;

    const double start = RARITY_TABLE[dropRarity];

    const double end =
        dropRarity == cap
            ? 1.0
            : RARITY_TABLE[dropRarity + 1];

    const double scale =
    (RELATIVE_RARITY[0] /
        RELATIVE_RARITY[mobRarity]) * 5.0;

    const double logStart =
        scale *
        ConstexprMath::log1p(
            -baseChance * (1.0 - start)
        );

    const double logEnd =
        scale *
        ConstexprMath::log1p(
            -baseChance * (1.0 - end)
        );

    const double delta = logEnd - logStart;

    return ConstexprMath::exp(logStart) *
        ConstexprMath::expm1(delta);
}

constexpr std::array<std::array<StaticArray<PetalDropChance, MAX_DROPS_PER_MOB>, NUM_RARITIES>, NUM_MOBS>
makeMobDropChances()
{
    std::array<std::array<StaticArray<PetalDropChance, MAX_DROPS_PER_MOB>, NUM_RARITIES>, NUM_MOBS> result{};

    for (MobID::T mob_id = 0;
         mob_id < NUM_MOBS;
         mob_id++)
    {
        for (RarityID::T mob_rarity = 0;
             mob_rarity < NUM_RARITIES;
             mob_rarity++)
        {
            for (uint32_t i = 0; i < MOB_DATA[mob_id][mob_rarity].drops.size(); i++)
            {
                PetalID::T drop_id = MOB_DATA[mob_id][mob_rarity].drops[i].first;
                RarityID::T min_drop_rarity = PETAL_DATA[MOB_DATA[mob_id][mob_rarity].drops[i].first][RarityID::kCommon].min_drop_rarity;
                PetalDropChance petal_drop_chance;
                petal_drop_chance.id = drop_id;
                for (RarityID::T drop_rarity = 0; drop_rarity <= getDropCap(mob_rarity); drop_rarity++)
                {
                    double chance = 0.0;
                    if (drop_rarity >= min_drop_rarity)
                    {
                        chance = CalculateDropChance(MOB_DATA[mob_id][mob_rarity].drops[i].second, mob_rarity,
                                    drop_rarity);
                    }
                    petal_drop_chance.chances.push(chance);
                }
                result[mob_id][mob_rarity].push(petal_drop_chance);
            }
        }
    }
    return result;
}

constexpr std::array<std::array<StaticArray<PetalDropChance, MAX_DROPS_PER_MOB>, NUM_RARITIES>, NUM_MOBS> MOB_DROP_CHANCES = makeMobDropChances();
//map extends from (0,0) to (ARENA_WIDTH,ARENA_HEIGHT)
inline std::array const MAP_DATA = std::to_array<struct ZoneDefinition>({
    {
        .left = 0,
        .top = 0,
        .right = 10000,
        .bottom = 4000,
        .density = 1,
        .drop_multiplier = 0.3,
        .spawns = {
            {MobID::kRock, 500000},
            {MobID::kLadybug, 100000},
            {MobID::kBee, 100000},
            {MobID::kCentipede, 10000},
            //{MobID::kBoulder, 10000},
            {MobID::kSquare, 1},
            {MobID::kEllipse, 1},
            {MobID::kSegment, 1},
        },
        .difficulty = 0,
        .color = 0xff1ea761,
        .name = "Easy"
    },
    {
        .left = 10000,
        .top = 0,
        .right = 20000,
        .bottom = 4000,
        .density = 1,
        .drop_multiplier = 0.15,
        .spawns = {
            {MobID::kCactus, 400000},
            {MobID::kBeetle, 100000},
            {MobID::kSandstorm, 50000},
            {MobID::kBee, 50000},
            {MobID::kScorpion, 50000},
            {MobID::kLadybug, 50000},
            {MobID::kDesertCentipede, 10000},
            {MobID::kAntHole, 2000},
            {MobID::kShinyLadybug, 1000},
            {MobID::kSquare, 1}
        },
        .difficulty = 1,
        .color = 0xffdecf7c,
        .name = "Medium"
    },
    {
        .left = 20000,
        .top = 0,
        .right = 30000,
        .bottom = 4000,
        .density = 1,
        .drop_multiplier = 0.1,
        .spawns = {
            {MobID::kSpider, 100000},
            //{MobID::kBoulder, 100000},
            {MobID::kBee, 100000},
            {MobID::kHornet, 100000},
            {MobID::kBeetle, 50000},
            {MobID::kLadybug, 50000},
            {MobID::kCentipede, 10000},
            {MobID::kEvilCentipede, 10000},
            //{ MobID::kMassiveBeetle, 2000 },
            {MobID::kAntHole, 2000},
            {MobID::kSquare, 1}
        },
        .difficulty = 2,
        .color = 0xffb06655,
        .name = "Hard"
    },
    {
        .left = 30000,
        .top = 0,
        .right = 40000,
        .bottom = 4000,
        .density = 1,
        .drop_multiplier = 0.025,
        .spawns = {
            {MobID::kDarkLadybug, 150000},
            {MobID::kBeetle, 150000},
            {MobID::kHornet, 150000},
            {MobID::kSpider, 150000},
            //{MobID::kBoulder, 100000},
            {MobID::kEvilCentipede, 10000},
            //{ MobID::kMassiveBeetle, 2500 },
            {MobID::kAntHole, 2500},
            {MobID::kSquare, 1}
        },
        .difficulty = 3,
        .color = 0xff777777,
        .name = "???"
    }
});

std::array const ANTHOLE_SPAWNS = std::to_array<StaticArray<MobID::T, 3>>({
    {MobID::kBabyAnt},
    {MobID::kWorkerAnt, MobID::kBabyAnt},
    {MobID::kWorkerAnt, MobID::kWorkerAnt},
    {MobID::kSoldierAnt, MobID::kWorkerAnt},
    {MobID::kBabyAnt, MobID::kWorkerAnt, MobID::kSoldierAnt},
    {MobID::kWorkerAnt, MobID::kSoldierAnt},
    {MobID::kSoldierAnt, MobID::kWorkerAnt, MobID::kWorkerAnt},
    {MobID::kSoldierAnt, MobID::kSoldierAnt},
    {MobID::kQueenAnt},
    {MobID::kSoldierAnt, MobID::kSoldierAnt},
    {MobID::kSoldierAnt, MobID::kSoldierAnt, MobID::kSoldierAnt}
});


extern uint32_t score_to_pass_level(uint32_t);
extern uint32_t score_to_level(uint32_t);
extern uint32_t level_to_score(uint32_t);
extern uint32_t loadout_slots_at_level(uint32_t);

extern float hp_at_level(uint32_t);
