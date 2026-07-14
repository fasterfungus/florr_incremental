#pragma once
#include <Helpers/Vector.hh>
#include <cstdint>
#include <vector>
#include <Helpers//Collision/GJK/Simplex.hh>
#include <algorithm>

enum class WindingType : int8_t
{
    Unknown = 0,
    Clockwise = -1,
    CounterClockwise = 1
};

class Edge
{
public:
    Vector p1;
    Vector p2;
    Vector normal;
    WindingType winding;
    float distance;
    Edge(const Vector&, const Vector&, WindingType);
    bool operator<(const Edge&);
    bool operator>(const Edge&);
};

class ExpandingSimplex
{
public:
    Vector p1;
    Vector p2;
    Vector normal;
    WindingType winding;
    float distance;
    ExpandingSimplex(const Simplex&);
    void Expand(const Vector&);
    const Edge GetClosestEdge();

private:
    std::vector<Edge> edges;
    void Winding(const Simplex&);
    void Edges(const Simplex&);
};
