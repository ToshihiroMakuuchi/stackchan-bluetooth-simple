// Copyright (c) Shinya Ishikawa. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#include "Face.h"

// TODO(meganetaaan): move to another file
void transformSprite(M5Canvas *from, M5Canvas *to, float r, float s) {
  int width = to->width();
  int height = to->height();
  int fromWidth = from->width();
  int fromHeight = from->height();
  int cx = width / 2;
  int cy = height / 2;
  float cosr = cos(r);
  float sinr = sin(r);

  for (int y2 = 0; y2 < height; y2++) {
    for (int x2 = 0; x2 < width; x2++) {
      int x1 = ((((x2 - cx) * cosr) - ((y2 - cy) * sinr)) + cx) / s;
      int y1 = ((((x2 - cx) * sinr) + ((y2 - cy) * cosr)) + cy) / s;
      if (x1 < 0 || x1 >= fromWidth || y1 < 0 || y1 >= fromHeight) {
        continue;
      }
      int color = from->readPixel(x1, y1);
      to->drawPixel(x2, y2, color);
    }
  }
}

namespace m5avatar {
Balloon b;
Effect h;
BoundingRect br;

Face::Face()
    : Face(new Mouth(50, 90, 4, 60), new BoundingRect(148, 163),
           new Eye(8, false), new BoundingRect(93, 90), new Eye(8, true),
           new BoundingRect(96, 230), new Eyeblow(32, 0, false),
           new BoundingRect(67, 96), new Eyeblow(32, 0, true),
           new BoundingRect(72, 230)) {}

Face::Face(Drawable *mouth, Drawable *eyeR, Drawable *eyeL, Drawable *eyeblowR,
           Drawable *eyeblowL)
    : Face(mouth, new BoundingRect(148, 163), eyeR, new BoundingRect(93, 90),
           eyeL, new BoundingRect(96, 230), eyeblowR, new BoundingRect(67, 96),
           eyeblowL, new BoundingRect(72, 230)) {}

Face::Face(Drawable *mouth, BoundingRect *mouthPos, Drawable *eyeR,
           BoundingRect *eyeRPos, Drawable *eyeL, BoundingRect *eyeLPos,
           Drawable *eyeblowR, BoundingRect *eyeblowRPos, Drawable *eyeblowL,
           BoundingRect *eyeblowLPos)
    : mouth{mouth},
      eyeR{eyeR},
      eyeL{eyeL},
      eyeblowR{eyeblowR},
      eyeblowL{eyeblowL},
      mouthPos{mouthPos},
      eyeRPos{eyeRPos},
      eyeLPos{eyeLPos},
      eyeblowRPos{eyeblowRPos},
      eyeblowLPos{eyeblowLPos},
      boundingRect{new BoundingRect(0, 0, 320, 240)},
      sprite{new M5Canvas(&M5.Lcd)},
      tmpSpr{new M5Canvas(&M5.Lcd)} {}

Face::Face(LGFX_Device* device)
    : Face(new Mouth(50, 90, 4, 60), new BoundingRect(148, 163),
           new Eye(8, false), new BoundingRect(93, 90), new Eye(8, true),
           new BoundingRect(96, 230), new Eyeblow(32, 0, false),
           new BoundingRect(67, 96), new Eyeblow(32, 0, true),
           new BoundingRect(72, 230), device) {}

Face::Face(Drawable *mouth, Drawable *eyeR, Drawable *eyeL, Drawable *eyeblowR,
           Drawable *eyeblowL, LGFX_Device* device)
    : Face(mouth, new BoundingRect(148, 163), eyeR, new BoundingRect(93, 90),
           eyeL, new BoundingRect(96, 230), eyeblowR, new BoundingRect(67, 96),
           eyeblowL, new BoundingRect(72, 230), device) {}

Face::Face(Drawable *mouth, BoundingRect *mouthPos, Drawable *eyeR,
           BoundingRect *eyeRPos, Drawable *eyeL, BoundingRect *eyeLPos,
           Drawable *eyeblowR, BoundingRect *eyeblowRPos, Drawable *eyeblowL,
           BoundingRect *eyeblowLPos, LGFX_Device* device)
    : mouth{mouth},
      eyeR{eyeR},
      eyeL{eyeL},
      eyeblowR{eyeblowR},
      eyeblowL{eyeblowL},
      mouthPos{mouthPos},
      eyeRPos{eyeRPos},
      eyeLPos{eyeLPos},
      eyeblowRPos{eyeblowRPos},
      eyeblowLPos{eyeblowLPos},
      boundingRect{new BoundingRect(0, 0, 320, 240)},
      sprite{new M5Canvas(device)},
      tmpSpr{new M5Canvas(device)} {}

Face::~Face() {
  delete mouth;
  delete mouthPos;
  delete eyeR;
  delete eyeRPos;
  delete eyeL;
  delete eyeLPos;
  delete eyeblowR;
  delete eyeblowRPos;
  delete eyeblowL;
  delete eyeblowLPos;
  delete sprite;
  delete tmpSpr;
  delete boundingRect;
}

void Face::setMouth(Drawable *mouth) { this->mouth = mouth; }

void Face::setLeftEye(Drawable *eyeL) { this->eyeL = eyeL; }

void Face::setRightEye(Drawable *eyeR) { this->eyeR = eyeR; }

Drawable *Face::getMouth() { return mouth; }

Drawable *Face::getLeftEye() { return eyeL; }

Drawable *Face::getRightEye() { return eyeR; }

BoundingRect *Face::getBoundingRect() { return boundingRect; }

void Face::draw(DrawContext *ctx) {
  sprite->setColorDepth(ctx->getColorDepth());
  sprite->setPaletteColor(1, 0x000000U);    // �p���b�g1�Ԃ����ɐݒ�
  sprite->setPaletteColor(2, 0xFFFFFFU);    // �p���b�g2�Ԃ𔒂ɐݒ�
  sprite->setPaletteColor(3, 0xFF0000U);    // �p���b�g3�Ԃ�Ԃɐݒ�
//  sprite->setPaletteColor(4, 0xF6F399U);    // �p���b�g4�Ԃ�X�ɐݒ�
  sprite->setPaletteColor(4, 0x0000FFU);    // �p���b�g4�Ԃ�ɐݒ�
  sprite->setPaletteColor(5, 0x00FF00U);    // �p���b�g5�Ԃ�΂ɐݒ�
  sprite->setPaletteColor(6, 0xFFFF00U);    // �p���b�g6�Ԃ����F�ɐݒ�
  sprite->setPaletteColor(7, 0xFFC0CBU);    // �p���b�g7�Ԃ��s���N�ɐݒ�
  sprite->setPaletteColor(8, 0x964800U);    // �p���b�g8�Ԃ�BROWN�ɐݒ�
  sprite->setPaletteColor(9, 0xEEE8AAU);    // �p���b�g9�Ԃ�BROWN�ɐݒ�
  sprite->setPaletteColor(10, 0xD3D3D3U);    // �p���b�g10�Ԃ�LIGHTGREY�ɐݒ�
  
  // NOTE: setting below for 1-bit color depth
  sprite->setBitmapColor(ctx->getColorPalette()->get(COLOR_PRIMARY),
    ctx->getColorPalette()->get(COLOR_BACKGROUND));
  sprite->createSprite(boundingRect->getWidth(), boundingRect->getHeight());
  if (ctx->getColorDepth() != 1) {
    sprite->fillSprite(ctx->getColorPalette()->get(COLOR_BACKGROUND));
  }
  float breath = _min(1.0f, ctx->getBreath());

  // TODO(meganetaaan): unify drawing process of each parts
  BoundingRect rect = *mouthPos;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  // copy context to each draw function
  mouth->draw(sprite, rect, ctx);

  rect = *eyeRPos;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  eyeR->draw(sprite, rect, ctx);

  rect = *eyeLPos;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  eyeL->draw(sprite, rect, ctx);

  rect = *eyeblowRPos;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  eyeblowR->draw(sprite, rect, ctx);

  rect = *eyeblowLPos;
  rect.setPosition(rect.getTop() + breath * 3, rect.getLeft());
  eyeblowL->draw(sprite, rect, ctx);

  // TODO(meganetaaan): make balloons and effects selectable
  b.draw(sprite, br, ctx);
  h.draw(sprite, br, ctx);
  // drawAccessory(sprite, position, ctx);

  // TODO(meganetaaan): rethink responsibility for transform function
  float scale = ctx->getScale();
  float rotation = ctx->getRotation();
  int offsetX = ctx->getOffsetX();
  int offsetY = ctx->getOffsetY();

  if (scale != 1.0 || rotation != 0) {
    uint16_t w = boundingRect->getWidth();
    uint16_t h = boundingRect->getHeight();
    sprite->pushRotateZoom((int)(w/2)+offsetX, (int)(h/2)+offsetY, (float)rotation, (float)scale, (float)scale);
  } else {
    sprite->pushSprite(boundingRect->getLeft(), boundingRect->getTop());
  }
  sprite->deleteSprite();
}
}  // namespace m5avatar
