
#include "EngineApiBridge.hxx"

#include <chunklands/engine/engine_exception.hxx>
#include <chunklands/engine/gl/gl_exception.hxx>

namespace chunklands::core {

  inline BOOST_NORETURN void FatalAbort(const engine::engine_exception& e) {
    const std::string message = engine::get_engine_exception_message(e);
    Napi::Error::Fatal(__FILE__, message.data());
  }

  inline BOOST_NORETURN void FatalAbort(const engine::gl::gl_exception& e) {
    const std::string message = engine::gl::get_gl_exception_message(e);
    Napi::Error::Fatal(__FILE__, message.data());
  }

  template<class T, class F>
  void EngineApiBridge::RunInNodeThread(std::unique_ptr<T> data, F&& fn) {
    assert(NotIsNodeThread());

    const napi_status status = fn_.NonBlockingCall(data.get(), std::forward<F>(fn));
    if (status == napi_ok) {
      data.release();
    }
  }

  template<class T>
  struct X {
    X(boost::future<T> result) : result(std::move(result)) {}

    boost::future<T> result;
  };

  template<class T, class F>
  JSPromise EngineApiBridge::FromNodeThreadRunApiResultInNodeThread(JSEnv env, boost::future<T> result, F fn) {
    assert(IsNodeThread());

    JSDeferred deferred = JSDeferred::New(env);
    JSPromise promise = deferred.Promise();
    result.then([this, deferred = std::move(deferred), fn = std::move(fn)](boost::future<T> result) {
      assert(NotIsNodeThread());

      std::unique_ptr<X<T>> data = std::make_unique<X<T>>(std::move(result));
      
      const napi_status status = fn_.NonBlockingCall(data.get(), [this, deferred = std::move(deferred), fn = std::move(fn)](JSEnv env, JSFunction, X<T>* data_ptr) {
        assert(IsNodeThread());

        std::unique_ptr<X<T>> data(data_ptr);
        try {
          fn(env, std::move(data->result), std::move(deferred));
        } catch (const engine::engine_exception& e) {
          FatalAbort(e);
        } catch (const engine::gl::gl_exception& e) {
          FatalAbort(e);
        }
      });

      if (status == napi_ok) {
        data.release();
      }
    });

    return promise;
  }

  template<class T, class F, class R>
  inline JSValue EngineApiBridge::RunInNodeThread(JSEnv env, boost::future<T> result, F&& fn) {
    JSDeferred deferred = JSDeferred::New(env);
    const JSPromise promise = deferred.Promise();
    RunInNodeThread(std::move(result), [deferred = std::move(deferred), fn = std::forward<F>(fn)](boost::future<T> result) {
      fn(std::move(result), std::move(deferred));
    });

    return promise;
  }

  struct data_t {
    data_t(JSRef2 ref) : ref(std::move(ref)) {}
    JSRef2 ref;
  };

  template<class Event, class F, class F2>
  JSValue EngineApiBridge::EventHandler(JSEnv env, JSValue js_type, JSValue js_callback, F&& fn_calls_api, F2&& fn_result) {
    std::string type = js_type.ToString();
    JSRef2 js_ref = JSRef2::New(env, js_callback);
    boost::signals2::scoped_connection conn = fn_calls_api(std::move(type), [this, js_ref = std::move(js_ref), fn_result = std::forward<F2>(fn_result)](Event&& event) {

      RunInNodeThread(std::make_unique<data_t>(std::move(js_ref)), [event = std::forward<Event>(event), fn_result = std::move(fn_result)](JSEnv env, JSFunction, data_t* data_ptr) {
        std::unique_ptr<data_t> data(data_ptr);
        JSFunction js_callback = data->ref.Value().As<JSFunction>();

        JSObject js_event = JSObject::New(env);
        js_event["type"] = JSString::New(env, event.type);
        
        fn_result(event, js_event);
        js_callback.Call({js_event});
      });
    });

    return JSFunction::New(env, [conn = conn.release()](JSCbi) {
      conn.disconnect();
    }, "cleanup", nullptr);
  }

} // namespace chunklands::core