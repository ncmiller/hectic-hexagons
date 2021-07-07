#include "window.h"
#include "game_state.h"
#include "time_now.h"

#define RETURN_IF_FALSE(x) if ((x) == false) { return 1; }
#define CLOSE_AND_RETURN_IF_FALSE(x) if ((x) == false) { window_close(); return 1; }

// All game state data is stored in here
GameState g_state = {0};

int main(int argc, char* argv[]) {
    RETURN_IF_FALSE(window_init());
    RETURN_IF_FALSE(window_create());

    CLOSE_AND_RETURN_IF_FALSE(input_init());
    CLOSE_AND_RETURN_IF_FALSE(game_init());
    CLOSE_AND_RETURN_IF_FALSE(graphics_init());

    g_state.running = true;
    while (g_state.running) {
        uint64_t start = now_ms();
        input_update();
        game_update();
        graphics_update();
        uint64_t update_diff = now_ms() - start;
        graphics_flip();
        uint64_t render_diff = now_ms() - start;

        Statistics* stats = &g_state.statistics;
        float smoothing = 0.9f;
        stats->update_ave_ms = (stats->update_ave_ms * smoothing) + ((float)update_diff * (1.0f - smoothing));
        stats->render_ave_ms = (stats->render_ave_ms * smoothing) + ((float)render_diff * (1.0f - smoothing));
        stats->total_frames++;
    }

    window_close();
    return 0;
}
