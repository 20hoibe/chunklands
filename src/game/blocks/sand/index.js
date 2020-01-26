
module.exports = {
  id: 'block.sand',
  vertexData: {
      // VERTEX1      NORMAL1          UV1        VERTEX2         NORMAL2          UV2        VERTEX3         NORMAL3          UV3
    front: [
      0.0, 1.0, 0.0,  0.0, 0.0, -1.0,  0.0, 1.0,  0.0, 0.0, 0.0,  0.0, 0.0, -1.0,  0.0, 0.0,  1.0, 0.0, 0.0,  0.0, 0.0, -1.0,  1.0, 0.0,
      0.0, 1.0, 0.0,  0.0, 0.0, -1.0,  0.0, 1.0,  1.0, 0.0, 0.0,  0.0, 0.0, -1.0,  1.0, 0.0,  1.0, 1.0, 0.0,  0.0, 0.0, -1.0,  1.0, 1.0,
    ],
    back: [
      1.0, 1.0, 1.0,  0.0, 0.0, +1.0,  0.0, 1.0,  1.0, 0.0, 1.0,  0.0, 0.0, +1.0,  0.0, 0.0,  0.0, 0.0, 1.0,  0.0, 0.0, +1.0,  1.0, 0.0,
      1.0, 1.0, 1.0,  0.0, 0.0, +1.0,  0.0, 1.0,  0.0, 0.0, 1.0,  0.0, 0.0, +1.0,  1.0, 0.0,  0.0, 1.0, 1.0,  0.0, 0.0, +1.0,  1.0, 1.0,
    ],
    left: [
      0.0, 1.0, 1.0,  -1.0, 0.0, 0.0,  0.0, 1.0,  0.0, 0.0, 1.0,  -1.0, 0.0, 0.0,  0.0, 0.0,  0.0, 0.0, 0.0,  -1.0, 0.0, 0.0,  1.0, 0.0,
      0.0, 1.0, 1.0,  -1.0, 0.0, 0.0,  0.0, 1.0,  0.0, 0.0, 0.0,  -1.0, 0.0, 0.0,  1.0, 0.0,  0.0, 1.0, 0.0,  -1.0, 0.0, 0.0,  1.0, 1.0,
    ],
    right: [
      1.0, 1.0, 0.0,  +1.0, 0.0, 0.0,  0.0, 1.0,  1.0, 0.0, 0.0,  +1.0, 0.0, 0.0,  0.0, 0.0,  1.0, 0.0, 1.0,  +1.0, 0.0, 0.0,  1.0, 0.0,
      1.0, 1.0, 0.0,  +1.0, 0.0, 0.0,  0.0, 1.0,  1.0, 0.0, 1.0,  +1.0, 0.0, 0.0,  1.0, 0.0,  1.0, 1.0, 1.0,  +1.0, 0.0, 0.0,  1.0, 1.0,
    ],
    top: [
      0.0, 1.0, 1.0,  0.0, +1.0, 0.0,  0.0, 1.0,  0.0, 1.0, 0.0,  0.0, +1.0, 0.0,  0.0, 0.0,  1.0, 1.0, 0.0,  0.0, +1.0, 0.0,  1.0, 0.0,
      0.0, 1.0, 1.0,  0.0, +1.0, 0.0,  0.0, 1.0,  1.0, 1.0, 0.0,  0.0, +1.0, 0.0,  1.0, 0.0,  1.0, 1.0, 1.0,  0.0, +1.0, 0.0,  1.0, 1.0,
    ],
    bottom: [
      0.0, 0.0, 0.0,  0.0, -1.0, 0.0,  0.0, 1.0,  0.0, 0.0, 1.0,  0.0, -1.0, 0.0,  0.0, 0.0,  1.0, 0.0, 1.0,  0.0, -1.0, 0.0,  1.0, 0.0,
      0.0, 0.0, 0.0,  0.0, -1.0, 0.0,  0.0, 1.0,  1.0, 0.0, 1.0,  0.0, -1.0, 0.0,  1.0, 0.0,  1.0, 0.0, 0.0,  0.0, -1.0, 0.0,  1.0, 1.0,
    ],
  },
  opaque: true,
  texture: `${__dirname}/sand.png`
};