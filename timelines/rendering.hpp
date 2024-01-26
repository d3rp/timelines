#pragma once

#include "details/graphics.hpp"
#include "details/utilities.h"
#include "resource_mgr.hpp"
#include "time_abstractions.hpp"

namespace tl
{
struct MouseMove
{
    Sint32 x;
    Sint32 y;
};

struct Renderer
{
    using EntityPtr = std::unique_ptr<Entity>;

    enum class Flavour : int
    {
        Horizontal = 1,
        Vertical = 2
    };

    std::vector<Entity*> select_from(std::vector<EntityPtr>& _entities) const
    {
        std::vector<Entity*> selected_entities;
        Global::instance().years->clear();
        for (auto& e : _entities)
        {
            if (e->start_year < year_range.end && e->end_year > year_range.start)
            {
                selected_entities.push_back(e.get());
                years().insert(e.get());
            }
        }

        return selected_entities;
    }

    size_t entities_in_interval(int start, int end) const
    {
        size_t max_entities_in_interval = 0;
        for (auto i = start; i < end; ++i)
            max_entities_in_interval =
              std::max(years().year_bins[year_to_index(i)], max_entities_in_interval);

        if (max_entities_in_interval == 0)
            std::cout << "no entities in time frame..\n";

        return max_entities_in_interval;
    };

    size_t lane(size_t max_entities_in_interval, int start, int end)
    {
        size_t lane = 0;
        for (lane = 0; lane < max_entities_in_interval; ++lane)
        {
            auto lanes_begin = std::begin(lanes) + year_to_index(start);
            auto lanes_end = std::begin(lanes) + year_to_index(end);
            if (std::all_of(lanes_begin,
                            lanes_end,
                            [&](Uint8 ui) { return (bool)Uint8(1 << lane & ui); }))
            {
                // mark lane
                auto marked_begin = std::begin(lanes) + year_to_index(start);
                auto marked_end = std::begin(lanes) + year_to_index(end);
                for (auto vbi = marked_begin; vbi != marked_end; ++vbi)
                    *vbi = (*vbi) - (1 << lane);
                break;
            }
        }
        return lane;
    }

    virtual ~Renderer() = default;

    virtual void render_range(std::vector<EntityPtr>& _entities, YearRange* yrRange) = 0;

    // TODO : consider other options, state in interface..
    YearRange year_range;
    std::vector<Uint8> lanes;
    Flavour flavour = Flavour::Horizontal;
};

struct ThreadPool
{
};

struct RenderingController
{
    RenderingController() = default;

    bool is_horizontal() const { return renderer->flavour == Renderer::Flavour::Horizontal; }
    bool is_vertical() const { return renderer->flavour == Renderer::Flavour::Vertical; }

    void scroll_y(int delta_y, int x, int y) const
    {
        if (renderer == nullptr)
            return;

        auto screen_dim = is_horizontal() ? screen_w : screen_h;
        YearRange* year_range = &renderer->year_range;
        YearRange timescaled = new_scaled_year_range(delta_y, year_range, x);

        // TODO : shift range when zooming in to keep center of zoom under the
        // mouse pointer
        const int mid_x = screen_dim / 2;
        constexpr double scale = 0.5e-2f;
        int rel_mid_point = (x - mid_x) * (delta_y * scale);
        *year_range = timescaled;
        renderer->render_range(Global::instance().data, year_range);
    }

    void button_left_drag(MouseMove m, const float multiplier = 1.5f) const
    {
        if (renderer == nullptr)
            return;

        const Sint32 multiplied_value =
          is_horizontal() ? ((float)-m.x) * multiplier : ((float)-m.y) * multiplier;

        YearRange* year_range = &renderer->year_range;
        YearRange adjusted = new_relative_year_range(multiplied_value, year_range);
        *year_range = adjusted;
        std::cout << "rel.range: " << std::to_string(year_range->start) << " - "
                  << std::to_string(year_range->end) << "\n";
        renderer->render_range(Global::instance().data, year_range);
    }

    void toggle_renderer()
    {
        toggle = !toggle;
        renderer = renderer_container[(int)toggle].get();
        renderer->render_range(Global::instance().data, &renderer_container[!toggle]->year_range);
    }

    void button_right_drag() {}

    void paint() {}

    bool toggle{ true };
    std::vector<std::unique_ptr<Renderer>> renderer_container;
    Renderer* renderer = nullptr;
};

class Vertical : public Renderer
{
  public:
    Vertical()
      : font_size(36)
      , font{ get_title_font(font_size) }
    {
        std::cout << __PRETTY_FUNCTION__ << "\n";
        flavour = Renderer::Flavour::Vertical;

        assert(font != nullptr);

        // TODO : clean up to their own wrapper
        SDL_SetRenderDrawColor(g.ren, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(g.ren);

        lanes.resize(MAX_BINS);
    }

    ~Vertical() override
    {
        if (font != nullptr)
            TTF_CloseFont(font);
    }

    // TODO : feels quite repeated too..
    void render_range(std::vector<EntityPtr>& _entities, YearRange* yr_range) override
    {
        const auto render_start = yr_range->start;
        const auto render_end = yr_range->end;

        //        std::cout << "rendering time frame [" << render_start << ", " <<
        //        render_end << "]\n";
        assert(render_start <= render_end);
        auto maxW = screen_w / 2;

        std::vector<Entity*> selected_entities = select_from(_entities);

        auto max_entities_in_interval = entities_in_interval(render_start, render_end);
        if (max_entities_in_interval == 0)
            return;

        auto w = maxW / max_entities_in_interval;

        clear();

        assert(!_entities.empty());
        Uint8 colour_incr = 255 / _entities.size();

        const auto render_start_y = year_to_index(render_start);
        const auto render_end_y = year_to_index(render_end);
        const auto interval = util::limit<int>(0, MAX_BINS, render_end_y - render_start_y);
        const double scale_y = screen_h / (double)interval;

        std::fill(lanes.begin(), lanes.end(), std::numeric_limits<uint8_t>::max());

        for (auto& e : selected_entities)
        {
            //            std::cout << "Entity found, rendering..\n";
            // draw_grid(render_start, render_end, xScale);

            const int entity_start_year = e->start_year;
            const int entity_end_year = e->end_year;

            auto start_bound = std::max(entity_start_year, render_start);
            const int rect_start_y = year_to_index(start_bound) - render_start_y;

            auto end_bound = std::min(entity_end_year, render_end);
            const int rect_end_y = year_to_index(end_bound) - render_start_y;

            // non const part
            const size_t lane_index =
              lane(max_entities_in_interval, entity_start_year, entity_end_year);

            e->bounds.x = 10 + (w * lane_index);
            e->bounds.y = rect_start_y * scale_y;
            e->bounds.h = (rect_end_y - rect_start_y) * scale_y;
            e->bounds.w = w;

            SDL_Rect r;
            r.x = e->bounds.x;
            r.y = e->bounds.y;
            r.w = e->bounds.w;
            r.h = e->bounds.h;

            const Uint8 colour_fill = e->id * colour_incr;
            const Uint8 colour_border = colour_fill + colour_incr;

            render_entity_box(r, e, colour_border, colour_fill);
            SDL_RenderDrawLine(g.ren,
                               r.x + r.w,
                               r.y + font_size / 2,
                               screen_w / 2 + 20,
                               r.y + font_size / 2);

            SDL_Rect rt;
            rt.x = screen_w / 2 + 10;
            rt.y = rect_start_y * scale_y;
            rt.h = font_size;
            rt.w = screen_w / 2 - 20;

            SDL_Color color{ 255, 255, 255, 0xDD };
            render_text(font, &color, &rt, e->name.c_str(), font_size);
        }

        SDL_RenderPresent(g.ren);
        // TODO : probably due to scope
        //        SDL_Delay(50);
    }

    void render_entity_box(SDL_Rect& r, Entity* e, uint8_t colour_border, uint8_t colour_fill) const
    {
        // outline
        SDL_SetRenderDrawColor(g.ren, 0xFF, 0xFF, 0xFF, 0x20);
        SDL_RenderDrawRect(g.ren, &r);

        // fill
        SDL_SetRenderDrawColor(g.ren,
                               0x9F - (colour_fill * 0.5f),
                               0x90 + (0xFF - colour_fill) * 0.2f,
                               0xFF - (colour_fill * 0.8f),
                               0x70);
        SDL_RenderFillRect(g.ren, &r);
    }

    size_t font_size;
    TTF_Font* font;
};

class Horizontal : public Renderer
{
  public:
    Horizontal()
      : font_size(36)
      , font{ get_title_font(font_size) }
    {
        std::cout << __PRETTY_FUNCTION__ << "\n";

        assert(font != nullptr);

        SDL_SetRenderDrawColor(g.ren, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(g.ren);

        lanes.resize(MAX_BINS);
    }

    ~Horizontal() override
    {
        if (font != nullptr)
            TTF_CloseFont(font);
    }

    void draw_grid(int year_start, int year_end, const double scale_x) const
    {
        const int interval = year_end - year_start;
        int splits = interval / 8.0f;
        splits = ((int)(splits / 10.0f)) * 10;
        splits = splits > 0 ? splits : 1;
        auto start_idx = year_to_index(year_range.start);
        for (int i = year_start; i < year_end; ++i)
        {
            if (i % splits == 0)
            {
                SDL_SetRenderDrawColor(g.ren, 0x5F, 0x5F, 0x5F, 0x20);
                const int x = (year_to_index(i) - start_idx) * scale_x;
                SDL_RenderDrawLine(g.ren, x, 0, x, screen_h);

                const int label_w = 50;
                const int label_h = 20;

                SDL_Rect grid_label_bounds;
                grid_label_bounds.x = x - (label_w / 2);
                grid_label_bounds.y = screen_h - label_h;
                grid_label_bounds.w = label_w;
                grid_label_bounds.h = label_h;

                SDL_Color c{ 255, 255, 255, 100 };
                render_text_2(font, &c, &grid_label_bounds, std::to_string(i).c_str(), 28);
            }
        }
    }

    void render_range(std::vector<EntityPtr>& _entities, YearRange* year_range) override
    {
        const auto render_start = year_range->start;
        const auto render_end = year_range->end;

        //    std::cout << "rendering time frame [" << render_start << ", " << render_end << "]\n";
        assert(render_start <= render_end);
        auto max_h = screen_h - 80;

        std::vector<Entity*> selected_entities = select_from(_entities);

        auto max_entities_in_interval = entities_in_interval(render_start, render_end);
        if (max_entities_in_interval == 0)
            return;

        auto h = max_h / max_entities_in_interval;

        clear();

        assert(_entities.size() > 0);
        Uint8 colour_incr = 255 / _entities.size();

        const auto render_start_x = year_to_index(render_start);
        const auto render_end_x = year_to_index(render_end);
        const auto interval = util::limit<int>(0, MAX_BINS, render_end_x - render_start_x);
        const double xScale = screen_w / (double)interval;

        std::fill(lanes.begin(), lanes.end(), std::numeric_limits<uint8_t>::max());

        for (auto& e : selected_entities)
        {
            //            std::cout << "Entity found, rendering..\n";

            draw_grid(render_start, render_end, xScale);

            const int entity_start = e->start_year;
            const int entity_end = e->end_year;

            auto bound_start = std::max(entity_start, render_start);
            const int rect_start_x = year_to_index(bound_start) - render_start_x;

            auto bound_end = std::min(entity_end, render_end);
            const int rect_end_x = year_to_index(bound_end) - render_start_x;

            // non const part
            const size_t lane_idx = lane(max_entities_in_interval, entity_start, entity_end);

            SDL_Rect r;
            r.x = rect_start_x * xScale;
            r.y = 10 + (h * lane_idx);
            r.w = (rect_end_x - rect_start_x) * xScale;
            r.h = h;

            const Uint8 fillColour = e->id * colour_incr;
            const Uint8 borderColour = fillColour + colour_incr;

            render_entity_box(r, e, borderColour, fillColour);
        }

        SDL_RenderPresent(g.ren);
        // TODO : probably due to scope
        //        SDL_Delay(50);
    }

    // TODO : probably can be refactored to a free function for both renderers
    void render_entity_box(SDL_Rect& r, Entity* e, uint8_t borderColour, uint8_t fillColour) const
    {
        // fill
        SDL_SetRenderDrawColor(g.ren,
                               0x9F - (fillColour * 0.5f),
                               0x90 + (0xFF - fillColour) * 0.2f,
                               0xFF - (fillColour * 0.8f),
                               0x70);
        SDL_RenderFillRect(g.ren, &r);

        // outline
        SDL_SetRenderDrawColor(g.ren, 0xFF, 0xFF, 0xFF, 0x20);

        SDL_RenderDrawRect(g.ren, &r);
        SDL_Color color{ 255, 255, 255, 0x80 };
        render_text_2(font, &color, &r, e->name.c_str(), font_size);
    }

    void test()
    {
        SDL_Rect r;
        r.x = 10;
        r.y = 10;
        r.w = 100;
        r.h = 100;

        // outline
        SDL_SetRenderDrawColor(g.ren, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderDrawRect(g.ren, &r);

        // fill
        SDL_RenderFillRect(g.ren, &r);

        SDL_Color color{ 255, 255, 255 };
        SDL_RenderPresent(g.ren);
    }
    size_t font_size;
    TTF_Font* font;
};
}