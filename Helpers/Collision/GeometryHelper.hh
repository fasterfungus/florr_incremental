#pragma once
#include <Shared/Entity.hh>
Vector GetFarthestProjectionPointAfterTransform(const Entity &e, Vector dir);
Vector Support(const Entity &e1, const Entity &e2, Vector dir);