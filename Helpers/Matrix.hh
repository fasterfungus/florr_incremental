#pragma once
#include "Vector.hh"
#include <cmath>

class Matrix
{
public:
    float m11;
    float m12;
    float m13;
    float m21;
    float m22;
    float m23;
    float m31;
    float m32;
    float m33;

    Matrix(float m11, float m12, float m13,
           float m21, float m22, float m23,
           float m31, float m32, float m33);
    Matrix(const Matrix& matrix);
    Matrix operator*(const Matrix& m);
    static Matrix CreateTranslationMatrix(const Vector& p);
    static Matrix CreateTranslationMatrix(float x, float y);
    static Matrix CreateRotationMatrix(float radian);
    static Vector Transform(const Vector& vector, const Matrix& matrix);
};
