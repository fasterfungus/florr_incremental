// Client/Ui/InGame/BroadcastDisplay.cc
#include <Client/Game.hh>
#include <Client/Ui/Extern.hh>
#include <Client/Render/Renderer.hh>
#include <Client/Ui/Ui.hh>

#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <cstdio>

namespace Ui {

    // ����
    static constexpr double SHOW_DURATION = 8.0;    // ��ʾʱ�䣨�룩
    static constexpr double FADE_DURATION = 1.0;    // ����ʱ�䣨�룩
    static constexpr float  ROW_SPACING = 12.0f;  // ����֮�����С��ࣨ��ֱ��
    static constexpr float  PADDING_X = 18.0f;  // ���������ڱ߾�
    static constexpr float  PADDING_Y = 8.0f;   // ���������ڱ߾�
    static constexpr float  TEXT_SIZE = 24.0f;  // �ı���С�����أ�
    static constexpr float  RADIUS = 12.0f;  // Բ�ǰ뾶
    static constexpr float  MIN_BOX_WIDTH = 140.0f; // ������С����
    static constexpr float  BASE_Y = 60.0f;  // ������һ�� y
    static constexpr float  LERP_SPEED = 0.18f;  // y_pos ��ֵ�ٶ�
    static constexpr size_t MAX_ACTIVE = 6;      // ͬʱ�����ʾ����

    // Ĭ����ɫ
    static constexpr uint32_t DEFAULT_BUBBLE_RGBA = 0x80000000; // ��͸���� (0x80 alpha)
    static constexpr uint32_t DEFAULT_TEXT_RGBA = 0xffffffff; // ��͸����

    struct ActiveMsg {
        std::string text;
        uint32_t bubble_color = DEFAULT_BUBBLE_RGBA; // �� alpha
        uint32_t text_color = DEFAULT_TEXT_RGBA;   // �� alpha (0xff)
        double elapsed = 0.0;    // ��
        float y_pos = BASE_Y;    // ��ǰ���� y��������
        float target_y = BASE_Y; // Ŀ�� y������ index ���㣩
        float alpha = 0.0f;      // ��ǰ alpha�������ã�
    };

    static std::vector<ActiveMsg> active; // newest at index 0

    // ���� "#RRGGBB" -> rgb24������ true/false
    static bool try_parse_rgb_hex(const std::string& s, uint32_t& out_rgb24) {
        if (s.size() != 7 || s[0] != '#') return false;
        unsigned int val = 0;
        if (sscanf(s.c_str() + 1, "%x", &val) != 1) return false;
        out_rgb24 = val & 0x00ffffffu;
        return true;
    }

    // ���� raw_text��֧��ǰ�� 0/1/2 ����ɫ����
    static void push_parsed_broadcast(const std::string& raw_text) {
        std::istringstream iss(raw_text);
        std::string token1, token2;
        std::string rest;

        ActiveMsg msg;
        msg.bubble_color = DEFAULT_BUBBLE_RGBA;
        msg.text_color = DEFAULT_TEXT_RGBA;
        msg.elapsed = 0.0;
        msg.alpha = 0.0f;

        // ����һ�� token
        if (!(iss >> token1)) return; // ��ֱ�ӷ���

        uint32_t rgb24;
        bool first_is_color = false;
        bool second_is_color = false;

        if (try_parse_rgb_hex(token1, rgb24)) {
            // ��һ������ɫ -> ����ɫ��ǿ�ư�͸����
            msg.bubble_color = (0x80u << 24) | rgb24;
            first_is_color = true;
            if (iss >> token2) {
                if (try_parse_rgb_hex(token2, rgb24)) {
                    // �ڶ�������ɫ -> �ı�ɫ����͸����
                    msg.text_color = (0xffu << 24) | rgb24;
                    second_is_color = true;
                }
                else {
                    // �ڶ���������ɫ����Ϊ�ı���ͷ
                    rest = token2;
                }
            }
        }
        else {
            // ��һ��������ɫ����Ϊ�ı���ͷ
            rest = token1;
        }

        // ȡ����������Ϊʣ���ı�
        std::string remaining;
        std::getline(iss, remaining);
        if (!remaining.empty()) {
            size_t p = remaining.find_first_not_of(' ');
            if (p != std::string::npos) remaining = remaining.substr(p);
            else remaining.clear();
        }

        std::string final_text;
        if (!rest.empty()) {
            final_text = rest;
            if (!remaining.empty()) {
                final_text += " ";
                final_text += remaining;
            }
        }
        else {
            final_text = remaining;
        }

        // ���������Ϊ�գ������ʹ�� token1/token2����ֹֻ����ɫ����ʱΪ�գ�
        if (final_text.empty()) {
            if (!first_is_color) final_text = token1;
            else if (!second_is_color && !token2.empty()) final_text = token2;
        }

        // trim
        auto trim = [](std::string& s) {
            size_t a = s.find_first_not_of(' ');
            size_t b = s.find_last_not_of(' ');
            if (a == std::string::npos) { s.clear(); return; }
            s = s.substr(a, b - a + 1);
            };
        trim(final_text);
        if (final_text.empty()) return;

        msg.text = final_text;

        // ���뵽��ͷ���������ϣ�
        active.insert(active.begin(), msg);

        // ������� MAX_ACTIVE�����������Ƴ������ɣ�
        if (active.size() > MAX_ACTIVE) {
            active.pop_back();
        }

        // ��ʼ���²������ y_pos �Ի�ø��õĵ���У�
        // �����ڲ���ʱ index = 0 -> target_y Ϊ BASE_Y��Ϊ�˵��룬�����Ϸ���ʼ
        if (!active.empty()) {
            active.front().y_pos = BASE_Y - 18.0f; // ���Ϸ���΢����
        }
    }

    // ÿ֡�ƽ������� Game::broadcasts������ʱ�䲢����Ŀ��λ�ã���ֵ y_pos & alpha
    static void step_broadcasts(double dt_ms) {
        double dt = dt_ms / 1000.0;

        // 1) ���� Game::broadcasts��������/������������ raw �ı���
        while (!Game::broadcasts.empty()) {
            push_parsed_broadcast(Game::broadcasts.front().text);
            Game::broadcasts.erase(Game::broadcasts.begin());
        }

        // 2) ����ʱ�䲢ɾ����ȫ��ʱ����Ϣ
        for (auto it = active.begin(); it != active.end();) {
            it->elapsed += dt;
            if (it->elapsed > (SHOW_DURATION + FADE_DURATION)) {
                it = active.erase(it);
            }
            else ++it;
        }

        // 3) ����Ŀ�� y��index 0 �ڶ��� BASE_Y��index �������£�
        float y = BASE_Y;
        for (size_t i = 0; i < active.size(); ++i) {
            active[i].target_y = y;
            // ���� boxH �� TEXT_SIZE + padding ���㣨�����߶��ڻ���ʱ����һ�£�
            float boxH = TEXT_SIZE + PADDING_Y * 2.0f;
            y += boxH + ROW_SPACING;
        }

        // 4) ��ֵ y_pos �� alpha������/������
        for (auto& m : active) {
            // ��ֵ y_pos -> ƽ���ƶ�
            m.y_pos += (m.target_y - m.y_pos) * LERP_SPEED;

            // alpha������ 0.25s����ʾ�������׶����Եݼ�
            if (m.elapsed < 0.25) {
                m.alpha = float(m.elapsed / 0.25);
            }
            else if (m.elapsed > SHOW_DURATION) {
                double t = (m.elapsed - SHOW_DURATION) / FADE_DURATION;
                if (t > 1.0) t = 1.0;
                m.alpha = float(1.0 - t);
            }
            else {
                m.alpha = 1.0f;
            }
        }
    }

    // ���ƺ���������������ı����ȣ�ȷ��ʹ����ͬ�ı���С��
    static void draw_broadcasts(Renderer& ctx) {
        ctx.set_text_size(TEXT_SIZE);
        ctx.set_line_width(TEXT_SIZE * 0.12f);
        ctx.center_text_align();
        ctx.center_text_baseline();

        for (auto const& m : active) {
            // ȷ�����������ʹ��һ������ߴ�
            ctx.set_text_size(TEXT_SIZE);
            float textWidth = ctx.get_text_size(m.text.c_str());
            float boxW = std::max(MIN_BOX_WIDTH, textWidth + PADDING_X * 2.0f);
            float boxH = TEXT_SIZE + PADDING_Y * 2.0f;

            // �ƶ�����Ļ�е����Ϣ�� y_pos
            ctx.reset_transform();
            ctx.translate(ctx.width * 0.5f, m.y_pos);

            // ����
            ctx.set_global_alpha(m.alpha);
            ctx.set_fill(m.bubble_color);
            ctx.begin_path();
            ctx.round_rect(-boxW / 2.0f, -boxH / 2.0f, boxW, boxH, RADIUS);
            ctx.fill();

            // �ı�
            ctx.set_global_alpha(m.alpha);
            ctx.set_text_size(TEXT_SIZE);
            ctx.set_line_width(TEXT_SIZE * 0.12f);
            ctx.set_fill(m.text_color);
            ctx.fill_text(m.text.c_str());
        }

        ctx.reset_transform();
        ctx.set_global_alpha(1.0f);
    }

    // Element �����������ӵ� game_ui_window
    Element* make_broadcast_display() {
        class BroadcastElement : public Element {
        public:
            BroadcastElement() : Element(0, 0) {
                style.h_justify = Style::Center;
                style.v_justify = Style::Top;
                style.animate = [](Element*, Renderer&) {
                    step_broadcasts(Ui::dt);
                    };
                // ʼ����Ⱦ������԰���� should_render��
            }

            virtual void on_render(Renderer& ctx) override {
                draw_broadcasts(ctx);
            }
        };

        return new BroadcastElement();
    }

} // namespace Ui
