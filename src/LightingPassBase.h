#ifndef __CHUNKLANDS_LIGHTINGPASSBASE_H__
#define __CHUNKLANDS_LIGHTINGPASSBASE_H__

#include "gl.h"
#include "js.h"
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "RenderPass.h"
#include "GLUniform.h"

namespace chunklands {
  class LightingPassBase : public JSObjectWrap<LightingPassBase>, public RenderPass {
    JS_IMPL_WRAP(LightingPassBase, ONE_ARG({
      JS_SETTER(Program)
    }))

  public:

    void Begin() override {
      glClearColor(0.f, 0.f, 0.f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      RenderPass::Begin();
    }

    void End() override {
      RenderPass::End();
    }

    void UpdateRenderDistance(GLfloat value) {
      glUniform1f(uniforms_.render_distance, value);
    }

    void UpdateSunPosition(const glm::vec3& value) {
      glUniform3fv(uniforms_.sun_position, 1, glm::value_ptr(value));
    }

    void BindPositionTexture(GLuint texture) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
    }

    void BindNormalTexture(GLuint texture) {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, texture);
    }
    
    void BindColorTexture(GLuint texture) {
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, texture);
    }

    void BindSSAOTexture(GLuint texture) {
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, texture);
    }

  protected:
    void InitializeProgram() override;

  private:
    struct {
      GLUniform render_distance{"u_render_distance"},
                sun_position{"u_sun_position"};
    } uniforms_;
  };
}

#endif
