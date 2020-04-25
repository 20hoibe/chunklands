import {SkyboxPassBase} from './SkyboxPassBase'
import {ChunkGeneratorBase} from './ChunkGeneratorBase'
import {GBufferPassBase} from './GBufferPassBase'
import {SSAOBlurPassBase} from './SSAOBlurPassBase'
import {SSAOPassBase} from './SSAOPassBase'
import {SkyboxBase} from './SkyboxBase'
export {WorldBase}


declare class WorldBase {
  setChunkGenerator(chunkGenerator: ChunkGeneratorBase): void
  setSkybox(skybox: SkyboxBase): void

  setSkyboxPass(pass: SkyboxPassBase): void
  setGBufferPass(pass: GBufferPassBase): void
  setSSAOPass(pass: SSAOPassBase): void
  setSSAOBlurPass(pass: SSAOBlurPassBase): void
}
