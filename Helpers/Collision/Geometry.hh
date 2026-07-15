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
    float angle;
    std::vector<Vector> vertics;
    CollisionShape shape;
    float x;
    float y;
    float last_x;
    float last_y;
    float rotation;

    Geometry(const Entity&);
    Geometry(float,float,float,float,CollisionShape);
};

float GetDistance2(const Vector&, const Vector&, const Vector&);
float GetDistance2(const Vector&, const Vector&);
bool Contains(const Geometry&, const Vector&);
Vector GetFarthestProjectionPoint(const Geometry&, const Vector&);
