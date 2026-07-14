#include <Helpers/Collision/GJK/Simplex.hh>
#include <Helpers/Collision/GJK/ExpandingSimplex.hh>
#include <Helpers/Collision/GeometryHelper.hh>
#include <Shared/Entity.hh>
#include "EPA.hh"
void CheckPenetration(const Simplex &simplex, const Geometry &g1, const Geometry &g2, Vector &normal ,float &depth)
        {
            ExpandingSimplex expandingSimplex(simplex);
            Edge edge = expandingSimplex.GetClosestEdge();
            Vector point(0,0);

            for (uint8_t i = 0; i < 50; i++)
            {
                edge = expandingSimplex.GetClosestEdge();
                point = Support(g1,g2,edge.normal);
                float projection = Vector::Dot(point, edge.normal);
                if (projection - edge.distance < EPSILON)
                {
                    normal.x = edge.normal.x;
                    normal.y = edge.normal.y;
                    depth = projection;

                    return;
                }

                expandingSimplex.Expand(point);
            }

            normal.x = edge.normal.x;
            normal.y = edge.normal.y;
            depth = Vector::Dot(point, edge.normal);
        }