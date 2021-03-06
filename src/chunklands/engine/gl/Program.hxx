#ifndef __CHUNKLANDS_ENGINE_GL_PROGRAM_HXX__
#define __CHUNKLANDS_ENGINE_GL_PROGRAM_HXX__

#include <cassert>
#include <chunklands/libcxx/ThreadGuard.hxx>
#include <chunklands/libcxx/glfw.hxx>
#include <string>

namespace chunklands::engine::gl {

class Program {
public:
    Program(const char* vsh_source, const char* fsh_source);

    void Use() const
    {
        assert(libcxx::ThreadGuard::IsOpenGLThread());
        glUseProgram(program_);
    }

    void Unuse() const
    {
        assert(libcxx::ThreadGuard::IsOpenGLThread());
        glUseProgram(0);
    }

    GLint GetUniformLocation(const std::string& name) const;
    GLint GetUniformLocation(const std::string& name, const int index) const;

private:
    GLuint program_ = 0;
};

} // namespace chunklands::gl

#endif