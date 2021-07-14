#include "window.h"
#include "game_state.h"
#include "time_now.h"
#include "constants.h"
#include "statistics.h"
#include "graphics.h"
#include "bump_allocator.h"

#define RETURN_IF_FALSE(x) if ((x) == false) { return 1; }
#define CLOSE_AND_RETURN_IF_FALSE(x) if ((x) == false) { window_close(); return 1; }

// All game state data is stored in here
GameState g_state = {0};

int main(int argc, char* argv[]) {
    bump_allocator_init(g_state.temporary_allocations, sizeof(g_state.temporary_allocations));
    RETURN_IF_FALSE(window_init());
    RETURN_IF_FALSE(window_create());

    CLOSE_AND_RETURN_IF_FALSE(constants_init());
    CLOSE_AND_RETURN_IF_FALSE(input_init());
    CLOSE_AND_RETURN_IF_FALSE(game_init());
    CLOSE_AND_RETURN_IF_FALSE(graphics_init());

    g_state.running = true;
    while (g_state.running) {
        uint64_t start = now_ns();
        input_update();
        game_update();
        graphics_update();
        uint64_t update_diff = now_ns() - start;
        graphics_flip();
        uint64_t render_diff = now_ns() - start;

        statistics_update(update_diff, render_diff);
        bump_allocator_free_all();
    }

    window_close();
    return 0;
}
