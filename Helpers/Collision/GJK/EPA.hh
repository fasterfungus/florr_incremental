#pragma once
#include <Helpers/Collision/GJK/Simplex.hh>
#include <Helpers/Collision/Geometry.hh>
void CheckPenetration(const Simplex &simplex, const Geometry &g1, const Geometry &g2, Vector &normal ,float &depth);