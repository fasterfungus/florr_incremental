//
// Created by administrator on 2026/7/22.
//

#include "Wall.hh"
#include <Helpers/Vector.hh>
#include <Helpers/Math.hh>
#include <cmath>

Wall::Wall(float x, float y, float length, float angle)
{
    this->x = x;
    this->y = y;
    this->length = length;
    this->angle = angle;

    // A wall is a line segment centered at (x, y): its two endpoints sit at
    // ±length/2 along `angle`. The AABB is just the min/max of those endpoints.
    float half = length * 0.5f;
    float dx = std::cos(angle) * half;
    float dy = std::sin(angle) * half;
    Vector a(x - dx, y - dy);
    Vector b(x + dx, y + dy);
    minx = Min(a.x, b.x);
    maxx = Max(a.x, b.x);
    miny = Min(a.y, b.y);
    maxy = Max(a.y, b.y);
}
