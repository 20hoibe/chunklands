#ifndef __CHUNKLANDS_ENGINE_LIGHTING_PASS_HXX__
#define __CHUNKLANDS_ENGINE_LIGHTING_PASS_HXX__

#include "Window.hxx"
#include "gl/Program.hxx"
#include "gl/Uniform.hxx"
#include <boost/move/core.hpp>
#include <chunklands/libcxx/glfw.hxx>
#include <glm/vec3.hpp>
#include <memory>

namespace chunklands::engine {

class LightingPass {
public:
    LightingPass(std::unique_ptr<gl::Program> program);
    ~LightingPass();

    void BeginPass(GLuint position_texture, GLuint normal_texture, GLuint color_texture);
    void EndPass();

private:
    std::unique_ptr<gl::Program> program_;

    // gl::Uniform<GLfloat> u_render_distance_;
    // gl::Uniform<glm::vec3> u_sun_position_;

    // gl::Uniform<GLint> u_position_texture;
    // gl::Uniform<GLint> u_normal_texture;
    gl::Uniform<GLint> u_color_texture;

    boost::signals2::scoped_connection window_resize_conn_;

    BOOST_MOVABLE_BUT_NOT_COPYABLE(LightingPass)
};

} // namespace chunklands::engine

#endif