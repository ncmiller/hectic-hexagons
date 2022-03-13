#!/bin/bash

MY_DIR=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )
cd $MY_DIR
# cd demo
# emcc \
#     -v \
#     -O3 \
#     -DIS_WASM_BUILD \
#     -s USE_SDL=2 \
#     -s USE_SDL_TTF=2 \
#     -s USE_SDL_IMAGE=2 \
#     -s SDL2_IMAGE_FORMATS='["png"]' \
#     -s USE_SDL_MIXER=2 \
#     --preload-file ../assets \
#     -I ../src/include \
#     -I ../src/test \
#     ../src/*.c \
#     ../src/test/*.c \
#     -o hectic-hexagons.js

emcc \
    -v \
    -O3 \
    -DIS_WASM_BUILD \
    -s USE_SDL=2 \
    -s USE_SDL_TTF=2 \
    -s USE_SDL_IMAGE=2 \
    -s SDL2_IMAGE_FORMATS='["png"]' \
    -s USE_SDL_MIXER=2 \
    -s ASSERTIONS=1 \
    -s ALLOW_MEMORY_GROWTH=1 \
    --preload-file assets \
    -I src/include \
    -I src/test \
    src/*.c \
    src/test/*.c \
    -o demo/hectic-hexagons.js
