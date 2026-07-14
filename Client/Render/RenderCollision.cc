#include <Client/Render/RenderEntity.hh>
#include <Shared/Entity.hh>
#include <Client/Render/Renderer.hh>
void render_collision(Renderer &ctx, Entity const & ent)
{
    CollisionShape shape = static_cast<CollisionShape>(ent.get_shape());
    if (shape == CollisionShape::kCapsule)
    {
        ctx.set_stroke(0x80ff0000);
        ctx.set_line_width(1);
        ctx.begin_path();
        ctx.partial_arc(ent.get_length()/2, 0, ent.get_radius(), -M_PI/2, M_PI/2, 0 );
        ctx.line_to(-ent.get_length()/2, ent.get_radius());
        ctx.partial_arc(-ent.get_length()/2, 0, ent.get_radius(), M_PI/2, -M_PI/2, 0);
        ctx.line_to(ent.get_length()/2, -ent.get_radius());
        ctx.stroke();

    }
    if (shape == CollisionShape::kCircle)
    {
        ctx.set_stroke(0x80ff0000);
        ctx.set_line_width(1);
        ctx.begin_path();
        ctx.arc(0,0,ent.get_radius());
        ctx.stroke();
    }
    if (shape == CollisionShape::kEllipse)
    {
        ctx.set_stroke(0x80ff0000);
        ctx.set_line_width(1);
        ctx.begin_path();
        ctx.ellipse(0,0,ent.get_width()/2,ent.get_height()/2);
        ctx.stroke();
    }
    if (shape == CollisionShape::kRectangle)
    {
        ctx.set_stroke(0x80ff0000);
        ctx.set_line_width(1);
        ctx.begin_path();
        ctx.move_to(-ent.get_width()/2,ent.get_height()/2);
        ctx.line_to(ent.get_width()/2,ent.get_height()/2);
        ctx.line_to(ent.get_width()/2,-ent.get_height()/2);
        ctx.line_to(-ent.get_width()/2,-ent.get_height()/2);
        ctx.line_to(-ent.get_width()/2,ent.get_height()/2);
        ctx.stroke();
    }
    if (shape == CollisionShape::kSegment) {
        ctx.set_stroke(0x80ff0000);
        ctx.set_line_width(1);
        ctx.begin_path();
        ctx.move_to(-ent.get_length() * 0.5f, 0.0f);
        ctx.line_to( ent.get_length() * 0.5f, 0.0f);
        ctx.stroke();
    }
}