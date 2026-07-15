#include <Shared/Entity.hh>
#include <Helpers/Matrix.hh>
#include <Helpers/Collision/GJK/EPA.hh>
#include <Helpers/Collision/GeometryHelper.hh>
#include "GJK.hh"

#include "Helpers/Collision/Geometry.hh"

bool Detect(Simplex& simplex, const Geometry& g1, const Geometry& g2, const Vector& dir)
{
    Vector d(dir.x, dir.y);
    if (d.magnitude2() < EPSILON)
    {
        d = Vector(1, 0);
    }

    Vector pt = Support(g1, g2, d);
    simplex.Add(pt);

    if (pt.Dot(d) <= 0.0f)
    {
        return false;
    }

    d.negative();

    for (int i = 0; i < 100; i++) //最多执行100次，实际使用中不会达到这个次数
    {
        pt = Support(g1, g2, d);

        simplex.Add(pt);

        if (pt.Dot(d) <= 0.0f)
        {
            return false;
        }

        if (simplex.Check(d))
        {
            return true;
        }
    }

    return false;
}
bool SweepDetect(const Geometry& c1, const Geometry& s2, Vector& normal, float& depth)
{
        Vector dir = Vector(c1.x - s2.x, c1.y - s2.y);
        Simplex s1;
        float l = std::hypot(c1.x - c1.last_x, c1.y - c1.last_y);
        if (l < EPSILON) return false;
        float x = (c1.x + c1.last_x)/2;
        float y = (c1.y + c1.last_y)/2;
        Geometry g = Geometry(c1.radius,l,x,y,CollisionShape::kCapsule);
        if (Detect(s1, g, s2,dir))
        {
            CheckPenetration(s1, g, s2, normal, depth);
            normal.negative();
            return true;
        }
        return false;
}
bool Detect(const Geometry& g1, const Geometry& g2, Vector& normal, float& depth)
{
    Simplex simplex;
    Vector dir = Vector(g1.x - g2.x, g1.y - g2.y);
    if (g1.shape == CollisionShape::kCircle && g2.shape == CollisionShape::kCircle)
    {
        if (IsCircleOverlapsWithCircle(g1,g2,normal,depth))
        {
            normal.negative();
            return true;
        }
        return false;

    }
    if (Detect(simplex, g1, g2, dir))
    {
        CheckPenetration(simplex, g1, g2, normal, depth);
        normal.negative();
        return true;
    }

    if (g1.shape == CollisionShape::kCircle && g2.shape == CollisionShape::kSegment)
    {
        if (SweepDetect(g1, g2, normal, depth))return true;
    }
    if (g2.shape == CollisionShape::kCircle && g1.shape == CollisionShape::kSegment)
    {
        if (SweepDetect(g2, g1, normal, depth))return true;
    }

    return false;
};
