#pragma once

#include <cstdint>
#include <string>

#ifndef M_PI
constexpr double M_PI = 3.14159265358979323846;
#endif

#ifndef EPSILON
constexpr float EPSILON = 0.000001f;
#endif
#ifndef DEG2RAD
constexpr float DEG2RAD = (float) M_PI / 180.0f;
#endif
#ifndef RAD2DEG
constexpr float RAD2DEG = 180.0f / (float) M_PI;
#endif
constexpr uint32_t div_round_up(uint32_t a, uint32_t b) { return (a + b - 1) / b; }

// True if x is NOT NaN and NOT ±Inf. Uses a bit check on the IEEE-754 exponent
// field so it works even under -ffast-math / -ffinite-math-only, where the
// compiler is allowed to assume every float is finite and optimizes
// std::isfinite(x) into a constant `true`, silently defeating NaN guards.
static inline bool is_finite(float x) {
    uint32_t bits;
    __builtin_memcpy(&bits, &x, sizeof(bits));
    return (bits & 0x7f800000u) != 0x7f800000u;
}
// double-precision variant (for values that may be double-promoted).
static inline bool is_finite(double x) {
    uint64_t bits;
    __builtin_memcpy(&bits, &x, sizeof(bits));
    return (bits & 0x7ff0000000000000ull) != 0x7ff0000000000000ull;
}

double frand();
float fclamp(float, float, float);
float lerp(float, float, float);
float angle_lerp(float, float, float);
float normalize_angle(float);
float angle_within(float, float, float);

class LerpFloat {
    float value;
    float lerp_value;
    uint32_t touched;
public:
    LerpFloat();
    void operator=(float);
    void set(float);
    operator float() const;
    void step(float);
    void step_angle(float);
    float anchor() const;
};

class SeedGenerator {
    uint32_t seed;
public:
    SeedGenerator(uint32_t);
    float next();
    float binext();
};

class RangeValue {
public:
    float lower;
    float upper;
    RangeValue(float, float);
    RangeValue(float);
    float get_single(float) const;
    std::string const to_string() const;
};

std::string format_pct(float);
std::string format_score(float);
std::string format_number(float);
float Max(float, float);
float Max(float, float, float); 
float Max(float, float, float, float);
float Min(float, float);
float Min(float, float, float);
float Min(float, float, float, float);
float Sqrt(float);
float Clamp(float, float, float);
bool Equals(float a, float b);
bool Sign(float value);
namespace ConstexprMath {

constexpr double LN2 =
    0.693147180559945309417232121458176568;

constexpr double log(double x) {
    if (x <= 0.0)
        return -1.0e300;

    int exponent = 0;

    while (x >= 2.0) {
        x *= 0.5;
        ++exponent;
    }

    while (x < 0.5) {
        x *= 2.0;
        --exponent;
    }

    const double z =
        (x - 1.0) / (x + 1.0);

    const double z2 = z * z;

    double term = z;
    double result = 0.0;

    for (int n = 1; n <= 99; n += 2) {
        result +=
            term / static_cast<double>(n);

        term *= z2;
    }

    return
        2.0 * result +
        static_cast<double>(exponent) * LN2;
}


constexpr double log1p(double x) {
    if (x <= -1.0)
        return -1.0e300;

    if (x > -0.25 && x < 0.25) {
        double term = x;
        double result = 0.0;

        for (int n = 1; n <= 99; ++n) {
            double value =
                term / static_cast<double>(n);

            if (n & 1)
                result += value;
            else
                result -= value;

            term *= x;
        }

        return result;
    }

    return log(1.0 + x);
}


constexpr double exp(double x) {
    if (x <= -745.0)
        return 0.0;

    if (x >= 709.0)
        return 1.0e300;

    int exponent =
        static_cast<int>(x / LN2);

    double remainder =
        x -
        static_cast<double>(exponent) * LN2;

    double result = 1.0;
    double term = 1.0;

    for (int i = 1; i <= 50; ++i) {
        term *=
            remainder /
            static_cast<double>(i);

        result += term;
    }

    if (exponent > 0) {
        for (int i = 0; i < exponent; ++i)
            result *= 2.0;
    }
    else {
        for (int i = 0; i > exponent; --i)
            result *= 0.5;
    }

    return result;
}


constexpr double expm1(double x) {
    if (x > -0.25 && x < 0.25) {
        double term = x;
        double result = x;

        for (int i = 2; i <= 50; ++i) {
            term *=
                x / static_cast<double>(i);

            result += term;
        }

        return result;
    }

    return exp(x) - 1.0;
}

}