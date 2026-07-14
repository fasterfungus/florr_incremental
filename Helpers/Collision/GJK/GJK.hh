#pragma once
#include <Shared/Entity.hh>
#include <Helpers/Matrix.hh>
#include <Helpers/Collision/GJK/EPA.hh>
bool Detect(Simplex&, const Geometry&, const Geometry&, const Vector&);
bool Detect(const Geometry&, const Geometry&, Vector&, float&);

