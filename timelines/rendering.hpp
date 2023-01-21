#pragma once

#include "details/utilities.h"
#include "entities.hpp"
#include "graphics.hpp"
#include "time_abstractions.hpp"

class Renderer
{
public:
  using EntityPtr = std::unique_ptr<Entity>;

  enum class Flavour : int
  {
    Horizontal = 1,
    Vertical = 2
  };

  std::vector<Entity*>
  selectFrom(std::vector<EntityPtr>& _entities) const
  {
    std::vector<Entity*> selectedEntities;
    Years::getInstance()->clear();
    for (auto& e : _entities)
    {
      if (e->startYear < yearRange.end && e->endYear > yearRange.start)
      {
        selectedEntities.push_back(e.get());
        Years::getInstance()->insert(e.get());
      }
    }

    return selectedEntities;
  }

  size_t
  maxEntitiesInInterval(int start, int end) const
  {
    size_t max_entities_in_interval = 0;
    for (auto i = start; i < end; ++i)
      max_entities_in_interval =
        std::max(Years::getInstance()->year_bins[yearToIndex(i)], max_entities_in_interval);

    if (max_entities_in_interval == 0)
      std::cout << "no entities in time frame..\n";

    return max_entities_in_interval;
  };

  size_t
  lane(size_t max_entities_in_interval, int start, int end)
  {
    size_t lane = 0;
    for (lane = 0; lane < max_entities_in_interval; ++lane)
    {
      auto lanes_begin = std::begin(lanes) + yearToIndex(start);
      auto lanes_end = std::begin(lanes) + yearToIndex(end);
      if (std::all_of(lanes_begin,
                      lanes_end,
                      [&](Uint8 ui) { return (bool)Uint8(1 << lane & ui); }))
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

  virtual ~Renderer() = default;

  virtual void
  renderRange(std::vector<EntityPtr>& _entities, YearRange* yrRange) = 0;

  // TODO : consider other options, state in interface..
  YearRange yearRange;
  std::vector<Uint8> lanes;
  Flavour flavour = Flavour::Horizontal;
};

class ThreadPool
{
};

class RenderingController
{
public:
  RenderingController() = default;

  bool
  isHorizontal()
  {
    return renderer->flavour == Renderer::Flavour::Horizontal;
  }
  bool
  isVertical()
  {
    return renderer->flavour == Renderer::Flavour::Vertical;
  }

  void
  yScroll(int y_delta, int x, int y)
  {
    if (renderer == nullptr)
      return;

    if (isHorizontal())
    {
      YearRange* yearRange = &renderer->yearRange;
      YearRange timescaled = YearRange::newScaledYearRange(y_delta, yearRange, x);

      // TODO : shift range when zooming in to keep center of zoom under the
      // mouse pointer
      const int midX = screenW / 2;
      constexpr double scaleCoeff = 0.5e-2f;
      int relativeMidPoint = (x - midX) * (y_delta * scaleCoeff);
      YearRange ranged = YearRange::newRelativeYearRange(relativeMidPoint, &timescaled);
      *yearRange = timescaled;
      renderer->renderRange(EntitiesSingleton::getInstance()->data, yearRange);
    }
    else if (isVertical())
    {
      YearRange* yearRange = &renderer->yearRange;
      YearRange timescaled = YearRange::newScaledYearRange(y_delta, yearRange, y, screenH);

      // TODO : shift range when zooming in to keep center of zoom under the
      // mouse pointer
      const int midY = screenH / 2;
      constexpr double scaleCoeff = 0.5e-2f;
      int relativeMidPoint = (y - midY) * (y_delta * scaleCoeff);
      YearRange ranged = YearRange::newRelativeYearRange(relativeMidPoint, &timescaled);
      *yearRange = timescaled;
      renderer->renderRange(EntitiesSingleton::getInstance()->data, yearRange);
    }
  }

  void
  buttonLeftDrag(SDL_MouseMotionEvent& e)
  {
    if (renderer == nullptr)
      return;

    if (isHorizontal())
    {
      YearRange* yearRange = &renderer->yearRange;
      YearRange adjusted = YearRange::newRelativeYearRange(-e.xrel, yearRange);
      *yearRange = adjusted;
      renderer->renderRange(EntitiesSingleton::getInstance()->data, yearRange);
    }
    else if (isVertical())
    {

      YearRange* yearRange = &renderer->yearRange;
      YearRange adjusted = YearRange::newRelativeYearRange(-e.yrel, yearRange);
      *yearRange = adjusted;
      renderer->renderRange(EntitiesSingleton::getInstance()->data, yearRange);
    }
  }

  void
  toggleRenderer()
  {
    toggle = !toggle;
    renderer = rendererContainer_[(int)toggle].get();
    renderer->renderRange(EntitiesSingleton::getInstance()->data,
                           &rendererContainer_[!toggle]->yearRange);
  }

  void
  buttonRightDrag()
  {
  }

  void paint()
  {

  }

  bool toggle = 1;
  std::vector<std::unique_ptr<Renderer>> rendererContainer_;
  Renderer* renderer = nullptr;
};

class Vertical : public Renderer
{
public:
  Vertical()
    : fontSize(36)
    , font{ getTitleFont(fontSize) }
  {
    std::cout << __PRETTY_FUNCTION__ << "\n";
    flavour = Renderer::Flavour::Vertical;

    assert(font != nullptr);

    SDL_SetRenderDrawColor(g.ren, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(g.ren);

    lanes.resize(MAX_BINS);
  }

  ~Vertical() override
  {
    if (font != nullptr)
      TTF_CloseFont(font);
  }

  void
  renderRange(std::vector<EntityPtr>& _entities, YearRange* yrRange) override
  {
    renderRange(_entities, yrRange->start, yrRange->end);
  }

  void
  renderRange(std::vector<EntityPtr>& _entities, int renderStart, int renderEnd)
  {
    //        std::cout << "rendering time frame [" << renderStart << ", " <<
    //        renderEnd << "]\n";
    assert(renderStart <= renderEnd);
    auto maxW = screenW / 2;

    std::vector<Entity*> selectedEntities = selectFrom(_entities);

    auto max_entities_in_interval = maxEntitiesInInterval(renderStart, renderEnd);
    if (max_entities_in_interval == 0)
      return;

    auto w = maxW / max_entities_in_interval;

    clear();

    assert(_entities.size() > 0);
    Uint8 colourIncr = 255 / _entities.size();

    auto renderStartY = yearToIndex(renderStart);
    auto renderEndY = yearToIndex(renderEnd);
    auto interval = limit<int>(0, MAX_BINS, renderEndY - renderStartY);
    const double yScale = screenH / (double)interval;

    std::fill(lanes.begin(), lanes.end(), std::numeric_limits<uint8_t>::max());

    for (auto& e : selectedEntities)
    {
      //            std::cout << "Entity found, rendering..\n";

      // drawGrid(renderStart, renderEnd, xScale);

      const int entityStartYear = e->startYear;
      const int entityEndYear = e->endYear;

      auto startBound = std::max(entityStartYear, renderStart);
      const int rectStartY = yearToIndex(startBound) - renderStartY;

      auto endBound = std::min(entityEndYear, renderEnd);
      const int rectEndY = yearToIndex(endBound) - renderStartY;

      // non const part
      const size_t laneIndex = lane(max_entities_in_interval, entityStartYear, entityEndYear);

      e->bounds.x = 10 + (w * laneIndex);
      e->bounds.y = rectStartY * yScale;
      e->bounds.h = (rectEndY - rectStartY) * yScale;
      e->bounds.w = w;

      SDL_Rect r;
      r.x = e->bounds.x;
      r.y = e->bounds.y;
      r.w = e->bounds.w;
      r.h = e->bounds.h;

      const Uint8 fillColour = e->id * colourIncr;
      const Uint8 borderColour = fillColour + colourIncr;

      renderEntityBox(r, e, borderColour, fillColour);
      SDL_RenderDrawLine(g.ren,
                         r.x + r.w,
                         r.y + fontSize / 2,
                         screenW / 2 + 20,
                         r.y + fontSize / 2);

      SDL_Rect rt;
      rt.x = screenW / 2 + 10;
      rt.y = rectStartY * yScale;
      rt.h = fontSize;
      rt.w = screenW / 2 - 20;

      SDL_Color color{ 255, 255, 255, 0xDD };
      renderText(font, &color, &rt, e->name.c_str(), fontSize);
    }

    SDL_RenderPresent(g.ren);
    // TODO : probably due to scope
    //        SDL_Delay(50);
  }

  void
  renderEntityBox(SDL_Rect& r, Entity* e, uint8_t borderColour, uint8_t fillColour) const
  {
    // outline
    SDL_SetRenderDrawColor(g.ren, 0xFF, 0xFF, 0xFF, 0x20);
    SDL_RenderDrawRect(g.ren, &r);

    // fill
    SDL_SetRenderDrawColor(g.ren,
                           0x9F - (fillColour * 0.5f),
                           0x90 + (0xFF - fillColour) * 0.2f,
                           0xFF - (fillColour * 0.8f),
                           0x70);
    SDL_RenderFillRect(g.ren, &r);
  }

  size_t fontSize;
  TTF_Font* font;
};

class Horizontal : public Renderer
{
public:
  Horizontal()
    : fontSize(36)
    , font{ getTitleFont(fontSize) }
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

  void
  drawGrid(int startYear, int endYear, const double xScale) const
  {
    int interval = endYear - startYear;
    int splits = interval / 8.0f;
    splits = ((int)(splits / 10.0f)) * 10;
    splits = splits > 0 ? splits : 1;
    auto startIndex = yearToIndex(yearRange.start);
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

        SDL_Color c{ 255, 255, 255, 100 };
        renderText2(font, &c, &gridLabelBounds, std::to_string(i).c_str(), 28);
      }
    }
  }

  void
  renderRange(std::vector<EntityPtr>& _entities, YearRange* yrRange) override
  {
    renderRange(_entities, yrRange->start, yrRange->end);
  }

  void
  renderRange(std::vector<EntityPtr>& _entities, int renderStart, int renderEnd)
  {
    //    std::cout << "rendering time frame [" << renderStart << ", " << renderEnd << "]\n";
    assert(renderStart <= renderEnd);
    auto maxH = screenH - 80;

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
    const double xScale = screenW / (double)interval;

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

      // non const part
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

  void
  renderEntityBox(SDL_Rect& r, Entity* e, uint8_t borderColour, uint8_t fillColour) const
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
    renderText2(font, &color, &r, e->name.c_str(), fontSize);
  }

  void
  test()
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
  size_t fontSize;
  TTF_Font* font;
};

class ScopedGraphics
{
public:
  /**
   * RAII for graphics
   */
  ScopedGraphics() { graphicsInit(); }
  ~ScopedGraphics() { graphicsQuit(); }

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

    g.bg = SDL_CreateTexture(g.ren,
                             SDL_PIXELFORMAT_RGBA8888,
                             SDL_TEXTUREACCESS_TARGET,
                             screenW,
                             screenH);

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
};
