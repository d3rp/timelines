#pragma once

#include <cassert>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <set>

#include <SDL.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_timer.h>
#include <SDL_ttf.h>
#include <SDL_mouse.h>

#include "globals.h"
#include "utilities.h"
#include "entities.h"
#include "calendars.h"


static struct Graphics
{
    SDL_Window* win;
    SDL_Renderer* ren;
    SDL_Texture* bg;
} g;

void
renderText2(SDL_Color* color, SDL_Rect* msgBounds, const char* text, int ptsize = 40)
{
    TTF_Font* font = TTF_OpenFont("../vera-fonts/dejavuSansMono.ttf", ptsize);
    if (font == nullptr || text == nullptr)
    {
        std::cout << "font or text was null. Exiting text rendering..";
        return;
    }
    SDL_Surface* msgSurface;
    if (!(msgSurface = TTF_RenderUTF8_Solid(font, text, *color)))
    {
        std::cout << __PRETTY_FUNCTION__ << ": " << TTF_GetError << "\n";
        return;
    }

    const int ptsizeSmooth = ptsize / 2;
    const int cstrL = length(text);
    const int cstrW = cstrL * (ptsizeSmooth / 2);

    SDL_Rect msgBox;
    int margins = 2 * ptsizeSmooth;
    int offsetMsgBoundsW = msgBounds->w + (2 * margins);
    int offsetMsgBoundsX = msgBounds->x - offsetMsgBoundsW - margins;
    msgBox.w = std::min(cstrW, offsetMsgBoundsW);
    msgBox.h = ptsizeSmooth;
    msgBox.x = msgBounds->x - margins + std::max(offsetMsgBoundsW - cstrW, 0) / 2;
    msgBox.y = msgBounds->y + std::max(msgBounds->h - msgBox.h, 0) / 2;
    //        SDL_BlitSurface(msgSurface, NULL, screenSurface, msgBounds);
    SDL_Texture* msgTexture = SDL_CreateTextureFromSurface(g.ren, msgSurface);
    SDL_RenderCopy(g.ren, msgTexture, NULL, &msgBox);

    SDL_DestroyTexture(msgTexture);
    SDL_FreeSurface(msgSurface);
}

void
clear()
{
    // TODO : paint background
    Uint8 grey = 0x30;

    Uint8 r,gr,b,a;
    SDL_GetRenderDrawColor(g.ren, &r, &gr, &b, &a);
    SDL_SetRenderDrawColor(g.ren, grey, grey, grey, 0xFF);
    SDL_RenderClear(g.ren);
    SDL_SetRenderDrawColor(g.ren, r, gr, b, a);

}
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
        int slots = 1;
        auto split = MAX_BINS / 2;
        for (auto i = e->startYear; i < e->endYear; ++i)
        {
            auto year = Years::getInstance()->bins[yearToIndex(i)];
            slots = std::max((int) slots, (int) year.size());
        }

        return slots;
    }

    void drawGrid(int startYear, int endYear, const int xScale)
    {
        for (int i = startYear; i < endYear; ++i)
        {
            if (i % 25 == 0)
            {
                SDL_SetRenderDrawColor(g.ren, 0xFF, 0xFF, 0xFF, 0x10);
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

    static void renderYear(const int startYear, const int endYear)
    {
        Timelines tl;
        tl.yearRenderStart = startYear;
        tl.yearRenderEnd = endYear;
        tl.renderYear(Entities::getInstance()->data, startYear, endYear);
        tl.wait_for_endkey();
    }

    void adjustTimeScale(Sint32 value)
    {
        constexpr double scaleCoeff = 1e-4f;
        timescale += limit(-0.9, 0.9, scaleCoeff * (double) value);
        const double start = yearRenderStart;
        const double mid = (yearRenderStart + yearRenderEnd) * 0.5f;
        const double end = yearRenderEnd;
    std::cout << "ts: " << timescale << ", s: " << start << ", m: " << mid << ", e: " << end << "\n";
        yearRenderStart = (int) (((start - mid) * timescale) + mid);
        yearRenderEnd = (int) (((end - mid) * timescale) + mid);
        std::cout << "ts: " << timescale << ", s: " << yearRenderStart << ", e: " << yearRenderEnd << "\n";
        renderYear(Entities::getInstance()->data, yearRenderStart, yearRenderEnd);
    }

    void renderYear(std::vector<EntityPtr>& _entities, int startYear, int endYear)
    {
        std::cout << "rendering time frame [" << startYear << ", " << endYear << "]\n";
        auto maxH = 400;
        auto interval = limit<int>(0, MAX_BINS, endYear - startYear);

//        std::cout << "interval: " << interval << "\n";


        // Get max simultaneous _entities in interval
        size_t max_entities_in_interval = 0;
        for (auto i = 0; i < interval; ++i)
            max_entities_in_interval = std::max(Years::getInstance()->year_bins[i], max_entities_in_interval);

        if (max_entities_in_interval == 0)
        {
            std::cout << "no _entities in time frame..\n";
            return;
        }

        std::vector<Uint8> lanes;
        lanes.resize(MAX_BINS);
        std::fill_n(lanes.begin(), MAX_BINS, std::numeric_limits<uint8_t>::max());

        auto h = maxH / max_entities_in_interval;

        std::vector<Entity*> selectedEntities;
        for (auto& e : _entities)
        {
            if (e->startYear < endYear && e->endYear > startYear)
                selectedEntities.push_back(e.get());
        }

        clear();

        for (auto& e : selectedEntities)
        {
//            std::cout << "Entity found, rendering..\n";

            size_t lane = 0;
            for (lane = 0; lane < max_entities_in_interval; ++lane)
            {
                auto lanes_begin = std::begin(lanes) + yearToIndex(e->startYear);
                auto lanes_end = std::begin(lanes) + yearToIndex(e->endYear);
                if (std::all_of(lanes_begin, lanes_end, [&](Uint8 ui) { return (bool) Uint8(1 << lane & ui); }))
                {
                    // mark lane
                    auto lanesBegin = std::begin(lanes) + yearToIndex(e->startYear);
                    auto lanesEnd = std::begin(lanes) + yearToIndex(e->endYear);
                    for (auto vbi = lanesBegin; vbi != lanesEnd; ++vbi)
                        *vbi = (*vbi) - (1 << lane);
                    break;
                }
            }

            const int rectEnd = yearToIndex(std::min(e->endYear, endYear));
            const int rectStart = yearToIndex(std::max(e->startYear, startYear));

            const int xScale = screenW / (interval * 1.5);
            drawGrid(startYear, endYear, xScale);

            SDL_Rect r;
            r.x = rectStart * xScale;
            r.y = 10 + (h * lane);
            r.w = (rectEnd - rectStart) * xScale;
            r.h = h;

            SDL_SetRenderDrawColor(g.ren, 0xFF, 0xFF, 0xFF, 0xFF);

            uint8_t entityColour = 0;
            assert(_entities.size() > 0);
            uint8_t colourIncr = 255 / _entities.size();

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
        // TODO : probably due to scope
//        SDL_Delay(50);
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

    void
    wait_for_endkey()
    {
        SDL_Event e;

        bool isRunning = true;

        SDL_Delay(500);
        while (isRunning)
        {
//            SDL_PollEvent(&e);
            SDL_WaitEvent(&e);
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
            else if(e.type == SDL_MOUSEWHEEL)
            {
                if(e.wheel.y != 0) // scroll up
                {
//                    std::cout << "wheel up, y: " << e.wheel.y << "\n";
                    adjustTimeScale(e.wheel.y);
                }
            }
            SDL_Delay(30);
        }
    }

    int yearRenderOffset = 0;
    int yearRenderStart = 0;
    int yearRenderEnd = 0;
    double timescale = 1.0f;
};



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

    g.win = SDL_CreateWindow("Timelines", 0, 0, screenW, screenH, SDL_WINDOW_SHOWN);
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

    // TODO : paint background
    Uint8 grey = 0x30;
    SDL_SetRenderDrawColor(g.ren, grey, grey, grey, 0xFF);
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

struct ScopedGraphics
{
    /**
     * RAII for graphics
     */
    ScopedGraphics() { graphicsInit(); }
    ~ScopedGraphics() { graphicsQuit(); }
};

struct ScopedEntities
{
    /**
     * RAII for Singleton to invoke its child objects' DTORs upon exiting
     */
    ScopedEntities()
        : _entities{ Entities::getInstance() }
        , _years{ Years::getInstance() }
    {
    }

    ~ScopedEntities() {}

    std::unique_ptr<Entities> _entities;
    std::unique_ptr<Years> _years;
};
void
populateEntitiesTest()
{
    auto& philip = "Philip II of Macedon"_e | -382 | -336;
    "Alexander the Great"_e | -356 | -323;
    "Aristoteles"_e | -384 | -322;
    "Plato"_e | -428 | -348;
    "Macedonia"_e | -808 | -168;
    "Minna the Great"_e | -3200 | 2019;

//    for (auto& e : Entities::getInstance()->data)
//        years.insert(e.get());
}
