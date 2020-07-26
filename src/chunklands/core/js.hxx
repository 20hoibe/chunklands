#ifndef __CHUNKLANDS_JS_H__
#define __CHUNKLANDS_JS_H__

#define NAPI_CPP_EXCEPTIONS
#include <napi.h>
#include <cassert>
#include <sstream>

// Wrap
#define JS_IMPL_WRAP(CLASS, CLASS_DEF)                public: \
                                                        CLASS(JSCbi info) \
                                                          : JSObjectWrap<CLASS>(info) {} \
                                                        JS_IMPL_INITCTOR(CLASS, CLASS_DEF)

#define JS_IMPL_INITCTOR(CLASS, CLASS_DEF)            public: \
                                                        static Napi::FunctionReference constructor; \
                                                        static JSFunction Initialize(JSEnv env) { \
                                                          using JSCurrentWrap = CLASS; \
                                                          auto&& clazz = DefineClass(env, #CLASS, CLASS_DEF); \
                                                          constructor = Napi::Persistent(clazz); \
                                                          constructor.SuppressDestruct(); \
                                                          return clazz; \
                                                        }

#define JS_DECL_INITCTOR()                            public: \
                                                        static Napi::FunctionReference constructor; \
                                                        static JSFunction Initialize(JSEnv env);

#define JS_DEF_INITCTOR(CLASS, CLASS_DEF)             Napi::FunctionReference CLASS::constructor; \
                                                      JSFunction CLASS::Initialize(JSEnv env) { \
                                                        using JSCurrentWrap = CLASS; \
                                                        auto&& clazz = DefineClass(env, #CLASS, CLASS_DEF); \
                                                        constructor = Napi::Persistent(clazz); \
                                                        constructor.SuppressDestruct(); \
                                                        return clazz; \
                                                      }

#define JS_DEF_INITCTOR_NOCLASSDEF(CLASS)             Napi::FunctionReference CLASS::constructor; \
                                                      JSFunction CLASS::Initialize(JSEnv env) { \
                                                        auto&& clazz = DefineClass(env, #CLASS, {}); \
                                                        constructor = Napi::Persistent(clazz); \
                                                        constructor.SuppressDestruct(); \
                                                        return clazz; \
                                                      }

// Wrap - Class definition
#define JS_SETTER(WHAT)                               InstanceMethod("set" #WHAT,           &JSCurrentWrap::JSCall_Set##WHAT)
#define JS_GETTER(WHAT)                               InstanceMethod("get" #WHAT,           &JSCurrentWrap::JSCall_Get##WHAT)
#define JS_CB(WHAT)                                   InstanceMethod(      #WHAT,           &JSCurrentWrap::JSCall_   ##WHAT)
#define JS_CB_STATIC(WHAT)                            StaticMethod(        #WHAT,           &JSCurrentWrap::JSCall_   ##WHAT)
#define JS_ABSTRACT_WRAP(TYPE, WHAT)                  InstanceAccessor(typeid(TYPE).name(), &JSCurrentWrap::JSCall_As ##WHAT, \
                                                        nullptr, napi_default)

// Wrap - DECL
#define JS_DECL_SETTER_WRAP(TYPE, WHAT)               _JS_IMPL_SETTER_WRAP(TYPE, WHAT, ;)
#define JS_DECL_SETTER_OBJECT(WHAT)                   _JS_IMPL_SETTER_OBJECT(WHAT, ;)
#define JS_DECL_CB(WHAT)                              protected:        JSValue JSCall_##WHAT(JSCbi info);
#define JS_DECL_CB_VOID(WHAT)                         protected:        void    JSCall_##WHAT(JSCbi info);
#define JS_DECL_CB_STATIC(WHAT)                       protected: static JSValue JSCall_##WHAT(JSCbi info);
#define JS_DECL_CB_STATIC_VOID(WHAT)                  protected: static void    JSCall_##WHAT(JSCbi info);

// Wrap - DEF
#define JS_DEF_WRAP(CLASS)                            Napi::FunctionReference CLASS::constructor;
#define JS_DEF_SETTER_WRAP(CLASS, WHAT)               void CLASS::JSCall_Set##WHAT(JSCbi info) { \
                                                        js_##WHAT = info[0]; \
                                                      }
#define JS_DEF_SETTER_OBJECT(CLASS, WHAT)             void CLASS::JSCall_Set##WHAT(JSCbi info) {  \
                                                        js_##WHAT = Napi::Persistent(info[0].ToObject()); \
                                                      }

// Wrap - ATTR
#define JS_ATTR_WRAP(TYPE, WHAT)                      protected: \
                                                        JSWrapRef<TYPE> js_##WHAT;

// Wrap - IMPL
#define JS_IMPL_SETTER_WRAP(TYPE, WHAT)               _JS_IMPL_SETTER_WRAP(TYPE, WHAT, ONE_ARG({ \
                                                        js_##WHAT = info[0]; \
                                                      }))
#define JS_IMPL_SETTER_OBJECT(WHAT)                   _JS_IMPL_SETTER_OBJECT(WHAT, ONE_ARG({ \
                                                        js_##WHAT = Napi::Persistent(info[0].ToObject()); \
                                                      }))
#define JS_IMPL_ABSTRACT_WRAP(TYPE, WHAT)             protected: \
                                                        JSValue JSCall_As##WHAT(JSCbi info) { \
                                                          return JSExternal<TYPE>::New(info.Env(), this); \
                                                        }
#define JS_IMPL_ABSTRACT_WRAP_SETTER(TYPE, WHAT)      protected: \
                                                        void JSCall_Set##WHAT(JSCbi info) { \
                                                          js_##WHAT = info[0]; \
                                                        } \
                                                      private: \
                                                        JSAbstractUnwrap<TYPE> js_##WHAT;
#define _JS_IMPL_SETTER_WRAP(TYPE, WHAT, IMPL)        JS_ATTR_WRAP(TYPE, WHAT) \
                                                      protected: \
                                                        void JSCall_Set##WHAT(JSCbi info) IMPL
#define _JS_IMPL_SETTER_OBJECT(WHAT, IMPL)            protected: \
                                                        JSObjectRef js_##WHAT; \
                                                        void JSCall_Set##WHAT(JSCbi info) IMPL

// Assert
#define JS_ASSERT(ENV, COND)                          JS_ASSERT_MSG((ENV), (COND), "failed condition " #COND)
#define JS_ASSERT_MSG(ENV, COND, MSG)                 do { \
                                                        if (!(COND)) { \
                                                          throw js_create_error(ENV, MSG); \
                                                        } \
                                                      } while (0)


// solves comma problems: e.g. `MACRO({ InstanceMethod(...), ... })` -> `MACRO(ONE_ARG({ ... }))`
#define ONE_ARG(...) __VA_ARGS__

namespace chunklands {
  template<class BASE>
  using JSObjectWrap  = Napi::ObjectWrap<BASE>;
  using JSObjectRef   = Napi::ObjectReference;
  using JSFunctionRef = Napi::FunctionReference;
  using JSValue       = Napi::Value;
  template<class T>
  using JSRef         = Napi::Reference<T>;
  using JSValueRef    = JSRef<JSValue>;
  using JSCbi         = const Napi::CallbackInfo&;
  using JSObject      = Napi::Object;
  using JSEnv         = Napi::Env;
  using JSBoolean     = Napi::Boolean;
  using JSNumber      = Napi::Number;
  using JSArray       = Napi::Array;
  using JSArrayBuffer = Napi::ArrayBuffer;
  using JSInt32Array  = Napi::Int32Array;
  using JSString      = Napi::String;
  using JSHandleScope = Napi::HandleScope;
  using JSFunction    = Napi::Function;
  using JSError       = Napi::Error;
  using JSSymbol      = Napi::Symbol;
  template<class T>
  using JSExternal    = Napi::External<T>;
  using JSTSFunction  = Napi::ThreadSafeFunction;
  using JSDeferred    = Napi::Promise::Deferred;
  using JSPromise     = Napi::Promise;

  inline uint32_t js_index(int i) {
    return i;
  }

  template <typename T>
  class JSWrapRef {
  public:
    JSWrapRef() {}

    JSWrapRef(JSObjectWrap<T>& wrap) {
      (*this) = wrap.Value();
    }

    JSWrapRef(JSObject object) {
      (*this) = object;
    }

    JSWrapRef(JSValue value) {
      (*this) = value;
    }

    ~JSWrapRef() {
      object_ = nullptr;
    }

    JSWrapRef& operator=(JSValue value) {
      return (*this) = value.ToObject();
    }

    JSWrapRef& operator=(JSObject object) {
      ref_    = Napi::Persistent(object);
      object_ = JSObjectWrap<T>::Unwrap(object);
      return *this;
    }

    operator napi_ref() {
      return ref_;
    }

    operator T*() {
      CheckObject();
      return object_;
    }

    const T* operator->() const {
      CheckObject();
      return object_;
    }

    T* operator->() {
      CheckObject();
      return object_;
    }

    T& operator*() {
      CheckObject();
      return *object_;
    }

    T* Get() {
      CheckObject();
      return object_;
    }

    const T* Get() const {
      CheckObject();
      return object_;
    }

    const T& operator*() const {
      CheckObject();
      return *object_;
    }

    bool IsEmpty() const {
      return object_ == nullptr;
    }

  private:
    void CheckObject() const {
      if (!object_) {
        throw std::runtime_error("object is nullptr");
      }
    }

  private:
    JSObjectRef ref_;
    T* object_ = nullptr;
  };

  JSError js_create_error(JSEnv env, const std::string& msg);

  template<class A>
  class JSAbstractUnwrap {
  public:
    JSAbstractUnwrap() {}
    JSAbstractUnwrap(JSValue wrap) {
      JSObject object = wrap.ToObject();
      const char* name = typeid(A).name();
      JSValue v = object.Get(name);
      JS_ASSERT(wrap.Env(), v.IsExternal());

      JSExternal<A> abstract = v.As<Napi::External<A>>();

      if (abstract.IsEmpty()) {
        std::stringstream ss;
        ss << "ObjectWrap is not convertible to " << name << "!";
        throw js_create_error(wrap.Env(), ss.str());
      }

      ptr_ = abstract.Data();
      assert(ptr_ != nullptr);

      wrap_.Reset(object, 1);
      abstract_.Reset(abstract, 1);
    }

    A* operator->() {
      return ptr_;
    }

    JSObject GetWrap() const {
      assert(!wrap_.IsEmpty());
      return wrap_.Value();
    }

  private:
    JSRef<JSObject> wrap_;
    JSRef<JSExternal<A>> abstract_;
    A* ptr_ = nullptr;
  };

  class JSRef2 {
  public:
    static JSRef2 New(napi_env env, napi_value value) {
      napi_ref ref = nullptr;
      napi_status status = napi_create_reference(env, value, 1, &ref);
      
      NAPI_THROW_IF_FAILED(env, status, JSRef2());
      return JSRef2(env, ref);
    }

    JSRef2() {}
    JSRef2(napi_env env, napi_ref ref) : env_(env), ref_(ref) {}

    JSRef2(const JSRef2& other) {
      (*this) = other;
    }

    JSRef2(JSRef2&& other) {
      (*this) = std::move(other);
    }

    JSRef2& operator=(const JSRef2& other) {
      Reset();
      env_ = other.env_;
      ref_ = other.ref_;

      const napi_status status = napi_reference_ref(env_, ref_, nullptr);
      NAPI_THROW_IF_FAILED(env_, status, *this);

      return *this;
    }

    JSRef2& operator=(JSRef2&& other) {
      std::swap(env_, other.env_);
      std::swap(ref_, other.ref_);
      return *this;
    }

    ~JSRef2() {
      Reset();
    }

    JSValue Value() const {
      napi_value value = nullptr;
      const napi_status status = napi_get_reference_value(env_, ref_, &value);
      NAPI_THROW_IF_FAILED(env_, status, nullptr);

      return Napi::Value(env_, value);
    }

    inline JSEnv Env() const {
      return JSEnv(env_);
    }

  private:
    void Reset() {
      if (ref_) {
        napi_reference_unref(env_, ref_, nullptr);
        ref_ = nullptr;
      }

      env_ = nullptr;
    }
  private:
    napi_env env_ = nullptr;
    napi_ref ref_ = nullptr;
  };
}

#endif