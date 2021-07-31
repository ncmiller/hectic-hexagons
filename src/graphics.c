#include "graphics.h"
#include "game_state.h"
#include "window.h"
#include "constants.h"
#include "macros.h"
#include "statistics.h"
#include <SDL_image.h>

#define HEX_RADIUS 30
#define HEX_SOURCE_WIDTH 60
#define HEX_SOURCE_HEIGHT 52
#define DISPLAY_HEX_COORDS

#if 0 // 1080p
#define CURSOR_RADIUS 12
#define FONT_SIZE 24
#define LOCAL_SCORE_FONT_SIZE 20
#define HEX_COORD_FONT_SIZE 12
#else // 720p
#define CURSOR_RADIUS 8
#define FONT_SIZE 20
#define LOCAL_SCORE_FONT_SIZE 18
#define HEX_COORD_FONT_SIZE 12
#endif

typedef struct {
    SDL_Texture* hex_basic_texture;
    TTF_Font* font;
    TTF_Font* local_score_font;
    TTF_Font* hex_coord_font;

    Text level_text;
    Text combos_text;
    Text score_text;
    Text fps_text;
    Text update_text;
    Text render_text;
    Text hex_coord_text[HEX_NUM_COLUMNS][HEX_NUM_ROWS];
} Graphics;

static Graphics _graphics;

static const SDL_Color white = { .r = 0xff, .g = 0xff, .b = 0xff, .a = 0xff };
static const SDL_Color darkorchid = { .r = 0x99, .g = 0x32, .b = 0xcc, .a = 0xff };
static const SDL_Color black = { .r = 0, .g = 0, .b = 0, .a = 0xff };

static SDL_Texture* load_texture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (surface == NULL) {
        SDL_Log("IMG_Load(%s) failed: %s", path, SDL_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(window_renderer(), surface);
    if (texture == NULL) {
        SDL_Log("SDL_CreateTextureFromSurface(%s) failed: %s", path, SDL_GetError());
        return NULL;
    }

    SDL_FreeSurface(surface);
    return texture;
}

static bool load_all_media(void) {
    bool all_loaded = true;

    _graphics.hex_basic_texture = load_texture("media/hex_basic.png");
    all_loaded &= (_graphics.hex_basic_texture != NULL);
    _graphics.font = TTF_OpenFont("media/Caviar_Dreams_Bold.ttf", FONT_SIZE);
    all_loaded &= (_graphics.font != NULL);
    _graphics.local_score_font = TTF_OpenFont("media/Caviar_Dreams_Bold.ttf", LOCAL_SCORE_FONT_SIZE);
    all_loaded &= (_graphics.local_score_font != NULL);
    _graphics.hex_coord_font = TTF_OpenFont("media/Caviar_Dreams_Bold.ttf", HEX_COORD_FONT_SIZE);
    all_loaded &= (_graphics.hex_coord_font != NULL);

    if (!all_loaded) {
        SDL_Log("Failed to load media. Exiting.");
        return false;
    }
    return true;
}


static void draw_circle(Point center, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(window_renderer(), color.r, color.g, color.b, color.a);
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                SDL_RenderDrawPoint(window_renderer(), center.x + x, center.y + y);
            }
        }
    }
}

static int compare_point_y(const void* point1, const void* point2) {
    Point p1 = *(Point*)point1;
    Point p2 = *(Point*)point2;
    return p1.y - p2.y;
}

static void draw_bottom_flat_triangle(Point p1, Point p2, Point p3) {
    float invslope1 = (p2.x - p1.x) / (p2.y - p1.y);
    float invslope2 = (p3.x - p1.x) / (p3.y - p1.y);
    float curx1 = p1.x;
    float curx2 = p1.x;
    for (int scanline_y = p1.y; scanline_y <= p2.y; scanline_y++) {
        SDL_RenderDrawLine(window_renderer(), curx1, scanline_y, curx2, scanline_y);
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

static void draw_top_flat_triangle(Point p1, Point p2, Point p3) {
    float invslope1 = (p3.x - p1.x) / (p3.y - p1.y);
    float invslope2 = (p3.x - p2.x) / (p3.y - p2.y);
    float curx1 = p3.x;
    float curx2 = p3.x;
    for (int scanline_y = p3.y; scanline_y >= p1.y; scanline_y--) {
        SDL_RenderDrawLine(window_renderer(), curx1, scanline_y, curx2, scanline_y);
        curx1 -= invslope1;
        curx2 -= invslope2;
    }
}

// Ref: http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html
static void draw_filled_triangle(Point p1, Point p2, Point p3, SDL_Color color) {
    // Sort points by y value, lowest to highest
    Point points[3] = {p1, p2, p3};
    qsort(points, 3, sizeof(Point), compare_point_y);

    SDL_SetRenderDrawColor(window_renderer(), color.r, color.g, color.b, color.a);
    if (points[1].y == points[2].y) {
        draw_bottom_flat_triangle(points[0], points[1], points[2]);
    } else if (points[0].y == points[1].y) {
        draw_top_flat_triangle(points[0], points[1], points[2]);
    } else {
        // General case: Split into bottom-flat and top-flat triangles
        Point p4 = {
            .x = (int)(
                    points[0].x +
                    ((float)(points[1].y - points[0].y) / (float)(points[2].y - points[0].y)) *
                    (points[2].x - points[0].x)),
            .y = points[1].y
        };
        draw_bottom_flat_triangle(points[0], points[1], p4);
        draw_top_flat_triangle(points[1], p4, points[2]);
    }

    // SDL_RenderDrawLine(window_renderer(), p1.x, p1.y, p2.x, p2.y);
    // SDL_RenderDrawLine(window_renderer(), p2.x, p2.y, p3.x, p3.y);
    // SDL_RenderDrawLine(window_renderer(), p3.x, p3.y, p1.x, p1.y);
}

static void draw_hex(Point middle, int radius, SDL_Color color) {
    int h = (int)(sqrt(3.0f) * (float)radius);
    int w = 2 * radius;
    Point top_left = {
        .x = middle.x - w / 2,
        .y = middle.y - h / 2,
    };
    draw_filled_triangle(
            (Point){top_left.x + w / 2, top_left.y + h / 2},
            (Point){top_left.x, top_left.y + h / 2},
            (Point){top_left.x + w / 4, top_left.y},
            color);
    draw_filled_triangle(
            (Point){top_left.x + w / 2, top_left.y + h / 2},
            (Point){top_left.x + w / 4, top_left.y},
            (Point){top_left.x + 3 * w / 4, top_left.y},
            color);
    draw_filled_triangle(
            (Point){top_left.x + w / 2, top_left.y + h / 2},
            (Point){top_left.x + 3 * w / 4, top_left.y},
            (Point){top_left.x + w, top_left.y + h / 2},
            color);
    draw_filled_triangle(
            (Point){top_left.x + w / 2, top_left.y + h / 2},
            (Point){top_left.x, top_left.y + h / 2},
            (Point){top_left.x + w / 4, top_left.y + h},
            color);
    draw_filled_triangle(
            (Point){top_left.x + w / 2, top_left.y + h / 2},
            (Point){top_left.x + w / 4, top_left.y + h},
            (Point){top_left.x + 3 * w / 4, top_left.y + h},
            color);
    draw_filled_triangle(
            (Point){top_left.x + w / 2, top_left.y + h / 2},
            (Point){top_left.x + 3 * w / 4, top_left.y + h},
            (Point){top_left.x + w, top_left.y + h / 2},
            color);
}

bool graphics_init(void) {
    if (!load_all_media()) {
        return false;
    }

    // Upper Left
    Text* score_text = &_graphics.score_text;
    text_init(score_text);
    text_set_font(score_text, _graphics.font);
    snprintf(text_buffer(score_text), TEXT_MAX_LEN, "Score: %d", g_state.game.score);
    text_set_point(score_text, 20, 20);
    text_set_color(score_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(score_text);

    Text* level_text = &_graphics.level_text;
    text_init(level_text);
    text_set_font(level_text, _graphics.font);
    snprintf(text_buffer(level_text), TEXT_MAX_LEN, "Level: %d", g_state.game.level);
    text_set_point(level_text, 20, score_text->point.y + score_text->height + 20);
    text_set_color(level_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(level_text);

    Text* combos_text = &_graphics.combos_text;
    text_init(combos_text);
    text_set_font(combos_text, _graphics.font);
    snprintf(text_buffer(combos_text), TEXT_MAX_LEN, "Combos remaining: %d", g_state.game.combos_remaining);
    text_set_point(combos_text, 20, level_text->point.y + level_text->height + 20);
    text_set_color(combos_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(combos_text);

    // Upper right
    Text* fps_text = &_graphics.fps_text;
    text_init(fps_text);
    text_set_font(fps_text, _graphics.font);
    snprintf(text_buffer(fps_text), TEXT_MAX_LEN, "FPS: %3.1f", 100.0f);
    text_set_point(fps_text, LOGICAL_WINDOW_WIDTH, 20);
    text_set_color(fps_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(fps_text);
    text_set_point(fps_text, LOGICAL_WINDOW_WIDTH - fps_text->width - 20, 20);
    text_draw(fps_text);

    Text* update_text = &_graphics.update_text;
    text_init(update_text);
    text_set_font(update_text, _graphics.font);
    snprintf(text_buffer(update_text), TEXT_MAX_LEN, "Upd: %3.1f", 100.0f);
    text_set_point(update_text, LOGICAL_WINDOW_WIDTH, 40);
    text_set_color(update_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(update_text);
    text_set_point(update_text, LOGICAL_WINDOW_WIDTH - update_text->width - 20, 40);
    text_draw(update_text);

#if 0
    Text* render_text = &_graphics.render_text;
    text_init(render_text);
    text_set_font(render_text, _graphics.font);
    snprintf(text_buffer(render_text), TEXT_MAX_LEN, "Rnd: %3.1f", 100.0f);
    text_set_point(render_text, LOGICAL_WINDOW_WIDTH, 60);
    text_set_color(render_text, 0xFF, 0xFF, 0xFF, 0xFF);
    text_draw(render_text);
    text_set_point(render_text, LOGICAL_WINDOW_WIDTH - render_text->width - 20, 60);
    text_draw(render_text);
#endif

#ifdef DISPLAY_HEX_COORDS
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            Text* coord_text = &_graphics.hex_coord_text[q][r];
            text_init(coord_text);
            text_set_font(coord_text, _graphics.hex_coord_font);
            snprintf(text_buffer(coord_text), TEXT_MAX_LEN, "%d,%d", q, r);
            Point p = transform_hex_to_screen(q, r);
            text_set_point(
                    coord_text,
                    p.x + HEX_WIDTH / 2 - 10,
                    p.y + HEX_HEIGHT / 2 - 8);
            text_set_color(coord_text, 0, 0, 0, 0xFF);
            text_draw(coord_text);
        }
    }
#endif

    return true;
}

void draw_animated_hex(const Hex* hex, Point animation_center, bool is_cursor_hex) {
    if (!hex->is_valid) {
        return;
    }

    SDL_Rect src = {
        .x = hex->type * HEX_SOURCE_WIDTH,
        .y = 0,
        .w = HEX_SOURCE_WIDTH,
        .h = HEX_SOURCE_HEIGHT,
    };


    // Center point must be relative to hex
    SDL_Point center = {
        .x = animation_center.x - hex->hex_point.x,
        .y = animation_center.y - hex->hex_point.y,
    };

    // Translate center point to origin
    // Scale
    // Translate back
    double dest_x = 0.0f;
    double dest_y = 0.0f;
    dest_x -= (double)center.x;
    dest_y -= (double)center.y;
    dest_x *= (double)hex->scale;
    dest_y *= (double)hex->scale;
    dest_x += (double)center.x;
    dest_y += (double)center.y;
    dest_x += (double)hex->hex_point.x;
    dest_y += (double)hex->hex_point.y;

    SDL_Rect dest = {
        .x = dest_x,
        .y = dest_y,
        .w = HEX_WIDTH * hex->scale,
        .h = HEX_HEIGHT * hex->scale,
    };

    // Center point is relative to dest, so we have to account for scaling factor here too.
    center.x *= hex->scale;
    center.y *= hex->scale;

    SDL_SetTextureAlphaMod(_graphics.hex_basic_texture, hex->alpha * 255.0f);
    SDL_RenderCopyEx(
        window_renderer(),
        _graphics.hex_basic_texture,
        &src,
        &dest,
        hex->rotation_angle,
        &center,
        SDL_FLIP_NONE);
    SDL_SetTextureAlphaMod(_graphics.hex_basic_texture, 255);
}

void draw_static_hex(const Hex* hex) {
    if (!hex->is_valid) {
        return;
    }

    SDL_Rect src = {
        .x = hex->type * HEX_SOURCE_WIDTH,
        .y = 0,
        .w = HEX_SOURCE_WIDTH,
        .h = HEX_SOURCE_HEIGHT,
    };

    SDL_Rect dest = {
        .x = hex->hex_point.x,
        .y = hex->hex_point.y,
        .w = HEX_WIDTH,
        .h = HEX_HEIGHT,
    };

    if (0 != SDL_RenderCopy(window_renderer(), _graphics.hex_basic_texture, &src, &dest)) {
        SDL_Log("SDL_RenderCopy error %s", SDL_GetError());
    }
}

void graphics_update(void) {
    SDL_SetRenderDrawColor(window_renderer(), 0x44, 0x44, 0x44, 0xFF);
    SDL_RenderClear(window_renderer());

    SDL_SetRenderDrawColor(window_renderer(), 0x11, 0x11, 0x11, 0xFF);
    SDL_Rect board_rect = {
        .x = g_constants.board.x,
        .y = g_constants.board.y,
        .w = g_constants.board_width,
        .h = g_constants.board_height
    };
    SDL_RenderFillRect(window_renderer(), &board_rect);

    const RotationAnimation* rotation_animation = &g_state.game.rotation_animation;
    bool cursor_active =
        hex_all_stationary_no_animation() || rotation_animation->in_progress;
    bool drawn[HEX_NUM_COLUMNS][HEX_NUM_ROWS] = {0};
    bool in_cursor[HEX_NUM_COLUMNS][HEX_NUM_ROWS] = {0};

    // Mark hexes that are under the cursor
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            in_cursor[q][r] =
                cursor_active &&
                cursor_contains_hex(&g_state.cursor, (HexCoord){q,r});
        }
    }

    // Non-animated/static hexes, non-cursor
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            const Hex* hex = hex_at(q,r);
            if (hex->is_stationary && !hex_is_animating(hex) && !in_cursor[q][r]) {
                draw_static_hex(hex);
                drawn[q][r] = true;
            }
        }
    }

    // Rotation animation, non-cursor
    if (rotation_animation->in_progress) {
        for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
            for (int r = 0; r < HEX_NUM_ROWS; r++) {
                const Hex* hex = hex_at(q,r);
                if (!drawn[q][r] && hex->is_rotating && !in_cursor[q][r]) {
                    draw_animated_hex(hex, rotation_animation->rotation_center, false);
                    drawn[q][r] = true;
                }
            }
        }
    }

    // Cursor highlights
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            if (in_cursor[q][r]) {
                const Hex* hex = hex_at(q,r);
                if (!hex->is_rotating) {
                    Point middle = {
                        hex->hex_point.x + HEX_WIDTH / 2,
                        hex->hex_point.y + HEX_HEIGHT / 2,
                    };
                    draw_hex(middle, HEX_RADIUS + 6, white);
                }
            }
        }
    }

    // Non-animated/static hexes, cursor
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            const Hex* hex = hex_at(q,r);
            if (hex->is_stationary && !hex_is_animating(hex) && in_cursor[q][r]) {
                draw_static_hex(hex);
                drawn[q][r] = true;
            }
        }
    }

    // Rotation animation, cursor
    if (rotation_animation->in_progress) {
        for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
            for (int r = 0; r < HEX_NUM_ROWS; r++) {
                const Hex* hex = hex_at(q,r);
                if (!drawn[q][r] && hex->is_rotating && in_cursor[q][r]) {
                    draw_animated_hex(hex, rotation_animation->rotation_center, true);
                    drawn[q][r] = true;
                }
            }
        }
    }

    // Hexes with cluster match animations
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            const Hex* hex = hex_at(q,r);
            if (!drawn[q][r] && hex->cluster_match_animation.in_progress) {
                Point center = {
                    .x = hex->hex_point.x + (HEX_WIDTH / 2),
                    .y = hex->hex_point.y + (HEX_HEIGHT / 2),
                };
                draw_animated_hex(hex, center, false);
                drawn[q][r] = true;
            }
        }
    }

    // Hexes with flower match animations
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            const Hex* hex = hex_at(q,r);
            if (!drawn[q][r] && hex->flower_match_animation.in_progress) {
                draw_animated_hex(hex, hex->flower_match_animation.flower_center, false);
                drawn[q][r] = true;
            }
        }
    }

    // All remaining hexes (should just be the ones falling)
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            const Hex* hex = hex_at(q,r);
            if (!drawn[q][r]) {
                draw_static_hex(hex);
                drawn[q][r] = true;
            }
        }
    }

    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            ASSERT(drawn[q][r]);
        }
    }

    if (cursor_active) {
        // Draw cursor
        draw_circle(g_state.cursor.screen_point, CURSOR_RADIUS+3, white);
        draw_circle(g_state.cursor.screen_point, CURSOR_RADIUS, black);
        draw_circle(g_state.cursor.screen_point, CURSOR_RADIUS-1, darkorchid);
    }

    // Local score animations
    LocalScoreAnimation* lsas = (LocalScoreAnimation*)vector_data_at(g_state.game.local_score_animations, 0);
    for (size_t i = 0; i < vector_size(g_state.game.local_score_animations); i++) {
        LocalScoreAnimation* lsa = &lsas[i];
        text_set_font(&lsa->text, _graphics.local_score_font);
        text_set_point(&lsa->text, lsa->current_point.x, lsa->current_point.y);
        text_set_color(&lsa->text, 0xFF, 0xFF, 0xFF, (int)(255.0f * lsa->alpha));
        snprintf(text_buffer(&lsa->text), TEXT_MAX_LEN, "%u", lsa->score);
        text_draw(&lsa->text);
    }

    snprintf(text_buffer(&_graphics.level_text), TEXT_MAX_LEN, "Level: %u", g_state.game.level);
    text_draw(&_graphics.level_text);

    snprintf(text_buffer(&_graphics.combos_text), TEXT_MAX_LEN, "Combos remaining: %u", g_state.game.combos_remaining);
    text_draw(&_graphics.combos_text);

    snprintf(text_buffer(&_graphics.score_text), TEXT_MAX_LEN, "Score: %u", g_state.game.score);
    text_draw(&_graphics.score_text);

#ifdef DISPLAY_HEX_COORDS
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            if (hex_coord_is_valid((HexCoord){q,r})) {
                text_draw(&_graphics.hex_coord_text[q][r]);
            }
        }
    }
#endif

    // Statistics rendering
    Text* fps_text = &_graphics.fps_text;
    Text* update_text = &_graphics.update_text;

    uint32_t frames = g_state.frame_count;
    if (frames > 0 && frames % 60 == 0) {
        snprintf(text_buffer(fps_text), TEXT_MAX_LEN, "FPS: %3.1f", statistics_fps());
        snprintf(text_buffer(update_text), TEXT_MAX_LEN, "Upd: %3.1f", statistics_get()->update_ave_ns / 1000000.0f);
    }
    text_draw(fps_text);
    text_draw(update_text);
}

void graphics_flip(void) {
    SDL_RenderPresent(window_renderer());
}
