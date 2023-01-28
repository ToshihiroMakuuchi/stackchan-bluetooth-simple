// Copyright (c) Shinya Ishikawa. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.
// modified by robo8080

#include "DannEye.h"
namespace m5avatar {

DannEye::DannEye(uint16_t x, uint16_t y, uint16_t r, bool isLeft) : DannEye(r, isLeft) {}

DannEye::DannEye(uint16_t r, bool isLeft) : r{r}, isLeft{isLeft} {}
void DannEye::draw(M5Canvas *spi, BoundingRect rect, DrawContext *ctx) {
  Expression exp = ctx->getExpression();
  uint32_t x = rect.getCenterX();
  uint32_t y = rect.getCenterY();
  Gaze g = ctx->getGaze();
  float openRatio = ctx->getEyeOpenRatio();
  uint32_t offsetX = g.getHorizontal() * 5;
  uint32_t offsetY = g.getVertical() * 7;
  uint32_t primaryColor = ctx->getColorPalette()->get(COLOR_PRIMARY);
  uint32_t secondaryColor = ctx->getColorPalette()->get(COLOR_SECONDARY);
  uint32_t backgroundColor = ctx->getColorPalette()->get(COLOR_BACKGROUND);
  if (openRatio > 0) {
    spi->fillEllipse(x , y , r + 8, r , (uint16_t)primaryColor);
    spi->fillEllipse(x , y , r + 6, r-2 , (uint16_t)secondaryColor);
    if (exp != Expression::Happy) {
      spi->fillCircle(x + offsetX, y + offsetY, 13, (uint16_t)primaryColor);
      spi->fillCircle(x + offsetX-4, y + offsetY-4, 3, (uint16_t)secondaryColor);
    }
    // TODO(meganetaaan): Refactor
    if (exp == Expression::Angry || exp == Expression::Sad) {
      int x0, y0, x1, y1, x2, y2;
      x0 = x - r -8 ;
      y0 = y - r ;
      x1 = x0 + (r+8) * 2;
      y1 = y0;
      x2 = !isLeft != !(exp == Expression::Sad) ? x0 : x1;
      y2 = y0 + r;
      spi->fillTriangle(x0, y0, x1, y1, x2, y2, (uint16_t)backgroundColor);
    }
    if (exp == Expression::Happy || exp == Expression::Sleepy) {
      int x0, y0, w, h;
      x0 = x - r;
      y0 = y - r;
      w = r * 2 + 4;
      h = r + 7;
      if (exp == Expression::Happy) {
        y0 += (r + 5);
        spi->fillEllipse(x , y , r + 8, r , (uint16_t)primaryColor);
        spi->fillEllipse(x , y , r + 3, r - 5, (uint16_t)backgroundColor);
      }
      spi->fillRect(x0-8, y0-3, w+15, h, (uint16_t)backgroundColor);
    }
  } else {
    int x1 = x - r + offsetX;
    int y1 = y - 2 + offsetY;
    int w = r * 2;
    int h = 4;
    spi->fillRect(x1, y1, w, h, (uint16_t)primaryColor);
  }
}
}  // namespace m5avatar
