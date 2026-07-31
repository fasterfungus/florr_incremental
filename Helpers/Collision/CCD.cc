#include <Helpers/Collision/CCD.hh>

#include <algorithm>
#include <cmath>
#include <limits>

namespace CCD
{
    // --- Internal helpers -------------------------------------------------------

    // Solve a*t^2 + b*t + c = 0, write the smallest root in [tmin,tmax] to `t`.
    // Returns false if no such root exists.
    static bool solve_quadratic(float a, float b, float c, float tmin, float tmax, float& t)
    {
        if (std::fabs(a) < 1e-8f)
        {
            // Degenerate: linear.
            if (std::fabs(b) < 1e-8f) return false;
            float r = -c / b;
            if (r < tmin || r > tmax) return false;
            t = r;
            return true;
        }
        float disc = b * b - 4.0f * a * c;
        if (disc < 0.0f) return false;
        float sq = std::sqrt(disc);
        float t0 = (-b - sq) / (2.0f * a);
        float t1 = (-b + sq) / (2.0f * a);
        // Prefer the smaller root first (first contact).
        if (t0 > t1) std::swap(t0, t1);
        if (t0 >= tmin && t0 <= tmax)
        {
            t = t0;
            return true;
        }
        if (t1 >= tmin && t1 <= tmax)
        {
            t = t1;
            return true;
        }
        return false;
    }

    // Sweep a circle center along the ray start+t*dir against a point P (radius r).
    // Writes the first t in [0,1] and returns true if hit.
    static bool sweep_ray_vs_circle(Vector start, Vector dir, Vector P, float r, float& t)
    {
        // |start + t*dir - P|^2 = r^2
        float dx = start.x - P.x, dy = start.y - P.y;
        float a = dir.x * dir.x + dir.y * dir.y;
        float b = 2.0f * (dx * dir.x + dy * dir.y);
        float c = dx * dx + dy * dy - r * r;
        return solve_quadratic(a, b, c, 0.0f, 1.0f, t);
    }

    // Sweep circle of radius `r` moving `start+t*dir` against the INFINITE line
    // through seg_a→seg_b (the side face of the Minkowski sum). Writes t if the
    // circle center hits the line at distance r and is clamped to the segment.
    // Returns true on hit. The surface normal is the segment's outward side normal
    // (perpendicular to the segment, pointing toward the initial position of start).
    static bool sweep_side(Vector start, Vector dir, float r,
                           Vector seg_a, Vector seg_b, float& t_out, Vector& n_out)
    {
        // Segment direction and perpendicular.
        Vector seg = seg_b - seg_a;
        float seg_len = std::sqrt(seg.x * seg.x + seg.y * seg.y);
        if (seg_len < 1e-8f) return false;
        Vector seg_hat(seg.x / seg_len, seg.y / seg_len);
        // Outward normal (two sides; pick the one facing start).
        Vector n0(-seg_hat.y, seg_hat.x); // left side normal
        float side_sign = (start.x - seg_a.x) * n0.x + (start.y - seg_a.y) * n0.y;
        Vector n = (side_sign >= 0.0f) ? n0 : Vector(-n0.x, -n0.y);

        // Project circle center onto segment axis.
        // The circle touches the infinite line when dot(start+t*dir-seg_a, n) = r.
        float d0 = (start.x - seg_a.x) * n.x + (start.y - seg_a.y) * n.y;
        float dv = dir.x * n.x + dir.y * n.y;
        // Already beyond or moving away.
        if (std::fabs(dv) < 1e-8f) return false;
        float t = (r - d0) / dv; // linear (side is a flat face)
        if (t < 0.0f || t > 1.0f) return false;

        // Check that the contact point is within the segment extent.
        Vector contact(start.x + t * dir.x, start.y + t * dir.y);
        float along = (contact.x - seg_a.x) * seg_hat.x + (contact.y - seg_a.y) * seg_hat.y;
        if (along < 0.0f || along > seg_len) return false;

        t_out = t;
        n_out = n;
        return true;
    }

    SweepHit swept_circle_segment(Vector start, Vector dir, float radius,
                                  Vector seg_a, Vector seg_b)
    {
        SweepHit result{false, 1.0f, Vector(0, 0)};

        float best_t = 1.0f;
        Vector best_n(0, 0);
        bool hit = false;

        // --- Side faces ---
        {
            float t;
            Vector n;
            if (sweep_side(start, dir, radius, seg_a, seg_b, t, n))
            {
                if (t < best_t)
                {
                    best_t = t;
                    best_n = n;
                    hit = true;
                }
            }
        }
        // Other side (mirror: swap seg_a/seg_b to flip the normal selection).
        {
            float t;
            Vector n;
            if (sweep_side(start, dir, radius, seg_b, seg_a, t, n))
            {
                if (t < best_t)
                {
                    best_t = t;
                    best_n = n;
                    hit = true;
                }
            }
        }

        // --- End caps ---
        for (int i = 0; i < 2; ++i)
        {
            Vector P = (i == 0) ? seg_a : seg_b;
            float t;
            if (sweep_ray_vs_circle(start, dir, P, radius, t))
            {
                if (t < best_t)
                {
                    // Normal = direction from cap center to contact point.
                    Vector cp(start.x + t * dir.x - P.x, start.y + t * dir.y - P.y);
                    float len = std::sqrt(cp.x * cp.x + cp.y * cp.y);
                    if (len > 1e-8f) { best_n = Vector(cp.x / len, cp.y / len); }
                    best_t = t;
                    hit = true;
                }
            }
        }

        if (hit)
        {
            result.hit = true;
            result.t = best_t;
            result.normal = best_n;
        }
        return result;
    }
}
