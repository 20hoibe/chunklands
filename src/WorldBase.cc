#include "WorldBase.h"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/vector_relational.hpp>

#include "log.h"
#include "prof.h"

namespace chunklands {

  constexpr int RENDER_DISTANCE   = 6;
  constexpr int PREFETCH_DISTANCE = RENDER_DISTANCE + 2;
  constexpr int RETAIN_DISTANCE   = RENDER_DISTANCE + 4;

  static_assert(PREFETCH_DISTANCE > RENDER_DISTANCE, "damn");
  static_assert(RETAIN_DISTANCE >= PREFETCH_DISTANCE, "damn");
  
  constexpr unsigned MAX_CHUNK_VIEW_UPDATES = 12;
  constexpr unsigned MAX_CHUNK_MODEL_GENERATES = 10;
  constexpr unsigned MAX_CHUNK_MODEL_PROCESSES = 20;

  DEFINE_OBJECT_WRAP_DEFAULT_CTOR(WorldBase, ONE_ARG({
    InstanceMethod("setChunkGenerator", &WorldBase::SetChunkGenerator),
    InstanceMethod("setShader", &WorldBase::SetShader)
  }))

  void WorldBase::SetChunkGenerator(const Napi::CallbackInfo& info) {
    chunk_generator_ = info[0].ToObject();
  }

  void WorldBase::SetShader(const Napi::CallbackInfo& info) {
    scene_ = info[0].ToObject();
  }

  void WorldBase::Prepare() {
    PROF();

    { // matrices
      view_uniform_location_ = scene_->GetUniformLocation("u_view");
      proj_uniform_location_ = scene_->GetUniformLocation("u_proj");
    }

    { // texture
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      texture_location_ = scene_->GetUniformLocation("u_texture");
    }

    { // positional
      render_distance_location_ = scene_->GetUniformLocation("u_render_distance");
    }

    { // general

      // depth test
      glEnable(GL_DEPTH_TEST);

      // culling
      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
      glFrontFace(GL_CCW);
    }

    { // calculate nearest chunks
      nearest_chunks_.clear();
      // 4/3 * PI * r*r*r, for r = PREFETCH_DISTANCE + 0.5
      nearest_chunks_.reserve(5 * PREFETCH_DISTANCE * PREFETCH_DISTANCE * PREFETCH_DISTANCE);

      for (int cx = -PREFETCH_DISTANCE; cx <= PREFETCH_DISTANCE; cx++) {
        for (int cy = -PREFETCH_DISTANCE; cy <= PREFETCH_DISTANCE; cy++) {
          for (int cz = -PREFETCH_DISTANCE; cz <= PREFETCH_DISTANCE; cz++) {
            if (glm::length(glm::vec3(cx, cy, cz)) <= PREFETCH_DISTANCE) {
              nearest_chunks_.push_back(glm::ivec3(cx, cy, cz));
            }
          }
        }
      }

      std::sort_heap(nearest_chunks_.begin(), nearest_chunks_.end(), [](const glm::ivec3& a, const glm::ivec3& b) {
        glm::vec3 af(a);
        glm::vec3 bf(b);

        // more important to draw chunk of the same y level first
        af.y *= af.y;
        bf.y *= bf.y;

        return glm::length(af) < glm::length(bf);
      });
    }
  }


  void WorldBase::Update(double diff) {
    PROF();

    // Calculation of the current chunk we are standing on:
    //   for n = ]-16;+16[, n / 16 = 0, but we want:
    //   - 1. for n = [0;+16[ => 0
    //   - 2. for n = ]-16;0[ => -1
    // thus for negative dimension values we need to subtract chunk size
    glm::ivec3 center_chunk_pos = glm::ivec3(pos_.x >= 0 ? pos_.x : pos_.x - Chunk::SIZE,
                                             pos_.y >= 0 ? pos_.y : pos_.y - Chunk::SIZE,
                                             pos_.z >= 0 ? pos_.z : pos_.z - Chunk::SIZE
                                            ) / (int)Chunk::SIZE;

    // map cleanup: remove chunks outside prefetch distance
    for (auto&& it = chunk_map_.begin(); it != chunk_map_.end(); ) {
      auto&& pos = it->first;

      // check chunk inside retain distance
      if (glm::length(glm::vec3(pos - center_chunk_pos)) <= RETAIN_DISTANCE) {
        it++; // goto next chunk
      } else {
        // remove chunk
        it = chunk_map_.erase(it); // next it
      }
    }

    unsigned chunk_view_updates = 0;
    unsigned chunk_model_updates = 0;

    // map update: insert/update chunks
    for (auto&& nearest_chunk_it = nearest_chunks_.cbegin(); nearest_chunk_it != nearest_chunks_.cend(); nearest_chunk_it++) {
      glm::ivec3 pos(*nearest_chunk_it + center_chunk_pos);

      // find or create chunk
      auto&& chunk_it = chunk_map_.find(pos);
      if (chunk_it == chunk_map_.end()) {
        auto&& insert_it = chunk_map_.insert(std::make_pair(pos,
                                                            std::make_shared<Chunk>(pos)));
        chunk_it = insert_it.first;
      }

      assert(chunk_it != chunk_map_.end());
      assert(chunk_it->first == pos);
      auto&& chunk = chunk_it->second;

      // we will call `Generate*` in reverse order to not update too much in one iteration

      // 1. GenerateView
      if ( // check chunk inside render distance
        chunk_view_updates < MAX_CHUNK_VIEW_UPDATES &&
        chunk->GetState() == kModelPrepared
      ) {
        do { // to call "break;"
          auto&& left_chunk_it = chunk_map_.find(glm::ivec3(pos.x-1, pos.y, pos.z));
          if (left_chunk_it == chunk_map_.end() ||
              left_chunk_it->second->GetState() < kModelPrepared) { break; }
          auto&& right_chunk_it = chunk_map_.find(glm::ivec3(pos.x+1, pos.y, pos.z));
          if (right_chunk_it == chunk_map_.end() ||
              right_chunk_it->second->GetState() < kModelPrepared) { break; }
          auto&& top_chunk_it = chunk_map_.find(glm::ivec3(pos.x, pos.y+1, pos.z));
          if (top_chunk_it == chunk_map_.end() ||
              top_chunk_it->second->GetState() < kModelPrepared) { break; }
          auto&& bottom_chunk_it = chunk_map_.find(glm::ivec3(pos.x, pos.y-1, pos.z));
          if (bottom_chunk_it == chunk_map_.end() ||
              bottom_chunk_it->second->GetState() < kModelPrepared) { break; }
          auto&& front_chunk_it = chunk_map_.find(glm::ivec3(pos.x, pos.y, pos.z-1));
          if (front_chunk_it == chunk_map_.end() ||
              front_chunk_it->second->GetState() < kModelPrepared) { break; }
          auto&& back_chunk_it = chunk_map_.find(glm::ivec3(pos.x, pos.y, pos.z+1));
          if (back_chunk_it == chunk_map_.end() ||
              back_chunk_it->second->GetState() < kModelPrepared) { break; }
          
          const Chunk* neighbors[kNeighborCount] = {
            &*left_chunk_it->second,
            &*right_chunk_it->second,
            &*top_chunk_it->second,
            &*bottom_chunk_it->second,
            &*front_chunk_it->second,
            &*back_chunk_it->second
          };
          
          chunk_view_updates++;
          chunk_generator_->GenerateView(*chunk, neighbors);
        } while (0);
      }

      // 2. GenerateModel
      // always inside prefetch distance
      if (chunk_model_updates < MAX_CHUNK_MODEL_GENERATES && chunk->GetState() == kEmpty) {
        chunk_model_updates++;
        chunk_generator_->GenerateModel(chunk);
      }

    }

    for (int i = 0; i < MAX_CHUNK_MODEL_PROCESSES; i++) {
      if (!chunk_generator_->ProcessNextModel()) {
        break;
      }
    }
    
    glm::vec3 look_center(-sinf(look_.x) * cosf(look_.y),
                          sinf(look_.y),
                          -cosf(look_.x) * cosf(look_.y));
    view_ = glm::lookAt(pos_, pos_ + look_center, glm::vec3(0.f, 1.f, 0.f));
  }

  void WorldBase::Render(double diff) {
    PROF();

    glm::ivec3 center_chunk_pos = glm::ivec3(pos_.x >= 0 ? pos_.x : pos_.x - Chunk::SIZE,
                                             pos_.y >= 0 ? pos_.y : pos_.y - Chunk::SIZE,
                                             pos_.z >= 0 ? pos_.z : pos_.z - Chunk::SIZE
                                            ) / (int)Chunk::SIZE;

    glm::vec3 sky_color(.70f, .92f, .97f);
    glm::vec3 underground_color(0.f, 0.f, 0.f);
    glm::vec3 clear_color = glm::mix(underground_color, sky_color, glm::smoothstep(-30.f, 0.f, pos_.y));
    
    glClearColor(clear_color.r, clear_color.g, clear_color.b, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    scene_->Use();
    glUniformMatrix4fv(view_uniform_location_, 1, GL_FALSE, glm::value_ptr(view_));
    glUniformMatrix4fv(proj_uniform_location_, 1, GL_FALSE, glm::value_ptr(proj_));
    glUniform1i(texture_location_, 0);

    chunk_generator_->BindTexture();
    glUniform1f(render_distance_location_, ((float)RENDER_DISTANCE - 0.5f) * Chunk::SIZE);

    CHECK_GL();

    // map render all chunks
    unsigned rendered_index_count = 0;
    unsigned rendered_chunk_count = 0;
    for (auto&& chunk_entry : chunk_map_) {
      auto&& chunk = chunk_entry.second;

      if (chunk->GetState() != kViewPrepared) {
        continue; // no view data
      }

      if (glm::length(glm::vec3(chunk->GetPos() - center_chunk_pos)) > RENDER_DISTANCE) {
        continue;
      }

      rendered_index_count += chunk->GetIndexCount();
      rendered_chunk_count++;
      chunk->Render();
    }

    std::cout << "Rendered index count: " << rendered_index_count << ", chunk count: " << rendered_chunk_count << std::endl;

    glUseProgram(0);
  }

  void WorldBase::UpdateViewportRatio(int width, int height) {
    constexpr float fovy_degree = 75.f;
    
    proj_ = glm::perspective(glm::radians(fovy_degree), (float)width / height, 0.1f, 1000.0f);
  }

  void WorldBase::AddLook(float yaw_rad, float pitch_rad) {
    constexpr float pitch_break = (.5f * M_PI) - .1;
    
    look_.x += yaw_rad;
    look_.x = fmodf(look_.x, 2.f * M_PI);

    look_.y += pitch_rad;
    look_.y = std::max(std::min(look_.y, pitch_break), -pitch_break);
  }

  void WorldBase::AddPos(const glm::vec3& v) {
    pos_ += v;
  }
}
