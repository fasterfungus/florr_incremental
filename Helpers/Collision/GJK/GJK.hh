#pragma once
#include <Shared/Entity.hh>
#include <Helpers/Matrix.hh>
#include <Helpers/Collision/GJK/EPA.hh>
bool Detect(Simplex &simplex, const Entity &e1, const Entity &e2, const Vector &dir);
bool Detect(const Entity &e1, const Entity &e2,Vector &normal, float &depth);

