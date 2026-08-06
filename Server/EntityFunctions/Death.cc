#include <Server/EntityFunctions.hh>

#include <Server/PetalTracker.hh>
#include <Server/Spawn.hh>

#include <Shared/Entity.hh>
#include <Shared/Map.hh>
#include <Shared/Simulation.hh>

#include <algorithm>
#include <iostream>

static void _alloc_drops(Simulation *sim, std::vector<PetalID::T> &success_drops_id,std::vector<RarityID::T> &success_drops_rarity, float x, float y) {
    #ifdef DEBUG
    for (PetalID::T id : success_drops)
        assert(id != PetalID::kNone && id < PetalID::kNumPetals);
    #endif
    size_t count = success_drops_id.size();
    for (size_t i = count; i > 0; --i) {
        PetalID::T drop_id = success_drops_id[i - 1];
        RarityID::T drop_rarity = success_drops_rarity[i - 1];
        /*
        if (PETAL_DATA[drop_id].rarity == RarityID::kUnique && PetalTracker::get_count(sim, drop_id) > 0) {
            success_drops[i - 1] = success_drops[count - 1];
            --count;
            success_drops.pop_back();
        }
        */
    }
    DEBUG_ONLY(assert(success_drops.size() == count);)
    if (count > 1) {
        for (size_t i = 0; i < count; ++i) {
            Entity &drop = alloc_drop(sim, success_drops_id[i],success_drops_rarity[i]);
            drop.set_x(x);
            drop.set_y(y);
            drop.velocity.unit_normal(i * 2 * M_PI / count).set_magnitude(25);
        }
    } else if (count == 1) {
        Entity &drop = alloc_drop(sim, success_drops_id[0],success_drops_rarity[0]);
        drop.set_x(x);
        drop.set_y(y);
    }
}

static void _add_score(Simulation *sim, EntityID const killer_id, Entity const &target) {
    if (!sim->ent_exists(killer_id)) return;
    Entity &killer = sim->get_ent(killer_id);
    if (killer.has_component(kScore))
        killer.set_score(killer.get_score() + target.score_reward);
}

void entity_on_death(Simulation *sim, Entity const &ent) {
    //don't do on_death for any despawned entity
    uint8_t natural_despawn = BitMath::at(ent.flags, EntityFlags::kIsDespawning) && ent.despawn_tick == 0;
    if (ent.score_reward > 0 && sim->ent_exists(ent.last_damaged_by) && !natural_despawn) {
        EntityID killer_id = sim->get_ent(ent.last_damaged_by).base_entity;
        _add_score(sim, killer_id, ent);
    }
    if (ent.has_component(kFlower) && sim->ent_alive(ent.get_parent())) {
        Entity &camera = sim->get_ent(ent.get_parent());
        EntityID killer_id = sim->ent_exists(ent.last_damaged_by) ?
            sim->get_ent(ent.last_damaged_by).base_entity : NULL_ENTITY;
        if (sim->ent_alive(killer_id)) {
            Entity const &killer = sim->get_ent(killer_id);
            if (killer.has_component(kName)) camera.set_killed_by(killer.get_name());
            else camera.set_killed_by("");
        } else if (ent.poison_ticks > 0) camera.set_killed_by("Poison");
        else camera.set_killed_by("");
    }
    if (ent.has_component(kMob)) {
        std::cout<<(int)ent.get_mob_id();
        if (BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone))
            Map::remove_mob(sim, ent.zone);
        if (!natural_despawn && !(BitMath::at(ent.flags, EntityFlags::kNoDrops))) {
            struct MobData const &mob_data = MOB_DATA[ent.get_mob_id()];
            std::vector<PetalID::T> success_drops_id = {};
            std::vector<RarityID::T> success_drops_rarity = {};
            StaticArray<float, MAX_DROPS_PER_MOB> const &drop_chances = MOB_DROP_CHANCES[ent.get_mob_id()];
            RarityID::T success_drop_rarity = RarityID::kCommon; //TODO 修改这里的内容
            for (uint32_t i = 0; i < mob_data.drops.size(); ++i)
                if (frand() < drop_chances[i])
                {
                    success_drops_id.push_back(mob_data.drops[i]);
                    success_drops_rarity.push_back(success_drop_rarity);
                }
            _alloc_drops(sim, success_drops_id,success_drops_rarity, ent.get_x(), ent.get_y());
        }
        if (ent.get_mob_id() == MobID::kAntHole &&
            BitMath::at(ent.flags, EntityFlags::kSpawnedFromZone) &&
            frand() < DIGGER_SPAWN_CHANCE) {
            EntityID team = NULL_ENTITY;
            if (sim->ent_exists(ent.last_damaged_by))
                team = sim->get_ent(ent.last_damaged_by).get_team();
            alloc_mob(sim, MobID::kDigger, ent.get_x(), ent.get_y(), team);
        }

    } else if (ent.has_component(kPetal)) {
        if (ent.get_petal_id() == PetalID::kWeb || ent.get_petal_id() == PetalID::kTriweb)
            alloc_web(sim, 100, ent);
    } else if (ent.has_component(kFlower)) {
        if (!sim->ent_alive(ent.get_parent()))
            return;
        Entity &camera = sim->get_ent(ent.get_parent());
        for (uint32_t i = 0; i < 2 * MAX_SLOT_COUNT; ++i)
        {
            camera.set_inventory_ids(i,ent.get_inventory_ids(i));
            camera.set_inventory_rarities(i, ent.get_inventory_rarities(i));
        }
        camera.set_respawn_level(score_to_level(ent.get_score()));
    }
    else if (ent.has_component(kDrop)) {
        if (BitMath::at(ent.flags, EntityFlags::kIsDespawning))
            PetalTracker::remove_petal(sim, ent.get_drop_id());
    }
}