#include "graphics.h"
#include "game_state.h"
#include <SDL_image.h>

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

static double fps(void) {
    return 1000.0f / g_state.statistics.render_ave_ms;
}

static bool load_all_media(void) {
    bool all_loaded = true;

    g_state.graphics.hex_basic_texture = load_texture("media/hex_basic.png");
    all_loaded &= (g_state.graphics.hex_basic_texture != NULL);
    g_state.graphics.font = TTF_OpenFont("media/Caviar_Dreams_Bold.ttf", FONT_SIZE);
    all_loaded &= (g_state.graphics.font != NULL);

    if (!all_loaded) {
        SDL_Log("Failed to load media. Exiting.");
        return false;
    }
    return true;
}

static void draw_circle(Point center, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(g_state.renderer, color.r, color.g, color.b, color.a);
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(g_state.renderer, center.x + x, center.y + y);
            }
        }
    }
}

bool graphics_init(void) {
    if (!load_all_media()) {
        return false;
    }

    // Upper Left
    Text* score_text = &g_state.score_text;
    text_init(score_text);
    text_set_font(score_text, g_state.graphics.font);
    snprintf(text_buffer(score_text), TEXT_MAX_LEN, "Score: %d", g_state.game.score);
    text_set_point(score_text, 20, 20);
    text_set_color(score_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(score_text);

    Text* level_text = &g_state.level_text;
    text_init(level_text);
    text_set_font(level_text, g_state.graphics.font);
    snprintf(text_buffer(level_text), TEXT_MAX_LEN, "Level: %d", g_state.game.level);
    text_set_point(level_text, 20, score_text->point.y + score_text->height + 20);
    text_set_color(level_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(level_text);

    Text* combos_text = &g_state.combos_text;
    text_init(combos_text);
    text_set_font(combos_text, g_state.graphics.font);
    snprintf(text_buffer(combos_text), TEXT_MAX_LEN, "Combos remaining: %d", g_state.game.combos_remaining);
    text_set_point(combos_text, 20, level_text->point.y + level_text->height + 20);
    text_set_color(combos_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(combos_text);

    // Upper right
    Text* fps_text = &g_state.fps_text;
    text_init(fps_text);
    text_set_font(fps_text, g_state.graphics.font);
    snprintf(text_buffer(fps_text), TEXT_MAX_LEN, "FPS: %3.1f", 100.0f);
    text_set_point(fps_text, LOGICAL_WINDOW_WIDTH, 20);
    text_set_color(fps_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(fps_text);
    text_set_point(fps_text, LOGICAL_WINDOW_WIDTH - fps_text->width - 20, 20);
    text_draw(fps_text);

    return true;
}

void draw_hex(Hex* hex) {
    if (!hex->is_valid) {
        return;
    }

    SDL_Rect src = {
        .x = hex->hex_type * HEX_SOURCE_WIDTH,
        .y = 0,
        .w = HEX_SOURCE_WIDTH,
        .h = HEX_SOURCE_HEIGHT,
    };

    if (!hex->is_rotating) {
        SDL_Rect dest = {
            .x = hex->hex_point.x,
            .y = hex->hex_point.y,
            .w = HEX_WIDTH,
            .h = HEX_HEIGHT,
        };
        if (0 != SDL_RenderCopy(g_state.renderer, g_state.graphics.hex_basic_texture, &src, &dest)) {
            SDL_Log("SDL_RenderCopy error %s", SDL_GetError());
        }
    } else {
        // Rotation point is relative to hex
        SDL_Point rotation_point = {
            .x = g_state.cursor.screen_point.x - hex->hex_point.x,
            .y = g_state.cursor.screen_point.y - hex->hex_point.y,
        };

        // Translate rotation point to origin
        // Scale
        // Translate back
        // Without this, the hexes look overlapped when rotating.
        double dest_x = 0.0f;
        double dest_y = 0.0f;
        dest_x -= (double)rotation_point.x;
        dest_y -= (double)rotation_point.y;
        dest_x *= (double)hex->scale;
        dest_y *= (double)hex->scale;
        dest_x += (double)rotation_point.x;
        dest_y += (double)rotation_point.y;
        dest_x += (double)hex->hex_point.x;
        dest_y += (double)hex->hex_point.y;

        SDL_Rect dest = {
            .x = dest_x,
            .y = dest_y,
            .w = HEX_WIDTH * hex->scale,
            .h = HEX_HEIGHT * hex->scale,
        };

        // Rotation point is relative to dest, so we have to account for scaling factor here too.
        rotation_point.x *= hex->scale;
        rotation_point.y *= hex->scale;

#if 0
        if (0 != SDL_SetTextureAlphaMod(
                g_state.graphics.hex_basic_texture,
                hex->alpha * 255.0f)) {
            SDL_Log("SDL_SetTextureAlphaMod error %s", SDL_GetError());
            return;
        }
#endif
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
}

void graphics_update(void) {
    SDL_SetRenderDrawColor(g_state.renderer, 0x44, 0x44, 0x44, 0xFF);
    SDL_RenderClear(g_state.renderer);

    // Non-rotating hexes
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            Hex* hex = &g_state.hexes[q][r];
            if (!hex->is_rotating) {
                draw_hex(hex);
            }
        }
    }

    // Rotating hexes
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            Hex* hex = &g_state.hexes[q][r];
            if (hex->is_rotating) {
                draw_hex(hex);
            }
        }
    }

    SDL_Color darkorchid = { .r = 0x99, .g = 0x32, .b = 0xcc, .a = 0xff };
    SDL_Color black = { .r = 0, .g = 0, .b = 0, .a = 0xff };
    draw_circle(g_state.cursor.screen_point, 12, black);
    draw_circle(g_state.cursor.screen_point, 11, darkorchid);

    snprintf(text_buffer(&g_state.level_text), TEXT_MAX_LEN, "Level: %u", g_state.game.level);
    text_draw(&g_state.level_text);

    snprintf(text_buffer(&g_state.combos_text), TEXT_MAX_LEN, "Combos remaining: %u", g_state.game.combos_remaining);
    text_draw(&g_state.combos_text);

    snprintf(text_buffer(&g_state.score_text), TEXT_MAX_LEN, "Score: %u", g_state.game.score);
    text_draw(&g_state.score_text);

    Statistics* stats = &g_state.statistics;
    Text* fps_text = &g_state.fps_text;
    if (stats->total_frames > 0 && stats->total_frames % 60 == 0) {
        snprintf(text_buffer(fps_text), TEXT_MAX_LEN, "FPS: %3.1f", fps());
    }
    text_draw(fps_text);
}

void graphics_flip(void) {
    SDL_RenderPresent(g_state.renderer);
}
