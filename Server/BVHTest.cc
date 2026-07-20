// Standalone correctness test for the persistent dynamic BVH broad phase.
// Compiles the REAL Server/BVH.hh and checks that its candidate-pair set and
// region queries are a SUPERSET of brute force (fat AABBs may add extras but
// must never MISS a true overlap), plus persistent-tree lifecycle + invariants.
//
//   g++ -std=c++20 -DSERVERSIDE=1 -I. -IServer -IShared \
//       Server/BVHTest.cc Shared/EntityDef.cc -o bvhtest && ./bvhtest

#include <BVH.hh>
#include <Helpers/Collision/CCD.hh>

#include <cassert>
#include <cstdio>
#include <cmath>
#include <random>
#include <set>
#include <vector>

struct Circle { float x, y, r; };

static std::set<std::pair<int, int>> brute_pairs(std::vector<Circle> const &c) {
    std::set<std::pair<int, int>> out;
    for (int i = 0; i < (int)c.size(); ++i)
        for (int j = i + 1; j < (int)c.size(); ++j) {
            AABB a = AABB::from_circle(c[i].x, c[i].y, c[i].r);
            AABB b = AABB::from_circle(c[j].x, c[j].y, c[j].r);
            if (a.overlaps(b)) out.insert({i, j});
        }
    return out;
}

static std::set<int> brute_query(std::vector<Circle> const &c, float x, float y, float w, float h) {
    std::set<int> out;
    AABB region = AABB::from_half_extents(x, y, w, h);
    for (int i = 0; i < (int)c.size(); ++i)
        if (AABB::from_circle(c[i].x, c[i].y, c[i].r).overlaps(region)) out.insert(i);
    return out;
}

// ---------------------------------------------------------------------------
static void test_aabb_helpers() {
    AABB big(0, 0, 10, 10), small(2, 2, 4, 4), outside(9, 9, 12, 12);
    assert(big.contains(small));
    assert(!big.contains(outside));
    AABB c = AABB::combine(small, outside);
    assert(c.min_x == 2 && c.min_y == 2 && c.max_x == 12 && c.max_y == 12);
    assert(big.perimeter() == 20.0f);
    std::printf("test_aabb_helpers passed\n");
}

// ---------------------------------------------------------------------------
// Broad phase must be a SUPERSET of brute force (no misses); extras allowed.
static int test_superset_scenes() {
    std::mt19937 rng(12345);
    std::uniform_real_distribution<float> pos(0, 2000);
    std::uniform_real_distribution<float> rad(5, 60);

    int scenes = 200;
    for (int s = 0; s < scenes; ++s) {
        int n = s % 250;
        std::vector<Circle> circles(n);
        BVH tree;
        for (int i = 0; i < n; ++i) {
            circles[i] = {pos(rng), pos(rng), rad(rng)};
            tree.create_proxy(AABB::from_circle(circles[i].x, circles[i].y, circles[i].r),
                              EntityID((uint16_t)i, 0));
        }
        assert(tree.validate() && "tree invalid after builds");

        std::set<std::pair<int, int>> got;
        tree.collide([&](EntityID a, EntityID b) {
            int i = a.id, j = b.id;
            if (i > j) std::swap(i, j);
            assert(got.insert({i, j}).second && "duplicate pair from BVH.collide");
        });
        auto want = brute_pairs(circles);
        for (auto const &p : want) {
            if (!got.count(p)) {
                std::printf("scene %d (n=%d): MISSED pair (%d,%d)\n", s, n, p.first, p.second);
                return 1;
            }
        }

        for (int q = 0; q < 5; ++q) {
            float qx = pos(rng), qy = pos(rng);
            float qw = rad(rng) * 4, qh = rad(rng) * 4;
            std::set<int> qgot;
            tree.query(AABB::from_half_extents(qx, qy, qw, qh),
                       [&](EntityID e) { qgot.insert(e.id); });
            auto qwant = brute_query(circles, qx, qy, qw, qh);
            for (int idx : qwant) {
                if (!qgot.count(idx)) {
                    std::printf("scene %d: query MISSED %d\n", s, idx);
                    return 1;
                }
            }
        }
    }
    std::printf("All %d scenes passed: BVH broad phase superset-of brute force.\n", scenes);
    return 0;
}

// ---------------------------------------------------------------------------
// Persistent create/move/destroy across many "frames"; invariants + superset.
static void test_persistent_lifecycle() {
    std::mt19937 rng(999);
    std::uniform_real_distribution<float> pos(0, 3000), rad(5, 40), vel(-50, 50);
    BVH tree;
    int N = 300;
    std::vector<Circle> c(N);
    std::vector<int32_t> px(N);
    for (int i = 0; i < N; ++i) {
        c[i] = {pos(rng), pos(rng), rad(rng)};
        px[i] = tree.create_proxy(AABB::from_circle(c[i].x, c[i].y, c[i].r), EntityID((uint16_t)i, 0));
    }
    assert(tree.validate());

    for (int frame = 0; frame < 100; ++frame) {
        for (int i = 0; i < N; ++i) {
            float dx = vel(rng), dy = vel(rng);
            c[i].x += dx; c[i].y += dy;
            tree.move_proxy(px[i], AABB::from_circle(c[i].x, c[i].y, c[i].r), Vector(dx, dy));
        }
        assert(tree.validate() && "invariant broke after move frame");

        if (frame % 10 == 0) {
            std::set<std::pair<int, int>> got;
            tree.collide([&](EntityID a, EntityID b) {
                int i = a.id, j = b.id; if (i > j) std::swap(i, j);
                assert(got.insert({i, j}).second && "dup pair");
            });
            for (auto const &p : brute_pairs(c))
                assert(got.count(p) && "MISSED overlap in persistent tree");
        }
    }

    for (int i = 0; i < N; i += 2) tree.destroy_proxy(px[i]);
    assert(tree.validate());
    for (int i = 0; i < N; i += 2) {          // recreate: exercises free-list reuse
        c[i] = {pos(rng), pos(rng), rad(rng)};
        px[i] = tree.create_proxy(AABB::from_circle(c[i].x, c[i].y, c[i].r), EntityID((uint16_t)i, 0));
    }
    assert(tree.validate());
    std::printf("test_persistent_lifecycle passed\n");
}

// ---------------------------------------------------------------------------
// Fat-box amortization: small moves must be no-ops, large moves must re-insert.
static void test_move_noop_when_inside_fat() {
    BVH tree;
    int32_t p = tree.create_proxy(AABB(0, 0, 10, 10), EntityID(1, 0));
    bool touched = tree.move_proxy(p, AABB(1, 1, 11, 11), Vector(1, 1));
    assert(!touched && "small move should be absorbed by fat AABB");
    bool touched2 = tree.move_proxy(p, AABB(500, 500, 510, 510), Vector(500, 500));
    assert(touched2 && "large move must re-insert");
    assert(tree.validate());
    std::printf("test_move_noop_when_inside_fat passed\n");
}

// ---------------------------------------------------------------------------
static void test_edge_cases() {
    BVH empty;
    int calls = 0;
    empty.collide([&](EntityID, EntityID) { ++calls; });
    empty.query(AABB(-1, -1, 1, 1), [&](EntityID) { ++calls; });
    assert(calls == 0);
    assert(empty.validate());

    BVH one;
    int32_t p = one.create_proxy(AABB(0, 0, 1, 1), EntityID(0, 0));
    int pairs = 0; one.collide([&](EntityID, EntityID) { ++pairs; });
    assert(pairs == 0 && "no self-pair");
    int hits = 0; one.query(AABB(0, 0, 1, 1), [&](EntityID) { ++hits; });
    assert(hits == 1);
    one.destroy_proxy(p);
    assert(one.validate() && one.size() == 0);
    std::printf("test_edge_cases passed\n");
}

// ---------------------------------------------------------------------------
// CCD swept-circle-vs-AABB analytic sweep.
static void test_ccd_sweep() {
    // 1. A circle (r=5) moving right from x=0 toward a box at [100,110]x[100,110]
    //    at y=105. Expanded box left edge is at 100-5=95, so contact at 95 units
    //    of a 120-unit move → t ≈ 95/120 ≈ 0.7917.
    {
        AABB box(100, 100, 110, 110);
        CCD::SweepHit h = CCD::swept_circle_aabb(Vector(0, 105), Vector(120, 0), 5, box);
        assert(h.hit && "should detect collision along path");
        assert(std::fabs(h.t - 95.0f / 120.0f) < 0.01f && "contact fraction wrong");
        assert(h.normal.x == -1 && h.normal.y == 0 && "normal should face left");
    }

    // 2. Motion that stops short of the box → no hit.
    {
        AABB box(100, 100, 110, 110);
        CCD::SweepHit h = CCD::swept_circle_aabb(Vector(0, 105), Vector(50, 0), 5, box);
        assert(!h.hit && "motion too short, no contact");
        assert(h.t == 1.0f);
    }

    // 3. Motion that misses the box entirely (parallel, off to the side).
    {
        AABB box(100, 100, 110, 110);
        CCD::SweepHit h = CCD::swept_circle_aabb(Vector(0, 0), Vector(120, 0), 5, box);
        assert(!h.hit && "y far from box, no contact");
    }

    // 4. High-speed tunneling case: without CCD the circle would jump PAST the
    //    box in one step (start left of box, end right of box). CCD must catch it.
    {
        AABB box(2400, 2450, 2500, 2550);   // 100x100 box
        Vector start(0, 2500), end(5000, 2500);
        CCD::SweepHit h = CCD::swept_circle_aabb(start, end - start, 10, box);
        assert(h.hit && "must catch tunneling through the box");
        float contact_x = start.x + (end.x - start.x) * h.t;
        assert(contact_x < 2400 && "contact must be before the box's near edge");
    }

    // 5. Start already inside → immediate contact at t=0.
    {
        AABB box(0, 0, 100, 100);
        CCD::SweepHit h = CCD::swept_circle_aabb(Vector(50, 50), Vector(10, 0), 5, box);
        assert(h.hit && h.t == 0.0f && "inside box → immediate contact");
    }

    std::printf("test_ccd_sweep passed\n");
}

int main() {
    test_aabb_helpers();
    test_edge_cases();
    test_move_noop_when_inside_fat();
    test_persistent_lifecycle();
    test_ccd_sweep();
    if (test_superset_scenes() != 0) return 1;
    std::printf("ALL BVH TESTS PASSED\n");
    return 0;
}
