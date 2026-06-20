#include <algorithm>
#include "../Vector.hh"
#include "../Math.hh"
#include "../Matrix.hh"
#include <Shared/Entity.hh>
#include <Helpers/Collision/Geometry.hh>
#include <Helpers/Collision/GeometryHelper.hh>
Vector GetFarthestProjectionPointAfterTransform(const Entity &e, Vector dir)
        {
            float angle = e.get_angle();
            Matrix m1 = Matrix::CreateRotationMatrix(-angle);
            dir = Matrix::Transform(dir, m1);

            Vector pt = GetFarthestProjectionPoint(e,dir);
            Matrix m2 = Matrix::CreateRotationMatrix(angle);
            Vector p = Vector(e.get_x(), e.get_y());
            Matrix m3 = Matrix::CreateTranslationMatrix(p);
            return Matrix::Transform(pt, m2 * m3); // 经检验无问题
        }
Vector Support(const Entity &e1, const Entity &e2, Vector dir)
{

    Vector p1 = GetFarthestProjectionPointAfterTransform(e1, dir);
    dir.negative();
    Vector p2 = GetFarthestProjectionPointAfterTransform(e2, dir);
    return p1 - p2;
    }