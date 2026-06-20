#pragma once

class Entity;
class Renderer;

void render_drop(Renderer &, Entity const &);
void render_flower(Renderer &, Entity const &);
void render_flower_health(Renderer &, Entity const &);
void render_mob_health(Renderer &, Entity const &);
void render_mob(Renderer &, Entity const &);
void render_name(Renderer &, Entity const &);
void render_petal(Renderer &, Entity const &);
void render_web(Renderer &, Entity const &);
void render_collision(Renderer &, Entity const &);