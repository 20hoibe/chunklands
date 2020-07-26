#ifndef __CHUNKLANDS_ENGINE_ENGINE_EXCEPTION_HXX__
#define __CHUNKLANDS_ENGINE_ENGINE_EXCEPTION_HXX__

#include <boost/exception/all.hpp>
#include <boost/exception/exception.hpp>
#include <boost/exception/errinfo_api_function.hpp>
#include <chunklands/libcxx/exception.hxx>
#include <string>
#include <sstream>

namespace chunklands::engine {

  struct engine_exception : virtual std::exception, virtual boost::exception {
    static const char* name() {
      return "engine_exception";
    }
  };

  inline BOOST_NORETURN void throw_engine_exception(const char* api_function) {
    BOOST_THROW_EXCEPTION(engine_exception()
      << boost::errinfo_api_function(api_function)
      << libcxx::exception::create_errinfo_stacktrace(1)
    );
  }

  inline std::string get_engine_exception_message(const engine_exception& e) {
    std::ostringstream ss;
    libcxx::exception::add_exception_name(ss, e);
    libcxx::exception::add_api_function_message(ss, e);
    libcxx::exception::add_stacktrace_message(ss, e);

    return ss.str();
  }

} // namespace chunklands::engine

#endif