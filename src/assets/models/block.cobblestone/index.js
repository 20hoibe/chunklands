const { promisify } = require('util');
const readFile = promisify(require('fs').readFile);

module.exports = async () => ({
  id: 'block.cobblestone',
  faces: require('../_geometry/cube-one-texture'),
  opaque: true,
  texture: await readFile(`${__dirname}/cobblestone.png`)
});