//
// Created by j on 1/13/20.
//

#pragma once
#include <SDL_quit.h>
#include <SDL_render.h>
#include <algorithm>
#include <cmath>
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
