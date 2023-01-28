// Copyright (c) robo8080. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.
//どーもくん (C)NHK・TYO  

#ifndef FACES_DoomoFACE_H_
#define FACES_DoomoFACE_H_

#include <M5Unified.h>
#include "DoomoEye.h"
#include "DoomoMouth.h"
#include "Eyeblow.h"

namespace m5avatar {
class DoomoFace : public Face {
 public:
  DoomoFace()
      : Face(new DoomoMouth(140, 140, 40, 100), new BoundingRect(110, 160),  //Doomoface
             new DoomoEye(18, false), new BoundingRect(50, 75),
             new DoomoEye(18, true),  new BoundingRect(50, 245),
             new Eyeblow(32, 0, false), new BoundingRect(67, 96),
             new Eyeblow(32, 0, true), new BoundingRect(72, 230)) {}
  DoomoFace(LGFX_Device* device)
      : Face(new DoomoMouth(140, 140, 40, 100), new BoundingRect(110, 160),  //Doomoface
             new DoomoEye(18, false), new BoundingRect(50, 75),
             new DoomoEye(18, true),  new BoundingRect(50, 245),
             new Eyeblow(32, 0, false), new BoundingRect(67, 96),
             new Eyeblow(32, 0, true), new BoundingRect(72, 230), device) {}
};

}  // namespace m5avatar

#endif  // FACES_DoomoFACE_H_
