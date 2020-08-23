
#include <chunklands/engine/engine/Engine.hxx>
#include <chunklands/engine/engine/shared.hxx>

namespace chunklands::engine {

AsyncEngineResult<CENone>
Engine::SceneAddChunk(CEChunkHandle* handle)
{
    EASY_FUNCTION();
    ENGINE_FN();

    return EnqueueTask(data_->executors.opengl, [this, handle]() -> EngineResultX<CENone> {
        EASY_FUNCTION();
        ENGINE_CHECKX(has_handle(data_->chunk.chunks, handle));
        Chunk* const chunk = reinterpret_cast<Chunk*>(handle);
        auto insert_result = data_->chunk.scene.insert(chunk);
        ENGINE_CHECKX_MSG(insert_result.second, "chunk is should not already be added to scene");

        return Ok();
    });
}

AsyncEngineResult<CENone>
Engine::SceneRemoveChunk(CEChunkHandle* handle)
{
    EASY_FUNCTION();
    ENGINE_FN();

    return EnqueueTask(data_->executors.opengl, [this, handle]() -> EngineResultX<CENone> {
        EASY_FUNCTION();
        ENGINE_CHECKX(has_handle(data_->chunk.chunks, handle));
        Chunk* const chunk = reinterpret_cast<Chunk*>(handle);
        const std::size_t count = data_->chunk.scene.erase(chunk);
        assert(count == 1);

        return Ok();
    });
}

} // namespace chunklands::engine