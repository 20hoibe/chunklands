# Chunklands

> This page is designed for developers.

If you want to read more of the project, goals, etc. please have a look at [chunklands.de](https://chunklands.de).

## Notice: Breaking change

Former Chunklands architecture (single-threaded) had performance issues.
POC of `executor-architecture` was merged into master to see daily process on master.

Still the set of features is not fully implemented, but we decided to merge it now.

It will take some time to support all old features.
Please also consider the documentation to be out-dated.

## Technologies

![C++](doc/cpp.svg)
![NodeJS](doc/nodejs.svg)
![N-API](doc/napi.svg)
![boost](doc/boost.svg)
![OpenGL](doc/opengl.svg)
![clang](doc/clang.svg)


## Installation

Requirements:

- node.js 14+
- clang-10+
- cmake 3.0+ (for glfw)

**Linux**

```bash
make setup
```

**MacOS**

```bash
make setup
```

MacOS ships apple clang. It's likely to be incompatible, so another vendor-free clang version is needed.

```bash
brew install llvm

# add this to your .env file
cat .env
# CLANG_BIN=/usr/local/opt/llvm/bin/clang
# CLANG_TIDY_BIN=/usr/local/opt/llvm/bin/clang-tidy
```

**Windows:**

*not supported, sorry*

## Development

watch files and build
```bash
npm run cxx:watch
npm run ts:watch
```

start game
```bash
npm start
```

## Links

- [GitHub Issues](https://github.com/chunklands/chunklands/issues)
- [GitHub Project](https://github.com/chunklands/chunklands/projects/1)


## Progress

Current status:

![Current Result](./status.png)
