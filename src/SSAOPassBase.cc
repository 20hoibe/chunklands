#include "SSAOPassBase.h"

#include <random>

#define  GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

namespace chunklands {
  JS_DEF_WRAP(SSAOPassBase)

  void SSAOPassBase::InitializeProgram() {
    uniforms_ = {
      .proj     = js_Program->GetUniformLocation("u_proj"),
    };
    
    glUniform1i(js_Program->GetUniformLocation("u_position"), 0);
    glUniform1i(js_Program->GetUniformLocation("u_normal"),   1);
    glUniform1i(js_Program->GetUniformLocation("u_noise"),    2);

    std::uniform_real_distribution<GLfloat> random_floats(0.f, 1.f);
    std::default_random_engine generator;
    for (int i = 0; i < 64; i++) {
      glm::vec3 sample(
        random_floats(generator) * 2.f - 1.f,
        random_floats(generator) * 2.f - 1.f,
        random_floats(generator)
      );

      sample = glm::normalize(sample);
      sample *= random_floats(generator);
      float scale = float(i) / 64.f;

      scale = glm::lerp(.1f, 1.f, scale * scale);
      sample *= scale;

      std::string uniform = std::string("u_samples[") + std::to_string(i) + "]";
      GLint location = js_Program->GetUniformLocation(uniform.c_str());
      glUniform3fv(location, 1, glm::value_ptr(sample));
    }
  }
}
