#pragma once
#include <Shared/Entity.hh>
#include <Helpers/Collision/Geometry.hh>
Vector GetFarthestProjectionPointAfterTransform(const Geometry&,Vector);
Vector Support(const Geometry&,const Geometry&,Vector);

bool IsCircleOverlapsWithCircle(const Geometry&,const Geometry&,Vector&, float&);
bool IsPolygonOverlapsWithCircle(const Geometry &c,const Geometry &p);