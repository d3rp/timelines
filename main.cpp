#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <array>
#include <bits/stdint-uintn.h>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <set>

constexpr int screenW = 1280;
constexpr int screenH = 768;

const double pi = std::acos(-1);

static struct Graphics
{
    SDL_Window*   win;
    SDL_Renderer* ren;
    SDL_Texture*  bg;
} g;

static size_t ids = 0;

struct Entity
{
    Entity(std::string _name, int _start, int _end)
      : name(_name)
      , startYear(_start)
      , endYear(_end)
      , id(ids++)
    {
        // birth before death, right?
        assert(startYear <= endYear);
    }

    const std::string name;
    const int         startYear;
    const int         endYear;
    const size_t      id;
};

// static const uint16_t MAX_BINS = 16384;
static const uint16_t MAX_BINS   = 1024;
constexpr uint16_t    BINS_SPLIT = MAX_BINS / 2;

int
yearToIndex(int year)
{
    const int index = BINS_SPLIT + year;
    assert(index >= 0 && index < MAX_BINS);
    return index;
}
int
indexToYear(int index)
{
    const int year = index - BINS_SPLIT;
    return year;
}

struct Years
{
    typedef std::unique_ptr<Entity> EntityPtr;

    std::array<std::vector<Entity*>, MAX_BINS> bins{};
    std::array<size_t, MAX_BINS>               year_bins{ 0 };

    void insert(Entity* e)
    {
        for (auto i = yearToIndex(e->startYear); i < yearToIndex(e->endYear); ++i)
            ++year_bins[i];
    }
    struct IdEquals
    {
        const int id;
        IdEquals(Entity* e)
          : id(e->id)
        {
        }
        bool operator()(Entity* x) const { return x->id == id; }
    };

    std::set<Entity*> getEntitiesInInterval(int start, int end)
    {
        std::set<Entity*> v;
        for (auto yi = start; yi < end; ++yi)
        {
            auto year = bins[yearToIndex(yi)];
            for (auto* e : year)
                v.insert(e);
        }

        return v;
    }
} years;

void
renderText(SDL_Surface* screen)
{
    // Render some UTF8 text in solid black to a new surface
    // then blit to the upper left of the screen
    // then free the text surface
    // SDL_Surface *screen;
    SDL_Color color = { 0xFF, 0xFF, 0xFF };
    auto      font  = TTF_OpenFont("Consolas", 12);

    SDL_Surface* text_surface;
    if (!(text_surface = TTF_RenderUTF8_Solid(font, "Hello World!", color)))
    {
        // handle error here, perhaps print TTF_GetError at least
        std::cout << TTF_GetError << "\n";
    }
    else
    {
        SDL_BlitSurface(text_surface, NULL, screen, NULL);
        // perhaps we can reuse it, but I assume not for simplicity.
        SDL_FreeSurface(text_surface);
    }
}

void
renderText2(SDL_Color* color, SDL_Rect* message_bounds, const char* text)
{
    TTF_Font* font = TTF_OpenFont("/home/j/.fonts/Hack-Regular.ttf", 24);
    if (font == nullptr || text == nullptr)
    {
        std::cout << "font or text was null. Exiting text rendering..";
        return;
    }

    SDL_Surface* msgSurface = TTF_RenderText_Solid(font, text, *color);
    SDL_Texture* msgTexture = SDL_CreateTextureFromSurface(g.ren, msgSurface);
    SDL_RenderCopy(g.ren, msgTexture, NULL, message_bounds);

    SDL_DestroyTexture(msgTexture);
    SDL_FreeSurface(msgSurface);
}

// void all_of_skip(begin, endYear, step, value) {}
struct Timelines
{
    typedef std::unique_ptr<Entity> EntityPtr;

    Timelines()
    {
        std::cout << __PRETTY_FUNCTION__ << "\n";
        SDL_SetRenderDrawColor(g.ren, 0x000, 0x00, 0x00, 0x00);
        SDL_RenderClear(g.ren);
    }

    ~Timelines()
    {
        // SDL_SetRenderTarget(ren, NULL);
        // SDL_RenderCopy(ren, bg, NULL, NULL);
        // SDL_RenderPresent(ren);
    }

    int assign_slots(Entity* e)
    {
        int  slots = 1;
        auto split = MAX_BINS / 2;
        for (auto i = e->startYear; i < e->endYear; ++i)
        {
            auto year = years.bins[yearToIndex(i)];
            slots     = std::max((int)slots, (int)year.size());
        }

        return slots;
    }

    void drawGrid(int startYear, int endYear, const int xScale)
    {
        for (int i = startYear; i < endYear; ++i)
        {
            if (i % 25 == 0)
            {
                SDL_SetRenderDrawColor(g.ren, 0xFF, 0xFF, 0xFF, 0x55);
                const int x = yearToIndex(i) * xScale;
                SDL_RenderDrawLine(g.ren, x, 0, x, screenH);

                const int labelW = 50;
                const int labelH = 20;

                SDL_Rect gridLabelBounds;
                gridLabelBounds.x = x - (labelW / 2);
                gridLabelBounds.y = screenH - labelH;
                gridLabelBounds.w = labelW;
                gridLabelBounds.h = labelH;

                SDL_Color c{ 255, 255, 255 };
                renderText2(&c, &gridLabelBounds, std::to_string(i).c_str());
            }
        }
    }
    void renderYear(std::vector<EntityPtr>& entities, int startYear, int endYear)
    {
        auto maxH     = 400;
        auto interval = endYear - startYear;

        std::cout << "interval: " << interval << "\n";

        SDL_SetRenderDrawColor(g.ren, 0xFF, 0xFF, 0xFF, 0xFF);

        uint8_t entityColour = 0;
        uint8_t colourIncr   = 255 / entities.size();

        // Get max simultaneous entities in interval
        size_t max_entities_in_interval = 0;
        for (auto i = 0; i < interval; ++i)
            max_entities_in_interval = std::max(years.year_bins[i], max_entities_in_interval);

        if (max_entities_in_interval == 0)
        {
            std::cout << "no entities in time frame..\n";
            return;
        }

        std::vector<Uint8> lanes;
        lanes.resize(MAX_BINS);
        std::fill_n(lanes.begin(), MAX_BINS, std::numeric_limits<uint8_t>::max());

        auto h = maxH / max_entities_in_interval;

        std::vector<Entity*> selectedEntities;
        for (auto& e : entities)
        {
            if (e->startYear < endYear || e->endYear > startYear)
                selectedEntities.push_back(e.get());
        }

        for (auto& e : selectedEntities)
        {
            std::cout << "Entity found, rendering..\n";

            size_t lane = 0;
            for (lane = 0; lane < max_entities_in_interval; ++lane)
            {
                auto lanes_begin = std::begin(lanes) + yearToIndex(e->startYear);
                auto lanes_end   = std::begin(lanes) + yearToIndex(e->endYear);
                if (std::all_of(lanes_begin, lanes_end, [&](Uint8 ui) { return (bool) Uint8((1 << lane) & ui); }))
                {
                    // mark lane
                    auto lanesBegin = std::begin(lanes) + yearToIndex(e->startYear);
                    auto lanesEnd = std::begin(lanes) + yearToIndex(e->endYear);
                    for (auto vbi = lanesBegin; vbi != lanesEnd; ++vbi)
                        *vbi = (*vbi) - (1 << lane);
                    break;
                }
            }

            const int rectEnd   = yearToIndex(std::min(e->endYear, endYear));
            const int rectStart = yearToIndex(std::max(e->startYear, startYear));

            const int xScale = screenW / (interval * 1.5);
            drawGrid(startYear, endYear, xScale);

            SDL_Rect r;
            r.x = 100 + rectStart * xScale;
            r.y = 100 + (h * lane);
            r.w = (rectEnd - rectStart) * xScale;
            r.h = h;

            // outline
            entityColour += colourIncr;
            const Uint8 eColour = entityColour;
            SDL_SetRenderDrawColor(g.ren, eColour, eColour, 0xFF - eColour, 0xFF);
            SDL_RenderDrawRect(g.ren, &r);

            // fill
            SDL_SetRenderDrawColor(g.ren, eColour / 2, eColour, 0xFF - eColour, 0x50);
            SDL_RenderFillRect(g.ren, &r);

            SDL_Color color{ 255, 255, 255 };
            renderText2(&color, &r, e->name.c_str());
        }

        SDL_RenderPresent(g.ren);
        SDL_Delay(50);
    }
};

void
wait_for_endkey()
{
    SDL_Event e;

    bool isRunning = true;

    while (isRunning)
    {
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT)
            break;
        else if (e.type == SDL_KEYDOWN)
        {
            switch (e.key.keysym.sym)
            {
                case SDLK_q:
                    isRunning = false;
                    break;
            }
        }
    }
}

void
graphicsInit()
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cout << "SDL_Init Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() == -1)
    {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(2);
    }

    g.win = SDL_CreateWindow("Hello World!", 0, 0, screenW, screenH, SDL_WINDOW_SHOWN);
    if (g.win == nullptr)
    {
        std::cout << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    SDL_SetWindowFullscreen(g.win, 0);
    SDL_ShowCursor(1);

    g.ren = SDL_CreateRenderer(g.win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (g.ren == nullptr)
    {
        std::cout << "SDL_CreateRenderer Error" << SDL_GetError() << std::endl;
        exit(EXIT_FAILURE);
    }
    SDL_SetRenderDrawBlendMode(g.ren, SDL_BLENDMODE_BLEND);

    g.bg = SDL_CreateTexture(g.ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, screenW, screenH);

    SDL_SetRenderDrawColor(g.ren, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(g.ren);
    SDL_RenderPresent(g.ren);
}

void
graphicsQuit()
{
    TTF_Quit();

    SDL_DestroyTexture(g.bg);
    SDL_DestroyRenderer(g.ren);
    SDL_DestroyWindow(g.win);
    SDL_Quit();
}

int
main()
{
    graphicsInit();

    {
        std::vector<Years::EntityPtr> entities;
        entities.push_back(std::make_unique<Entity>("Philip II of Macedon", -382, -336));
        entities.push_back(std::make_unique<Entity>("Alexander the Great", -356, -323));
        entities.push_back(std::make_unique<Entity>("Aristoteles", -384, -322));
        entities.push_back(std::make_unique<Entity>("Plato", -428, -348));
        for (auto& e : entities)
            years.insert(e.get());

        Timelines tl;
        tl.renderYear(entities, -430, -199);
        SDL_RenderPresent(g.ren);
        wait_for_endkey();
    }

    graphicsQuit();

    return EXIT_SUCCESS;
}
