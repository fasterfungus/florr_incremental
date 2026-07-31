#include <Server/Process.hh>

#include <Shared/Simulation.hh>
#include <Shared/Entity.hh>
#include <Helpers/Collision/Map/Wall.hh>
#include <iostream>
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
static void apply_ccd(Simulation *sim, Entity &ent, Vector start)
{
    if (ent.pending_delete || !ent.has_component(kPhysics))
        return;

    Vector current(start.x, start.y);

    Vector target(
        ent.get_x(),
        ent.get_y()
    );

    // 本 tick 的总位移
    Vector remaining = target - start;


    float r = ent.get_radius() * ent.get_scale();

    constexpr int MAX_CCD_ITER = 4;
    constexpr float EPS = 0.001f;
    constexpr float SKIN = 0.05f;


    for (int iter = 0; iter < MAX_CCD_ITER; iter++)
    {
        if (remaining.magnitude() < EPS)
            break;


        AABB swept =
            AABB::combine(
                AABB::from_circle(
                    current.x,
                    current.y,
                    r
                ),
                AABB::from_circle(
                    current.x + remaining.x,
                    current.y + remaining.y,
                    r
                )
            );


        float earliest_t = 1.0f;
        Vector hit_normal(0, 0);


        sim->chunk_bvh_collision_manager.query_walls_in_aabb(
            swept,
            [&](Wall const &wall)
        {
            float half = wall.length * 0.5f;

            float c = std::cos(wall.rotation);
            float s = std::sin(wall.rotation);


            Vector a(
                wall.x - c * half,
                wall.y - s * half
            );

            Vector b(
                wall.x + c * half,
                wall.y + s * half
            );


            CCD::SweepHit h =
                CCD::swept_circle_segment(
                    current,
                    remaining,
                    r,
                    a,
                    b
                );


            if (h.hit && h.t < earliest_t)
            {
                earliest_t = h.t;
                hit_normal = h.normal;
            }
        });



        // 没有碰撞，直接结束
        if (earliest_t >= 1.0f)
        {
            current += remaining;
            break;
        }



        // =============================
        // 1. 移动到碰撞点
        // =============================

        current += remaining * earliest_t;


        // 防止浮点误差重新进入墙
        current += hit_normal * SKIN;



        // =============================
        // 2. 计算剩余位移
        // =============================

        remaining *= (1.0f - earliest_t);



        // =============================
        // 3. 投影到墙的切向方向
        // =============================

        float vn =
            Vector::Dot(
                remaining,
                hit_normal
            );


        if (vn < 0)
        {
            remaining -= hit_normal * vn;
        }



        // =============================
        // 4. 同步速度方向
        //    只去掉法向速度
        //    不改变速度大小
        // =============================

        float velocity_normal =
            Vector::Dot(
                ent.velocity,
                hit_normal
            );


        if (velocity_normal < 0)
        {
            ent.velocity -=
                hit_normal * velocity_normal;
        }
    }


    // 最终位置
    ent.set_x(current.x);
    ent.set_y(current.y);
}