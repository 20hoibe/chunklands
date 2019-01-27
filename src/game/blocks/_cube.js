const assert = require('assert');

module.exports = {
  vertexData: [
    // VERTEX1      NORMAL1          UV1        VERTEX2         NORMAL2          UV2        VERTEX3         NORMAL3          UV3
    0.0, 1.0, 0.0,  0.0, 0.0, -1.0,  0.0, 1.0,  0.0, 0.0, 0.0,  0.0, 0.0, -1.0,  0.0, 0.0,  1.0, 0.0, 0.0,  0.0, 0.0, -1.0,  1.0, 0.0, // front
    0.0, 1.0, 0.0,  0.0, 0.0, -1.0,  0.0, 1.0,  1.0, 0.0, 0.0,  0.0, 0.0, -1.0,  1.0, 0.0,  1.0, 1.0, 0.0,  0.0, 0.0, -1.0,  1.0, 1.0, // front
    1.0, 1.0, 1.0,  0.0, 0.0, +1.0,  0.0, 1.0,  1.0, 0.0, 1.0,  0.0, 0.0, +1.0,  0.0, 0.0,  0.0, 0.0, 1.0,  0.0, 0.0, +1.0,  1.0, 0.0, // back
    1.0, 1.0, 1.0,  0.0, 0.0, +1.0,  0.0, 1.0,  0.0, 0.0, 1.0,  0.0, 0.0, +1.0,  1.0, 0.0,  0.0, 1.0, 1.0,  0.0, 0.0, +1.0,  1.0, 1.0, // back
    0.0, 1.0, 1.0,  -1.0, 0.0, 0.0,  0.0, 1.0,  0.0, 0.0, 1.0,  -1.0, 0.0, 0.0,  0.0, 0.0,  0.0, 0.0, 0.0,  -1.0, 0.0, 0.0,  1.0, 0.0, // left
    0.0, 1.0, 1.0,  -1.0, 0.0, 0.0,  0.0, 1.0,  0.0, 0.0, 0.0,  -1.0, 0.0, 0.0,  1.0, 0.0,  0.0, 1.0, 0.0,  -1.0, 0.0, 0.0,  1.0, 1.0, // left
    1.0, 1.0, 0.0,  +1.0, 0.0, 0.0,  0.0, 1.0,  1.0, 0.0, 0.0,  +1.0, 0.0, 0.0,  0.0, 0.0,  1.0, 0.0, 1.0,  +1.0, 0.0, 0.0,  1.0, 0.0, // right
    1.0, 1.0, 0.0,  +1.0, 0.0, 0.0,  0.0, 1.0,  1.0, 0.0, 1.0,  +1.0, 0.0, 0.0,  1.0, 0.0,  1.0, 1.0, 1.0,  +1.0, 0.0, 0.0,  1.0, 1.0, // right
    0.0, 1.0, 1.0,  0.0, +1.0, 0.0,  0.0, 1.0,  0.0, 1.0, 0.0,  0.0, +1.0, 0.0,  0.0, 0.0,  1.0, 1.0, 0.0,  0.0, +1.0, 0.0,  1.0, 0.0, // top
    0.0, 1.0, 1.0,  0.0, +1.0, 0.0,  0.0, 1.0,  1.0, 1.0, 0.0,  0.0, +1.0, 0.0,  1.0, 0.0,  1.0, 1.0, 1.0,  0.0, +1.0, 0.0,  1.0, 1.0, // top
    0.0, 0.0, 0.0,  0.0, -1.0, 0.0,  0.0, 1.0,  0.0, 0.0, 1.0,  0.0, -1.0, 0.0,  0.0, 0.0,  1.0, 0.0, 1.0,  0.0, -1.0, 0.0,  1.0, 0.0, // bottom
    0.0, 0.0, 0.0,  0.0, -1.0, 0.0,  0.0, 1.0,  1.0, 0.0, 1.0,  0.0, -1.0, 0.0,  1.0, 0.0,  1.0, 0.0, 0.0,  0.0, -1.0, 0.0,  1.0, 1.0, // bottom
  ]
};

const POSITION_VERTICES_IN_BLOCK = 6 * 2 * 3;
const NORMAL_VERTICES_IN_BLOCK = POSITION_VERTICES_IN_BLOCK;
const UV_VERTICES_IN_BLOCK     = POSITION_VERTICES_IN_BLOCK;
const FLOATS_IN_BLOCK = (POSITION_VERTICES_IN_BLOCK + NORMAL_VERTICES_IN_BLOCK) * 3 + UV_VERTICES_IN_BLOCK * 2;

assert.strictEqual(module.exports.vertexData.length, FLOATS_IN_BLOCK);