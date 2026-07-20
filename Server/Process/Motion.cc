#include <Server/Process.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>

#include <Helpers/Collision/CCD.hh>

#include <cmath>

constexpr float BASE_TPS = 20;

static void apply_ccd(Simulation *sim, Entity &ent, Vector start);

void tick_entity_motion(Simulation *sim, Entity &ent) {
    if (ent.pending_delete) return;
    // Capture the pre-motion position so CCD can sweep the path this tick.
    Vector const ccd_start(ent.get_x(), ent.get_y());
    if (ent.slow_ticks > 0) {
        ent.speed_ratio *= 0.5;
        --ent.slow_ticks;
    }
    float const dt = (BASE_TPS / TPS);
    if (ent.friction <= 0) {
        Vector const add = ent.velocity * dt + ent.acceleration * (0.5 * dt * dt);
        ent.velocity += ent.acceleration * dt;
        ent.set_x(ent.get_x() + add.x + ent.collision_velocity.x);
        ent.set_y(ent.get_y() + add.y + ent.collision_velocity.y);
    }
    else if (ent.friction >= 1) {
        ent.velocity.set(0,0);
        ent.set_x(ent.get_x() + ent.acceleration.x * dt + ent.collision_velocity.x);
        ent.set_y(ent.get_y() + ent.acceleration.y * dt + ent.collision_velocity.y);
    }
    else {
        float const f = 1 - ent.friction;
        Vector const term_vel = ent.acceleration * (ent.speed_ratio / ent.friction);
        Vector const v = ent.velocity - term_vel;
        Vector const add = term_vel * dt + v * ((std::powf(f, dt) - 1) / std::logf(f));
        ent.set_x(ent.get_x() + add.x + ent.collision_velocity.x);
        ent.set_y(ent.get_y() + add.y + ent.collision_velocity.y);
        ent.velocity = term_vel + v * (std::powf(f, dt));
    }
    ent.velocity += ent.collision_velocity * 0.5;
    if (!ent.has_component(kPetal) && !ent.has_component(kWeb)) {
        ent.set_x(fclamp(ent.get_x(), ent.get_radius(), ARENA_WIDTH - ent.get_radius()));
        ent.set_y(fclamp(ent.get_y(), ent.get_radius(), ARENA_HEIGHT - ent.get_radius()));
    }
    //ent.acceleration.set(0,0);
    ent.collision_velocity.set(0,0);
    ent.speed_ratio = 1;
    apply_ccd(sim, ent, ccd_start);
}

// Continuous collision detection. Fast entities can move far enough in one tick
// that their discrete start/end positions straddle a thin obstacle, so the
// per-tick overlap test never fires and they tunnel through. CCD sweeps the
// entity's tight circle along this tick's displacement, and if it would first
// contact an enemy obstacle mid-path, clamps the entity's position to that
// contact point. The next tick's narrow phase (GJK in on_collide) then registers
// the now-touching pair and applies damage/knockback as usual.
static void apply_ccd(Simulation *sim, Entity &ent, Vector start) {
    if (ent.pending_delete || !ent.has_component(kPhysics)) return;
    Vector end(ent.get_x(), ent.get_y());
    Vector dir = end - start;

    // Trigger on displacement (step size) — that is what actually risks
    // tunneling — not on velocity units. Threshold in world units per tick.
    const float CCD_THRESHOLD = 80.0f;
    if (dir.magnitude() < CCD_THRESHOLD) return;

    float r = ent.get_radius() * ent.get_scale();

    // Swept AABB covering the whole path, for the broad-phase query.
    AABB tight_start = AABB::from_circle(start.x, start.y, r);
    AABB tight_end = AABB::from_circle(end.x, end.y, r);
    AABB swept = AABB::combine(tight_start, tight_end);

    float earliest_t = 1.0f;
    Vector earliest_normal(0, 0);
    EntityID const self_id = ent.id;
    EntityID const parent_id = ent.get_parent();
    EntityID const team = ent.get_team();

    sim->spatial_hash.query(
        swept.center_x(), swept.center_y(),
        (swept.max_x - swept.min_x) * 0.5f,
        (swept.max_y - swept.min_y) * 0.5f,
        [&](Simulation *s, Entity &other) {
            if (other.id == self_id) return;
            if (other.id == parent_id) return;
            if (!other.has_component(kPhysics)) return;
            if (other.pending_delete) return;
            // Only stop for things we would actually collide with: enemies.
            // Same-team pass-through matches the discrete collision filter.
            if (other.get_team() == team) return;
            if (!other.has_component(kMob) && !other.has_component(kFlower)) return;

            CCD::SweepHit h = CCD::swept_circle_aabb(start, dir, r, AABB::from_entity(other));
            if (h.hit && h.t < earliest_t) {
                earliest_t = h.t;
                earliest_normal = h.normal;
            }
        });

    if (earliest_t < 1.0f) {
        ent.set_x(start.x + dir.x * earliest_t);
        ent.set_y(start.y + dir.y * earliest_t);
        // Cancel the velocity component going into the obstacle along the hit
        // normal. Without this the entity re-triggers CCD every tick and freezes.
        // Keeping the tangential component allows sliding along walls.
        float dot = Vector::Dot(ent.velocity, earliest_normal);
        if (dot < 0.0f)
            ent.velocity += earliest_normal * (-dot);
        ++ent.ccd_hits;
    }
}