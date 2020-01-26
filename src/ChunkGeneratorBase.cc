#include "ChunkGeneratorBase.h"

#include <cmath>

#include "napi/flow.h"

namespace chunklands {
  DEFINE_OBJECT_WRAP_DEFAULT_CTOR(ChunkGeneratorBase, ONE_ARG({
    InstanceMethod("setBlockRegistrar", &ChunkGeneratorBase::SetBlockRegistrar),
    InstanceMethod("setWorldGenerator", &ChunkGeneratorBase::SetWorldGenerator)
  }))

  void ChunkGeneratorBase::SetBlockRegistrar(const Napi::CallbackInfo& info) {
    block_registrar_ = info[0].ToObject();
  }

  void ChunkGeneratorBase::SetWorldGenerator(const Napi::CallbackInfo& info) {
    world_generator_ = Napi::Persistent(info[0].ToObject());
  }

  struct generate_model_callback_data {
    ChunkGeneratorBase *generator;
    std::shared_ptr<Chunk> chunk;
  };

  void ChunkGeneratorBase::GenerateModel(std::shared_ptr<Chunk>& chunk) {
    assert(chunk->GetState() == kEmpty);

    auto&& pos = chunk->GetPos();
    {
      Napi::Env env = world_generator_.Env();
      Napi::HandleScope scope(env);

      chunk->state_ = kModelIsPreparing;
      
      world_generator_.Get("generateChunk").As<Napi::Function>().Call(world_generator_.Value(), {
        Napi::Number::New(env, pos.x),
        Napi::Number::New(env, pos.y),
        Napi::Number::New(env, pos.z),
        Napi::Number::New(env, Chunk::SIZE),
        Napi::Function::New(env, [](const Napi::CallbackInfo& info) {
          std::unique_ptr<generate_model_callback_data> data(static_cast<generate_model_callback_data *>(info.Data()));
          
          auto&& js_err = info[0];
          auto&& js_chunk = info[1];

          if (!js_err.IsUndefined() && !js_err.IsNull()) {
            js_err.As<Napi::Error>().ThrowAsJavaScriptException();
            return;
          }

          assert(js_chunk.IsArray());
          auto&& js_chunk_arr = js_chunk.As<Napi::Array>();

          assert(js_chunk_arr.Length() == Chunk::BLOCK_COUNT);

          auto&& block_registrar = data->generator->block_registrar_;

          int i = 0;
          data->chunk->ForEachBlock([&](Chunk::BlockType& block_type, int x, int y, int z) {
            auto&& block_name = js_chunk_arr.Get(i);
            assert(block_name.IsString());

            block_type = block_registrar->Find(js_chunk_arr.Get(i).As<Napi::String>().Utf8Value());
            i++;
          });

          data->chunk->state_ = kModelPrepared;
        }, nullptr, new generate_model_callback_data{.generator = this, .chunk = chunk})
      });
    }
  }

  // for now just an estimation: allocation vs. growing vector
  // faces * triangle_per_face * vertices_per_triangle * floats_per_vertex * 0.5 load
  constexpr int estimated_avg_floats_per_block = (6 * 2 * 3 * 8) / 2;
  constexpr int estimated_floats_in_buffer = Chunk::SIZE * Chunk::SIZE * Chunk::SIZE * estimated_avg_floats_per_block;

  void ChunkGeneratorBase::GenerateView(Chunk& chunk, const Chunk* neighbors[kNeighborCount]) {
    assert(chunk.GetState() == kModelPrepared);
    assert(neighbors[kLeft  ] != nullptr && neighbors[kLeft  ]->GetState() >= kModelPrepared);
    assert(neighbors[kRight ] != nullptr && neighbors[kRight ]->GetState() >= kModelPrepared);
    assert(neighbors[kTop   ] != nullptr && neighbors[kTop   ]->GetState() >= kModelPrepared);
    assert(neighbors[kBottom] != nullptr && neighbors[kBottom]->GetState() >= kModelPrepared);
    assert(neighbors[kFront ] != nullptr && neighbors[kFront ]->GetState() >= kModelPrepared);
    assert(neighbors[kBack  ] != nullptr && neighbors[kBack  ]->GetState() >= kModelPrepared);

    // allocate client side vertex buffer
    std::vector<GLfloat> vertex_buffer_data;
    vertex_buffer_data.reserve(estimated_floats_in_buffer);

    glm::vec3 chunk_offset(glm::vec3(chunk.GetPos()) * (float)Chunk::SIZE);

    chunk.ForEachBlock([&](const Chunk::BlockType& block_type, int x, int y, int z) {
      assert(block_type != nullptr);

      //                                                              << inside of chunk || check neighbor >>
      bool isLeftOpaque   = ((x >= 1            && chunk.blocks_[z][y][x-1]->IsOpaque()) || (x == 0             && neighbors[kLeft  ]->blocks_[z][y][Chunk::SIZE-1]->IsOpaque()));
      bool isRightOpaque  = ((x < Chunk::SIZE-1 && chunk.blocks_[z][y][x+1]->IsOpaque()) || (x == Chunk::SIZE-1 && neighbors[kRight ]->blocks_[z][y][0]            ->IsOpaque()));
      bool isBottomOpaque = ((y >= 1            && chunk.blocks_[z][y-1][x]->IsOpaque()) || (y == 0             && neighbors[kBottom]->blocks_[z][Chunk::SIZE-1][x]->IsOpaque()));
      bool isTopOpaque    = ((y < Chunk::SIZE-1 && chunk.blocks_[z][y+1][x]->IsOpaque()) || (y == Chunk::SIZE-1 && neighbors[kTop   ]->blocks_[z][0][x]            ->IsOpaque()));
      bool isFrontOpaque  = ((z >= 1            && chunk.blocks_[z-1][y][x]->IsOpaque()) || (z == 0             && neighbors[kFront ]->blocks_[Chunk::SIZE-1][y][x]->IsOpaque()));
      bool isBackOpaque   = ((z < Chunk::SIZE-1 && chunk.blocks_[z+1][y][x]->IsOpaque()) || (z == Chunk::SIZE-1 && neighbors[kBack  ]->blocks_[0][y][x]            ->IsOpaque()));

      if (isLeftOpaque && isRightOpaque && isBottomOpaque && isTopOpaque && isFrontOpaque && isBackOpaque) {
        // optimization: block is fully hidden
        return;
      }

      auto&& vertex_data = block_type->GetFacesVertexData();
      for (auto &&it = vertex_data.cbegin(); it != vertex_data.cend(); it++) {
        if (
          (it->first == "left"   && isLeftOpaque  ) ||
          (it->first == "right"  && isRightOpaque ) ||
          (it->first == "top"    && isTopOpaque   ) ||
          (it->first == "bottom" && isBottomOpaque) ||
          (it->first == "front"  && isFrontOpaque ) ||
          (it->first == "back"   && isBackOpaque )
        ) {
          // optimization: face is hidden
          continue;
        }

        auto&& vertex_data = it->second;
        assert(vertex_data.size() % 8 == 0);
        for (int fi = 0; fi < vertex_data.size(); ) { // float index

          // position vertices
          vertex_buffer_data.push_back(vertex_data[fi++] + (GLfloat)x + chunk_offset.x);
          vertex_buffer_data.push_back(vertex_data[fi++] + (GLfloat)y + chunk_offset.y);
          vertex_buffer_data.push_back(vertex_data[fi++] + (GLfloat)z + chunk_offset.z);

          // normal vertices
          vertex_buffer_data.push_back(vertex_data[fi++]);
          vertex_buffer_data.push_back(vertex_data[fi++]);
          vertex_buffer_data.push_back(vertex_data[fi++]);

          // uv vertices
          vertex_buffer_data.push_back(vertex_data[fi++]);
          vertex_buffer_data.push_back(vertex_data[fi++]);
        }
      }
    });

    
    chunk.vb_index_count_ = vertex_buffer_data.size();

    // copy vector to graphic card
    chunk.InitializeGL();

    glBindBuffer(GL_ARRAY_BUFFER, chunk.vb_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertex_buffer_data.size(),
                 vertex_buffer_data.data(), GL_STATIC_DRAW);

    glBindVertexArray(chunk.vao_);

    constexpr GLsizei stride = (3 + 3 + 2) * sizeof(GLfloat);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)(0 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // uv attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    CHECK_GL();

    chunk.state_ = kViewPrepared;
  }

  void ChunkGeneratorBase::BindTexture() {
    block_registrar_->BindTexture();
  }
}
