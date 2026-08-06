#pragma once

#include <Shared/StaticDefinitions.hh>

#include <array>
#include <cstdint>

constexpr std::size_t NUM_PETALS = static_cast<std::size_t>(PetalID::kNumPetals);
constexpr std::size_t NUM_RARITIES = static_cast<std::size_t>(RarityID::kNumRarities);


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
extern std::array<struct MobData, MobID::kNumMobs> const MOB_DATA;
template <typename T>
struct RarityValue {
    std::array<T, NUM_RARITIES> values;

    // 模式 A：如果你填的是一个常量 (比如 .health = 10.0)，自动赋值给所有品质
    constexpr RarityValue(const T& val) : values{} {
        for (std::size_t i = 0; i < NUM_RARITIES; ++i) values[i] = val;
    }

    // 模式 B：如果你填的是匿名函数 (Lambda)，自动在此刻计算出所有品质的值！
    template <typename F, typename = std::enable_if_t<std::is_invocable_v<F, int>>>
    constexpr RarityValue(F f) : values{} {
        for (std::size_t i = 0; i < NUM_RARITIES; ++i) {
            values[i] = f(static_cast<int>(i));
        }
    }

    constexpr RarityValue() : values{} {}
};

constexpr float ScaleBy3(float base, int rarity) {
    float result = base;
    for (int i = 0; i < static_cast<int>(rarity); ++i) result *= 3.0;
    return result;
}

struct PetalConfig {
    char const *name;
    char const *description;
    RarityValue<float> health;
    RarityValue<float> damage;
    RarityValue<float> scale;
    RarityValue<float> radius;
    RarityValue<float> reload;
    RarityValue<int> count;
    RarityValue<PetalAttributes> attributes;
};

constexpr std::array<PetalConfig, NUM_PETALS> PETAL_CONFIGS = {{
    // [0] None
    {
        .name = "None",
        .description = "How can you see this?",
        .health = 1.0,       // 直接写死常量！模板会广播给所有品质
        .damage = 1.0,
        .radius = 1.0,
        .reload = 1.0,
        .count = 1,
        .attributes = PetalAttributes{}
    },
    // [1] Basic
    {
        .name = "Basic",
        .description = "A nice petal, not too strong but not too weak",
        .health = 10.0,      // 常量，相当于全品质都是 10.0
        .damage = [](int r) {
            return r == RarityID::kMythic ? 99.0 : ScaleBy3(10.0, r);
        },
        .radius = 10.0,
        .reload = 2.5,
        .count = 1,
        .attributes = [](int r) -> PetalAttributes {
            PetalAttributes attr{}; // 自动享受你的所有默认值 (比如 vision_factor = 1)

            if (r >= RarityID::kEpic) {
                attr.armor = 15.0f;
                attr.extra_rotation_speed = 2.0f;
            }
            if (r == RarityID::kMythic) {
            }
            return attr;
        }
    },
    {
            .name = "Fast",
            .description = "Weaker than most petals, but reloads very quickly",
            .health = 5.0,
            .damage = 8.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 1.0,
            .count = 1,
            .attributes = PetalAttributes{}
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
            .attributes = PetalAttributes{}
        },
        {
            .name = "Stinger",
            .description = "It really hurts, but it's really fragile",
            .health = 5.0,
            .damage = 35.0,
            .scale = 1.0,
            .radius = 7.0,
            .reload = 3.5,
            .count = 1,
            .attributes = PetalAttributes{}
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
            }
        },
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
            }
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
            }
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
            }
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
            }
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
            }
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
            }
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
            .attributes = PetalAttributes{}
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
            }
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
            }
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
            }
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
            }
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
            }
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
            }
        },
        {
            .name = "Rose",
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
            }
        },
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
            }
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
            }
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
            }
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
            }
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
            }
        },
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
            }
        },
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
            }
        },
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
        {
            .name = "Heaviest",
            .description = "This thing is so heavy that nothing gets in the way",
            .health = 200.0,
            .damage = 10.0,
            .scale = 1.0,
            .radius = 12.0,
            .reload = 15.0,
            .count = 1,
            .attributes = PetalAttributes{}
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
            }
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
            }
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
            }
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
            }
        },
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
            }
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
            }
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
            }
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
            }
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
            }
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
            .attributes = PetalAttributes{}
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
            }
        },
        {
            .name = "Rice",
            .description = "Spawns instantly, but not very strong",
            .health = 1.0,
            .damage = 4.0,
            .scale = 1.0,
            .radius = 13.0,
            .reload = 0.05,
            .count = 1,
            .attributes = PetalAttributes{
                .icon_angle = 0.7
            }
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
            }
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
            }
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
            }
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
            }
        },
        {
            .name = "Stinger",
            .description = "TOO OP",
            .health = 1.0,
            .damage = 35.0,
            .scale = 1.0,
            .radius = 10.0,
            .reload = 0.1,
            .count = 1,
            .attributes = PetalAttributes{}
        },
}};

// ==========================================
// 6. 最终游戏数据结构 & 二维数组转置烘焙
// ==========================================


constexpr std::array<std::array<PetalData, NUM_RARITIES>, NUM_PETALS> BakePetalData() {
    std::array<std::array<PetalData, NUM_RARITIES>, NUM_PETALS> result{};

    for (std::size_t p = 0; p < NUM_PETALS; ++p) {
        const auto& config = PETAL_CONFIGS[p];
        for (std::size_t r = 0; r < NUM_RARITIES; ++r) {
            // 直接从提前算好的 RarityValue 的 .values 数组中提取对应的数据
            result[p][r].name        = config.name;
            result[p][r].description = config.description;
            result[p][r].health      = config.health.values[r];
            result[p][r].damage      = config.damage.values[r];
            result[p][r].radius      = config.radius.values[r];
            result[p][r].reload      = config.reload.values[r];
            result[p][r].count       = config.count.values[r];
            result[p][r].attributes  = config.attributes.values[r];
        }
    }
    return result;
}
inline constexpr auto PETAL_DATA = BakePetalData();
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
            { MobID::kRock, 500000 },
            { MobID::kLadybug, 100000 },
            { MobID::kBee, 100000 },
            { MobID::kCentipede, 10000 },
            { MobID::kBoulder, 10000 },
            { MobID::kMassiveLadybug, 200 },
            { MobID::kSquare, 1 },
            {MobID::kEllipse,1},
            {MobID::kSegment,1},
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
            { MobID::kCactus, 400000 },
            { MobID::kBeetle, 100000 },
            { MobID::kSandstorm, 50000 },
            { MobID::kBee, 50000 },
            { MobID::kScorpion, 50000 },
            { MobID::kLadybug, 50000 },
            { MobID::kDesertCentipede, 10000 },
            { MobID::kAntHole, 2000 },
            { MobID::kShinyLadybug, 1000 },
            { MobID::kSquare, 1 }
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
            { MobID::kSpider, 100000 },
            { MobID::kBoulder, 100000 },
            { MobID::kBee, 100000 },
            { MobID::kHornet, 100000 },
            { MobID::kBeetle, 50000 },
            { MobID::kLadybug, 50000 },
            { MobID::kCentipede, 10000 },
            { MobID::kEvilCentipede, 10000 },
            { MobID::kMassiveBeetle, 2000 },
            { MobID::kAntHole, 2000 },
            { MobID::kSquare, 1 }
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
            { MobID::kDarkLadybug, 150000 },
            { MobID::kBeetle, 150000 },
            { MobID::kHornet, 150000 },
            { MobID::kSpider, 150000 },
            { MobID::kBoulder, 100000 },
            { MobID::kEvilCentipede, 10000 },
            { MobID::kMassiveBeetle, 2500 },
            { MobID::kAntHole, 2500 },
            { MobID::kSquare, 1 }
        },
        .difficulty = 3,
        .color = 0xff777777,
        .name = "???"
    }
});

std::array const ANTHOLE_SPAWNS = std::to_array<StaticArray<MobID::T, 3>>({
    {MobID::kBabyAnt},
    {MobID::kWorkerAnt,MobID::kBabyAnt},
    {MobID::kWorkerAnt,MobID::kWorkerAnt},
    {MobID::kSoldierAnt,MobID::kWorkerAnt},
    {MobID::kBabyAnt,MobID::kWorkerAnt,MobID::kSoldierAnt},
    {MobID::kWorkerAnt,MobID::kSoldierAnt},
    {MobID::kSoldierAnt,MobID::kWorkerAnt,MobID::kWorkerAnt},
    {MobID::kSoldierAnt,MobID::kSoldierAnt},
    {MobID::kQueenAnt},
    {MobID::kSoldierAnt,MobID::kSoldierAnt},
    {MobID::kSoldierAnt,MobID::kSoldierAnt,MobID::kSoldierAnt}
});

extern std::array<StaticArray<float, MAX_DROPS_PER_MOB>, MobID::kNumMobs> const MOB_DROP_CHANCES;

extern uint32_t score_to_pass_level(uint32_t);
extern uint32_t score_to_level(uint32_t);
extern uint32_t level_to_score(uint32_t);
extern uint32_t loadout_slots_at_level(uint32_t);

extern float hp_at_level(uint32_t);