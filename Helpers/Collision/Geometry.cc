
#include <Shared/Entity.hh>
#include <Helpers/Math.hh>
#include <cmath>
#include "Geometry.hh"

#include <vector>

Geometry::Geometry(const Entity& ent)
{
    shape = static_cast<CollisionShape>(ent.get_shape());
    scale = ent.get_scale();
    radius = ent.get_radius() * scale;
    width = ent.get_width() * scale;
    height = ent.get_height() * scale;
    length = ent.get_length() * scale;
    x = ent.get_x();
    y = ent.get_y();
    rotation = ent.get_angle();  //实体角度
    angle = ent.get_angle(); //扇形圆心角
    for (uint8_t i = 0; i < ent.get_vertics_size(); i++)
    {
        vertics.push_back(Vector(ent.get_vertics_x(i), ent.get_vertics_y(i)));
    }
}

Geometry::Geometry(const Wall& wall)
{
    length = wall.length;
    x = wall.x;
    y = wall.y;
    rotation = wall.rotation;
    shape = CollisionShape::kSegment;
}

float GetDistance2(Vector a, Vector b)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;

    return dx * dx + dy * dy;
}
float GetDistance2(const Vector& pa, const Vector& pb, const Vector& pt)
{
    float px = pt.x - pa.x, py = pt.y - pa.y;
    float xx = pb.x - pa.x, yy = pb.y - pa.y;
    float h = Clamp((px * xx + py * yy) / (xx * xx + yy * yy), 0.0f, 1.0f);
    float dx = px - xx * h, dy = py - yy * h;
    return dx * dx + dy * dy;
}


bool Contains(const Geometry& geometry, const Vector& pt){
        switch (geometry.shape) {
            case CollisionShape::kCircle:{
                return pt.magnitude2() <= geometry.radius * geometry.radius;
            }
            case CollisionShape::kRectangle:{
                float w = geometry.width * 0.5f;
                float h = geometry.height * 0.5f;
                if (pt.x < -w || pt.x > w) return false;
                if (pt.y < -h || pt.y > h) return false;
                return true;
            }
            case CollisionShape::kPolygon:{
                return false;
            }
            case CollisionShape::kEllipse:{
                return false;
            }
            case CollisionShape::kPie:{
                return false;
            }
            case CollisionShape::kSegment:{
                return false;
            }
            case CollisionShape::kCapsule:{
                Vector p1 = Vector(geometry.length *  0.5f, 0);
                Vector p2 = Vector(geometry.length * -0.5f, 0);
                float dist2 = GetDistance2(p1, p2, pt);
                float radius2 = geometry.radius * geometry.radius;
                return dist2 <= radius2;
            }
        }

}


Vector GetFarthestProjectionPoint(const Geometry& geometry, const Vector& dir)
{
    float A = geometry.width * 0.5f;
    float B = geometry.height * 0.5f;
    Vector point(0, 0);
    switch (geometry.shape)
    {
    case CollisionShape::kCircle:
        {
            return dir.normalized() * geometry.radius;
            break;
        }
    case CollisionShape::kRectangle:
        {
            float w = geometry.width * 0.5f;
            float h = geometry.height * 0.5f;
            Vector vertices[4] = {
                Vector(-w, -h),
                Vector(w, -h),
                Vector(w, h),
                Vector(-w, h)
            };
            Vector pt = vertices[0];
            float max = Vector::Dot(pt, dir);
            for (int i = 1; i < 4; i++)
            {
                float dot = Vector::Dot(vertices[i], dir);
                if (dot > max)
                {
                    max = dot;
                    pt = vertices[i];
                }
            }
            return pt;
            break;
        }
    case CollisionShape::kPolygon:
        {
            point = geometry.vertics[0];
            float max = Vector::Dot(point, dir);
            for (int i = 1; i < geometry.vertics.size(); i++)
            {
                Vector vertic = geometry.vertics[i];
                float dot = Vector::Dot(vertic, dir);
                if (dot > max)
                {
                    max = dot;
                    point = vertic;
                }
            }
            return point;
            break;
        }
    case CollisionShape::kEllipse:
        {
            float x = 0;
            float y = 0;

            if (Equals(dir.x, 0.0f))
            {
                float sign = dir.y < 0.0f ? -1 : 1;
                y = sign * B;
            }
            else if (Equals(dir.y, 0.0f))
            {
                float sign = dir.x < 0.0f ? -1 : 1;
                x = sign * A;
            }
            else
            {
                float k = dir.y / dir.x;

                float a2 = std::pow(A, 2);
                float b2 = std::pow(B, 2);
                float k2 = std::pow(k, 2);

                float t = std::sqrt((a2 + b2 * k2) / k2);
                Vector v = Vector(0.0f, t);
                if (Vector::Dot(v, dir) < 0)
                {
                    t *= -1;
                }

                x = k * t - (b2 * k2 * k * t) / (a2 + b2 * k2);
                y = (b2 * k2 * t) / (a2 + b2 * k2);
            }
            return Vector(x, y);
            break;
        }
    case CollisionShape::kPie:
        {
            return Vector(0, 0);
            break;
        }
    case CollisionShape::kSegment:
        {
            return (dir.x >= 0) ? Vector(geometry.length * 0.5f, 0) : Vector(geometry.length * -0.5f, 0);
            break;
        }
    case CollisionShape::kCapsule:
        {
            Vector p = dir.normalized() * geometry.radius;
            float x = dir.x >= 0 ? geometry.length * 0.5f : -geometry.length * 0.5f;
            p.x += x;
            return p;
            break;
        }
    }
}
