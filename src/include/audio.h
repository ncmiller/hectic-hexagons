#include <stdbool.h>

typedef enum {
    AUDIO_MOVE_CURSOR,
    AUDIO_STARFLOWER,
    AUDIO_NUM_SOUND_EFFECTS,
} AudioSoundEffect;

bool audio_init(void);
void audio_play_sound_effect(AudioSoundEffect);
void audio_play_pause_music(void);
