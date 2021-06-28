#include "graphics.h"
#include "game_state.h"
#include <SDL_image.h>

#define HEX_WIDTH 60
#define HEX_HEIGHT 52
#define FONT_SIZE 24

typedef enum {
    HEX_ID_GREEN,
    HEX_ID_BLUE,
    HEX_ID_YELLOW,
    HEX_ID_MAGENTA,
    HEX_ID_PURPLE,
    HEX_ID_RED,
} HexID;

static SDL_Texture* load_texture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (surface == NULL) {
        SDL_Log("IMG_Load(%s) failed: %s", path, SDL_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(g_state.renderer, surface);
    if (texture == NULL) {
        SDL_Log("SDL_CreateTextureFromSurface(%s) failed: %s", path, SDL_GetError());
        return NULL;
    }

    SDL_FreeSurface(surface);
    return texture;
}

static bool load_all_media(void) {
    bool all_loaded = true;

    g_state.graphics.hex_basic_texture = load_texture("media/hex_basic.png");
    all_loaded &= (g_state.graphics.hex_basic_texture != NULL);
    g_state.graphics.font = TTF_OpenFont("media/clacon.ttf", FONT_SIZE);
    all_loaded &= (g_state.graphics.font != NULL);

    if (!all_loaded) {
        SDL_Log("Failed to load media. Exiting.");
        return false;
    }
    return true;
}

static void draw_hex_at(HexID id, int x, int y, bool centered) {
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

    if (centered) {
        dest.x -= HEX_WIDTH / 2;
        dest.y -= HEX_HEIGHT / 2;
    }
    SDL_RenderCopy(g_state.renderer, g_state.graphics.hex_basic_texture, &src, &dest);
}

bool graphics_init(void) {
    if (!load_all_media()) {
        return false;
    }

    // Upper Left
    Text* score_text = &g_state.graphics.score_text;
    text_init(score_text);
    text_set_font(score_text, g_state.graphics.font);
    snprintf(text_buffer(score_text), TEXT_MAX_LEN, "Score: %d", 550);
    text_set_position(score_text, 20, 20);
    text_set_color(score_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(score_text);

    Text* level_text = &g_state.graphics.level_text;
    text_init(level_text);
    text_set_font(level_text, g_state.graphics.font);
    snprintf(text_buffer(level_text), TEXT_MAX_LEN, "Level: %d", 1);
    text_set_position(level_text, 20, score_text->position.y + score_text->height + 20);
    text_set_color(level_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(level_text);

    Text* combos_text = &g_state.graphics.combos_text;
    text_init(combos_text);
    text_set_font(combos_text, g_state.graphics.font);
    snprintf(text_buffer(combos_text), TEXT_MAX_LEN, "Combos remaining: %d", 1);
    text_set_position(combos_text, 20, level_text->position.y + level_text->height + 20);
    text_set_color(combos_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(combos_text);

    // Upper right
    Text* fps_text = &g_state.graphics.fps_text;
    text_init(fps_text);
    text_set_font(fps_text, g_state.graphics.font);
    snprintf(text_buffer(fps_text), TEXT_MAX_LEN, "FPS: %0.3f\n", 0.0);
    text_set_position(fps_text, 1280, 20);
    text_set_color(fps_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(fps_text);
    text_set_position(fps_text, 1280 - fps_text->width - 20, 20);
    text_draw(fps_text);

    return true;
}

void graphics_update(void) {
    SDL_RenderClear(g_state.renderer);
    SDL_SetRenderDrawColor(g_state.renderer, 0x44, 0x44, 0x44, 0xFF);

    Uint32 hex_x = WINDOW_WIDTH / 2 - 3 * HEX_WIDTH;
    draw_hex_at(HEX_ID_GREEN, hex_x, WINDOW_HEIGHT / 2, false);
    draw_hex_at(HEX_ID_RED, hex_x + HEX_WIDTH, WINDOW_HEIGHT / 2, false);
    draw_hex_at(HEX_ID_MAGENTA, hex_x + 2 * HEX_WIDTH, WINDOW_HEIGHT / 2, false);
    draw_hex_at(HEX_ID_PURPLE, hex_x + 3 * HEX_WIDTH, WINDOW_HEIGHT / 2, false);
    draw_hex_at(HEX_ID_YELLOW, hex_x + 4 * HEX_WIDTH, WINDOW_HEIGHT / 2, false);
    draw_hex_at(HEX_ID_BLUE, hex_x + 5 * HEX_WIDTH, WINDOW_HEIGHT / 2, false);
    draw_hex_at(HEX_ID_BLUE, g_state.game.hex_position.x, g_state.game.hex_position.y, true);

    snprintf(text_buffer(&g_state.graphics.level_text), TEXT_MAX_LEN, "Level: %u", g_state.game.level);
    text_draw(&g_state.graphics.level_text);

    snprintf(text_buffer(&g_state.graphics.combos_text), TEXT_MAX_LEN, "Combos remaining: %u", g_state.game.combos_remaining);
    text_draw(&g_state.graphics.combos_text);

    snprintf(text_buffer(&g_state.graphics.score_text), TEXT_MAX_LEN, "Score: %u", g_state.game.score);
    text_draw(&g_state.graphics.score_text);

    Statistics* stats = &g_state.statistics;
    Text* fps_text = &g_state.graphics.fps_text;
    if (stats->total_frames > 0 && stats->total_frames % 60 == 0) {
        snprintf(text_buffer(fps_text), TEXT_MAX_LEN, "FPS: %0.3f", 1000.0f / stats->render_ave_ms);
    }
    text_draw(fps_text);
}

void graphics_flip(void) {
    SDL_RenderPresent(g_state.renderer);
}
