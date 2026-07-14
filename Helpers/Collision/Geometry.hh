#pragma once
#include <Shared/Entity.hh>
#include <Helpers/Math.hh>
#include <vector>
class Geometry
{
public:
    float scale;
    float radius;
    float width;
    float height;
    float length;
    std::vector<Vector> vertics;
    CollisionShape shape;
    float x;
    float y;
    float angle;
    Geometry(const Entity&);
};
float GetDistance2(const Vector&, const Vector&, const Vector&) ;
bool Contains(const Entity&, const Vector&);
Vector GetFarthestProjectionPoint(const Geometry& , const Vector&);
