#ifndef __CHUNKLANDS_SCENEBASE_H__
#define __CHUNKLANDS_SCENEBASE_H__

#include "gl.h"
#include "js.h"
#include "WindowBase.h"
#include "WorldBase.h"

namespace chunklands {
  class SceneBase : public JSObjectWrap<SceneBase> {
    JS_IMPL_WRAP(SceneBase, ONE_ARG({
      JS_SETTER(Window),
      JS_SETTER(World)
    }))

    JS_DECL_SETTER_WRAP(WindowBase, Window)
    JS_DECL_SETTER_WRAP(WorldBase, World)

  public:
    virtual ~SceneBase() {
      DeleteGLBuffers();
    }
  
  public:
    void Prepare() {
      js_World->Prepare();
    }

    void Update(double diff);
    void Render(double diff);

    void UpdateViewport();
    void UpdateViewport(int width, int height);

  private:
    void InitializeGLBuffers(int width, int height);
    void DeleteGLBuffers();

  private:
    boost::signals2::scoped_connection window_on_resize_conn_;
    boost::signals2::scoped_connection window_on_cursor_move_conn_;
    glm::ivec2 last_cursor_pos_;

    glm::ivec2 buffer_size_;

    struct {
      GLuint renderbuffer     = 0;
      GLuint framebuffer      = 0;
      GLuint position_texture = 0;
      GLuint normal_texture   = 0;
      GLuint color_texture    = 0;
    } g_buffer_;

    struct {
      GLuint framebuffer   = 0;
      GLuint color_texture = 0;
      GLuint noise_texture = 0;
    } ssao_;

    struct {
      GLuint framebuffer   = 0;
      GLuint color_texture = 0;
    } ssao_blur_;
  };
}

#endif