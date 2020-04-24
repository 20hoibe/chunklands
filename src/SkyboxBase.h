#ifndef __CHUNKLANDS_SKYBOXBASE_H__
#define __CHUNKLANDS_SKYBOXBASE_H__

#include "js.h"
#include "gl.h"
#include "GLSkyboxTexture.h"

namespace chunklands {
  class SkyboxBase : public JSWrap<SkyboxBase> {
    JS_DECL_WRAP(SkyboxBase)
    JS_DECL_CB_VOID(initialize)

  public:
    ~SkyboxBase();

  public:
    void Render();

  private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;

    GLSkyboxTexture texture_;
  };
}

#endif
