#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <array>
#include <bits/stdint-uintn.h>
#include <cmath>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <set>

const double pi = std::acos(-1);

// dot
class Dot
{
    double        angle = 0;
    double        x_vel;
    double        y_vel;
    int           last_x = 0;
    int           last_y = 0;
    float         acc    = 9.81f;
    float         speed  = 10.0f;
    Uint8         c      = 0xFF;
    Uint8         r      = 0xBF;
    Uint8         b      = 0xEF;
    SDL_Renderer* ren;

  public:
    Dot(double angle, int x, int y, float speed, SDL_Renderer* ren)
      : angle(angle)
      , x_vel(std::cos(angle))
      , y_vel(std::sin(angle))
      , last_x(x)
      , last_y(y)
      , speed(speed)
      , ren(ren)
    {}

    void iterate(int elapsed)
    {
        // speed = 0.1f * ((9.81f * (t * t)) / 2)
        // speed = std::max(speed * 0.9f, std::numeric_limits<float>::min());
        speed  = speed * 0.9f;
        last_x = last_x + (x_vel * speed);
        last_y = last_y + (y_vel * speed);
    }

    void draw()
    {
        c = std::max(c - 8, 0x01);
        r = std::max(r - 3, 0x01);
        b = std::max(b - 5, 0x01);
        // SDL_SetRenderDrawColor( ren, 0xFF, 0xFF, 0xFF, 0xFF );
        SDL_SetRenderDrawColor(ren, r, c, b, r);
        SDL_RenderDrawPoint(ren, last_x, last_y);
    }
};

static size_t ids = 0;

struct Entity
{
    Entity(std::string _name, int _start, int _end)
      : name(_name)
      , start(_start)
      , end(_end)
      , id(ids++)
    {}
    std::string  name;
    int          start;
    int          end;
    const size_t id;
};

// static const uint16_t MAX_BINS = 16384;
static const uint16_t MAX_BINS = 1024;
struct Years
{
    typedef std::unique_ptr<Entity>            EntityPtr;
    std::array<std::vector<Entity*>, MAX_BINS> bins{};
    void                                       insert(Entity* e)
    {
        auto split = MAX_BINS / 2;
        for (auto i = (split + e->start); i < (split + e->end); ++i)
            bins[i].push_back(e);
    }
    struct IdEquals
    {
        const int id;
        IdEquals(Entity* e)
          : id(e->id)
        {}
        bool operator()(Entity* x) const { return x->id == id; }
    };

    //    std::vector<Entity*>
    std::set<Entity*> getEntitiesInInterval(int start, int end)
    {
        //        std::vector<Entity*> v;
        std::set<Entity*> v;
        const auto        split = MAX_BINS / 2;
        for (auto yi = start; yi < end; ++yi) {
            auto index = yi + split;
            auto year  = bins[index];
            for (auto* e : year) {
                // std::cout << "checking entity\n";
                //                if (std::find(v.begin(), v.end(), IdEquals(e))
                //                != v.end())
                //                {
                //                    v.push_back(e);
                //                    std::cout << "adding entity\n";
                //
                //                }
                v.insert(e);
            }
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
    SDL_Color    color = { 0xFF, 0xFF, 0xFF };
    auto         font  = TTF_OpenFont("Consolas", 12);
    SDL_Surface* text_surface;
    if (!(text_surface = TTF_RenderUTF8_Solid(font, "Hello World!", color))) {
        // handle error here, perhaps print TTF_GetError at least
        std::cout << TTF_GetError << "\n";
    } else {
        SDL_BlitSurface(text_surface, NULL, screen, NULL);
        // perhaps we can reuse it, but I assume not for simplicity.
        SDL_FreeSurface(text_surface);
    }
}
struct Timelines
{
    typedef std::unique_ptr<Entity> EntityPtr;
    Timelines(SDL_Window* _win, SDL_Renderer* _ren, SDL_Texture* _bg)
      : win(_win)
      , ren(_ren)
      , bg(_bg)
    {
        std::cout << __PRETTY_FUNCTION__ << "\n";

        // SDL_SetRenderTarget(ren, bg);
        SDL_SetRenderDrawColor(ren, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(ren);
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
        for (auto i = e->start; i < e->end; ++i) {
            auto split = MAX_BINS / 2;
            auto index = i + split;
            auto year  = years.bins[index];

            slots = std::max((int)slots, (int)year.size());
        }

        return slots;
    }

    void render_year(int start, int end)
    {
        auto maxH  = 400;
        auto split = MAX_BINS / 2;

        auto interval = end - start;
        std::cout << "interval: " << interval << "\n";

        SDL_SetRenderDrawColor(ren, 0xFF, 0xFF, 0xFF, 0xFF);
        int colourIndex = 0;
        // TODO : this for all in total. should be by "sectors" or sth
        for (auto* e : years.getEntitiesInInterval(start, end)) {
            std::cout << "Entity found, rendering..\n";
            const Uint8 eColour = 100 * colourIndex++;
            SDL_SetRenderDrawColor(ren, eColour, eColour, 0xFF - eColour, 0xFF);
            for (auto i = 0; i < interval; ++i) {
                auto index = i + start + split;
                auto year  = years.bins[index];

                // auto h = maxH / year.size();
                auto h = maxH / assign_slots(e);
//                std::cout << "start: " << start << ", split: " << split
//                          << ", h: " << h << "\n";

                SDL_Rect r;
                r.x = 100 + (start - e->start);
                r.y = 100 + (h * colourIndex);
                r.w = std::min(e->end, end) - e->start;
                r.h = h;
                SDL_RenderDrawRect(ren, &r);

//                SDL_Surface();
//                renderText();

                SDL_RenderPresent(ren);
            }
        }

        SDL_Delay(50);
    }

    SDL_Window*   win;
    SDL_Renderer* ren;
    SDL_Texture*  bg;
};

void
wait_for_endkey()
{
    SDL_Event e;
    bool      isRunning = true;
    while (isRunning) {
        SDL_PollEvent(&e);
        if (e.type == SDL_QUIT)
            break;
        else if (e.type == SDL_KEYDOWN) {
            switch (e.key.keysym.sym) {
                case SDLK_q:
                    isRunning = false;
                    break;
            }
        }
    }
}

int
main()
{
    using std::cout;
    using std::endl;

    constexpr int screenW = 1280;
    constexpr int screenH = 768;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        cout << "SDL_Init Error: " << SDL_GetError() << endl;
        return EXIT_FAILURE;
    }

    if (TTF_Init() == -1) {
        printf("TTF_Init: %s\n", TTF_GetError());
        exit(2);
    }

    SDL_Window* win = SDL_CreateWindow(
      "Hello World!", 0, 0, screenW, screenH, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        return EXIT_FAILURE;
    }
    SDL_SetWindowFullscreen(win, 0);
    SDL_ShowCursor(1);

    SDL_Renderer* ren = SDL_CreateRenderer(
      win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
        cout << "SDL_CreateRenderer Error" << SDL_GetError() << endl;
        return EXIT_FAILURE;
    }

    SDL_Texture* bg = SDL_CreateTexture(ren,
                                        SDL_PIXELFORMAT_RGBA8888,
                                        SDL_TEXTUREACCESS_TARGET,
                                        screenW,
                                        screenH);

    // SDL_SetRenderTarget(ren, bg);
    SDL_SetRenderDrawColor(ren, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(ren);
    SDL_RenderPresent(ren);

    {
        std::vector<Years::EntityPtr> entities;
        entities.push_back(
          std::make_unique<Entity>("Alexander the Great", -356, -323));
        entities.push_back(std::make_unique<Entity>("Karjalainen", -390, -323));
        entities.push_back(std::make_unique<Entity>("Minna", -350, -283));
        entities.push_back(std::make_unique<Entity>("Aristoteles", -384, -322));
        entities.push_back(std::make_unique<Entity>("Pelle Hermanni", -304, -282));
        entities.push_back(std::make_unique<Entity>("hemmo", -404, -282));
        for (auto& e : entities)
            years.insert(e.get());

        Timelines tl(win, ren, bg);
        tl.render_year(-400, -200);
        SDL_RenderPresent(ren);
        wait_for_endkey();
    }

    TTF_Quit();

    SDL_DestroyTexture(bg);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return EXIT_SUCCESS;
}
