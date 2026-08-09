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