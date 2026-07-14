#include <algorithm>
#include "../Vector.hh"
#include "../Math.hh"
#include "../Matrix.hh"
#include <Shared/Entity.hh>
#include <Helpers/Collision/Geometry.hh>
#include <Helpers/Collision/GeometryHelper.hh>

Vector GetFarthestProjectionPointAfterTransform(const Geometry& g, Vector dir)
{
    float angle = g.angle;
    Matrix m1 = Matrix::CreateRotationMatrix(-angle);
    dir = Matrix::Transform(dir, m1);

    Vector pt = GetFarthestProjectionPoint(g, dir);
    Matrix m2 = Matrix::CreateRotationMatrix(angle);
    Vector p = Vector(g.x, g.y);
    Matrix m3 = Matrix::CreateTranslationMatrix(p);
    return Matrix::Transform(pt, m2 * m3); // 经检验无问题
}

Vector Support(const Geometry& g1, const Geometry& g2, Vector dir)
{
    Vector p1 = GetFarthestProjectionPointAfterTransform(g1, dir);
    dir.negative();
    Vector p2 = GetFarthestProjectionPointAfterTransform(g2, dir);
    return p1 - p2;
}
