#ifndef __CHUNKLANDS_CHUNKGENERATORBASE_H__
#define __CHUNKLANDS_CHUNKGENERATORBASE_H__

#include <napi.h>
#include "BlockRegistrarBase.h"
#include "Chunk.h"
#include "napi/object_wrap_util.h"
#include "napi/PersistentObjectWrap.h"

namespace chunklands {
  class ChunkGeneratorBase : public Napi::ObjectWrap<ChunkGeneratorBase> {
    DECLARE_OBJECT_WRAP(ChunkGeneratorBase)
    DECLARE_OBJECT_WRAP_CB(void SetBlockRegistrar)
    DECLARE_OBJECT_WRAP_CB(void SetWorldGenerator)

  public:
    void GenerateModel(std::shared_ptr<Chunk>& chunk);
    void GenerateView(Chunk& chunk, const Chunk* neighbors[kNeighborCount]);

    void BindTexture();

  private:
    NapiExt::PersistentObjectWrap<BlockRegistrarBase> block_registrar_;
    Napi::ObjectReference world_generator_;
  };
}

#endif
