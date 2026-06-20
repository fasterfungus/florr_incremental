#include <Shared/Entity.hh>
#include <Helpers/Matrix.hh>
#include <Helpers/Collision/GJK/EPA.hh>
#include <Helpers/Collision/GeometryHelper.hh>
#include "GJK.hh"
bool Detect(Simplex &simplex, const Entity &e1, const Entity &e2, const Vector &dir)
        {
            Vector d(dir.x,dir.y);
            if (d.magnitude2() < EPSILON)
            {
                d = Vector(1, 0);
            }

            Vector pt = Support(e1,e2,d);
            simplex.Add(pt);

            if (pt.Dot(d) <= 0.0f)
            {
                return false;
            }

            d.negative();

            for (int i = 0; i < 100; i++) //最多执行100次，实际使用中不会达到这个次数
            {
                pt = Support(e1,e2,d);

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
bool Detect(const Entity &e1, const Entity &e2,Vector &normal, float &depth)
        {
            Simplex simplex;
            Vector dir = Vector(e1.get_x() - e2.get_x(), e1.get_y() - e2.get_y());

            if (Detect(simplex, e1, e2, dir))
            {
                CheckPenetration(simplex, e1, e2, normal, depth);
                normal.negative();
                return true;
            }

            return false;
        };