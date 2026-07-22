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

// ---------------------------------------------------------------------------
// CCD swept-circle-vs-segment: exact test, must not fire for diagonal walls
// that don't actually overlap the swept path.
static void test_ccd_segment() {
    // 1. Circle moving right, horizontal segment directly in its path.
    //    Segment from (100,100) to (100,120), r=5. Contact when circle reaches x=95.
    {
        Vector a(100, 100), b(100, 120);
        CCD::SweepHit h = CCD::swept_circle_segment(Vector(0, 110), Vector(200, 0), 5, a, b);
        assert(h.hit && "horizontal motion into vertical segment should hit");
        float contact_x = 0 + 200 * h.t;
        assert(std::fabs(contact_x - 95.0f) < 1.0f && "contact at 95 (= 100 - radius)");
        assert(h.normal.x < 0 && "normal points left (away from segment)");
    }

    // 2. Circle moving right, vertical segment 90 degrees off to the side (above).
    //    This is the KEY regression: with AABB sweep, a diagonal wall's large AABB
    //    would incorrectly fire. With exact segment sweep it must NOT fire.
    {
        // 45-degree segment, positioned to the right of the path but not in it.
        // Start=(0,0), dir=(200,0), r=5. Segment at y=50 (above circle path).
        Vector a(80, 50), b(120, 90);  // segment is above the circle path (y=0)
        CCD::SweepHit h = CCD::swept_circle_segment(Vector(0, 0), Vector(200, 0), 5, a, b);
        assert(!h.hit && "diagonal segment above path must NOT fire (AABB would)");
    }

    // 3. Circle moving diagonally, misses segment endpoint by a hair.
    {
        Vector a(100, 100), b(100, 200);
        // Circle moves at y=100 - r - 1 = 94, so it barely clears the endpoint.
        CCD::SweepHit h = CCD::swept_circle_segment(Vector(0, 94), Vector(200, 0), 5, a, b);
        assert(!h.hit && "circle misses segment endpoint by 1 unit, no hit");
    }

    // 4. Tunneling: circle teleports past segment in one step, exact test catches it.
    {
        Vector a(100, 90), b(100, 110);
        CCD::SweepHit h = CCD::swept_circle_segment(Vector(50, 100), Vector(200, 0), 5, a, b);
        assert(h.hit && "tunneling past vertical segment must be caught");
        assert(h.t < 1.0f);
    }

    // 5. End-cap hit: circle hits the tip of a segment.
    {
        Vector a(100, 100), b(200, 100);  // horizontal segment
        // Circle moves vertically downward toward the left tip.
        CCD::SweepHit h = CCD::swept_circle_segment(Vector(100, 0), Vector(0, 200), 5, a, b);
        assert(h.hit && "circle hits segment tip (end cap)");
    }

    std::printf("test_ccd_segment passed\n");
}

// ---------------------------------------------------------------------------
// Tree-quality: after a churn of inserts + moves, the SAH cost (sum of internal
// node surface areas) and max height should stay low. Prints the metrics so a
// BVH_DISABLE_ROTATE build can be compared against the SAH-rotation build.
static void test_tree_quality() {
    std::mt19937 rng(2024);
    std::uniform_real_distribution<float> pos(0, 4000), rad(5, 40), vel(-80, 80);
    BVH tree;
    int N = 500;
    std::vector<Circle> c(N);
    std::vector<int32_t> px(N);
    for (int i = 0; i < N; ++i) {
        c[i] = {pos(rng), pos(rng), rad(rng)};
        px[i] = tree.create_proxy(AABB::from_circle(c[i].x, c[i].y, c[i].r), EntityID((uint16_t)i, 0));
    }
    for (int frame = 0; frame < 200; ++frame)
        for (int i = 0; i < N; ++i) {
            float dx = vel(rng), dy = vel(rng);
            c[i].x += dx; c[i].y += dy;
            tree.move_proxy(px[i], AABB::from_circle(c[i].x, c[i].y, c[i].r), Vector(dx, dy));
        }
    assert(tree.validate());

    float cost = tree.total_sah_cost();
    int32_t h = tree.max_height();
    int32_t ideal = 0; for (int32_t n = 1; n < N; n <<= 1) ++ideal;   // ceil(log2 N)
    std::printf("test_tree_quality: N=%d  SAH_cost=%.0f  max_height=%d (ideal ~%d)\n",
                N, cost, h, ideal);
    assert(h <= 3 * ideal && "tree height degenerate — rotations not keeping it balanced");
    std::printf("test_tree_quality passed\n");
}

// ---------------------------------------------------------------------------
// Cross-tree collision (dynamic vs stationary): query_pairs must emit a
// SUPERSET of the true dynamic-vs-static overlaps, with no self-pairs and no
// within-tree pairs. Models entities (tree A) hitting walls (tree B).
static void test_cross_tree_pairs() {
    std::mt19937 rng(77);
    std::uniform_real_distribution<float> pos(0, 2000), rad(5, 50);
    int NA = 120, NB = 80;
    std::vector<Circle> a(NA), b(NB);
    BVH dyn, stat;
    for (int i = 0; i < NA; ++i) {
        a[i] = {pos(rng), pos(rng), rad(rng)};
        dyn.create_proxy(AABB::from_circle(a[i].x, a[i].y, a[i].r), EntityID((uint16_t)i, 0));
    }
    for (int j = 0; j < NB; ++j) {
        b[j] = {pos(rng), pos(rng), rad(rng)};
        stat.create_proxy(AABB::from_circle(b[j].x, b[j].y, b[j].r), EntityID((uint16_t)j, 0));
    }

    // Brute-force truth: every (dynamic i, static j) whose tight AABBs overlap.
    std::set<std::pair<int, int>> want;
    for (int i = 0; i < NA; ++i)
        for (int j = 0; j < NB; ++j)
            if (AABB::from_circle(a[i].x, a[i].y, a[i].r)
                    .overlaps(AABB::from_circle(b[j].x, b[j].y, b[j].r)))
                want.insert({i, j});

    std::set<std::pair<int, int>> got;
    dyn.query_pairs(stat, [&](EntityID ea, EntityID eb) {
        assert(got.insert({ea.id, eb.id}).second && "duplicate cross pair");
    });
    for (auto const &p : want)
        assert(got.count(p) && "cross-tree query MISSED a dynamic-vs-static overlap");

    // Empty stationary tree → no pairs.
    BVH empty;
    int calls = 0;
    dyn.query_pairs(empty, [&](EntityID, EntityID) { ++calls; });
    assert(calls == 0 && "no pairs against empty tree");
    std::printf("test_cross_tree_pairs passed\n");
}

// ---------------------------------------------------------------------------
int main() {
    test_aabb_helpers();
    test_edge_cases();
    test_move_noop_when_inside_fat();
    test_persistent_lifecycle();
    test_tree_quality();
    test_cross_tree_pairs();
    test_ccd_sweep();
    test_ccd_segment();
    if (test_superset_scenes() != 0) return 1;
    std::printf("ALL BVH TESTS PASSED\n");
    return 0;
}
