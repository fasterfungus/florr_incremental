#include <Client/Game.hh>

#include <Client/Input.hh>
#include <Client/Particle.hh>

#include <Client/Ui/Extern.hh>
#include <Client/Render/RenderEntity.hh>
#include <Client/Render/Map/GardenTileLayers.hh>
#include <Helpers/Vector.hh>

#include <Shared/Map.hh>
#include <Shared/StaticData.hh>

#include <cmath>

void _apply_damage_filter(Renderer &ctx, Entity const &ent) {
    ctx.set_global_alpha(1 - ent.deletion_animation);
    ctx.scale(1 + 0.5 * ent.deletion_animation);
    if (ent.damage_flash > 0.66)
        ctx.add_color_filter(0xffffffff, ent.damage_flash);
    else if (ent.damage_flash > 0.1)
        ctx.add_color_filter(0xffff1200, ent.damage_flash * 1.5);
}

static float screen_shake_radius = 0;

void Game::render_game() {
    RenderContext context(&renderer);
    DEBUG_ONLY(assert(simulation.ent_exists(camera_id));)
    Entity const &camera = simulation.get_ent(camera_id);
    renderer.translate(renderer.width / 2, renderer.height / 2);
    renderer.scale(Ui::scale * camera.get_fov());
    renderer.translate(-camera.get_camera_x(), -camera.get_camera_y());
    if (alive()) {
        Entity const &player = simulation.get_ent(player_id);
        if (Game::timestamp - player.last_damaged_time < 150) {
            screen_shake_radius = lerp(screen_shake_radius, 5, Ui::lerp_amount);
        } else {
            screen_shake_radius = lerp(screen_shake_radius, 0, 2 * Ui::lerp_amount);
        }
        if (screen_shake_radius > 0.01) {
            Vector rand = Vector::rand(std::sqrt(frand()) * screen_shake_radius);
            renderer.translate(rand.x, rand.y);
        }
    }
    uint32_t alpha = (uint32_t)(camera.get_fov() * 255 * 0.2) << 24;
    {
        RenderContext context(&renderer);
        renderer.reset_transform();
        renderer.set_fill(0xff987d72);
        renderer.fill_rect(0,0,renderer.width,renderer.height);
        renderer.set_fill(alpha);
        renderer.fill_rect(0,0,renderer.width,renderer.height);
    }
    {
                RenderContext context(&renderer);
        float scale = 1 / (2 * camera.get_fov() * Game::scale);
        float leftX = camera.get_camera_x() - renderer.width * scale;
        float rightX = camera.get_camera_x() + renderer.width * scale;
        float topY = camera.get_camera_y() - renderer.height * scale;
        float bottomY = camera.get_camera_y() + renderer.height * scale;
        // External image slots are global; re-preload whenever the active map set changes.
        static int8_t tiles_for_gm = -1;
        if (Game::map == (uint8_t)Maps::kGarden)
        {
            // Chunk-baked terrain: 1 chunk per tile.
            float const ts = static_cast<float>(GardenTileLayers::kTileSize);
            int const gw = static_cast<int>(GardenTileLayers::kGridW);
            int const gh = static_cast<int>(GardenTileLayers::kGridH);
            int const subdiv = 1;
            float const sub_ts = ts / static_cast<float>(subdiv);
            int const sx0 = std::max(0, static_cast<int>(std::floor(leftX / sub_ts)) - 1);
            int const sy0 = std::max(0, static_cast<int>(std::floor(topY / sub_ts)) - 1);
            int const sx1 = std::min(gw * subdiv - 1, static_cast<int>(std::ceil(rightX / sub_ts)) + 1);
            int const sy1 = std::min(gh * subdiv - 1, static_cast<int>(std::ceil(bottomY / sub_ts)) + 1);
            //uint32_t const grass_col = GardenTerrain::color_of(GardenTerrain::kGrass);
            uint32_t const grass_col = 0;
            auto garden_tile = +[](uint32_t layer, int tx, int ty, uint8_t* img_i, uint8_t* flags, void*) -> int
            {
                uint8_t const pal = GardenTileLayers::at_layer_tile(layer, tx, ty);
                if (pal == 0 || pal >= GardenTileLayers::kPaletteSize) return 0;
                uint8_t const ii = GardenTileLayers::kPalImage[pal];
                if (ii >= GardenTileLayers::kImageCount) return 0;
                *img_i = ii;
                *flags = GardenTileLayers::kPalFlags[pal];
                return 1;
            };

            for (int sy = sy0; sy <= sy1; ++sy)
            {
                for (int sx = sx0; sx <= sx1; ++sx)
                {
                    int const ok = renderer.ensure_and_draw_terrain_chunk(
                        sx, sy, subdiv, ts, gw, gh,
                        GardenTileLayers::kLayerCount, grass_col,
                        garden_tile, nullptr);
                    if (!ok)
                    {
                        float const wx = sx * sub_ts;
                        float const wy = sy * sub_ts;
                        renderer.set_fill(grass_col);
                        renderer.fill_rect(wx, wy, sub_ts + 1.f, sub_ts + 1.f);
                        int const tx = sx / subdiv;
                        int const ty = sy / subdiv;
                        for (uint32_t layer = 0; layer < GardenTileLayers::kLayerCount; ++layer)
                        {
                            uint8_t img_i = 0, flags = 0;
                            if (!garden_tile(layer, tx, ty, &img_i, &flags, nullptr)) continue;
                            renderer.draw_external_image(
                                img_i, tx * ts, ty * ts, ts + 0.5f, ts + 0.5f, flags);
                        }
                    }
                }
            }
            // Object-layer decorations (entrances, etc.).
            /*
            for (uint32_t i = 0; i < GardenTileLayers::kActiveDecorCount; ++i)
            {
                GardenTileLayers::Decor const& d = GardenTileLayers::kDecors[i];
                if (d.x + d.w < leftX || d.x > rightX ||
                    d.y + d.h < topY || d.y > bottomY)
                    continue;
                if (d.image >= GardenTileLayers::kImageCount) continue;
                renderer.draw_external_image(d.image, d.x, d.y, d.w, d.h, d.flags);
            }
            */
        }
    }

    if (alive() && Input::movement_helper && !Input::keyboard_movement && !Input::is_mobile) {
        Entity const &player = simulation.get_ent(player_id);
        float norm_mouse_x = (Input::mouse_x - renderer.width / 2) / Ui::scale;
        float norm_mouse_y = (Input::mouse_y - renderer.height / 2) / Ui::scale;
        Vector delta{norm_mouse_x, norm_mouse_y};
        float dist = delta.magnitude();
        if (dist >= player.get_radius() + 80) {
            RenderContext context(&renderer);
            renderer.reset_transform();
            renderer.translate(renderer.width/2,renderer.height/2);
            renderer.scale(Ui::scale);
            uint8_t alpha = (uint8_t) (fclamp((dist - player.get_radius() - 80) / 80, 0, 1) * 255 * 0.15);
            delta.set_magnitude(player.get_radius() + 40);
            renderer.set_line_width(18);
            renderer.round_line_cap();
            renderer.round_line_join();
            renderer.set_stroke(alpha << 24);
            renderer.begin_path();
            renderer.move_to(delta.x,delta.y);
            renderer.line_to(norm_mouse_x,norm_mouse_y);
            renderer.translate(norm_mouse_x,norm_mouse_y);
            renderer.rotate(delta.angle());
            renderer.rotate(2.5);
            renderer.move_to(0,0);
            renderer.line_to(40,0);
            renderer.rotate(-5);
            renderer.move_to(0,0);
            renderer.line_to(40,0);
            renderer.stroke();
        }
    }

    Particle::tick_game(renderer, Ui::dt);

    simulation.for_each<kWeb>([](Simulation *sim, Entity const &ent){
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        renderer.rotate(ent.get_angle());
        renderer.scale(ent.animation);
        render_web(renderer, ent);
        if (Game::show_collision)
        {
            render_collision(renderer, ent);
        }
    });
    simulation.for_each<kDrop>([](Simulation *sim, Entity const &ent){
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        renderer.rotate(ent.get_angle() + (ent.animation - 1) * 3 * M_PI);
        renderer.scale(ent.animation);
        render_drop(renderer, ent);
        if (Game::show_collision)
        {
            render_collision(renderer, ent);
        }
    });
    simulation.for_each<kHealth>([](Simulation *sim, Entity const &ent){
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        if (ent.has_component(kFlower))
        {
            render_flower_health(renderer, ent);
        }
        if (ent.has_component(kMob))
        {
            render_mob_health(renderer, ent);
        }
    });
    simulation.for_each<kPetal>([](Simulation *sim, Entity const &ent){
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        renderer.rotate(ent.get_angle());
        _apply_damage_filter(renderer, ent);
        render_petal(renderer, ent);
        if (Game::show_collision)
        {
            render_collision(renderer, ent);
        }
    });
    simulation.for_each<kMob>([](Simulation *sim, Entity const &ent){
        if (ent.get_mob_id() != MobID::kAntHole) return;
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        if (!ent.has_component(kFlower))
            renderer.rotate(ent.get_angle());
        _apply_damage_filter(renderer, ent);
        render_mob(renderer, ent);

    });
    simulation.for_each<kMob>([](Simulation *sim, Entity const &ent){
        if (ent.get_mob_id() == MobID::kAntHole) return;
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        if (!ent.has_component(kFlower))
            renderer.rotate(ent.get_angle());
        _apply_damage_filter(renderer, ent);
        render_mob(renderer, ent);
        if (Game::show_collision)
        {
            render_collision(renderer, ent);
        }
    });
    simulation.for_each<kFlower>([](Simulation *sim, Entity const &ent){
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        _apply_damage_filter(renderer, ent);
        render_flower(renderer, ent);
        if (Game::show_collision)
        {
            render_collision(renderer, ent);
        }
    });
    simulation.for_each<kName>([](Simulation *sim, Entity const &ent){
        RenderContext context(&renderer);
        renderer.translate(ent.get_x(), ent.get_y());
        render_name(renderer, ent);
    });
}

void Game::render_title_screen() {
    RenderContext context(&renderer);
    renderer.reset_transform();
    renderer.set_fill(0xff1ea761);
    renderer.fill_rect(0,0,renderer.width,renderer.height);
}