#pragma once

#include <SDL_ttf.h>

#include <SDL.h>
#include <SDL_mouse.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_timer.h>

static struct Graphics
{
  SDL_Window* win;
  SDL_Renderer* ren;
  SDL_Texture* bg;
} g;

static void
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

static TTF_Font*
get_title_font(int fontSize)
{
  return TTF_OpenFont("../timelines/details/vera-fonts/dejavuSansMono.ttf", fontSize);
}

static void
renderText(TTF_Font* font, SDL_Color* color, SDL_Rect* msgBounds, const char* text, int ptsize = 40)
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
  int margins = -1 * ptsizeSmooth;
  int offsetMsgBoundsW = msgBounds->w + (2 * margins);
  int offsetMsgBoundsX = msgBounds->x - offsetMsgBoundsW - margins;
  msgBox.w = std::min(cstrW, offsetMsgBoundsW);
  msgBox.h = ptsizeSmooth;
  msgBox.x = msgBounds->x - margins; // + std::max(offsetMsgBoundsW - cstrW, 0) / 2;
  msgBox.y = msgBounds->y + std::max(msgBounds->h - msgBox.h, 0) / 2;
  //        SDL_BlitSurface(msgSurface, NULL, screenSurface, msgBounds);
  SDL_Texture* msgTexture = SDL_CreateTextureFromSurface(g.ren, msgSurface);
  SDL_RenderCopy(g.ren, msgTexture, NULL, &msgBox);

  SDL_DestroyTexture(msgTexture);
  SDL_FreeSurface(msgSurface);
}

static void
render_text_2(TTF_Font* font,
            SDL_Color* color,
            SDL_Rect* msgBounds,
            const char* text,
            int ptsize = 40)
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
