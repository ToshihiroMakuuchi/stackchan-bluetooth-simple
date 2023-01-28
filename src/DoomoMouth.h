// Copyright (c) Shinya Ishikawa. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.
//どーもくん (C)NHK・TYO  

#ifndef DoomoMOUTH_H_
#define DoomoMOUTH_H_

#include <M5Unified.h>
#include "BoundingRect.h"
#include "DrawContext.h"
#include "Drawable.h"

namespace m5avatar {
class DoomoMouth final : public Drawable {
 private:
  uint16_t minWidth;
  uint16_t maxWidth;
  uint16_t minHeight;
  uint16_t maxHeight;

 public:
  // constructor
  DoomoMouth() = delete;
  ~DoomoMouth() = default;
  DoomoMouth(const DoomoMouth &other) = default;
  DoomoMouth &operator=(const DoomoMouth &other) = default;
  DoomoMouth(uint16_t minWidth, uint16_t maxWidth, uint16_t minHeight,
        uint16_t maxHeight);
  void draw(M5Canvas *spi, BoundingRect rect,
            DrawContext *drawContext) override;
};

}  // namespace m5avatar

#endif  // DoomoMOUTH_H_
