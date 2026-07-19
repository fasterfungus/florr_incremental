
#include "BoundingBoxHelper.hh"
#include <Helpers/Collision/Geometry.hh>
#include <Helpers/Vector.hh>
#include <Shared/Entity.hh>

#include "Helpers/Collision/GeometryHelper.hh"

void Set(Entity& e)
{
    Vector p1 = GetFarthestProjectionPointAfterRotation(e, up);
    Vector p2 = GetFarthestProjectionPointAfterRotation(e, right);
    Vector p3 = GetFarthestProjectionPointAfterRotation(e, down);
    Vector p4 = GetFarthestProjectionPointAfterRotation(e, left);
    e.set_maxx(Max(p1.x, p2.x, p3.x, p4.x));
    e.set_maxy(Max(p1.y, p2.y, p3.y, p4.y));
    e.set_minx(Min(p1.x, p2.x, p3.x, p4.x));
    e.set_miny(Min(p1.y, p2.y, p3.y, p4.y));
}
