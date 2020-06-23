#ifndef __CHUNKLANDS_ENGINE_H__
#define __CHUNKLANDS_ENGINE_H__

#include <chunklands/debug.hxx>
#include <chunklands/js.hxx>
#include <chunklands/modules/gl/gl_module.hxx>
#include <glm/vec3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <boost/signals2.hpp>
#include <chunklands/math.hxx>
#include <unordered_map>

namespace chunklands::modules::engine {
  
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
    virtual ~RenderPass();

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
    void Begin() override {
      glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
      glClearColor(0.f, 0.f, 0.f, 1.0);
      glClear(GL_COLOR_BUFFER_BIT);
      RenderPass::Begin();
      BindNoiseTexture();
    }

    void End() override {
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

  class GameOverlayRenderer : public JSObjectWrap<GameOverlayRenderer>, public RenderPass {
    JS_IMPL_WRAP(GameOverlayRenderer, ONE_ARG({
      JS_SETTER(Program)
    }))

  public:
    virtual void Begin() override {
      glDepthFunc(GL_ALWAYS);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      RenderPass::Begin();
    }

    virtual void End() override {
      RenderPass::End();
      glDepthFunc(GL_LESS);
      glDisable(GL_BLEND);
    }

    void UpdateProjection() {
      glUniformMatrix4fv(uniforms_.proj, 1, GL_FALSE, glm::value_ptr(proj_));
    }

    void UpdateBufferSize(int width, int height) override {
      const float scale = 4.f; // TODO(daaitch): use window pixel scale (4k screens)
      proj_ = glm::ortho(-float(width) / scale, float(width) / scale, -float(height) / scale, float(height) / scale);
    }

  protected:
    void InitializeProgram() override {
      gl::Uniform texture{"u_texture"};
      *js_Program >> texture >> uniforms_.proj;
      texture.Update(0);
    }

  private:
    struct {
      gl::Uniform proj{"u_proj"};
    } uniforms_;

    glm::mat4 proj_;
  };

  class Window : public JSObjectWrap<Window> {
    JS_IMPL_WRAP(Window, ONE_ARG({
      JS_CB(initialize),
      JS_CB(makeContextCurrent),
      JS_CB(shouldClose),
      JS_CB(close),
      JS_GETTER(GameControl),
      JS_SETTER(GameControl),
    }))

    JS_DECL_CB_VOID(initialize)
    JS_DECL_CB_VOID(makeContextCurrent)
    JS_DECL_CB(shouldClose)
    JS_DECL_CB_VOID(close)

  protected:
    JSValue JSCall_GetGameControl(JSCbi info) {
      return JSBoolean::New(info.Env(), game_control_);
    }

    void JSCall_SetGameControl(JSCbi info) {
      bool next_game_control = info[0].ToBoolean().Value();
      if (next_game_control == game_control_) {
        return;
      }

      if (next_game_control) {
        StartMouseGrab();
        game_control_ = true;
      } else {
        StopMouseGrab();
        game_control_ = false;
      }
    }

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
    static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);

  public:
    boost::signals2::signal<void(int width, int height)> on_resize;
    boost::signals2::signal<void(double xdiff, double ydiff)> on_game_control_look;

    JSObjectRef events_;

  private:
    GLFWwindow* window_ = nullptr;
    bool game_control_ = false;
    glm::dvec2 game_control_last_cursor_pos_;
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

  class Camera : public JSObjectWrap<Camera> {
    JS_IMPL_WRAP(Camera, ONE_ARG({
      JS_SETTER(Position),
      JS_GETTER(Position),
      JS_GETTER(Look),
    }))

    JS_DECL_CB_VOID(SetPosition)
    JS_DECL_CB(GetPosition)
    JS_DECL_CB(GetLook)

  public:
    void UpdateViewportRatio(int width, int height);

    void Update(double diff);
    
    void AddLook(float yaw_rad, float pitch_rad);
    void AddPos(const glm::vec3& v) {
      pos_ += v;
    }

    const glm::vec3& GetPosition() const {
      return pos_;
    }

    const glm::vec2& GetLook() const {
      return look_;
    }

    const glm::mat4& GetProjection() const {
      return proj_;
    }

    const glm::mat4& GetView() const {
      return view_;
    }

    const glm::mat4& GetViewSkybox() const {
      return view_skybox_;
    }
  private:

    glm::vec3 pos_ {0.f, 0.f, 0.f};
    glm::vec2 look_ {0.f, 0.f};

    glm::mat4 view_;
    glm::mat4 proj_;
    
    glm::mat4 view_skybox_;
  };

  struct collision_result {
    int prio;
    int axis;
    float ctime;
    math::fvec3 collisionfree_movement;
    math::fvec3 outstanding_movement;
  };

  std::ostream& operator<<(std::ostream& os, const collision_result& c);

  class ICollisionSystem {
  public:
    virtual collision_result ProcessNextCollision(const math::fAABB3 &box, const math::fvec3 &movement) = 0;
  };

  class MovementController : public JSObjectWrap<MovementController> {
    JS_IMPL_WRAP(MovementController, ONE_ARG({
      JS_SETTER(CollisionSystem),
      JS_SETTER(Camera),
    }))

    JS_IMPL_ABSTRACT_WRAP_SETTER(ICollisionSystem, CollisionSystem)
    JS_IMPL_SETTER_WRAP(Camera, Camera)

  public:
    int AddMovement(math::fvec3 outstanding_movement);

  private:
    math::fAABB3 player_box_ {
      {-.4f, -1.4f, -.4f},
      { .8f,  1.6f,  .8f}
    };
  };

  struct font_loader_char {
    glm::ivec2  pos,
                size,
                origin;
    int         advance;
  };

  class FontLoader : public JSObjectWrap<FontLoader> {
    JS_IMPL_WRAP(FontLoader, ONE_ARG({
      JS_CB(load),
    }))

    JS_DECL_CB_VOID(load)

  public:
  
    const std::optional<font_loader_char> Get(const std::string& ch) const {
      auto&& it = meta_.find(ch);
      if (it == meta_.cend()) {
        return {};
      }

      return {it->second};
    }

    const gl::Texture& GetTexture() const {
      return texture_;
    }

  private:
    std::unordered_map<std::string, font_loader_char> meta_;
    gl::Texture texture_;
  };

  class TextRenderer : public JSObjectWrap<TextRenderer>, public RenderPass {
    JS_IMPL_WRAP(TextRenderer, ONE_ARG({
      JS_SETTER(Program),
      JS_SETTER(FontLoader),
      JS_CB(write),
    }))

    JS_IMPL_SETTER_WRAP(FontLoader, FontLoader)
    JS_DECL_CB_VOID(write)

  public:
    void Render();
    virtual void Begin() override {
      glDepthFunc(GL_ALWAYS);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      RenderPass::Begin();
    }

    virtual void End() override {
      RenderPass::End();
      glDepthFunc(GL_LESS);
      glDisable(GL_BLEND);
    }

    void UpdateBufferSize(int width, int height) override {
      height_ = height;
      proj_ = glm::ortho(0.f, float(width), 0.f, float(height));
    }

  protected:
    void InitializeProgram() override;

  private:
    GLuint  vao_ = 0,
            vbo_ = 0;
    GLsizei count_ = 0;

    glm::mat4 proj_;

    struct {
      gl::Uniform proj{"u_proj"};
    } uniforms_;

    int height_ = 0;
  };

}

#endif