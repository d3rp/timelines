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
renderText2(TTF_Font* font, SDL_Color* color, SDL_Rect* msgBounds, const char* text, int ptsize = 40)
{
    if (font == nullptr || text == nullptr)
    {
        std::cout << "font was null. Exiting text rendering..";
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

    Uint8 r, gr, b, a;
    SDL_GetRenderDrawColor(g.ren, &r, &gr, &b, &a);
    SDL_SetRenderDrawColor(g.ren, grey, grey, grey, 0xFF);
    SDL_RenderClear(g.ren);
    SDL_SetRenderDrawColor(g.ren, r, gr, b, a);
}
struct Renderer
{
    typedef std::unique_ptr<Entity> EntityPtr;
};

struct Timelines : public Renderer
{

    Timelines()
        : fontSize(36)
        , font{ TTF_OpenFont("../vera-fonts/dejavuSansMono.ttf", fontSize) }
    {
        std::cout << __PRETTY_FUNCTION__ << "\n";

        assert(font != nullptr);

        SDL_SetRenderDrawColor(g.ren, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(g.ren);

        lanes.resize(MAX_BINS);
    }

    ~Timelines() {}

    void drawGrid(int startYear, int endYear, const double xScale)
    {
        int interval = endYear - startYear;
        int splits = interval / 8.0f;
        splits = ((int) (splits / 10.0f)) * 10;
        splits = splits > 0 ? splits : 1;
        auto startIndex = yearToIndex(yearRenderStart);
        for (int i = startYear; i < endYear; ++i)
        {
            if (i % splits == 0)
            {
                SDL_SetRenderDrawColor(g.ren, 0x5F, 0x5F, 0x5F, 0x20);
                const int x = (yearToIndex(i) - startIndex) * xScale;
                SDL_RenderDrawLine(g.ren, x, 0, x, screenH);

                const int labelW = 50;
                const int labelH = 20;

                SDL_Rect gridLabelBounds;
                gridLabelBounds.x = x - (labelW / 2);
                gridLabelBounds.y = screenH - labelH;
                gridLabelBounds.w = labelW;
                gridLabelBounds.h = labelH;

                SDL_Color c{ 255, 255, 255, 100};
                renderText2(font, &c, &gridLabelBounds, std::to_string(i).c_str(), 28);
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

    void adjustRange(Sint32 xrel)
    {
        float xScale = (float) (yearRenderEnd - yearRenderStart) / screenW;
        int scaledRelativeX = xrel * xScale;
        yearRenderStart += scaledRelativeX;
        yearRenderEnd += scaledRelativeX;
        renderYear(Entities::getInstance()->data, yearRenderStart, yearRenderEnd);
    }
    void adjustTimeScale(Sint32 value, int midX = (1440/2))
    {
        float midPoint = 1.0f - ((float) midX / screenW);
        std::cout << value << "\n";
        constexpr double scaleCoeff = 1e-2f;
        timescale = 1.0 - limit(-0.9, 0.9, scaleCoeff * (double) value);
        const double start = yearRenderStart;
        const double mid = (yearRenderStart + yearRenderEnd) * midPoint;
        const double end = yearRenderEnd;
        std::cout << "ts: " << timescale << ", s: " << start << ", m: " << mid << ", e: " << end << "\n";
        yearRenderStart = (int) (((start - mid) * timescale) + mid);
        std::cout << (start - mid) * timescale << "\n";
        yearRenderEnd = (int) (((end - mid) * timescale) + mid);
        std::cout << "ts: " << timescale << ", s: " << yearRenderStart << ", e: " << yearRenderEnd << "\n";
        renderYear(Entities::getInstance()->data, yearRenderStart, yearRenderEnd);
    }

    std::vector<Entity*> selectFrom(std::vector<EntityPtr>& _entities)
    {
        std::vector<Entity*> selectedEntities;
        Years::getInstance()->clear();
        for (auto& e : _entities)
        {
            if (e->startYear < yearRenderEnd && e->endYear > yearRenderStart)
            {
                selectedEntities.push_back(e.get());
                Years::getInstance()->insert(e.get());
            }
        }

        return selectedEntities;
    }

    size_t maxEntitiesInInterval(int start, int end)
    {
        size_t max_entities_in_interval = 0;
        for (auto i = start; i < end; ++i)
            max_entities_in_interval = std::max(Years::getInstance()->year_bins[yearToIndex(i)],
                                                max_entities_in_interval);

        if (max_entities_in_interval == 0)
            std::cout << "no _entities in time frame..\n";

        return max_entities_in_interval;
    };

    void renderYear(std::vector<EntityPtr>& _entities, int renderStart, int renderEnd)
    {
        std::cout << "rendering time frame [" << renderStart << ", " << renderEnd << "]\n";
        assert(renderStart <= renderEnd);
        auto maxH = screenH - 80;

        //        std::cout << "interval: " << interval << "\n";

        std::vector<Entity*> selectedEntities = selectFrom(_entities);

        auto max_entities_in_interval = maxEntitiesInInterval(renderStart, renderEnd);
        if (max_entities_in_interval == 0)
            return;

        auto h = maxH / max_entities_in_interval;

        clear();

        assert(_entities.size() > 0);
        Uint8 colourIncr = 255 / _entities.size();


        auto renderStartX = yearToIndex(renderStart);
        auto renderEndX = yearToIndex(renderEnd);
        auto interval = limit<int>(0, MAX_BINS, renderEndX - renderStartX);
        const double xScale = screenW / (double) interval;

        std::fill(lanes.begin(), lanes.end(), std::numeric_limits<uint8_t>::max());

        for (auto& e : selectedEntities)
        {
            //            std::cout << "Entity found, rendering..\n";

            drawGrid(renderStart, renderEnd, xScale);

            const int entityStartYear = e->startYear;
            const int entityEndYear = e->endYear;

            auto startBound = std::max(entityStartYear, renderStart);
            const int rectStartX = yearToIndex(startBound) - renderStartX;

            auto endBound = std::min(entityEndYear, renderEnd);
            const int rectEndX = yearToIndex(endBound) - renderStartX;

            const size_t laneIndex = lane(max_entities_in_interval, entityStartYear, entityEndYear);

            SDL_Rect r;
            r.x = rectStartX * xScale;
            r.y = 10 + (h * laneIndex);
            r.w = (rectEndX - rectStartX) * xScale;
            r.h = h;


            const Uint8 fillColour = e->id * colourIncr;
            const Uint8 borderColour = fillColour + colourIncr;

            renderEntityBox(r, e, borderColour, fillColour);
        }

        SDL_RenderPresent(g.ren);
        // TODO : probably due to scope
        //        SDL_Delay(50);
    }

    size_t lane(size_t max_entities_in_interval, int start, int end)
    {
        size_t lane = 0;
        for (lane = 0; lane < max_entities_in_interval; ++lane)
        {
            auto lanes_begin = std::begin(lanes) + yearToIndex(start);
            auto lanes_end = std::begin(lanes) + yearToIndex(end);
            if (std::all_of(lanes_begin, lanes_end, [&](Uint8 ui) { return (bool) Uint8(1 << lane & ui); }))
            {
                // mark lane
                auto lanesBegin = std::begin(lanes) + yearToIndex(start);
                auto lanesEnd = std::begin(lanes) + yearToIndex(end);
                for (auto vbi = lanesBegin; vbi != lanesEnd; ++vbi)
                    *vbi = (*vbi) - (1 << lane);
                break;
            }
        }
        return lane;
    }

    void renderEntityBox(SDL_Rect& r, Entity* e, uint8_t borderColour, uint8_t fillColour)
    {

        // fill
        SDL_SetRenderDrawColor(g.ren, 0x9F - (fillColour*0.5f), 0x90 + (0xFF - fillColour) * 0.2f, 0xFF - (fillColour*0.8f), 0x70);
        SDL_RenderFillRect(g.ren, &r);

        // outline
        SDL_SetRenderDrawColor(g.ren, 0xFF, 0xFF, 0xFF, 0x20);

//        SDL_SetRenderDrawColor(g.ren, borderColour, borderColour, 0xFF - borderColour, 0xFF);

        SDL_RenderDrawRect(g.ren, &r);
        SDL_Color color{ 255, 255, 255, 0x80 };
        renderText2(font, &color, &r, e->name.c_str(), fontSize);
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

    void wait_for_endkey()
    {
        SDL_Event e;

        bool isRunning = true;

        SDL_Delay(500);
        while (isRunning)
        {
            while (SDL_PollEvent(&e) != 0)
            {
                switch (e.type)
                {
                    case SDL_QUIT:
                        break;
                    case SDL_MOUSEMOTION:
                        if (e.motion.state == 1)
                        {
                            adjustRange(-e.motion.xrel);
                        }
                        break;
                    case SDL_KEYDOWN:
                        switch (e.key.keysym.sym)
                        {
                            case SDLK_q:
                                isRunning = false;
                                break;
                        }
                        break;
                    case SDL_MOUSEWHEEL:
                        if (e.wheel.y != 0)
                        {
//                        std::cout << "mouse at (" << x << ", " << y << ")\n";
                            wheelEvents.push_back(e.wheel.y);
                        }
                        break;

                }
            }
            Sint32 y_delta = 0;
            while (wheelEvents.size() > 0)
            {
                y_delta += wheelEvents.back();
                wheelEvents.pop_back();
            }
            if (y_delta != 0)
            {

                int x = 0, y = 0;
                SDL_GetMouseState(&x, &y);
                adjustTimeScale(y_delta, x);
            }
            //            SDL_Delay(30);
        }
    }

    std::deque<Sint32> wheelEvents;
    Uint32 eventTimestamp = 0;
    int yearRenderOffset = 0;
    int yearRenderStart = 0;
    int yearRenderEnd = 0;
    double timescale = 1.0f;
    std::vector<Uint8> lanes;
    size_t fontSize;
    TTF_Font* font;
};

struct ScopedGraphics
{
    /**
     * RAII for graphics
     */
    ScopedGraphics() { graphicsInit(); }
    ~ScopedGraphics() { graphicsQuit(); }

    void graphicsInit()
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

    void graphicsQuit()
    {
        TTF_Quit();

        SDL_DestroyTexture(g.bg);
        SDL_DestroyRenderer(g.ren);
        SDL_DestroyWindow(g.win);
        SDL_Quit();
    }
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
