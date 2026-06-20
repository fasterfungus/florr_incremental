
#include "BoundingBoxHelper.hh"
#include <Helpers/Collision/Geometry.hh>
#include <Helpers/Vector.hh>
#include <Shared/Entity.hh>
void Set(Entity &e)
{
    Vector p1 = GetFarthestProjectionPoint(e, up);
    Vector p2 = GetFarthestProjectionPoint(e, right);
    Vector p3 = GetFarthestProjectionPoint(e,down);
    Vector p4 = GetFarthestProjectionPoint(e, left);
    e.set_maxx(Max(p1.x, p2.x, p3.x, p4.x));
    e.set_maxy(Max(p1.y, p2.y, p3.y, p4.y));
    e.set_minx(Min(p1.x, p2.x, p3.x, p4.x));
    e.set_miny(Min(p1.y, p2.y, p3.y, p4.y));
}
bool BoundingBoxContains(const Entity &e, const Vector &pt)
{
    float minx = e.get_minx() + e.get_x();
    float miny = e.get_miny() + e.get_y();
    float maxx = e.get_maxx() + e.get_x();
    float maxy = e.get_maxy() + e.get_y();

    return minx <= pt.x && maxx >= pt.x &&
           miny <= pt.y && maxy >= pt.y;
}
bool Overlaps(const Entity &e1,const Entity &e2)
{
    float minx1 = e1.get_minx() + e1.get_x();
    float miny1 = e1.get_miny() + e1.get_y();
    float maxx1 = e1.get_maxx() + e1.get_x();
    float maxy1 = e1.get_maxy() + e1.get_y();

    float minx2 = e2.get_minx() + e2.get_x();
    float miny2 = e2.get_miny() + e2.get_y();
    float maxx2 = e2.get_maxx() + e2.get_x();
    float maxy2 = e2.get_maxy() + e2.get_y();

    if (minx1 > maxx2 || maxx1 < minx2) return false;
    if (miny1 > maxy2 || maxy1 < miny2) return false;

    return true;
}