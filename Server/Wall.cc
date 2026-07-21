//
// Created by administrator on 2026/7/22.
//

#include "Wall.hh"
#include <Helpers/Vector.hh>
#include <Helpers/Collision/GeometryHelper.hh>
#include "Helpers/Math.hh"

Wall::Wall(float x, float y, float length, float angle)
{
    this->x = x;
    this->y = y;
    this->length = length;
    this->angle  = angle;
    Set(*this);

}

void Set(Wall wall)
{
    Geometry g = Geometry(wall);
    Vector p1 = GetFarthestProjectionPointAfterRotation(g, up);
    Vector p2 = GetFarthestProjectionPointAfterRotation(g, right);
    Vector p3 = GetFarthestProjectionPointAfterRotation(g, down);
    Vector p4 = GetFarthestProjectionPointAfterRotation(g, left);
    wall.maxx =Max(p1.x, p2.x, p3.x, p4.x);
    wall.maxy = Max(p1.y, p2.y, p3.y, p4.y);
    wall.minx = Min(p1.x, p2.x, p3.x, p4.x);
    wall.miny = Min(p1.y, p2.y, p3.y, p4.y);

}
