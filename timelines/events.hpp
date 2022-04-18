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
            // if (e.motion.state == (Uint32) MBUTTON::LEFT)
            if (e.motion.state == 1)
            {
              controller->buttonLeftDrag(e.motion);
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
        controller->yScroll(y_delta, x, y);
      }
    }
  }

  RenderingController* controller;
  std::deque<Sint32> wheelEvents;
  const float wheelYMult = 6.5f;
};
