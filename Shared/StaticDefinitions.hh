#pragma once

#include <Helpers/Array.hh>
#include <Helpers/Math.hh>

#include <cstdint>
#include <vector>

inline uint32_t const ARENA_WIDTH = 61952;
inline uint32_t const ARENA_HEIGHT = 61952;

inline uint32_t const MAX_SLOT_COUNT = 8;
inline uint32_t const MAX_POLY_VERTICES = 8;
inline uint32_t const LEVELS_PER_EXTRA_SLOT = 15;
inline uint32_t const LEADERBOARD_SIZE = 10;
inline uint32_t const MAX_PETALS_IN_CLUMP = 4;
inline uint32_t const MAX_DIFFICULTY = 3;
inline uint32_t const MAX_DROPS_PER_MOB = 6;

namespace DamageType {
    enum : uint8_t {
        kContact,
        kPoison,
        kReflect
    };
}
namespace CollisionShape{
        enum : uint8_t{
            kCapsule,
            kCircle,
            kEllipse,
            kPie,
            kPolygon,
            kRectangle,
            kSegment
        };
};
namespace PetalID {
    typedef uint8_t T;
    enum : T {
        kNone,
        kBasic,
        kLight,
        kHeavy,
        kStinger,
        kLeaf,
        kTwin,
        kRose,
        kIris,
        kMissile,
        kDandelion,
        kBubble,
        kFaster,
        kRock,
        kCactus,
        kWeb,
        kWing,
        kPeas,
        kSand,
        kPincer,
        kDahlia,
        kTriplet,
        kAntEgg,
        kBlueIris,
        kPollen,
        kPoisonPeas,
        kBeetleEgg,
        kAzalea,
        kStick,
        kTringer,
        kTriweb,
        kAntennae,
        kTricac,
        kHeaviest,
        kThirdEye,
        kObserver,
        kPoisonCactus,
        kSalt,
        kUniqueBasic,
        kSquare,
        kEllipse,
        kMoon,
        kLotus,
        kCutter,
        kYinYang,
        kYggdrasil,
        kRice,
        kBone,
        kYucca,
        kCorn,
        kGoldenLeaf,
        kRiceStinger,
        kNumPetals
    };
};
namespace Maps {
    enum : uint8_t {
        kGarden,
        kNumGamemodes
    };
}
namespace MapID
{
    typedef uint8_t T;
    enum : T
    {
        kanthole,
        kdesert_c_3,
        kfire_anthole,
        khel_c_3,
        korganic_wall_tri_0,
        kseaweed_tl_0,
        kwater_tri_0,
        kbridge_c_0,
        kdesert_c_4,
        kfoliage_bri_0,
        khel_c_4,
        kpvp_c_0,
        ksewage_c_0,
        kwood_c_0,
        kbridge_c_1,
        kdesert_l_0,
        kfoliage_t_0,
        khut_b_0,
        kpvp_c_1,
        ksewage_l_0,
        kworm_c_0,
        kbridge_r_0,
        kdesert_r_0,
        kfoliage_tl_0,
        khut_bl_0,
        kpvp_c_2,
        ksewage_tl_0,
        kbush_c_0,
        kdesert_t_0,
        kgrass_c_0,
        khut_bli_0,
        kpvp_c_3,
        ksewage_tri_0,
        kbush_t_0,
        kdirt_c_0,
        kgrass_c_1,
        khut_br_0,
        kpyramid,
        ksewer_entrance,
        kbush_tl_0,
        kdirt_l_0,
        kgrass_c_2,
        khut_bri_0,
        kpyramid_floor_c_0,
        ksewer_entrance2,
        kbush_tri_0,
        kdirt_tl_0,
        kgrass_c_3,
        khut_c_0,
        kpyramid_floor_c_1,
        ktermite_mound,
        kcastle_c_0,
        kdirt_tri_0,
        kgrass2_b_0,
        khut_l_0,
        kpyramid_floor_c_2,
        ktumbleweed_bri_0,
        kcastle_tl_0,
        kdirt2_c_0,
        kgrass2_c_0,
        khut_r_0,
        kpyramid_floor_c_3,
        ktumbleweed_t_0,
        kcastle_tri_0,
        kdirt2_c_1,
        kgrass2_c_1,
        khut_t_0,
        kpyramid_wall_bri_0,
        ktumbleweed_tl_0,
        kcellar_door,
        kfactory_entrance,
        kgrass2_c_2,
        khut_tl_0,
        kpyramid_wall_l_0,
        kvent_bri_0,
        kcobblestone_c_0,
        kfactory_exit,
        kgrass2_c_3,
        khut_tr_0,
        kpyramid_wall_tl_0,
        kvent_l_0,
        kcoral_c_0,
        kfactory_floor_c_0,
        kgrass2_t_0,
        khut_tri_0,
        kroot_bri_0,
        kvent_tl_0,
        kcoral_tl_0,
        kfactory_floor_c_1,
        kgrate_c_0,
        kocean_c_0,
        kroot_t_0,
        kvine_bri_0,
        kcoral_tri_0,
        kfactory_floor_c_2,
        kgrate_tl_0,
        kocean_c_1,
        kroot_tl_0,
        kvine_t_0,
        kdesert_b_0,
        kfactory_floor_c_3,
        kgrate_tri_0,
        kocean_c_2,
        kscliff_c_0,
        kvine_tl_0,
        kdesert_c_0,
        kfactory_wall_c_0,
        khel_c_0,
        kocean_c_3,
        kscliff_l_0,
        kwater_c_0,
        kdesert_c_1,
        kfactory_wall_l_0,
        khel_c_1,
        korganic_wall_c_0,
        kscliff_tl_0,
        kwater_l_0,
        kdesert_c_2,
        kfactory_wall_tl_0,
        khel_c_2,
        korganic_wall_l_0,
        kseaweed_bri_0,
        kwater_tl_0

    };
}
namespace MobID {
    typedef uint8_t T;
    enum : T {
        kBabyAnt,
        kWorkerAnt,
        kSoldierAnt,
        kBee,
        kLadybug,
        kBeetle,
        kMassiveLadybug,
        kMassiveBeetle,
        kDarkLadybug,
        kHornet,
        kCactus,
        kRock,
        kBoulder,
        kCentipede,
        kEvilCentipede,
        kDesertCentipede,
        kSandstorm,
        kScorpion,
        kSpider,
        kAntHole,
        kQueenAnt,
        kShinyLadybug,
        kSquare,
        //kCapsule,
        kEllipse,
        kSegment,
        kDigger,
        kNumMobs
    };
};

namespace RarityID {
    typedef uint8_t T;
    enum : T {
        kCommon,
        kUnusual,
        kRare,
        kEpic,
        kLegendary,
        kMythic,
        kUltra,
        kSuper,
        kNumRarities
    };
};

namespace ColorID {
    enum {
        kYellow,
        kGray,
        kBlue,
        kRed,
        kNumColors
    };
};

namespace AIState {
    enum {
        kIdle,
        kIdleMoving,
        kReturning,
        kBasicAggro
    };
};

namespace EntityFlags {
    enum {
        kIsDespawning,
        kNoFriendlyCollision,
        kDieOnParentDeath,
        kSpawnedFromZone,
        kNoDrops,
        kHasCulling,
        kIsCulled,
        kCPUControlled,
        kIsDetached
    };
};

namespace FaceFlags {
    enum {
        kAttacking,
        kDefending,
        kPoisoned,
        kDandelioned,
        kDeadEyes,
        kSquareEyes
    };
};

namespace EquipmentFlags {
    enum {
        kThirdEye,
        kAntennae,
        kObserver,
        kCutter,
        kNone
    };
};

namespace InputFlags {
    enum {
        kAttacking,
        kDefending
    };
}

struct PoisonDamage {
    float damage;
    float time;
};

struct PetalAttributes {
    enum {
        kPassiveRot,
        kNoRot,
        kFollowRot
    };
    float clump_radius = 0;
    float secondary_reload = 0;
    float constant_heal = 0;
    float burst_heal = 0;
    float mass = 0.1;
    float armor = 0;
    float poison_armor = 0;
    float dandelion_inflict_seconds = 0;
    float slow_inflict_seconds = 0;
    float vision_factor = 1;
    float extra_body_damage = 0;
    float extra_rotation_speed = 0;
    float extra_range = 0;
    float extra_health = 0;
    float damage_reflection = 0;
    float extra_damage_factor = 1;
    float extra_reload_factor = 1;
    struct PoisonDamage poison_damage;
    uint8_t defend_only = 0;
    float icon_angle = 0;
    uint8_t split_projectile = 0;
    uint8_t rotation_style = kPassiveRot;
    uint8_t spawns = MobID::kNumMobs;
    uint8_t spawn_count = 0;
    uint8_t equipment = EquipmentFlags::kNone;
};



struct PetalData {
    char const *name;
    char const *description;
    float health;
    float damage;
    float scale;
    float radius;
    float reload;
    int count;
    PetalAttributes attributes;
};
struct MobAttributes {
    float aggro_radius = 500;
    uint8_t segments = 1;
    uint8_t stationary;
    struct PoisonDamage poison_damage;
};

struct MobData {
    char const *name;
    char const *description;
    uint8_t rarity;
    float health;
    float damage;
    float scale;
    float radius;
    float width;
    float height;
    float length;
    StaticArray<std::array<float,2>,8> vertics;
    uint8_t shape;

    uint32_t xp;
    MobAttributes attributes;
};

struct SpawnChance {
    MobID::T id;
    float chance;
};

struct ZoneDefinition {
    float left;
    float top;
    float right;
    float bottom;
    float density;
    float drop_multiplier;
    StaticArray<struct SpawnChance, MobID::kNumMobs> spawns;
    uint32_t difficulty;
    uint32_t color;
    char const *name;
};
namespace Map {
    enum : uint8_t {
        kGarden,
        kDesert,
        kAntRoom,
        kNumGamemodes
    };
}
