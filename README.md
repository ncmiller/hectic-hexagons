![build](https://github.com/ncmiller/hectic-hexagons/actions/workflows/build.yml/badge.svg)

# Hectic Hexagons

A clone of Hexic HD, written in C using the SDL2 library.

### Install Dependencies

MacOS:

```sh
brew install sdl2 sdl2_image sdl2_ttf sdl2_mixer
```

Ubuntu:

```sh
sudo apt-get install libsdl2-dev libsdl2-ttf-dev libsdl2-image-dev libsdl2-mixer-dev
```

### Build

```sh
./build.sh
```

### Run

After building, to run the game executable:

```
./build/hectic-hexagons
```

### Run in the browser

You can also run this game in the browser, but it requires you
to install emscripten.

Install dependencies (MacOS):

```sh
brew install emscripten
```

Build:

```sh
./build_wasm.sh
```

This will generate files in the `demo` folder, which you can view
in your browser by starting an http file server, such as:

```sh
python -m http.server
```

Then open `localhost:8000` in your browser, and the game should play.
