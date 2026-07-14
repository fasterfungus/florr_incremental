#pragma once
#include <Shared/Entity.hh>
#include <Helpers/Collision/Geometry.hh>
Vector GetFarthestProjectionPointAfterTransform(const Geometry&, Vector);
Vector Support(const Geometry&, const Geometry&, Vector);
