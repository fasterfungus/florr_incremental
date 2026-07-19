#include <algorithm>
#include "../Vector.hh"
#include "../Math.hh"
#include "../Matrix.hh"
#include <Shared/Entity.hh>
#include <Helpers/Collision/Geometry.hh>
#include <Helpers/Collision/GeometryHelper.hh>

bool Contain(const Geometry& g,const Vector& pt)
{
    Matrix m1 = Matrix::CreateTranslationMatrix(-Vector(g.x,g.y));
    Matrix m2 = Matrix::CreateRotationMatrix(-g.rotation);
    Matrix mt = m1 * m2;
    Vector p  = Matrix::Transform(pt, mt);

    return Contains(g,p);
}
Vector GetFarthestProjectionPointAfterTransform(const Geometry& g, Vector dir)
{
    float rotation = g.rotation;
    Matrix m1 = Matrix::CreateRotationMatrix(-rotation);
    dir = Matrix::Transform(dir, m1);

    Vector pt = GetFarthestProjectionPoint(g, dir);
    Matrix m2 = Matrix::CreateRotationMatrix(rotation);
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
bool IsCircleOverlapsWithCircle(const Geometry& c1, const Geometry& c2,
                                Vector& normal, float& depth) {
    float dx = c2.x - c1.x, dy = c2.y - c1.y;
    float d = std::sqrt(dx * dx + dy * dy);
    float r = c1.radius + c2.radius;
    if (d >= r) return false;

    if (d < EPSILON) {
        normal = Vector(1.0f, 0.0f);
        depth = r;
    } else {
        normal = Vector(dx / d, dy / d);
        depth = r - d;
    }
    return true;
}
Vector GetFarthestProjectionPointAfterRotation(const Geometry& g, Vector dir)
{
    Matrix m1 = Matrix::CreateRotationMatrix(-g.rotation);
    dir = Matrix::Transform(dir, m1);
    Vector pt = GetFarthestProjectionPoint(g,dir);
    Matrix m2 = Matrix::CreateRotationMatrix(g.rotation);
    return Matrix::Transform(pt, m2);
}