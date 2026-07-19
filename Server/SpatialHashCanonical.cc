#include <Server/SpatialHash.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>

SpatialHash::SpatialHash(Simulation *sim) : simulation(sim), width(1), height(1) {}

void SpatialHash::refresh(uint32_t _width, uint32_t _height) {
    DEBUG_ONLY(assert(_width <= ARENA_WIDTH && _height <= ARENA_HEIGHT));
    width = _width;
    height = _height;
    tree.clear();
}

void SpatialHash::insert(Entity const &ent) {
    DEBUG_ONLY(assert(ent.has_component(kPhysics));)
    tree.insert(ent.id, AABB::from_entity(ent));
}

void SpatialHash::collide(std::function<void(Simulation *, Entity &, Entity &)> on_collide) {
    tree.build();
    tree.collide([&](EntityID a, EntityID b) {
        on_collide(simulation, simulation->get_ent(a), simulation->get_ent(b));
    });
}

void SpatialHash::query(float x, float y, float w, float h, std::function<void(Simulation *, Entity &)> cb) {
    tree.build();
    AABB region = AABB::from_half_extents(x, y, w, h);
    tree.query(region, [&](EntityID id) {
        Entity &ent = simulation->get_ent(id);
        float world_minx = ent.get_x() + ent.get_minx();
        float world_maxx = ent.get_x() + ent.get_maxx();
        float world_miny = ent.get_y() + ent.get_miny();
        float world_maxy = ent.get_y() + ent.get_maxy();
        if (world_maxx < x - w || world_minx > x + w) return;
        if (world_maxy < y - h || world_miny > y + h) return;
        cb(simulation, ent);
    });
}
