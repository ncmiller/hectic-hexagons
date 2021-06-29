#include "graphics.h"
#include "game_state.h"
#include <SDL_image.h>

#define HEX_WIDTH 60
#define HEX_HEIGHT 52
#define FONT_SIZE 24

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

#if 0
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
#endif

bool graphics_init(void) {
    if (!load_all_media()) {
        return false;
    }

    // Upper Left
    Text* score_text = &g_state.graphics.score_text;
    text_init(score_text);
    text_set_font(score_text, g_state.graphics.font);
    snprintf(text_buffer(score_text), TEXT_MAX_LEN, "Score: %d", 550);
    text_set_point(score_text, 20, 20);
    text_set_color(score_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(score_text);

    Text* level_text = &g_state.graphics.level_text;
    text_init(level_text);
    text_set_font(level_text, g_state.graphics.font);
    snprintf(text_buffer(level_text), TEXT_MAX_LEN, "Level: %d", 1);
    text_set_point(level_text, 20, score_text->point.y + score_text->height + 20);
    text_set_color(level_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(level_text);

    Text* combos_text = &g_state.graphics.combos_text;
    text_init(combos_text);
    text_set_font(combos_text, g_state.graphics.font);
    snprintf(text_buffer(combos_text), TEXT_MAX_LEN, "Combos remaining: %d", 1);
    text_set_point(combos_text, 20, level_text->point.y + level_text->height + 20);
    text_set_color(combos_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(combos_text);

    // Upper right
    Text* fps_text = &g_state.graphics.fps_text;
    text_init(fps_text);
    text_set_font(fps_text, g_state.graphics.font);
    snprintf(text_buffer(fps_text), TEXT_MAX_LEN, "FPS: %0.3f\n", 0.0);
    text_set_point(fps_text, 1280, 20);
    text_set_color(fps_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(fps_text);
    text_set_point(fps_text, 1280 - fps_text->width - 20, 20);
    text_draw(fps_text);

    double s = 30.0f;
    double h = sqrt(3) * s;
    double w = 2 * s;
    double alpha = 1.0f;

    Hex* hex = &g_state.graphics.hexes[0]; // (q,r) = (0,0)
    hex->hex_id = HEX_ID_GREEN;
    hex->scale = 1.0f;
    hex->hex_point.x = 0;
    hex->hex_point.y = (int)round(0.5f * h);
    hex->rotation_point.x = (int)round(w);
    hex->rotation_point.y = (int)round(0.5f * h);
    hex->rotation_angle = 0.0f;
    hex->alpha = alpha;
    hex->hex_point.x += WINDOW_WIDTH / 2;
    hex->hex_point.y += WINDOW_HEIGHT / 2;

    hex = &g_state.graphics.hexes[1]; // (q, r) = (1,0)
    hex->hex_id = HEX_ID_BLUE;
    hex->scale = 1.0f;
    hex->hex_point.x = (int)round(0.75f * w);
    hex->hex_point.y = 0;
    hex->rotation_point.x = (int)round(0.25f * w);
    hex->rotation_point.y = (int)round(h);
    hex->rotation_angle = 0.0f;
    hex->alpha = alpha;
    hex->hex_point.x += WINDOW_WIDTH / 2;
    hex->hex_point.y += WINDOW_HEIGHT / 2;

    hex = &g_state.graphics.hexes[2]; // (q, r) = (1,1)
    hex->hex_id = HEX_ID_YELLOW;
    hex->scale = 1.0f;
    hex->hex_point.x = (int)round(0.75f * w);
    hex->hex_point.y = (int)round(h);
    hex->rotation_point.x = (int)round(0.25f * w);
    hex->rotation_point.y = 0;
    hex->rotation_angle = 0.0f;
    hex->alpha = alpha;
    hex->hex_point.x += WINDOW_WIDTH / 2;
    hex->hex_point.y += WINDOW_HEIGHT / 2;

    return true;
}

void draw_hex(Hex* hex) {
    SDL_Rect src = {
        .x = hex->hex_id * HEX_WIDTH,
        .y = 0,
        .w = HEX_WIDTH,
        .h = HEX_HEIGHT,
    };

    // Translate rotation_point to origin
    // Scale
    // Translate back
    double dest_x = 0.0f;
    double dest_y = 0.0f;
    dest_x -= (double)hex->rotation_point.x;
    dest_y -= (double)hex->rotation_point.y;
    dest_x *= (double)hex->scale;
    dest_y *= (double)hex->scale;
    dest_x += (double)hex->rotation_point.x;
    dest_y += (double)hex->rotation_point.y;
    dest_x += (double)hex->hex_point.x;
    dest_y += (double)hex->hex_point.y;

    SDL_Rect dest = {
        .x = dest_x,
        .y = dest_y,
        .w = HEX_WIDTH * hex->scale,
        .h = HEX_HEIGHT * hex->scale,
    };

    // Rotation point is relative to dest, so we have to account for scaling factor here too.
    SDL_Point rotation_point = {
        .x = hex->rotation_point.x * hex->scale,
        .y = hex->rotation_point.y * hex->scale,
    };

    if (0 != SDL_SetTextureAlphaMod(
            g_state.graphics.hex_basic_texture,
            hex->alpha * 255.0f)) {
        SDL_Log("SDL_SetTextureAlphaMod error %s", SDL_GetError());
        return;
    }

    if (0 != SDL_RenderCopyEx(
            g_state.renderer,
            g_state.graphics.hex_basic_texture,
            &src,
            &dest,
            hex->rotation_angle,
            &rotation_point,
            SDL_FLIP_NONE)) {
        SDL_Log("SDL_RenderCopyEx error %s", SDL_GetError());
    }
}

void graphics_update(void) {
    SDL_RenderClear(g_state.renderer);
    SDL_SetRenderDrawColor(g_state.renderer, 0x44, 0x44, 0x44, 0xFF);

    for (int i = 0; i < GRAPHICS_NUM_HEXES; i++) {
        draw_hex(&g_state.graphics.hexes[i]);
    }

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
