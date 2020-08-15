const assert = require('assert');

const RENDER_DISTANCE = 3;
const renderChunkOffsets = generatePosOffsets(RENDER_DISTANCE);

module.exports = class ChunkManager {
  constructor(api, blocks) {
    this._api = api;
    this._blocks = blocks;
    this._chunks = new Map();
    this._currentChunkPos = {x: NaN, y: NaN, z: NaN};
    this._api.cameraOn('positionchange', event => {
      this._handleUpdatePosition(event);
    });

    this._api.cameraGetPosition().then(cameraPos => this._handleUpdatePosition(cameraPos));
  }

  _handleUpdatePosition(cameraPos) {
    const pos = chunkPos(cameraPos);
    if (pos.x !== this._currentChunkPos.x || pos.y !== this._currentChunkPos.y || pos.z !== this._currentChunkPos.z) {
      this._currentChunkPos = pos;
      this._handleChunkChange(pos);
    }
  }

  async _handleChunkChange(chunkPos) {
    for (const [key, chunk] of this._chunks.entries()) {
      if (distanceCmp(chunk.pos, chunkPos, RENDER_DISTANCE) === 1) {
        const deleted = this._chunks.delete(key);
        assert(deleted);

        this._api.sceneRemoveChunk(chunk.hChunk);
        this._api.chunkDelete(chunk.hChunk);
      }
    }

    for (const renderChunkOffset of renderChunkOffsets) {
      const cx = renderChunkOffset.x + chunkPos.x;
      const cy = renderChunkOffset.y + chunkPos.y;
      const cz = renderChunkOffset.z + chunkPos.z;

      const key = `${cx}:${cy}:${cz}`;
      if (!this._chunks.has(key)) {
        const pos = {x: cx, y: cy, z: cz};
        const hChunk = await this._createChunk(pos);
        this._chunks.set(key, {
          pos,
          hChunk
        });
      }
    }
  }

  async _createChunk(chunkPos) {
    const chunk = await this._api.chunkCreate(chunkPos.x, chunkPos.y, chunkPos.z);
    await this._api.chunkUpdate(chunk, createChunk(this._blocks, chunkPos));
    await this._api.sceneAddChunk(chunk);
    return chunk;
  }
}

function distanceCmp(a, b, d) {
  const d2 = d ** 2;
  const diff2 = (a.x - b.x) ** 2 + (a.y - b.y) ** 2 + (a.z - b.z) ** 2;
  return diff2 < d2 ? -1 : (diff2 > d2 ? 1 : 0);
}

function chunkPos(coord) {
  return {
    x: chunkDimPos(coord.x),
    y: chunkDimPos(coord.y),
    z: chunkDimPos(coord.z)
  };
}

function chunkDimPos(dim) {
  return dim >= 0 ? Math.floor(dim / 32) : Math.floor((dim - 1) / 32);
}

function createChunk(blocks, coord) {
  const arrayBuffer = new ArrayBuffer(32 * 32 * 32 * BigUint64Array.BYTES_PER_ELEMENT);
  const buffer = new BigUint64Array(arrayBuffer);

  const dirtHandle = blocks['block.dirt'];
  const airHandle = blocks['block.air'];
  assert(dirtHandle);

  const offset = {x: coord.x * 32, y: coord.y * 32, z: coord.z * 32};

  let i = 0;
  for (let z = 0; z < 32; z++) {
    for (let y = 0; y < 32; y++) {
      for (let x = 0; x < 32; x++, i++) {
        if (offset.y + y === 0) {
          buffer[i] = dirtHandle;
        } else {
          buffer[i] = airHandle;
        }
      }
    }
  }

  return arrayBuffer;
}

function generatePosOffsets(distance) {
  const distance2 = distance ** 2;
  const posWithSquareDistances = [];
  for (let z = -distance; z <= distance; z++) {
    for (let y = -distance; y <= distance; y++) {
      for (let x = -distance; x <= distance; x++) {
        const dist2 = x**2 + y**2 + z**2;
        if (dist2 <= distance2) {
          posWithSquareDistances.push({dist2, pos: {x, y, z}});
        }
      }
    }
  }

  posWithSquareDistances.sort((a, b) => {
    return a.dist2 < b.dist2 ? 1 : (a.dist2 > b.dist2 ? -1 : 0);
  });

  return posWithSquareDistances.map(x => x.pos);
}