
#include <Client/Render/RenderEntity.hh>

#include <Client/Game.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Render/Renderer.hh>

#include <Shared/Entity.hh>
#include <Shared/StaticData.hh>
void render_mob_health(Renderer &ctx, Entity const &ent) {
    if (!ent.has_component(kMob) && ent.healthbar_opacity < 0.01) return;
    float w = ent.get_radius() *ent.get_scale()* 1.33;
    ctx.set_global_alpha((1 - ent.deletion_animation) * (ent.has_component(kMob) && !ent.has_component(kSegmented) ? 1 : ent.healthbar_opacity));
    ctx.scale(1 + 0.5 * ent.deletion_animation);
    ctx.translate(-w, w + 15);
    ctx.round_line_cap();
    ctx.set_stroke(0xff222222);
    ctx.set_line_width(9);
    ctx.begin_path();
    ctx.move_to(0, 0);
    ctx.line_to(2 * w, 0);
    ctx.stroke();
    if (ent.healthbar_lag > ent.get_health_ratio()) {
        ctx.set_stroke(0xffed2f31);
        ctx.set_line_width(7);
        ctx.begin_path();
        ctx.move_to(2 * w * ent.get_health_ratio(), 0);
        ctx.line_to(2 * w * ent.healthbar_lag, 0);
        ctx.stroke();
    }
    ctx.set_stroke(0xff75dd34);
    ctx.set_line_width(7);
    ctx.begin_path();
    ctx.move_to(0, 0);
    ctx.line_to(2 * w * ent.get_health_ratio(), 0);
    ctx.stroke();
    if (ent.has_component(kMob)) {
        float name_width = 12 * Renderer::get_ascii_text_size(ent.get_name().c_str());
        ctx.translate(-(9 / 2) + name_width / 2, -10);
        ctx.draw_text(ent.get_name().c_str(), { .size = 12 });
        float rarity_width = 12 * Renderer::get_ascii_text_size(RARITY_NAMES[MOB_DATA[ent.get_mob_id()].rarity]);
        ctx.translate(-(-(9 / 2) + name_width / 2) + 2 * w + (9 / 2) - rarity_width / 2, 20);
        ctx.draw_text(RARITY_NAMES[MOB_DATA[ent.get_mob_id()].rarity], { .fill = RARITY_COLORS[MOB_DATA[ent.get_mob_id()].rarity], .size = 12 });
    }
}