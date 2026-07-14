#pragma once
#include <Helpers/Vector.hh>
#include <Helpers/Math.hh>

#include <vector>

class Simplex
{
public:
    std::vector<Vector> vertics;
    void Add(const Vector&);
    bool Check(Vector&);
};
