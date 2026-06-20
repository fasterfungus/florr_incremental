#include "Vector.hh"
#include <cmath>
#include "Matrix.hh"

Matrix::Matrix(float m11, float m12, float m13,
               float m21, float m22, float m23,
               float m31, float m32, float m33) : m11(m11), m12(m12), m13(m13), m21(m21), m22(m22), m23(m23), m31(m31),
                                                  m32(m32), m33(m33)
{
}


Matrix::Matrix(const Matrix& matrix) : m11(matrix.m11), m12(matrix.m12), m13(matrix.m13),
                                       m21(matrix.m21), m22(matrix.m22), m23(matrix.m23),
                                       m31(matrix.m31), m32(matrix.m32), m33(matrix.m33)
{
}

Matrix Matrix::operator*(const Matrix& m)
{
    float m11 = this->m11 * m.m11 + this->m12 * m.m21 + this->m13 * m.m31;
    float m12 = this->m11 * m.m12 + this->m12 * m.m22 + this->m13 * m.m32;
    float m13 = this->m11 * m.m13 + this->m12 * m.m23 + this->m13 * m.m33;
    float m21 = this->m21 * m.m11 + this->m22 * m.m21 + this->m23 * m.m31;
    float m22 = this->m21 * m.m12 + this->m22 * m.m22 + this->m23 * m.m32;
    float m23 = this->m21 * m.m13 + this->m22 * m.m23 + this->m23 * m.m33;
    float m31 = this->m31 * m.m11 + this->m32 * m.m21 + this->m33 * m.m31;
    float m32 = this->m31 * m.m12 + this->m32 * m.m22 + this->m33 * m.m32;
    float m33 = this->m31 * m.m13 + this->m32 * m.m23 + this->m33 * m.m33;
    Matrix matrix(m11, m12, m13,
                  m21, m22, m23,
                  m31, m32, m33);
    return matrix;
}

Matrix Matrix::CreateTranslationMatrix(float x, float y)
{
    Matrix matrix(1, 0, 0,
                  0, 1, 0,
                  x, y, 1);
    return matrix;
}

Matrix Matrix::CreateTranslationMatrix(const Vector& p)
{
    return CreateTranslationMatrix(p.x, p.y);
}


Matrix Matrix::CreateRotationMatrix(float radian)
{
    float s = std::sin(radian);
    float c = std::cos(radian);

    Matrix matrix(c, s, 0,
                  -s, c, 0,
                  0, 0, 1);
    return matrix;
}
Vector Matrix::Transform(const Vector& vector, const Matrix& matrix)
{
    float x = ((vector.x * matrix.m11) + (vector.y * matrix.m21)) + (matrix.m31);
    float y = ((vector.x * matrix.m12) + (vector.y * matrix.m22)) + (matrix.m32);
    //float z = ((vector.x * matrix.m13) + (vector.y * matrix.m23)) + (vector.w * matrix.m33);

    return Vector(x, y);
}
