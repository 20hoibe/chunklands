#include "Chunk.h"
#include <vector>

namespace chunklands {
  Chunk::Chunk() {
    ForEachBlock([](char& block_type, int x, int y, int z) {
      block_type = (2*x + 3*y + 7*z) % 23 == 0 ? 1 : 0; // some pseudo-random stuff
    });
  }

  Chunk::~Chunk() {
    Cleanup();
  }

  void Chunk::Cleanup() {
    if (vb_) {
      glDeleteBuffers(1, &vb_);
      vb_ = 0;
    }

    if (vao_) {
      glDeleteVertexArrays(1, &vao_);
      vao_ = 0;
    }
  }

  constexpr int POSITION_VERTICES_IN_BLOCK = 6 * 2 * 3;
  constexpr int NORMAL_VERTICES_IN_BLOCK = POSITION_VERTICES_IN_BLOCK;
  constexpr int FLOATS_IN_BLOCK = (POSITION_VERTICES_IN_BLOCK + NORMAL_VERTICES_IN_BLOCK) * 3;
  GLfloat BLOCK_DATA[FLOATS_IN_BLOCK] = {
    // VERTEX       NORMAL            VERTEX          NORMAL            VERTEX          NORMAL

    // front
    0.f, 1.f, 0.f,  0.f, 0.f, -1.f,   0.f, 0.f, 0.f,  0.f, 0.f, -1.f,   1.f, 0.f, 0.f,  0.f, 0.f, -1.f,
    0.f, 1.f, 0.f,  0.f, 0.f, -1.f,   1.f, 0.f, 0.f,  0.f, 0.f, -1.f,   1.f, 1.f, 0.f,  0.f, 0.f, -1.f,

    // back
    1.f, 1.f, 1.f,  0.f, 0.f, +1.f,   1.f, 0.f, 1.f,  0.f, 0.f, +1.f,   0.f, 0.f, 1.f,  0.f, 0.f, +1.f,
    1.f, 1.f, 1.f,  0.f, 0.f, +1.f,   0.f, 0.f, 1.f,  0.f, 0.f, +1.f,   0.f, 1.f, 1.f,  0.f, 0.f, +1.f,

    // left
    0.f, 1.f, 1.f,  -1.f, 0.f, 0.f,   0.f, 0.f, 1.f,  -1.f, 0.f, 0.f,   0.f, 0.f, 0.f,  -1.f, 0.f, 0.f,
    0.f, 1.f, 1.f,  -1.f, 0.f, 0.f,   0.f, 0.f, 0.f,  -1.f, 0.f, 0.f,   0.f, 1.f, 0.f,  -1.f, 0.f, 0.f,

    // right
    1.f, 1.f, 0.f,  +1.f, 0.f, 0.f,   1.f, 0.f, 0.f,  +1.f, 0.f, 0.f,   1.f, 0.f, 1.f,  +1.f, 0.f, 0.f,
    1.f, 1.f, 0.f,  +1.f, 0.f, 0.f,   1.f, 0.f, 1.f,  +1.f, 0.f, 0.f,   1.f, 1.f, 1.f,  +1.f, 0.f, 0.f,

    // top
    0.f, 1.f, 1.f,  0.f, +1.f, 0.f,   0.f, 1.f, 0.f,  0.f, +1.f, 0.f,   1.f, 1.f, 0.f,  0.f, +1.f, 0.f,
    0.f, 1.f, 1.f,  0.f, +1.f, 0.f,   1.f, 1.f, 0.f,  0.f, +1.f, 0.f,   1.f, 1.f, 1.f,  0.f, +1.f, 0.f,

    // bottom
    0.f, 0.f, 0.f,  0.f, -1.f, 0.f,   0.f, 0.f, 1.f,  0.f, -1.f, 0.f,   1.f, 0.f, 1.f,  0.f, -1.f, 0.f,
    0.f, 0.f, 0.f,  0.f, -1.f, 0.f,   1.f, 0.f, 1.f,  0.f, -1.f, 0.f,   1.f, 0.f, 0.f,  0.f, -1.f, 0.f,
  };

  void Chunk::Prepare() {
    // count blocks, which are not AIR
    int block_count = 0;
    ForEachBlock([&](BlockType block_type, int x, int y, int z) {
      if (block_type != 0) { // blocks not AIR (=0)
        block_count++;
      }
    });
    assert(block_count <= BLOCK_COUNT);

    // allocate client side vertex buffer
    const int floats_in_buffer = block_count * FLOATS_IN_BLOCK;
    std::vector<GLfloat> vertex_buffer_data(floats_in_buffer);

    int vbi = 0; // vertex_buffer index
    ForEachBlock([&](BlockType block_type, int x, int y, int z) {
      // skip AIR
      if (block_type == 0) {
        return;
      }

      static_assert(FLOATS_IN_BLOCK % 6 == 0,
                    "loop needs floats divisible by 6");
      for (int fi = 0; fi < FLOATS_IN_BLOCK; ) { // float index

        // position vertices
        vertex_buffer_data[vbi++] = BLOCK_DATA[fi++] + (GLfloat)x;
        vertex_buffer_data[vbi++] = BLOCK_DATA[fi++] + (GLfloat)y;
        vertex_buffer_data[vbi++] = BLOCK_DATA[fi++] + (GLfloat)z;

        // normal vertices
        vertex_buffer_data[vbi++] = BLOCK_DATA[fi++];
        vertex_buffer_data[vbi++] = BLOCK_DATA[fi++];
        vertex_buffer_data[vbi++] = BLOCK_DATA[fi++];
      }
    });

    // vertex buffer did not grow
    assert(floats_in_buffer == vertex_buffer_data.size());

    // we counted everything, so this must be equal
    assert(vbi == vertex_buffer_data.size());

    vb_vertex_count_ = vertex_buffer_data.size();

    // copy vector to graphic card
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vb_);

    glBindBuffer(GL_ARRAY_BUFFER, vb_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertex_buffer_data.size(),
                 vertex_buffer_data.data(), GL_STATIC_DRAW);

    glBindVertexArray(vao_);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    CHECK_GL();
  }

  void Chunk::Render() {
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, vb_vertex_count_);
    CHECK_GL();
  }
}