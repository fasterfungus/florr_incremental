
#include <Helpers/Vector.hh>
#include <Helpers/Math.hh>
#include "Simplex.hh"

void Simplex::Add(const Vector& pt)
{
    vertics.push_back(pt);
}

// 检查原点是否在单纯形内，同时生成新方向
bool Simplex::Check(Vector& dir)
{
    if (vertics.size() == 2)
    {
        Vector a = vertics[1]; // 最新点
        Vector b = vertics[0]; // 次新点
        Vector ab = b - a;
        Vector ao = -a;
        dir = Vector::Mul3(ab, ao, ab);
        if (dir.magnitude2() < EPSILON)
        {
            dir = ab.Perpendicular();
        }
        return false;
    }
    else if (vertics.size() == 3)
    {
        Vector a = vertics[2]; // 最新
        Vector b = vertics[1]; // 次新
        Vector c = vertics[0]; // 最旧
        Vector ab = b - a;
        Vector ac = c - a;
        Vector ao = -a;

        Vector v = Vector::Mul3(ab, ac, ac);
        if (Vector::Dot(v, ao) >= 0.0f)
        {
            dir = v;
        }
        else
        {
            Vector u = Vector::Mul3(ac, ab, ab);
            if (Vector::Dot(u, ao) < 0.0f)
            {
                return true; // 原点在三角形内
            }
            // 原点在 AB 外侧，保留 A 和 B ，删除C → 单纯形变为 {B, A}
            vertics[0] = b;
            dir = u;
        }
        vertics[1] = a;
        vertics.pop_back();
    }
    return false; // 0点或1点不会调用Check
}
