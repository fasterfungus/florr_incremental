#pragma once
#include <Shared/Entity.hh>
#include <Helpers/Math.hh>
float GetDistance2(const Vector& pa, const Vector& pb, const Vector& pt) ;
bool Contains(const Entity& ent, const Vector& pt);
Vector GetFarthestProjectionPoint(const Entity& ent, const Vector& dir);
