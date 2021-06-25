#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// Constants and types
//-----------------------------------------------------------------------------
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define HEX_WIDTH 60
#define HEX_HEIGHT 52

typedef enum {
    HEX_ID_GREEN,
    HEX_ID_BLUE,
    HEX_ID_YELLOW,
    HEX_ID_MAGENTA,
    HEX_ID_PURPLE,
    HEX_ID_RED,
} HexID;

//-----------------------------------------------------------------------------
// Global Data
//-----------------------------------------------------------------------------
static SDL_Window* g_window = NULL;
static SDL_Renderer* g_renderer = NULL;
static SDL_Texture* g_texture_hex_basic = NULL;
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
    if (g_texture_hex_basic) {
        SDL_DestroyTexture(g_texture_hex_basic);
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

    g_texture_hex_basic = load_texture("media/hex_basic.png");
    all_loaded &= (g_texture_hex_basic != NULL);

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

void render_hex_at(HexID id, int x, int y) {
    SDL_Rect src = {
        .x = id * HEX_WIDTH,
        .y = 0,
        .w = HEX_WIDTH,
        .h = HEX_HEIGHT,
    };

    SDL_Rect dest = {
        .x = x,
        .y = y,
        .w = HEX_WIDTH,
        .h = HEX_HEIGHT
    };

    bool centered = true;
    if (centered) {
        dest.x -= HEX_WIDTH / 2;
        dest.y -= HEX_HEIGHT / 2;
    }
    SDL_RenderCopy(g_renderer, g_texture_hex_basic, &src, &dest);
}


static void render(void) {
    SDL_RenderClear(g_renderer);
    SDL_SetRenderDrawColor(g_renderer, 0x44, 0x44, 0x44, 0xFF);
    render_hex_at(HEX_ID_GREEN, 100, 100);
    render_hex_at(HEX_ID_RED, 100 + HEX_WIDTH, 100);
    render_hex_at(HEX_ID_MAGENTA, 100 + 2 * HEX_WIDTH, 100);
    render_hex_at(HEX_ID_PURPLE, 100 + 3 * HEX_WIDTH, 100);
    render_hex_at(HEX_ID_YELLOW, 100 + 4 * HEX_WIDTH, 100);
    render_hex_at(HEX_ID_BLUE, 100 + 5 * HEX_WIDTH, 100);
    render_hex_at(HEX_ID_BLUE, g_hex_x, g_hex_y);
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
