#add source files used by your project here
set(PROJECT_SRC

  ${PROJECT_DIR}/fastnoise/FastNoiseSIMD.cpp
  ${PROJECT_DIR}/fastnoise/FastNoiseSIMD_avx2.cpp
  ${PROJECT_DIR}/fastnoise/FastNoiseSIMD_internal.cpp
  ${PROJECT_DIR}/fastnoise/FastNoiseSIMD_sse2.cpp
  ${PROJECT_DIR}/fastnoise/FastNoiseSIMD_sse41.cpp

  ${PROJECT_DIR}/Chunk.cpp
  ${PROJECT_DIR}/Game.cpp
  ${PROJECT_DIR}/main.cpp
  ${PROJECT_DIR}/TerrainComponent.cpp
  ${PROJECT_DIR}/WorldClientState.cpp)