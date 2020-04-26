#ifndef __CHUNKLANDS_ARENDERPASS_H__
#define __CHUNKLANDS_ARENDERPASS_H__

#include "js.h"
#include "GLProgramBase.h"

namespace chunklands {
  class ARenderPass {
    JS_ATTR_WRAP(GLProgramBase, Program)

  public:
    void Begin() {
      js_Program->Use();
    }

    void End() {
      js_Program->Unuse();
    }
  
  protected:
    virtual void InitializeProgram() {}

    void JSCall_SetProgram(JSCbi info) {
      js_Program = info[0];

      js_Program->Use();
      InitializeProgram();
      js_Program->Unuse();
    }
  };
}

#endif