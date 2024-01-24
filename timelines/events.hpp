#pragma once

#include "rendering.hpp"

struct EventHandler
{
  EventHandler(RenderingController* controllerPtr)
    : controller(controllerPtr)
  {
  }

  void
  handleEvents()
  {
    SDL_Event e;

    bool isRunning = true;

    SDL_Delay(100);
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
              mouseMoveEvents.push_back({ e.motion.xrel, e.motion.yrel });
            }
            else if (e.motion.state == 4)
            {
              wheelEvents.push_back(-e.motion.yrel);
            }
            break;
          case SDL_KEYDOWN:
            switch (e.key.keysym.sym)
            {
              case SDLK_q:
                isRunning = false;
                break;
              case SDLK_t:
                controller->toggleRenderer();
                break;
            }
            break;
          case SDL_MOUSEWHEEL:
            if (e.wheel.y != 0)
              wheelEvents.push_back(e.wheel.y * wheelYMult);

            break;
        }
      }

      Sint32 y_scroll_delta = 0;
      while (!wheelEvents.empty())
      {
        y_scroll_delta += wheelEvents.back();
        wheelEvents.pop_back();
      }
      if (y_scroll_delta != 0)
      {
        int x = 0, y = 0;
        SDL_GetMouseState(&x, &y);
        controller->yScroll(y_scroll_delta, x, y);
      }

      MouseMove mouse_move_delta = { 0, 0 };
      while (!mouseMoveEvents.empty())
      {
        mouse_move_delta.x += mouseMoveEvents.back().x;
        mouse_move_delta.y += mouseMoveEvents.back().y;
        mouseMoveEvents.pop_back();
      }
      if (mouse_move_delta.x != mouse_move_delta.y != 0)
        controller->buttonLeftDrag(mouse_move_delta);
    }
  }

  RenderingController* controller;
  std::deque<MouseMove> mouseMoveEvents;
  std::deque<Sint32> wheelEvents;
  const float wheelYMult = 6.5f;
};
