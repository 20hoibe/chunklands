#ifndef __CHUNKLANDS_WORLDBASE_H__
#define __CHUNKLANDS_WORLDBASE_H__

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <boost/functional/hash.hpp>
#include <napi.h>
#include <queue>
#include <unordered_map>
#include <vector>
#include "Chunk.h"
#include "ChunkGeneratorBase.h"
#include "gl.h"
#include "napi/object_wrap_util.h"
#include "napi/PersistentObjectWrap.h"
#include "GLProgramBase.h"
#include "RenderQuad.h"
#include "SkyboxBase.h"
#include "GBufferPass.h"

namespace chunklands {

  class WorldBase : public Napi::ObjectWrap<WorldBase> {
    DECLARE_OBJECT_WRAP(WorldBase)
    DECLARE_OBJECT_WRAP_CB(void SetChunkGenerator)
    DECLARE_OBJECT_WRAP_CB(void SetGBufferShader)
    DECLARE_OBJECT_WRAP_CB(void SetSSAOShader)
    DECLARE_OBJECT_WRAP_CB(void SetSSAOBlurShader)
    DECLARE_OBJECT_WRAP_CB(void SetLightingShader)
    DECLARE_OBJECT_WRAP_CB(void SetSkyboxShader)
    DECLARE_OBJECT_WRAP_CB(void SetSkybox)

  private:
    struct ivec3_hasher {
      std::size_t operator()(const glm::ivec3& v) const {
        std::size_t seed = 0;
        boost::hash_combine(seed, boost::hash_value(v.x));
        boost::hash_combine(seed, boost::hash_value(v.y));
        boost::hash_combine(seed, boost::hash_value(v.z));

        return seed;
      }
    };

  public:
    void Prepare();
    void Update(double diff);
    void RenderGBufferPass(double diff);
    void RenderSSAOPass(double diff, GLuint position_texture, GLuint normal_texture, GLuint noise_texture);
    void RenderSSAOBlurPass(double diff, GLuint ssao_texture);
    void RenderDeferredLightingPass(double diff, GLuint position_texture, GLuint normal_texture, GLuint color_texture, GLuint ssao_texture);
    void RenderSkybox(double diff);

    void UpdateViewportRatio(int width, int height);

    void AddLook(float yaw_rad, float pitch_rad);
    void AddPos(const glm::vec3& v);

    const glm::vec2& GetLook() const {
      return look_;
    }

  private:

    NapiExt::PersistentObjectWrap<ChunkGeneratorBase> chunk_generator_;
    GBufferPass g_buffer_pass;
    NapiExt::PersistentObjectWrap<GLProgramBase> ssao_shader_;
    NapiExt::PersistentObjectWrap<GLProgramBase> lighting_shader_;
    NapiExt::PersistentObjectWrap<GLProgramBase> skybox_shader_;
    NapiExt::PersistentObjectWrap<GLProgramBase> ssao_blur_shader_;

    NapiExt::PersistentObjectWrap<SkyboxBase> skybox_;

    std::unordered_map<glm::ivec3, std::shared_ptr<Chunk>, ivec3_hasher> chunk_map_;

    glm::vec3 pos_ = glm::vec3(8.f, 0.7f, 60.f);
    glm::vec2 look_;

    glm::mat4 view_;
    glm::mat4 proj_;

    glm::mat4 view_skybox_;

    std::vector<glm::ivec3> nearest_chunks_;

    struct {
      GLint view    = -1;
      GLint proj    = -1;
      GLint texture = -1;
    } g_buffer_uniforms_;

    struct {
      GLint proj     = -1;
      GLint position = -1;
      GLint normal   = -1;
      GLint noise    = -1;
    } ssao_uniforms_;

    struct {
      GLint ssao = -1;
    } ssao_blur_uniforms_;

    struct {
      GLint position        = -1;
      GLint normal          = -1;
      GLint color           = -1;
      GLint ssao            = -1;
      GLint render_distance = -1;
      GLint sun_position    = -1;
    } lighting_uniforms_;

    struct {
      GLint view   = -1;
      GLint proj   = -1;
      GLint skybox = -1;
    } skybox_uniforms_;

    std::unique_ptr<RenderQuad> render_quad_;
  };
}



#endif
