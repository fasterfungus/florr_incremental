#pragma once
class Vector {
public:

    float x;
    float y;
    Vector();
    Vector(const Vector &);
    Vector(float, float);
    static Vector rand(float);
    Vector &operator=(Vector const &);
    Vector &operator+=(Vector const &);
    Vector &operator-=(Vector const &);
    Vector &operator*=(float);
    Vector operator+(Vector const &) const;
    Vector operator-(Vector const &) const;
    Vector operator-() const;
    Vector operator*(float) const;
    void set(float, float);
    float magnitude() const;
    float magnitude2() const;
    float angle() const;
    void Normalize();
    const Vector normalized() const;
    float Dot(const Vector &) const;
    static float Dot(const Vector &, const Vector &);
    static Vector Mul3(const Vector &, const Vector &, const Vector &);
    //static Vector Perpendicular(float,float);
    //static Vector Perpendicular(const Vector &);
    Vector Perpendicular() const;
    static float Cross(const Vector &, const Vector &);
    void negative();
    void CW90();
    void CCW90();
    Vector &set_magnitude(float);
    Vector &unit_normal(float);
};
inline const Vector zero(0,0);
inline const Vector one(1,1);
inline const Vector left(-1,0);
inline const Vector right(1,0);
inline const Vector up(0,-1);
inline const Vector down(0,1);