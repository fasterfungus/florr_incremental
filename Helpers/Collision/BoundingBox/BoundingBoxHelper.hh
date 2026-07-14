#pragma once
#include <Shared/Entity.hh>
void Set(Entity & e);
bool BoundingBoxContains(const Entity& e, const Vector& pt);
bool Overlaps(const Entity& e1, const Entity& e2);
