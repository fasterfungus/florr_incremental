#include <Client/Render/Renderer.hh>

#include <Helpers/Macros.hh>
#include <Helpers/Math.hh>
#include <Helpers/UTF8.hh>

#include <cmath>
#include <iostream>
#include <emscripten.h>

std::vector<Renderer *> Renderer::renderers;

RenderContext::RenderContext() {}

RenderContext::RenderContext(Renderer *r) {
    *this = r->context;
    renderer = r;
EM_ASM({
    Module.ctxs[$0].save();
}, r->id);
    //reset();
}

void RenderContext::reset() {
    amount = 0;
    color_filter = 0;
    clip_x = renderer->width / 2;
    clip_y = renderer->height / 2;
    //prevents premature unrendering
    clip_w = std::fmax(renderer->width, 10000.0);
    clip_h = std::fmax(renderer->height, 10000.0);
}

RenderContext::~RenderContext() {
    renderer->context = *this;
    EM_ASM({
        Module.ctxs[$0].restore();
    }, renderer->id);
}

Renderer::Renderer() : context() {
    id = EM_ASM_INT({
        let idx;
        if (Module.availableCtxs.length > 0)
            idx = Module.availableCtxs.pop();
        else
            idx = Module.ctxs.length;
        if (idx === 0) {
            Module.ctxs[idx] = document.getElementById('canvas').getContext('2d');
        } else {
            const canvas = new OffscreenCanvas(1,1);
            Module.ctxs[idx] = canvas.getContext('2d');
        }
        return idx;
    });
    DEBUG_ONLY(std::cout << "created canvas " << id << '\n';)
    Renderer::renderers.push_back(this);
    context.renderer = this;
    context.reset();
}

Renderer::~Renderer() {
    EM_ASM({
        if ($0 == 0)
            throw new Error('Tried to delete the main context');
        Module.ctxs[$0] = null;
        Module.availableCtxs.push($0);
    }, id);
    DEBUG_ONLY(std::cout << "removed canvas " << id << '\n';)
}

uint32_t Renderer::HSV(uint32_t c, float v) {
    return MIX(c >> 24 << 24, c, v);
}

uint32_t Renderer::MIX(uint32_t base, uint32_t mix, float v) {
    uint8_t b = fclamp((mix & 255) * v + (base & 255) * (1 - v), 0, 255);
    uint8_t g = fclamp(((mix >> 8) & 255) * v + ((base >> 8) & 255) * (1 - v), 0, 255);
    uint8_t r = fclamp(((mix >> 16) & 255) * v + ((base >> 16) & 255) * (1 - v), 0, 255);
    uint8_t a = base >> 24;
    return (a << 24) | (r << 16) | (g << 8) | b;
}

void Renderer::reset() {
    reset_transform();
    round_line_cap();
    round_line_join();
    center_text_align();
    center_text_baseline();
    context.reset();
}

void Renderer::set_dimensions(float w, float h) {
    width = w;
    height = h;
    EM_ASM({
        Module.ctxs[$0].canvas.width = $1;
        Module.ctxs[$0].canvas.height = $2;
    }, id, w, h);
}

void Renderer::add_color_filter(uint32_t c, float v) {
    context.color_filter = c;
    context.amount = v;
}

void Renderer::set_fill(uint32_t v) {
    v = MIX(v, context.color_filter, context.amount);
    EM_ASM({
    Module.ctxs[$0].fillStyle = "rgba("+$3+","+$2+","+$1+","+$4/255+")";
    }, id, v & 255, (v >> 8) & 255, (v >> 16) & 255, v >> 24);
}

void Renderer::set_stroke(uint32_t v) {
    v = MIX(v, context.color_filter, context.amount);
    EM_ASM({
    Module.ctxs[$0].strokeStyle = "rgba("+$3+","+$2+","+$1+","+$4/255+")";
    }, id, v & 255, (v >> 8) & 255, (v >> 16) & 255, v >> 24);
}

void Renderer::set_line_width(float v) {
    EM_ASM({
    Module.ctxs[$0].lineWidth = $1;
    }, id, v);
}

void Renderer::set_text_size(float v) {
    EM_ASM({
    Module.ctxs[$0].font = $1 + "px Ubuntu";
    }, id, v);
}

void Renderer::set_global_alpha(float v) {
    EM_ASM({
    Module.ctxs[$0].globalAlpha = $1;
    }, id, v);
}

void Renderer::round_line_cap() {
    EM_ASM({
    Module.ctxs[$0].lineCap = "round";
    }, id);
}

void Renderer::round_line_join() {
    EM_ASM({
    Module.ctxs[$0].lineJoin = "round";
    }, id);
}

void Renderer::center_text_align() {
    EM_ASM({
    Module.ctxs[$0].textAlign = "center";
    }, id);
}

void Renderer::center_text_baseline() {
    EM_ASM({
    Module.ctxs[$0].textBaseline = "middle";
    }, id);
}

static void update_transform(Renderer *r) {
EM_ASM({
    Module.ctxs[$0].setTransform($1, $2, $4, $5, $3, $6);
}, r->id, r->context.transform_matrix[0],r->context.transform_matrix[1],r->context.transform_matrix[2], 
r->context.transform_matrix[3],r->context.transform_matrix[4],r->context.transform_matrix[5]);
}

void Renderer::set_transform(float a, float b, float c, float d, float e, float f) {
    context.transform_matrix[0] = a;
    context.transform_matrix[1] = b;
    context.transform_matrix[2] = c;
    context.transform_matrix[3] = d;
    context.transform_matrix[4] = e;
    context.transform_matrix[5] = f;
    update_transform(this);
}

void Renderer::scale(float v) {
    context.transform_matrix[0] *= v;
    context.transform_matrix[1] *= v;
    context.transform_matrix[3] *= v;
    context.transform_matrix[4] *= v;
    update_transform(this);
}

void Renderer::scale(float x, float y) {
    context.transform_matrix[0] *= x;
    context.transform_matrix[1] *= x;
    context.transform_matrix[3] *= y;
    context.transform_matrix[4] *= y;
    update_transform(this);
}

void Renderer::translate(float x, float y) {
    context.transform_matrix[2] += x * context.transform_matrix[0] + y * context.transform_matrix[3];
    context.transform_matrix[5] += y * context.transform_matrix[4] + x * context.transform_matrix[1];
    update_transform(this);
}

void Renderer::rotate(float a) {
    float cos_a = cosf(a);
    float sin_a = sinf(a);
    float original0 = context.transform_matrix[0];
    float original1 = context.transform_matrix[1];
    float original3 = context.transform_matrix[3];
    float original4 = context.transform_matrix[4];
    context.transform_matrix[0] = original0 * cos_a + original1 * -sin_a;
    context.transform_matrix[1] = original0 * sin_a + original1 * cos_a;
    context.transform_matrix[3] = original3 * cos_a + original4 * -sin_a;
    context.transform_matrix[4] = original3 * sin_a + original4 * cos_a;
    update_transform(this);
}

void Renderer::reset_transform() {
    set_transform(1,0,0,0,1,0);
}

void Renderer::begin_path() {
    EM_ASM({
        Module.ctxs[$0].beginPath();
    }, id);
}

void Renderer::move_to(float x, float y) {
    EM_ASM({
        Module.ctxs[$0].moveTo($1, $2);
    }, id, x, y);
    }

void Renderer::line_to(float x, float y) {
    EM_ASM({
        Module.ctxs[$0].lineTo($1, $2);
    }, id, x, y);
}

void Renderer::qcurve_to(float x, float y, float x1, float y1) {
    EM_ASM({
        Module.ctxs[$0].quadraticCurveTo($1, $2, $3, $4);
    }, id, x, y, x1, y1);
}

void Renderer::bcurve_to(float x, float y, float x1, float y1, float x2, float y2) {
    EM_ASM({
        Module.ctxs[$0].bezierCurveTo($1, $2, $3, $4, $5, $6);
    }, id, x, y, x1, y1, x2, y2);
}


void Renderer::partial_arc(float x, float y, float r, float sa, float ea, uint8_t ccw) {
    EM_ASM({
        Module.ctxs[$0].arc($1, $2, $3, $4, $5, !!$6);
    }, id, x, y, r, sa, ea, ccw);
}

void Renderer::arc(float x, float y, float r) {
    partial_arc(x, y, r, 0, 2 * M_PI, 0);
}

void Renderer::reverse_arc(float x, float y, float r) {
    partial_arc(x, y, r, 0, 2 * M_PI, 1);
}

void Renderer::ellipse(float x, float y, float r1, float r2, float a) {
    EM_ASM({
        Module.ctxs[$0].ellipse($1, $2, $3, $4, $5, 2 * Math.PI, 0);
    }, id, x, y, r1, r2, a);
}

void Renderer::ellipse(float x, float y, float r1, float r2) {
    ellipse(x, y, r1, r2, 0);
}

void Renderer::fill_rect(float x, float y, float w, float h) {
    EM_ASM({
        Module.ctxs[$0].fillRect($1, $2, $3, $4);
    }, id, x, y, w, h);
}

void Renderer::stroke_rect(float x, float y, float w, float h) {
    EM_ASM({
        Module.ctxs[$0].strokeRect($1, $2, $3, $4);
    }, id, x, y, w, h);
}

void Renderer::rect(float x, float y, float w, float h) {
    EM_ASM({
        Module.ctxs[$0].rect($1, $2, $3, $4);
    }, id, x, y, w, h);
}

void Renderer::round_rect(float x, float y, float w, float h, float r) {
    move_to(x + r, y);
    line_to(x + w - r, y);
    qcurve_to(x + w, y, x + w, y + r);
    line_to(x + w, y + h - r);
    qcurve_to(x + w, y + h, x + w - r, y + h);
    line_to(x + r, y + h);
    qcurve_to(x, y + h, x, y + h - r);
    line_to(x, y + r);
    qcurve_to(x, y, x + r, y);
}

void Renderer::close_path() {
    EM_ASM({
        Module.ctxs[$0].closePath();
    }, id);
}

void Renderer::fill(uint8_t o) {
    EM_ASM({
        Module.ctxs[$0].fill($1 ? "nonzero" : "evenodd");
    }, id, o);
}

void Renderer::stroke() {
    EM_ASM({
        Module.ctxs[$0].stroke();
    }, id);
}

void Renderer::clip() {
    EM_ASM({
        Module.ctxs[$0].clip();
    }, id);
}

void Renderer::clip_rect(float x, float y, float w, float h) {
    //assumes axis oriented scaling
    context.clip_x = x * context.transform_matrix[0] + context.transform_matrix[2];
    context.clip_w = w * context.transform_matrix[0];
    context.clip_y = y * context.transform_matrix[4] + context.transform_matrix[5];
    context.clip_h = h * context.transform_matrix[4];
    begin_path();
    rect(x-w/2,y-h/2,w,h);
    clip();
}

void Renderer::draw_image(Renderer &ctx) {
    EM_ASM({
        Module.ctxs[$0].drawImage(Module.ctxs[$1].canvas, $2, $3);
    }, id, ctx.id, -ctx.width / 2, -ctx.height / 2);
}

void Renderer::fill_text(char const *text) {
    EM_ASM({
        Module.ctxs[$0].fillText(Module.TextDecoder.decode(HEAPU8.subarray($1, $1+$2)),0,0);
    }, id, text, std::strlen(text));
}

void Renderer::stroke_text(char const *text) {
    EM_ASM({
        Module.ctxs[$0].strokeText(Module.TextDecoder.decode(HEAPU8.subarray($1, $1+$2)),0,0);
    }, id, text, std::strlen(text));
}

void Renderer::draw_text(char const *text, struct TextArgs const args) {
    set_fill(args.fill);
    set_stroke(args.stroke);
    set_text_size(args.size);
    if (args.stroke_scale > 0) {
        set_line_width(args.size * args.stroke_scale);
        stroke_text(text);
    }
    fill_text(text);
}

float Renderer::get_text_size(char const *text) {
    return EM_ASM_DOUBLE({
        return Module.ctxs[$0].measureText(Module.TextDecoder.decode(HEAPU8.subarray($1, $1+$2)),0,0).width;
    }, id, text, std::strlen(text));
}

//precalculated ascii for standard Ubuntu font, only serves as an approximation (will fail for certain scripts)
constexpr float CHAR_WIDTHS[128] = {0,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0,0.24,0.24,0.24,0.24,0.24,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0,0.5,0.5,0.24,0.286,0.465,0.699,0.568,0.918,0.705,0.247,0.356,0.356,0.502,0.568,0.246,0.34,0.246,0.437,0.568,0.568,0.568,0.568,0.568,0.568,0.568,0.568,0.568,0.568,0.246,0.246,0.568,0.568,0.568,0.455,0.974,0.721,0.672,0.648,0.737,0.606,0.574,0.702,0.734,0.316,0.529,0.684,0.563,0.897,0.756,0.79,0.644,0.79,0.667,0.582,0.614,0.707,0.722,0.948,0.675,0.661,0.61,0.371,0.437,0.371,0.568,0.5,0.286,0.553,0.604,0.5,0.604,0.584,0.422,0.594,0.589,0.289,0.289,0.579,0.316,0.862,0.589,0.607,0.604,0.604,0.422,0.485,0.444,0.589,0.55,0.784,0.554,0.547,0.5,0.371,0.322,0.371,0.568,0.5};

float Renderer::get_ascii_text_size(char const *text) {
    float w = 0;
    UTF8Parser parser(text, std::strlen(text));
    while (1) {
        uint32_t c = parser.next_symbol();
        if (c == 0) break;
        if (c < 128)
            w += CHAR_WIDTHS[c];
        else
            w += 1;
    }
    return w;
}
EM_JS(void, gardn_terrain_init, (int mode), {
    if (!Module._terrainInit || Module._terrainMode !== mode) {
        Module._terrainInit = 1;
        Module._terrainMode = mode;
        Module._terrainChunks = new Map();
        Module._terrainBakeCtx = null;
    }
});

EM_JS(int, gardn_terrain_ready, (int key), {
    var e = Module._terrainChunks.get(key);
    return (e && e.ready) ? 1 : 0;
});

EM_JS(int, gardn_tile_img_ready, (int slot), {
    var slots = Module._extImgSlots;
    if (!slots) return 0;
    var img = slots[slot];
    if (!img) return 0;
    if (img._ready && img._canvas) return 1;
    if (img.complete && Module._bakeTile) {
        var s = Module._bakeTile(img);
        return s ? 1 : 0;
    }
    return 0;
});
EM_JS(void, gardn_terrain_begin, (int key, int w, int h, int r, int g, int b), {
    var cache = Module._terrainChunks;
    var entry = cache.get(key);
    if (!entry) {
        var canvas = document.createElement("canvas");
        canvas.width = w;
        canvas.height = h;
        var c2d = canvas.getContext("2d");
        c2d.imageSmoothingEnabled = true;
        if (c2d.imageSmoothingQuality !== undefined) c2d.imageSmoothingQuality = "medium";
        entry = { canvas: canvas, ctx: c2d, ready: 0 };
    } else {
        cache.delete(key);
        if (entry.canvas.width !== w || entry.canvas.height !== h) {
            entry.canvas.width = w;
            entry.canvas.height = h;
        }
    }
    cache.set(key, entry);
    if (cache.size > 800) {
        cache.delete(cache.keys().next().value);
    }
    var c = entry.ctx;
    c.setTransform(1, 0, 0, 1, 0, 0);
    c.clearRect(0, 0, w, h);
    c.fillStyle = "rgb(" + r + "," + g + "," + b + ")";
    c.fillRect(0, 0, w, h);
    entry.ready = 0;
    Module._terrainBakeCtx = c;
});

// Stamp one parent tile so sub-cell (sx,sy) of a div x div split fills this chunk canvas.
EM_JS(void, gardn_terrain_stamp_sub, (int slot, int dest_w, int fl, int sx, int sy, int div), {
    var c = Module._terrainBakeCtx;
    var slots = Module._extImgSlots;
    if (!c || !slots || div < 1) return;
    var img = slots[slot];
    if (!img) return;
    var src = (img._ready && img._canvas) ? img._canvas : null;
    if (!src && img.complete && Module._bakeTile) src = Module._bakeTile(img);
    if (!src) return;
    var full = dest_w * div;
    var ox = -sx * dest_w;
    var oy = -sy * dest_w;
    if (fl === 0) {
        try { c.drawImage(src, ox, oy, full, full); } catch (e) {}
        return;
    }
    var fh = (fl & 1) !== 0;
    var fv = (fl & 2) !== 0;
    var fd = (fl & 4) !== 0;
    c.save();
    c.translate(ox + full * 0.5, oy + full * 0.5);
    if (fd) {
        var t = fh;
        fh = fv;
        fv = !t;
        c.rotate(Math.PI * 0.5);
    }
    c.scale(fh ? -1 : 1, fv ? -1 : 1);
    try { c.drawImage(src, -full * 0.5, -full * 0.5, full, full); } catch (e) {}
    c.restore();
});

EM_JS(void, gardn_terrain_finish, (int key), {
    var e = Module._terrainChunks.get(key);
    if (e) e.ready = 1;
});

EM_JS(int, gardn_terrain_draw, (int ctxId, int key, float x, float y, float w, float h), {
    var ctx = Module.ctxs[ctxId];
    var e = Module._terrainChunks.get(key);
    if (!ctx || !e || !e.ready || !e.canvas) return 0;
    try {
        ctx.drawImage(e.canvas, x, y, w, h);
        return 1;
    } catch (err) {
        return 0;
    }
});
void Renderer::preload_external_images(char const *const *urls, uint32_t count) {
    EM_ASM({
        if (!Module._extImgSlots) Module._extImgSlots = [];
        if (!Module._bakeTile) {
            // Bake SVG/Image into a canvas only when non-transparent pixels exist.
            Module._bakeTile = function(img) {
                if (!img) return null;
                if (img._canvas && img._ready) return img._canvas;
                if (!img.complete) return null;
                try {
                    var c = document.createElement("canvas");
                    c.width = 256;
                    c.height = 256;
                    var g = c.getContext("2d");
                    g.clearRect(0, 0, 256, 256);
                    g.drawImage(img, 0, 0, 256, 256);
                    var data = g.getImageData(0, 0, 256, 256).data;
                    var any = 0;
                    for (var i = 3; i < data.length; i += 32) {
                        if (data[i] > 0) { any = 1; break; }
                    }
                    if (!any) {
                        img._ready = 0;
                        return null;
                    }
                    img._canvas = c;
                    img._ready = 1;
                    return c;
                } catch (e) {
                    img._ready = 0;
                    return null;
                }
            };
        }
    });
    for (uint32_t i = 0; i < count; ++i) {
        char const *url = urls[i];
        if (!url) continue;
        EM_ASM({
            if (!Module._extImgSlots) Module._extImgSlots = [];
            var slot = $0;
            var url = UTF8ToString($1);
            // Bust stale tile cache (SVG without xmlns painted fully transparent).
            // Append tile asset rev so dirt2 floor fix reloads without full VERSION_HASH bump.
            try {
                var vh = "1";
                if (window.localStorage && window.localStorage["version_hash"]) vh = window.localStorage["version_hash"];
                if (url.indexOf("?") < 0) url = url //+ "?v=" + vh + "-t2"; //我不知道为什么
            } catch (e) {}
            var prev = Module._extImgSlots[slot];
            if (prev && prev._url === url && (prev._ready || prev.complete || prev._loading)) return;
            var img = new Image();
            img._url = url;
            img._ready = 0;
            img._loading = 1;
            img.decoding = "async";
            img.onload = function() {
                img._loading = 0;
                if (Module._bakeTile) Module._bakeTile(img);
            };
            img.onerror = function() {
                img._loading = 0;
                img._ready = 0;
            };
            img.src = url;
            console.log("src =", img.src);
img.src = url;
console.log("after =", img.src);
            Module._extImgSlots[slot] = img;
        }, (int)i, url);
    }
}
int Renderer::ensure_and_draw_terrain_chunk(
    int cx, int cy, int subtile_div, float tile_size, int grid_w, int grid_h,
    uint32_t layer_count, uint32_t grass_color,
    int (*tile_fn)(uint32_t layer, int tx, int ty, uint8_t *img_i, uint8_t *flags, void *ud),
    void *ud)
{
    if (subtile_div < 1 || !(tile_size > 0.f) || !tile_fn) return 0;
    // 0.5-tile chunks: high bake so 256 world units stay sharp.
    int const px = (subtile_div <= 1) ? 256 : 192;
    int const key = ((cx & 0xffff) << 16) | (cy & 0xffff);
    int const gr = (int)((grass_color >> 16) & 255u);
    int const gg = (int)((grass_color >> 8) & 255u);
    int const gb = (int)(grass_color & 255u);

    int const tx = cx / subtile_div;
    int const ty = cy / subtile_div;
    if (tx < 0 || ty < 0 || tx >= grid_w || ty >= grid_h) return 0;
    int const sx = cx - tx * subtile_div;
    int const sy = cy - ty * subtile_div;

    gardn_terrain_init(subtile_div);

    if (!gardn_terrain_ready(key)) {
        int missing = 0;
        for (uint32_t layer = 0; layer < layer_count && !missing; ++layer) {
            uint8_t img_i = 0, flags = 0;
            if (!tile_fn(layer, tx, ty, &img_i, &flags, ud)) continue;
            if (!gardn_tile_img_ready((int)img_i)) missing = 1;
        }
        if (missing) return 0;

        gardn_terrain_begin(key, px, px, gr, gg, gb);
        for (uint32_t layer = 0; layer < layer_count; ++layer) {
            uint8_t img_i = 0, flags = 0;
            if (!tile_fn(layer, tx, ty, &img_i, &flags, ud)) continue;
            gardn_terrain_stamp_sub((int)img_i, px, (int)flags, sx, sy, subtile_div);
        }
        gardn_terrain_finish(key);
    }

    float const sub_ts = tile_size / static_cast<float>(subtile_div);
    float const world_x = cx * sub_ts;
    float const world_y = cy * sub_ts;
    return gardn_terrain_draw(id, key, world_x, world_y, sub_ts + 1.0f, sub_ts + 1.0f);
}
int Renderer::draw_external_image(uint32_t slot, float x, float y, float w, float h, uint8_t flags) {
    if (!(w > 0.f) || !(h > 0.f)) return 0;
    // Fast path: no flip — skip save/translate/scale/restore.
    if (flags == 0) {
        return EM_ASM_INT({
            var ctx = Module.ctxs[$0];
            var slots = Module._extImgSlots;
            if (!slots) return 0;
            var img = slots[$1];
            if (!img) return 0;
            var src = null;
            if (img._ready && img._canvas) src = img._canvas;
            else if (img.complete) {
                if (Module._bakeTile) src = Module._bakeTile(img);
                if (!src) src = img;
            }
            if (!src) return 0;
            try { ctx.drawImage(src, $2, $3, $4, $5); return 1; }
            catch (e) { return 0; }
        }, id, (int)slot, x, y, w, h);
    }
    int ok = EM_ASM_INT({
        var ctx = Module.ctxs[$0];
        var slots = Module._extImgSlots;
        if (!slots) return 0;
        var img = slots[$1];
        if (!img) return 0;
        var src = null;
        if (img._ready && img._canvas) {
            src = img._canvas;
        } else if (img.complete) {
            if (Module._bakeTile) src = Module._bakeTile(img);
            if (!src) src = img;
        }
        if (!src) return 0;
        var px = $2;
        var py = $3;
        var pw = $4;
        var ph = $5;
        var fl = $6;
        var fh = (fl & 1) != 0;
        var fv = (fl & 2) != 0;
        var fd = (fl & 4) != 0;
        ctx.save();
        ctx.translate(px + pw * 0.5, py + ph * 0.5);
        if (fd) {
            var t = fh;
            fh = fv;
            fv = !t;
            ctx.rotate(Math.PI * 0.5);
        }
        ctx.scale(fh ? -1 : 1, fv ? -1 : 1);
        try {
            ctx.drawImage(src, -pw * 0.5, -ph * 0.5, pw, ph);
        } catch (e) {
            ctx.restore();
            return 0;
        }
        ctx.restore();
        return 1;
    }, id, (int)slot, x, y, w, h, (int)flags);
    return ok;
}