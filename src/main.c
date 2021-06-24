#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------------
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

//-----------------------------------------------------------------------------
// Global Data
//-----------------------------------------------------------------------------
static SDL_Window* g_window = NULL;
static SDL_Renderer* g_renderer = NULL;
static SDL_Texture* g_texture = NULL;
static bool g_follow_mouse = true;
static int g_mouse_x = 0;
static int g_mouse_y = 0;
static int g_hex_x = 0;
static int g_hex_y = 0;
static bool g_game_running = true;

//-----------------------------------------------------------------------------
// Static local functions
//-----------------------------------------------------------------------------
static void init(void) {
    if (0 != SDL_Init(SDL_INIT_EVERYTHING)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    int flags = IMG_INIT_PNG;
    int initialized = IMG_Init(flags);
    if ((initialized & flags) != flags) {
        SDL_Log("IMG_Init failed init flags: 0x%08X", flags);
        exit(1);
    }

    SDL_Log("SDL initialized\n");
}

static void close(void) {
    if (g_texture) {
        SDL_DestroyTexture(g_texture);
    }
    if (g_renderer) {
        SDL_DestroyRenderer(g_renderer);
    }
    if (g_window) {
        SDL_DestroyWindow(g_window);
    }

    IMG_Quit();
    SDL_Quit();
}

static void create_window(void) {
    g_window = SDL_CreateWindow(
        "Hectic Hexagons",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        0
    );
    if (g_window == NULL) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        close();
        exit(1);
    }

    g_renderer = SDL_CreateRenderer(g_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (g_renderer == NULL) {
        SDL_Log("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        close();
        exit(1);
    }
}

static SDL_Texture* load_texture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (surface == NULL) {
        SDL_Log("IMG_Load(%s) failed: %s", path, SDL_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(g_renderer, surface);
    if (texture == NULL) {
        SDL_Log("SDL_CreateTextureFromSurface(%s) failed: %s", path, SDL_GetError());
        return NULL;
    }

    SDL_FreeSurface(surface);
    return texture;
}

static void load_all_media(void) {
    bool all_loaded = true;

    g_texture = load_texture("media/hex_red.png");
    all_loaded &= (g_texture != NULL);

    if (!all_loaded) {
        SDL_Log("Failed to load media. Exiting.");
        close();
        exit(1);
    }
}

static void process_input(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            g_game_running = false;
        }
        if (e.type == SDL_KEYDOWN) {
            g_game_running = false;
        }
        if (e.type == SDL_MOUSEBUTTONUP) {
            g_follow_mouse = !g_follow_mouse;
        }
        if (e.type == SDL_MOUSEMOTION) {
            SDL_GetMouseState(&g_mouse_x, &g_mouse_y);
        }
    }
}

static void update(void) {
    if (g_follow_mouse) {
        g_hex_x = g_mouse_x;
        g_hex_y = g_mouse_y;
    }
}

// Render the texture without stretching at (x,y).
static void render_texture_at(SDL_Texture* texture, int x, int y, bool centered) {
    int src_w = 0;
    int src_h = 0;
    SDL_QueryTexture(texture, NULL, NULL, &src_w, &src_h);

    SDL_Rect dest = {
        .x = x,
        .y = y,
        .w = src_w,
        .h = src_h
    };
    if (centered) {
        dest.x -= src_w / 2;
        dest.y -= src_h / 2;
    }
    SDL_RenderCopy(g_renderer, texture, NULL, &dest);
}

static void render_texture_centered_at(SDL_Texture* texture, int x, int y) {
    render_texture_at(texture, x, y, true);
}

static void render(void) {
    SDL_RenderClear(g_renderer);
    SDL_SetRenderDrawColor(g_renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    render_texture_centered_at(g_texture, g_hex_x, g_hex_y);
    SDL_RenderPresent(g_renderer);
}

//-----------------------------------------------------------------------------
// Global functions
//-----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    init();
    create_window();
    load_all_media();

    while (g_game_running) {
        process_input();
        update();
        render();
    }

    close();
    return 0;
}
