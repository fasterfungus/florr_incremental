#pragma once

#include <Shared/Entity.hh>
#include <Helpers/Vector.hh>

#include <algorithm>
#include <cstdint>
#include <vector>

struct AABB {
    float min_x, min_y, max_x, max_y;

    AABB() : min_x(0), min_y(0), max_x(0), max_y(0) {}
    AABB(float _min_x, float _min_y, float _max_x, float _max_y)
        : min_x(_min_x), min_y(_min_y), max_x(_max_x), max_y(_max_y) {}

    static AABB from_entity(Entity const &e) {
        float r = e.get_radius() * e.get_scale();
        float min_x = e.get_x() + e.get_minx();
        float min_y = e.get_y() + e.get_miny();
        float max_x = e.get_x() + e.get_maxx();
        float max_y = e.get_y() + e.get_maxy();
        return AABB(min_x, min_y, max_x, max_y);
    }

    static AABB from_half_extents(float x, float y, float w, float h) {
        return AABB(x - w, y - h, x + w, y + h);
    }

    static AABB from_circle(float x, float y, float r) {
        return AABB(x - r, y - r, x + r, y + r);
    }

    void expand(AABB const &o) {
        min_x = std::min(min_x, o.min_x);
        min_y = std::min(min_y, o.min_y);
        max_x = std::max(max_x, o.max_x);
        max_y = std::max(max_y, o.max_y);
    }

    bool overlaps(AABB const &o) const {
        return !(max_x < o.min_x || min_x > o.max_x ||
                 max_y < o.min_y || min_y > o.max_y);
    }

    // 2D SAH cost proxy (analogue of Box2D's b2Perimeter surface area).
    float perimeter() const {
        return (max_x - min_x) + (max_y - min_y);
    }

    static AABB combine(AABB const &a, AABB const &b) {
        return AABB(std::min(a.min_x, b.min_x), std::min(a.min_y, b.min_y),
                    std::max(a.max_x, b.max_x), std::max(a.max_y, b.max_y));
    }

    // Does *this* fully enclose o?
    bool contains(AABB const &o) const {
        return min_x <= o.min_x && min_y <= o.min_y &&
               max_x >= o.max_x && max_y >= o.max_y;
    }

    float center_x() const { return (min_x + max_x) * 0.5f; }
    float center_y() const { return (min_y + max_y) * 0.5f; }
};

// Persistent incremental dynamic AABB tree, modeled on Box2D's b2DynamicTree.
// Each physics entity owns one leaf "proxy" (an int32_t handle) that survives
// across ticks. Leaves store a FAT aabb (tight box + margin, extended along
// motion). As long as an entity's tight box stays inside its fat box, moving it
// is a no-op; only escaped leaves are re-inserted (remove + insert + rotate),
// costing O(log N) instead of an O(N log N) full rebuild every tick.
class BVH {
    static constexpr float AABB_MARGIN = 10.0f;      // fat-AABB skin (world units)
    static constexpr float AABB_MULTIPLIER = 2.0f;   // motion-prediction factor

    struct Node {
        AABB box;                 // FAT aabb for leaves; union of children for internal
        int32_t parent = -1;      // reused as free-list "next" when node is free
        int32_t child1 = -1;
        int32_t child2 = -1;
        int32_t height = -1;      // 0 = leaf, >0 = internal, -1 = free
        EntityID entity;
        bool is_leaf() const { return child1 == -1; }
    };

    std::vector<Node> nodes_;
    int32_t root_ = -1;
    int32_t free_list_ = -1;
    uint32_t proxy_count_ = 0;

    int32_t allocate_node() {
        if (free_list_ == -1) {
            int32_t id = (int32_t)nodes_.size();
            nodes_.emplace_back();
            return id;
        }
        int32_t id = free_list_;
        free_list_ = nodes_[id].parent;   // pop free list
        nodes_[id] = Node{};              // reset
        return id;
    }

    void free_node(int32_t id) {
        nodes_[id].parent = free_list_;   // push onto free list
        nodes_[id].height = -1;
        free_list_ = id;
    }

    static AABB fatten(AABB t, Vector d) {
        AABB f(t.min_x - AABB_MARGIN, t.min_y - AABB_MARGIN,
               t.max_x + AABB_MARGIN, t.max_y + AABB_MARGIN);
        float dx = d.x * AABB_MULTIPLIER, dy = d.y * AABB_MULTIPLIER;
        if (dx < 0) f.min_x += dx; else f.max_x += dx;
        if (dy < 0) f.min_y += dy; else f.max_y += dy;
        return f;
    }

    void insert_leaf(int32_t leaf) {
        if (root_ == -1) { root_ = leaf; nodes_[leaf].parent = -1; return; }

        AABB leafBox = nodes_[leaf].box;

        // 1. Descend from root to the best sibling (least SA growth).
        int32_t index = root_;
        while (!nodes_[index].is_leaf()) {
            int32_t c1 = nodes_[index].child1, c2 = nodes_[index].child2;
            float area = nodes_[index].box.perimeter();
            AABB combined = AABB::combine(nodes_[index].box, leafBox);
            float combinedArea = combined.perimeter();
            float cost = 2.0f * combinedArea;                    // cost of a new parent here
            float inheritCost = 2.0f * (combinedArea - area);    // cost pushed to descendants

            auto descendCost = [&](int32_t child) {
                AABB b = AABB::combine(leafBox, nodes_[child].box);
                float c = b.perimeter();
                if (!nodes_[child].is_leaf()) c -= nodes_[child].box.perimeter();
                return c + inheritCost;
            };
            float cost1 = descendCost(c1), cost2 = descendCost(c2);
            if (cost < cost1 && cost < cost2) break;             // stop: create parent here
            index = (cost1 < cost2) ? c1 : c2;
        }

        // 2. Create a new parent for the sibling and the new leaf.
        int32_t sibling = index;
        int32_t oldParent = nodes_[sibling].parent;
        int32_t newParent = allocate_node();
        nodes_[newParent].parent = oldParent;
        nodes_[newParent].box = AABB::combine(leafBox, nodes_[sibling].box);
        nodes_[newParent].height = nodes_[sibling].height + 1;
        nodes_[newParent].child1 = sibling;
        nodes_[newParent].child2 = leaf;
        nodes_[sibling].parent = newParent;
        nodes_[leaf].parent = newParent;

        if (oldParent != -1) {
            if (nodes_[oldParent].child1 == sibling) nodes_[oldParent].child1 = newParent;
            else nodes_[oldParent].child2 = newParent;
        } else {
            root_ = newParent;
        }

        // 3. Walk back up refitting boxes/heights and rotating for balance.
        int32_t i = nodes_[leaf].parent;
        while (i != -1) {
            i = rotate(i);
            int32_t c1 = nodes_[i].child1, c2 = nodes_[i].child2;
            nodes_[i].height = 1 + std::max(nodes_[c1].height, nodes_[c2].height);
            nodes_[i].box = AABB::combine(nodes_[c1].box, nodes_[c2].box);
            i = nodes_[i].parent;
        }
    }

    void remove_leaf(int32_t leaf) {
        if (leaf == root_) { root_ = -1; return; }
        int32_t parent = nodes_[leaf].parent;
        int32_t grandParent = nodes_[parent].parent;
        int32_t sibling = (nodes_[parent].child1 == leaf) ? nodes_[parent].child2
                                                          : nodes_[parent].child1;
        if (grandParent != -1) {
            if (nodes_[grandParent].child1 == parent) nodes_[grandParent].child1 = sibling;
            else nodes_[grandParent].child2 = sibling;
            nodes_[sibling].parent = grandParent;
            free_node(parent);
            // Refit ancestors.
            int32_t i = grandParent;
            while (i != -1) {
                i = rotate(i);
                int32_t c1 = nodes_[i].child1, c2 = nodes_[i].child2;
                nodes_[i].box = AABB::combine(nodes_[c1].box, nodes_[c2].box);
                nodes_[i].height = 1 + std::max(nodes_[c1].height, nodes_[c2].height);
                i = nodes_[i].parent;
            }
        } else {
            root_ = sibling;
            nodes_[sibling].parent = -1;
            free_node(parent);
        }
    }

    // Balance node iA by promoting a grandchild if one subtree is too tall.
    // Returns the node now occupying iA's position. (Box2D b2RotateNodes.)
    int32_t rotate(int32_t iA) {
        Node &A = nodes_[iA];
        if (A.is_leaf() || A.height < 2) return iA;
        int32_t iB = A.child1, iC = A.child2;
        int32_t balance = nodes_[iC].height - nodes_[iB].height;

        // C is too tall: promote C above A. A keeps B plus one grandchild; the
        // other grandchild stays with C. Put the grandchild that makes A's box
        // smallest down with A (keeps the lower node tight).
        if (balance > 1) {
            int32_t iF = nodes_[iC].child1, iG = nodes_[iC].child2;
            float costA_F = AABB::combine(nodes_[iB].box, nodes_[iF].box).perimeter();
            float costA_G = AABB::combine(nodes_[iB].box, nodes_[iG].box).perimeter();
            int32_t toA, toC;
            if (costA_F < costA_G) { toA = iF; toC = iG; } else { toA = iG; toC = iF; }

            int32_t oldParent = A.parent;
            nodes_[iC].child1 = iA;
            nodes_[iC].parent = oldParent;
            nodes_[iA].parent = iC;
            if (oldParent != -1) {
                if (nodes_[oldParent].child1 == iA) nodes_[oldParent].child1 = iC;
                else nodes_[oldParent].child2 = iC;
            } else root_ = iC;

            nodes_[iC].child2 = toC;
            nodes_[toC].parent = iC;
            nodes_[iA].child2 = toA;
            nodes_[toA].parent = iA;

            nodes_[iA].box = AABB::combine(nodes_[iB].box, nodes_[toA].box);
            nodes_[iC].box = AABB::combine(nodes_[iA].box, nodes_[toC].box);
            nodes_[iA].height = 1 + std::max(nodes_[iB].height, nodes_[toA].height);
            nodes_[iC].height = 1 + std::max(nodes_[iA].height, nodes_[toC].height);
            return iC;
        }

        // B is too tall (mirror).
        if (balance < -1) {
            int32_t iD = nodes_[iB].child1, iE = nodes_[iB].child2;
            float costA_D = AABB::combine(nodes_[iC].box, nodes_[iD].box).perimeter();
            float costA_E = AABB::combine(nodes_[iC].box, nodes_[iE].box).perimeter();
            int32_t toA, toB;
            if (costA_D < costA_E) { toA = iD; toB = iE; } else { toA = iE; toB = iD; }

            int32_t oldParent = A.parent;
            nodes_[iB].child1 = iA;
            nodes_[iB].parent = oldParent;
            nodes_[iA].parent = iB;
            if (oldParent != -1) {
                if (nodes_[oldParent].child1 == iA) nodes_[oldParent].child1 = iB;
                else nodes_[oldParent].child2 = iB;
            } else root_ = iB;

            nodes_[iB].child2 = toB;
            nodes_[toB].parent = iB;
            nodes_[iA].child1 = toA;
            nodes_[toA].parent = iA;

            nodes_[iA].box = AABB::combine(nodes_[iC].box, nodes_[toA].box);
            nodes_[iB].box = AABB::combine(nodes_[iA].box, nodes_[toB].box);
            nodes_[iA].height = 1 + std::max(nodes_[iC].height, nodes_[toA].height);
            nodes_[iB].height = 1 + std::max(nodes_[iA].height, nodes_[toB].height);
            return iB;
        }

        return iA;
    }

public:
    void clear() {
        nodes_.clear();
        root_ = -1;
        free_list_ = -1;
        proxy_count_ = 0;
    }

    uint32_t size() const { return proxy_count_; }

    int32_t create_proxy(AABB tight, EntityID entity) {
        int32_t id = allocate_node();
        nodes_[id].box = fatten(tight, Vector(0, 0));
        nodes_[id].entity = entity;
        nodes_[id].height = 0;
        nodes_[id].child1 = nodes_[id].child2 = -1;
        insert_leaf(id);
        ++proxy_count_;
        return id;
    }

    void destroy_proxy(int32_t id) {
        remove_leaf(id);
        free_node(id);
        --proxy_count_;
    }

    // Returns true iff the tree was modified (tight box escaped the fat box).
    bool move_proxy(int32_t id, AABB tight, Vector displacement) {
        if (nodes_[id].box.contains(tight)) return false;   // still inside fat box: no-op
        remove_leaf(id);
        nodes_[id].box = fatten(tight, displacement);
        insert_leaf(id);
        return true;
    }

    template <typename F>
    void query(AABB const &region, F &&cb) const {
        if (root_ == -1) return;
        int32_t stack[256];
        int32_t sp = 0;
        stack[sp++] = root_;
        while (sp > 0) {
            int32_t ni = stack[--sp];
            if (ni == -1) continue;
            Node const &n = nodes_[ni];
            if (!n.box.overlaps(region)) continue;
            if (n.is_leaf()) cb(n.entity);
            else { stack[sp++] = n.child1; stack[sp++] = n.child2; }
        }
    }

    // Emit every overlapping leaf pair exactly once (dedup by node index).
    template <typename F>
    void collide(F &&cb) const {
        if (root_ == -1) return;
        for (int32_t li = 0; li < (int32_t)nodes_.size(); ++li) {
            Node const &ln = nodes_[li];
            if (ln.height != 0) continue;              // only leaves
            int32_t stack[256];
            int32_t sp = 0;
            stack[sp++] = root_;
            while (sp > 0) {
                int32_t ni = stack[--sp];
                if (ni == -1) continue;
                Node const &n = nodes_[ni];
                if (!n.box.overlaps(ln.box)) continue;
                if (n.is_leaf()) { if (ni > li) cb(ln.entity, n.entity); }
                else { stack[sp++] = n.child1; stack[sp++] = n.child2; }
            }
        }
    }

    // Test-only invariant checker. O(N). Returns true if the tree is well-formed.
    bool validate() const {
        if (root_ == -1) return true;
        if (nodes_[root_].parent != -1) return false;
        for (int32_t i = 0; i < (int32_t)nodes_.size(); ++i) {
            Node const &n = nodes_[i];
            if (n.height == -1) continue;              // free node
            if (n.is_leaf()) { if (n.height != 0) return false; continue; }
            int32_t c1 = n.child1, c2 = n.child2;
            if (nodes_[c1].parent != i || nodes_[c2].parent != i) return false;
            if (!n.box.contains(nodes_[c1].box) || !n.box.contains(nodes_[c2].box)) return false;
            int32_t h = 1 + std::max(nodes_[c1].height, nodes_[c2].height);
            if (n.height != h) return false;
        }
        return true;
    }
};
