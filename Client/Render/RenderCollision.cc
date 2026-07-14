#include <Client/Render/RenderEntity.hh>
#include <Shared/Entity.hh>
#include <Client/Render/Renderer.hh>
void render_collision(Renderer &ctx, Entity const & ent)
{
    if (ent.get_shape() == 0)
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
    if (ent.get_shape() == 1)
    {
        ctx.set_stroke(0x80ff0000);
        ctx.set_line_width(1);
        ctx.begin_path();
        ctx.arc(0,0,ent.get_radius());
        ctx.stroke();
    }
    if (ent.get_shape() == 2)
    {
        ctx.set_stroke(0x80ff0000);
        ctx.set_line_width(1);
        ctx.begin_path();
        ctx.ellipse(0,0,ent.get_width()/2,ent.get_height()/2);
        ctx.stroke();
    }
    if (ent.get_shape() == 5)
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
    if (ent.get_shape() == 6) {
        ctx.set_stroke(0x80ff0000);
        ctx.set_line_width(1);
        ctx.begin_path();
        ctx.move_to(-ent.get_length() * 0.5f, 0.0f);
        ctx.line_to( ent.get_length() * 0.5f, 0.0f);
        ctx.stroke();
    }
}