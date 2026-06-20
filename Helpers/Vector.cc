#include <Helpers/Vector.hh>

#include <Helpers/Math.hh>

#include <cmath>
#include "Vector.hh"

Vector::Vector()
{
}

Vector::Vector(const Vector& o) : x(o.x), y(o.y)
{
}

Vector::Vector(float _x, float _y) : x(_x), y(_y)
{
}

Vector Vector::rand(float magnitude)
{
    float angle = frand() * 2 * M_PI;
    return {cosf(angle) * magnitude, sinf(angle) * magnitude};
}

void Vector::set(float _x, float _y)
{
    x = _x;
    y = _y;
}

Vector& Vector::operator=(Vector const& o)
{
    x = o.x;
    y = o.y;
    return *this;
}

Vector& Vector::operator+=(Vector const& o)
{
    x += o.x;
    y += o.y;
    return *this;
}

Vector& Vector::operator-=(Vector const& o)
{
    x -= o.x;
    y -= o.y;
    return *this;
}

Vector& Vector::operator*=(float v)
{
    x *= v;
    y *= v;
    return *this;
}

Vector Vector::operator+(Vector const& v) const
{
    return Vector{x + v.x, y + v.y};
}

Vector Vector::operator-(Vector const& v) const
{
    return Vector{x - v.x, y - v.y};
}

Vector Vector::operator*(float v) const
{
    return Vector{x * v, y * v};
}

Vector Vector::operator -() const
{
    return Vector(-x, -y);
}

float Vector::magnitude() const
{
    return sqrtf(magnitude2());
}

float Vector::magnitude2() const
{
    return x * x + y * y;
}

float Vector::angle() const
{
    if (x == 0 && y == 0) return 0;
    return atan2f(y, x);
}

void Vector::Normalize()
{
    float mag = magnitude();
    if (mag == 0.0f){return;}
    x /= mag;
    y /= mag;
}

const Vector Vector::normalized() const
{
    float mag = magnitude();
    if (mag == 0.0f){return Vector(0,0);}
    return Vector(x / mag, y / mag);
}

float Vector::Dot(const Vector& v) const
{
    return Dot(*this, v);
}

float Vector::Dot(const Vector& a, const Vector& b)
{
    return a.x * b.x + a.y * b.y;
}

Vector Vector::Mul3(const Vector& a, const Vector& b, const Vector& c)
{
    float ac = a.x * c.x + a.y * c.y; // a.dot(c)
    float bc = b.x * c.x + b.y * c.y; // b.dot(c)
    float x = b.x * ac - a.x * bc;
    float y = b.y * ac - a.y * bc;
    return Vector(x, y);
}

//static Vector Perpendicular(float x, float y) {
//    return Vector(y, -x);
//}
//static Vector Perpendicular(const Vector &v) {
//    return Perpendicular(v.x, v.y);
//}
Vector Vector::Perpendicular() const
{
    return Vector(y, -x);
}

void Vector::negative()
{
    x *= -1;
    y *= -1;
}

float Vector::Cross(const Vector& a, const Vector& b)
{
    return a.x * b.y - a.y * b.x;
}

void Vector::CW90()
{
    float temp = x;
    x = -y;
    y = temp;
}

void Vector::CCW90()
{
    float temp = x;
    x = y;
    y = -temp;
}

Vector& Vector::set_magnitude(float v)
{
    Normalize();
    x *= v;
    y *= v;
    return *this;
}

Vector& Vector::unit_normal(float a)
{
    x = cosf(a);
    y = sinf(a);
    return *this;
}
