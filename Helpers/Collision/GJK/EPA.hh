#pragma once
#include <Helpers/Collision/GJK/Simplex.hh>
#include <Shared/Entity.hh>
void CheckPenetration(const Simplex &simplex, const Entity &e1, const Entity &e2, Vector &normal ,float &depth);