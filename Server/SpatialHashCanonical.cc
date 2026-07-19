#include <Server/SpatialHash.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>

SpatialHash::SpatialHash(Simulation *sim) : simulation(sim), width(1), height(1) {}

void SpatialHash::refresh(uint32_t _width, uint32_t _height) {
    DEBUG_ONLY(assert(_width <= ARENA_WIDTH && _height <= ARENA_HEIGHT));
    width = _width;
    height = _height;
    // NO tree.clear() here — the dynamic tree persists across ticks now.
}

void SpatialHash::reset() {
    tree.clear();
}

void SpatialHash::sync(Entity &ent) {
    DEBUG_ONLY(assert(ent.has_component(kPhysics));)
    AABB tight = AABB::from_entity(ent);
    // Velocity-based motion prediction: extend the fat box along where the
    // entity is heading this tick (last_x/last_y is reset to x/y before motion,
    // so use velocity, which already holds this tick's intended movement).
    Vector disp = ent.velocity;
    if (ent.bvh_proxy < 0)
        ent.bvh_proxy = tree.create_proxy(tight, ent.id);
    else
        tree.move_proxy(ent.bvh_proxy, tight, disp);
}

void SpatialHash::remove(Entity &ent) {
    if (ent.bvh_proxy < 0) return;
    tree.destroy_proxy(ent.bvh_proxy);
    ent.bvh_proxy = -1;
}

void SpatialHash::collide(std::function<void(Simulation *, Entity &, Entity &)> on_collide) {
    tree.collide([&](EntityID a, EntityID b) {
        on_collide(simulation, simulation->get_ent(a), simulation->get_ent(b));
    });
}

void SpatialHash::query(float x, float y, float w, float h, std::function<void(Simulation *, Entity &)> cb) {
    AABB region = AABB::from_half_extents(x, y, w, h);
    tree.query(region, [&](EntityID id) {
        Entity &ent = simulation->get_ent(id);
        float r = ent.get_radius() * ent.get_scale();
        float world_minx = ent.get_x() + ent.get_minx();
        float world_maxx = ent.get_x() + ent.get_maxx();
        float world_miny = ent.get_y() + ent.get_miny();
        float world_maxy = ent.get_y() + ent.get_maxy();
        if (world_maxx < x - w || world_minx > x + w) return;
        if (world_maxy < y - h || world_miny > y + h) return;
        cb(simulation, ent);
    });
}
