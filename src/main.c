#include "window.h"
#include "game_state.h"
#include "time_utils.h"
#include "constants.h"
#include "statistics.h"
#include "graphics.h"
#include "audio.h"
#include "bump_allocator.h"
#ifdef IS_WASM_BUILD
#include <emscripten.h>
#endif

#define RETURN_IF_FALSE(x) if ((x) == false) { return 1; }
#define CLOSE_AND_RETURN_IF_FALSE(x) if ((x) == false) { window_close(); return 1; }

// All game state data is stored in here
GameState g_state = {0};

static void loop(void* arg) {
    static uint64_t prev_start = 0;
    uint64_t start = now_ns();

    uint64_t loop_iter_diff = start - prev_start;
    input_update();
    bool game_updated = game_update();
    graphics_update();
    uint64_t update_diff = now_ns() - start;
    graphics_flip();
    uint64_t render_diff = now_ns() - start;

    if (g_state.frame_count != 0) {
        statistics_update(update_diff, render_diff, loop_iter_diff);
    }
    prev_start = start;

    bump_allocator_free_all();

    if (game_updated) {
        g_state.frame_count++;
    }
}

int main(int argc, char* argv[]) {
    bump_allocator_init(g_state.temporary_allocations, sizeof(g_state.temporary_allocations));
    RETURN_IF_FALSE(window_init());
    RETURN_IF_FALSE(window_create());

    CLOSE_AND_RETURN_IF_FALSE(constants_init());
    CLOSE_AND_RETURN_IF_FALSE(input_init());
    CLOSE_AND_RETURN_IF_FALSE(game_init());
    CLOSE_AND_RETURN_IF_FALSE(graphics_init());
    CLOSE_AND_RETURN_IF_FALSE(audio_init());
    audio_play_pause_music();

#ifdef IS_WASM_BUILD
    const int simulate_infinite_loop = 1;
    const int fps = 60;
    emscripten_set_main_loop_arg(loop, NULL, fps, simulate_infinite_loop);
#else
    g_state.running = true;
    while (g_state.running) {
        loop(NULL);
    }
#endif

    window_close();
    return 0;
}
