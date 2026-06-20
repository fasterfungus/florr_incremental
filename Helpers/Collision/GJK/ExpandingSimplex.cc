#include <Helpers/Vector.hh>
#include <cstdint>
#include <vector>
#include <Helpers/Collision/GJK/Simplex.hh>
#include <algorithm>
#include "ExpandingSimplex.hh"

#include <cmath>
bool Edge::operator<(const Edge& e)
{
    return this->distance < e.distance;
}
bool Edge::operator>(const Edge& e)
{
    return this->distance > e.distance;
}
Edge::Edge(const Vector& p1, const Vector& p2, WindingType winding)
{
    this->p1 = p1;
    this->p2 = p2;
    this->winding = winding;
    normal = Vector(p2.x - p1.x, p2.y - p1.y);
    if (winding == WindingType::Clockwise)
    {
        normal.CW90();
    }
    else
    {
        normal.CCW90();
    }
    normal.Normalize();
    distance = std::abs(p1.x * normal.x + p1.y * normal.y);
}

ExpandingSimplex::ExpandingSimplex(const Simplex& simplex)
{
    Winding(simplex);
    Edges(simplex);
}

const Edge ExpandingSimplex::GetClosestEdge()
{
    return edges[0];
}

void ExpandingSimplex::Expand(const Vector& p)
{
    Edge edge = edges[0];
    edges.erase(edges.begin());
    Edge edge1 = Edge(edge.p1, p, winding);
    Edge edge2 = Edge(p, edge.p2, winding);
    edges.push_back(edge1);
    edges.push_back(edge2);
    std::sort(edges.begin(), edges.end());
}

void ExpandingSimplex::Edges(const Simplex& simplex)
{
    uint8_t n = simplex.vertics.size();
    for (int i = 0; i < n; i++)
    {
        Vector p1 = simplex.vertics[i];
        Vector p2 = simplex.vertics[(i + 1) % n];

        Edge edge = Edge(p1, p2, winding);
        edges.push_back(edge);
    }
    std::sort(edges.begin(), edges.end());
}

void ExpandingSimplex::Winding(const Simplex& simplex)
{
    uint8_t n = simplex.vertics.size();
    for (int i = 0; i < n; i++)
    {
        Vector p1 = simplex.vertics[i];
        Vector p2 = simplex.vertics[(i + 1) % n];

        float c = Vector::Cross(p1, p2);
        if (c > 0)
        {
            winding = WindingType::CounterClockwise;
        }
        else
        {
            winding = WindingType::Clockwise;
        }
    }
}
