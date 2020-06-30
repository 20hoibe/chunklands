
#include "init.hxx"

#include "engine/_.hxx"
#include "gl/_.hxx"

#include "modules/game/game_module.hxx"
#include "modules/misc/misc_module.hxx"

#define EXPORT(CLAZZ) exports[#CLAZZ] = CLAZZ::Initialize(env)

namespace chunklands::modules {

  JSObject InitGame(JSEnv env, JSObject exports) {
    using namespace modules::game;
    EXPORT(BlockRegistrar);
    EXPORT(SpriteRegistrar);
    EXPORT(ChunkGenerator);
    EXPORT(Scene);
    EXPORT(World);
    EXPORT(GameOverlay);
    return exports;
  }

  JSObject InitMisc(JSEnv env, JSObject exports) {
    using namespace modules::misc;
    EXPORT(Profiler);
    return exports;
  }

  JSObject Init(JSEnv env, JSObject exports) {
    exports["engine"] = engine::Init(env, JSObject::New(env));
    exports["game"]   = InitGame(env, JSObject::New(env));
    exports["gl"]     = gl::Init(env, JSObject::New(env));
    exports["misc"]   = InitMisc(env, JSObject::New(env));

    return exports;
  }
}
