#pragma once
#include <Helpers/Collision/GJK/Simplex.hh>
#include <Helpers/Collision/Geometry.hh>
void CheckPenetration(const Simplex&,const Geometry&,const Geometry&,Vector&,float&);
