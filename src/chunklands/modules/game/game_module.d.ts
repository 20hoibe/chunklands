import * as engine from '../engine/engine_module'
import { EventEmitter } from 'events'

export declare class BlockRegistrar {
  addBlock(block: {id: string, opaque: boolean, faces: {[face: string]: number[]}}): number
  loadTexture(texture: string): void
}

export declare interface IWorldGenerator {
  generateChunk(x: number, y: number, z: number, chunkDim: number, cb: (err: Error, chunk: number[]) => void): void
}

export declare class ChunkGenerator {
  setBlockRegistrar(value: BlockRegistrar): void
  setWorldGenerator(value: IWorldGenerator): void
}

export declare class Scene implements engine.IScene {
  setWindow(window: Window): void
  setWorld(world: World): void

  setGBufferPass(pass: engine.GBufferPass): void
  setSSAOPass(pass: engine.SSAOPass): void
  setSSAOBlurPass(pass: engine.SSAOBlurPass): void
  setLightingPass(pass: engine.LightingPass): void
  setSkyboxPass(pass: engine.SkyboxPass): void

  setTextRenderer(renderer: engine.TextRenderer): void

  setSkybox(skybox: engine.Skybox): void

  setMovementController(controller: engine.MovementController): void
  setCamera(camera: engine.Camera): void

  getFlightMode(): boolean
  setFlightMode(flightMode: boolean): void

  events: EventEmitter
}

export declare class World implements engine.ICollisionSystem {
  setChunkGenerator(chunkGenerator: ChunkGenerator): void
}