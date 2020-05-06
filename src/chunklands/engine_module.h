#ifndef __CHUNKLANDS_ENGINE_H__
#define __CHUNKLANDS_ENGINE_H__

#include "js.h"
#include "gl_module.h"
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/signals2.hpp>

namespace chunklands::engine {
  class IScene {
  public:
    virtual ~IScene() {}

  public:
    virtual void Prepare() = 0;
    virtual void Update(double update_diff) = 0;
    virtual void Render(double render_diff) = 0;
  };

  class RenderPass {
    JS_ATTR_WRAP(gl::ProgramBase, Program)

  public:
    virtual void Begin() {
      js_Program->Use();
    }

    virtual void End() {
      js_Program->Unuse();
    }

    virtual void UpdateBufferSize(int /*width*/, int /*height*/) {}
  
  protected:
    virtual void InitializeProgram() {}

    void JSCall_SetProgram(JSCbi info) {
      js_Program = info[0];

      js_Program->Use();
      InitializeProgram();
      js_Program->Unuse();
    }
  };

  class Environment : public JSObjectWrap<Environment> {
    JS_IMPL_WRAP(Environment, ONE_ARG({
      JS_CB_STATIC(initialize),
      JS_CB_STATIC(loadProcs),
      JS_CB_STATIC(terminate),
    }))

    JS_DECL_CB_STATIC_VOID(initialize)
    JS_DECL_CB_STATIC_VOID(loadProcs)
    JS_DECL_CB_STATIC_VOID(terminate)
  };

  class GameLoop : public JSObjectWrap<GameLoop> {
    JS_IMPL_WRAP(GameLoop, ONE_ARG({
      JS_CB(start),
      JS_CB(stop),
      JS_CB(loop),
      JS_SETTER(Scene)
    }))

    JS_DECL_CB_VOID(start)
    JS_DECL_CB_VOID(stop)
    JS_DECL_CB_VOID(loop)
    JS_IMPL_ABSTRACT_WRAP_SETTER(IScene, Scene)

  private:
    bool running_ = false;
    double last_update_ = .0;
    double last_render_ = .0;
  };

  class GBufferPass : public JSObjectWrap<GBufferPass>, public RenderPass {
    JS_IMPL_WRAP(GBufferPass, ONE_ARG({
      JS_SETTER(Program)
    }))

  public:
    virtual ~GBufferPass() {
      DeleteBuffers();
    }

  public:
    void Begin() override {
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
      glClearColor(0.f, 0.f, 0.f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      RenderPass::Begin();
    }

    void End() override {
      RenderPass::End();
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void UpdateProjection(const glm::mat4& proj) {
      glUniformMatrix4fv(uniforms_.proj, 1, GL_FALSE, glm::value_ptr(proj));
    }

    void UpdateView(const glm::mat4& view) {
      glUniformMatrix4fv(uniforms_.view, 1, GL_FALSE, glm::value_ptr(view));
    }

    void UpdateBufferSize(int width, int height) override;
    void DeleteBuffers();

  protected:
    void InitializeProgram() override;

  public:
    struct {
      gl::Uniform proj {"u_proj"},
                  view {"u_view"};
    } uniforms_;

    GLuint renderbuffer_ = 0;
    GLuint framebuffer_  = 0;

    struct {
      GLuint position = 0;
      GLuint normal   = 0;
      GLuint color    = 0;
    } textures_;
  };

  class LightingPass : public JSObjectWrap<LightingPass>, public RenderPass {
    JS_IMPL_WRAP(LightingPass, ONE_ARG({
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
      gl::Uniform render_distance{"u_render_distance"},
                sun_position{"u_sun_position"};
    } uniforms_;
  };

  class SkyboxPass : public JSObjectWrap<SkyboxPass>, public RenderPass {
    JS_IMPL_WRAP(SkyboxPass, ONE_ARG({
      JS_SETTER(Program)
    }))

  public:
    void Begin() override {
      glDisable(GL_CULL_FACE);
      glDepthFunc(GL_LEQUAL);
      RenderPass::Begin();
    }

    void End() override {
      RenderPass::End();
      glDepthFunc(GL_LESS);
      glEnable(GL_CULL_FACE);
    }

    void UpdateProjection(const glm::mat4& matrix) {
      glUniformMatrix4fv(uniforms_.proj, 1, GL_FALSE, glm::value_ptr(matrix));
    }
    
    void UpdateView(const glm::mat4& matrix) {
      glUniformMatrix4fv(uniforms_.view, 1, GL_FALSE, glm::value_ptr(matrix));
    }

    void BindSkyboxTexture(GLuint texture) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
    }

  protected:
    void InitializeProgram() override;
    
  private:

    struct {
      gl::Uniform proj {"u_proj"},
                  view {"u_view"};
    } uniforms_;
  };

  class SkyboxTexture {
  public:
    ~SkyboxTexture() {
      glDeleteTextures(1, &texture_);
    }

  public:
    void LoadTexture(const std::string& prefix);
    void ActiveAndBind(GLenum texture) {
      glActiveTexture(texture);
      glBindTexture(GL_TEXTURE_CUBE_MAP, texture_);
    }
  
  private:
    GLuint texture_ = 0;
  };

  class SSAOBlurPass : public JSObjectWrap<SSAOBlurPass>, public RenderPass {
    JS_IMPL_WRAP(SSAOBlurPass, ONE_ARG({
      JS_SETTER(Program)
    }))

  public:
    void Begin() override {
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
      glClear(GL_COLOR_BUFFER_BIT);
      RenderPass::Begin();
    }

    void End() override {
      RenderPass::End();
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void BindSSAOTexture(GLuint texture) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
    }

    void UpdateBufferSize(int width, int height) override;
    void DeleteBuffers();

  protected:
    void InitializeProgram() override {
      gl::Uniform ssao{"u_ssao"};
      *js_Program >> ssao;
      ssao.Update(0);
    }

  public:
    GLuint framebuffer_ = 0;

    struct {
      GLuint color = 0;
    } textures_;
  };

  class SSAOPass : public JSObjectWrap<SSAOPass>, public RenderPass {
    JS_IMPL_WRAP(SSAOPass, ONE_ARG({
      JS_SETTER(Program)
    }))

  public:
    ~SSAOPass() {
      DeleteBuffers();
    }

  public:
    void Begin() {
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
      glClearColor(0.f, 0.f, 0.f, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);
      RenderPass::Begin();
      BindNoiseTexture();
    }

    void End() {
      RenderPass::End();
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void UpdateProjection(const glm::mat4& proj) {
      glUniformMatrix4fv(uniforms_.proj, 1, GL_FALSE, glm::value_ptr(proj));
    }

    void BindPositionTexture(GLuint texture) {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, texture);
    }

    void BindNormalTexture(GLuint texture) {
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, texture);
    }

    void UpdateBufferSize(int width, int height) override;
    void DeleteBuffers();

  protected:
    void InitializeProgram() override;

  private:
    void BindNoiseTexture() {
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, textures_.noise);
    }

  public:
    struct {
      gl::Uniform proj{"u_proj"};
    } uniforms_;

    GLuint framebuffer_ = 0;

    struct {
      GLuint color = 0;
      GLuint noise = 0;
    } textures_;
  };

  class Window : public JSObjectWrap<Window> {
    JS_IMPL_WRAP(Window, ONE_ARG({
      JS_CB(initialize),
      JS_CB(makeContextCurrent),
      JS_CB(shouldClose),
      JS_CB(close)
    }))

    JS_DECL_CB_VOID(initialize)
    JS_DECL_CB_VOID(makeContextCurrent)
    JS_DECL_CB(shouldClose)
    JS_DECL_CB_VOID(close)

  public:
    void SwapBuffers();

    int GetKey(int key);
    int GetMouseButton(int button);
    glm::dvec2 GetCursorPos() const;

    void StartMouseGrab();
    void StopMouseGrab();

    glm::ivec2 GetSize() const;

  private:
    void UpdateViewport(int width, int height);

  public:
    boost::signals2::signal<void(int width, int height)> on_resize;
    boost::signals2::signal<void(double xpos, double ypos)> on_cursor_move;

  private:
    GLFWwindow* window_ = nullptr;
  };

  class Skybox : public JSObjectWrap<Skybox> {
    JS_IMPL_WRAP(Skybox, ONE_ARG({
      JS_CB(initialize)
    }))

    JS_DECL_CB_VOID(initialize)

  public:
    ~Skybox() {
      DeleteGLArrays();
    }

    void DeleteGLArrays() {
      glDeleteBuffers(1, &vbo_);
      glDeleteVertexArrays(1, &vao_);
    }

  public:
    void Render();

  private:
    GLuint vao_ = 0;
    GLuint vbo_ = 0;

    SkyboxTexture texture_;
  };
}

#endif