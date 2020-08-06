#ifndef __CHUNKLANDS_ENGINE_API_HXX__
#define __CHUNKLANDS_ENGINE_API_HXX__

#include <chunklands/libcxx/boost_thread.hxx>
#include <boost/signals2.hpp>
#include <set>
#include "api-types.hxx"

namespace chunklands::engine {

  class Api {
  public:
    Api(void* executor);
    ~Api();

  public:
    void Tick();

  public:
    boost::future<void>                 GLFWInit();
    void                                GLFWStartPollEvents(bool poll);
    bool                                GLFWStartPollEvents() const { return GLFW_start_poll_events_; }
    
    boost::future<CEWindowHandle*>      WindowCreate(int width, int height, std::string title);
    boost::future<void>                 WindowLoadGL(CEWindowHandle* handle);
    boost::signals2::scoped_connection  WindowOn(CEWindowHandle* handle, const std::string& event, std::function<void()> callback);
    
    boost::future<void>                 RenderPipelineInit(CEWindowHandle* handle, CERenderPipelineInit init);
    
    boost::future<CEBlockHandle*>       BlockCreate(CEBlockCreateInit init);
    boost::future<void>                 BlockBake();

    boost::future<CEChunkHandle*>       ChunkCreate(int x, int y, int z);
    boost::future<void>                 ChunkUpdateData(CEChunkHandle* handle, CEBlockHandle** blocks);

    boost::future<void>                 SceneAddChunk(CEChunkHandle* handle);
    boost::future<void>                 SceneRemoveChunk(CEChunkHandle* handle);

  private:
    void* executor_;

    bool GLFW_initialized = false;
    bool GLFW_start_poll_events_ = false;

    CEHandle* g_buffer_pass_handle_ = nullptr;
    CEHandle* lighting_pass_handle_ = nullptr;
    CEHandle* render_quad_handle_ = nullptr;
    CEHandle* camera_handle_ = nullptr;

    std::set<CEWindowHandle*> windows_;
    std::set<CEHandle*> unbaked_blocks_;
    std::set<CEBlockHandle*> blocks_;
    std::set<CEChunkHandle*> chunks_;
    std::set<CEChunkHandle*> chunks_by_state_[CE_CHUNK_STATE_COUNT];

    std::set<CEChunkHandle*> scene_chunks_;
    // std::set<CEGBufferMeshHandle*> g_buffer_meshes_;
  };

} // namespace chunklands::engine

#endif