#ifndef __CHUNKLANDS_ENGINE_ENGINE_ENGINE_HXX__
#define __CHUNKLANDS_ENGINE_ENGINE_ENGINE_HXX__

#include <boost/signals2.hpp>
#include <chunklands/engine/engine_types.hxx>
#include <chunklands/libcxx/boost_thread.hxx>
#include <variant>

namespace chunklands::engine {

  struct EngineData;

  class Engine {
  public:
    Engine();
    ~Engine();

  public:
    void Render();
    void RenderSwap();
    void Update();
    void Terminate();

  public:
    AsyncEngineResult<CENone>           GLFWInit();
    void                                GLFWStartPollEvents(bool poll);
    bool                                GLFWStartPollEvents() const;
    
    AsyncEngineResult<CEWindowHandle*>  WindowCreate(int width, int height, std::string title);
    AsyncEngineResult<CENone>           WindowLoadGL(CEWindowHandle* handle);
    EventConnection                     WindowOn(CEWindowHandle* handle, const std::string& event, std::function<void(CEWindowEvent)> callback);
    
    AsyncEngineResult<CENone>           RenderPipelineInit(CEWindowHandle* handle, CERenderPipelineInit init);
    
    AsyncEngineResult<CEBlockHandle*>   BlockCreate(CEBlockCreateInit init);
    AsyncEngineResult<CENone>           BlockBake();

    AsyncEngineResult<CEChunkHandle*>   ChunkCreate(int x, int y, int z);
    AsyncEngineResult<CENone>           ChunkDelete(CEChunkHandle* handle);
    AsyncEngineResult<CENone>           ChunkUpdateData(CEChunkHandle* handle, CEBlockHandle** blocks);

    AsyncEngineResult<CENone>           SceneAddChunk(CEChunkHandle* handle);
    AsyncEngineResult<CENone>           SceneRemoveChunk(CEChunkHandle* handle);

    AsyncEngineResult<CENone>           CameraAttachWindow(CEWindowHandle* handle);
    AsyncEngineResult<CENone>           CameraDetachWindow(CEWindowHandle* handle);
    AsyncEngineResult<CECameraPosition> CameraGetPosition();
    EventConnection                     CameraOn(const std::string& event, std::function<void(CECameraEvent)> callback);

  private:
    EngineData* data_ = nullptr;
  };

} // namespace chunklands::engine

#endif