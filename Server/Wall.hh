#pragma once

// A static, immovable line-segment obstacle. Defined by a center (x, y), a
// length, and an angle. Walls never move after map init, so they live in the
// BVHCollisionManager's stationary tree (built once, never re-balanced).
//
// The axis-aligned bounds (minx/miny/maxx/maxy) are computed once in the ctor
// from the segment's two endpoints and stored, so the broad phase can insert
// the wall without recomputing. Narrow-phase collision uses Geometry(const
// Wall&) (a kSegment), so nothing here needs to know about GJK.
class Wall
{
    public:
    float x;
    float y;
    float length;
    float angle;
    float minx;
    float miny;
    float maxx;
    float maxy;
    Wall(float x, float y, float length, float angle);
};
