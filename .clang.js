require('dotenv').config();
const os = require('os');
const clang = require('./clang');

const DEV = process.env.NODE_ENV !== 'production';

(async () => {
  const buildSet = new clang.BuildSet({
    clangfileAbsolutePath: __filename,
    rootAbsolutePath: __dirname,
    buildAbsolutePath: `${__dirname}/build`,
    debug: DEV,
    clangBin: process.env.CLANG_BIN,
    clangTidyBin: process.env.CLANG_TIDY_BIN,
    clangNoTidy: process.env.CLANG_NO_TIDY === 'true'
  })
  .addMakefileTarget('deps/glfw/src/libglfw3.a', {
    cmd: `@cd deps/glfw && cmake -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=${DEV ? 'Debug' : 'Release'} --build .`
      + ' && make'
  })
  .addMakefileTarget('deps/backward-cpp/libbackward.a', {
    cmd: `@cd deps/backward-cpp && cmake -DBACKWARD_HAS_BFD=1 -DCMAKE_BUILD_TYPE=${DEV ? 'Debug' : 'Release'} --build .`
    + ' && make'
  })
  .addMakefileTarget('deps/googletest/lib/libgtestd.a', {
    cmd: `@cd deps/googletest && cmake -DCMAKE_BUILD_TYPE=${DEV ? 'Debug' : 'Release'} -DCMAKE_CXX_STANDARD=17 -Dgtest_force_shared_crt=ON -DCMAKE_POSITION_INDEPENDENT_CODE=ON .`
      + ' && make'
  });

  // libeasy_profiler
  // cmake .. -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=ON

  const gladCs = await new clang.CompileSet(buildSet, {std: 'c99', fPIC: true})
    .addSystemInclude('deps/glfw/deps')
    .addSystemSource('deps/glfw/deps/glad_gl.c')
    .addToBuildSet();

  const systemInclude = [
    'deps/backward-cpp',
    'deps/boost',
    'deps/easy_profiler/easy_profiler_core/include',
    'deps/glfw/include',
    'deps/glfw/deps',
    'deps/glm',
    'deps/node/src',
    'deps/stb',
    'node_modules/node-addon-api',
  ];

  const chunklandsGenericCs = await new clang.CompileSet(buildSet, {std: 'c++2a', fPIC: true})
    .addSystemInclude(...systemInclude)
    .addInclude('src')
    .addSource(
      'src/chunklands/**/*.cxx',
    )
    .addToBuildSet();

  await new clang.CompileSet(buildSet, {std: 'c++2a', fPIC: true, shared: true})
    .addSystemInclude(...systemInclude)
    .addInclude('src')
    .addSource(
      'src/napimodule.cxx'
    )
    .addLibrary(
      ...chunklandsGenericCs.getObjectPaths(),
      ...gladCs.getObjectPaths(),
      'deps/glfw/src/libglfw3.a',
      'deps/backward-cpp/libbackward.a',
      'deps/boost/stage/lib/*.a',
      'deps/easy_profiler/build/bin/libeasy_profiler.a',
    )
    .addLink(...os.platform() === 'linux' ? ['X11', 'dl', 'bfd'] : [])
    .addMacOSFramework('CoreVideo', 'OpenGL', 'IOKit', 'Cocoa', 'Carbon')
    .addToBuildSet('build/chunklands.node');

  // await new clang.CompileSet(buildSet, {std: 'c++2a', fPIC: true, shared: true})
  //   .addSystemInclude(
  //     ...systemInclude,
  //     'deps/googletest/googletest/include'
  //   )
  //   .addInclude('src')
  //   .addSource(
  //     'src/chunklands_test/**/*.cxx'
  //   )
  //   .addLibrary(
  //     ...chunklandsGenericCs.getObjectPaths(),
  //     ...gladCs.getObjectPaths(),
  //     'deps/glfw/src/libglfw3.a',
  //     'deps/googletest/lib/libgtestd.a',
  //   )
  //   .addToBuildSet('build/chunklands_test.node');

  await buildSet.printMakefile('build/chunklands.node', process.stdout);
})();
