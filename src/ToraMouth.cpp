// Copyright (c) Shinya Ishikawa. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.
// modified by robo8080

#include "ToraMouth.h"

namespace m5avatar {

ToraMouth::ToraMouth(uint16_t minWidth, uint16_t maxWidth, uint16_t minHeight,
             uint16_t maxHeight)
    : minWidth{minWidth},
      maxWidth{maxWidth},
      minHeight{minHeight},
      maxHeight{maxHeight} {}

void ToraMouth::draw(M5Canvas *spi, BoundingRect rect, DrawContext *ctx) {
  Expression exp = ctx->getExpression();
  uint16_t primaryColor = ctx->getColorPalette()->get(COLOR_PRIMARY);
  uint16_t backgroundColor = ctx->getColorPalette()->get(COLOR_BACKGROUND);
  float breath = _min(1.0f, ctx->getBreath());
  float openRatio = ctx->getMouthOpenRatio();
  Gaze g = ctx->getGaze();                    //
  uint32_t offsetY = g.getVertical() * 10;    //
  int h = minHeight + (maxHeight - minHeight) * openRatio;
  int w = minWidth + (maxWidth - minWidth) * (1 - openRatio);
  int x = rect.getLeft() - w / 2;
//  int y = rect.getTop() - h / 2 + breath * 2;
  int y = rect.getTop() + breath * 2;
  if (openRatio == 0.0)
  {
//    y = rect.getTop() - h / 2 + breath * 2;
    h = maxHeight;
//    spi->fillRect(x-2, y-2, maxWidth+4, h+4, primaryColor);
    spi->fillRect(x, y, maxWidth, h, 3);
//    for(int i=0;i<4;i++)
//    spi->fillTriangle(x+35*i, y, x+17+35*i, y+20, x+35+35*i, y, TFT_WHITE);
//    spi->fillTriangle(x+35*i, y, x+17+35*i, y-20, x+35+35*i, y, TFT_WHITE);
  } else {
//    spi->fillRect(x-2, y-2, maxWidth+4, h+4, primaryColor);
    spi->fillRect(x, y, maxWidth, h, 3);
  }
    for(int i=0;i<4;i++){
      spi->fillTriangle(x+35*i, y, x+17+35*i, y+20, x+35+35*i, y, 10);
      spi->fillTriangle(x+35*i, y+h, x+17+35*i, y-20+h, x+35+35*i, y+h, 10);
    }

  spi->fillEllipse(x+w/2, y-45, 20, 15, 1);
  spi->fillEllipse(x+5+15, y-20, w/2-10, 30, 2);
  spi->fillEllipse(x+w-5-15, y-20, w/2-10, 30, 2);

  spi->fillEllipse(x+w/2, 8, 50, 7, 1);
  spi->fillEllipse(x+w/2, 30, 20, 5, 1);
  spi->fillEllipse(x+w/2, 0, 10, 50, 1);

  spi->fillEllipse(0, y-30, 40, 8, 1);
  spi->fillEllipse(0, y, 40, 8, 1);

  spi->fillEllipse(320, y-30, 40, 8, 1);
  spi->fillEllipse(320, y, 40, 8, 1);



}

}  // namespace m5avatar
