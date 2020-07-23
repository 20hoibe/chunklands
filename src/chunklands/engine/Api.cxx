
#include "Api.hxx"
#include <chunklands/libcxx/glfw.hxx>
#include "Window.hxx"

namespace chunklands::engine {

  void Api::RunCommands() {
    loop_.try_executing_one();

    if (GLFW_start_poll_events) {
      glfwPollEvents();
    }
  }

  boost::future<bool> Api::GLFWInit() {
    return EnqueueTask([]() {
      const int result = glfwInit();
      return result == GLFW_TRUE;
    });
  }

  void Api::GLFWStartPollEvents(bool poll) {
    GLFW_start_poll_events = poll;
  }

  boost::future<WindowHandle*> Api::WindowCreate(int width, int height, std::string title) {
    return EnqueueTask([width, height, title = std::move(title)]() mutable -> WindowHandle* {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      GLFWwindow* const glfw_window = glfwCreateWindow(width, height, title.data(), nullptr, nullptr);
      if (!glfw_window) {
        return nullptr;
      }
      
      Window* const window = new Window(glfw_window);
      return reinterpret_cast<WindowHandle*>(window);
    });
  }

  void Api::WindowMakeContextCurrent(WindowHandle* handle) {
    Window* const window = reinterpret_cast<Window*>(handle);
    assert(window);

    EnqueueTask([window]() {
      window->makeContextCurrent();
    });
  }

  boost::signals2::scoped_connection Api::WindowOn(WindowHandle* handle, const std::string& event, std::function<void()> callback) {
    Window* const window = reinterpret_cast<Window*>(handle);
    assert(window);

    if (event == "shouldclose") {
      return window->on_close.connect(std::move(callback));
    }

    return boost::signals2::scoped_connection();
  }

} // namespace chunklands::engine