#include "audio.h"
#include <SDL_mixer.h>
#include <SDL.h>

#define AUDIO_DEFAULT_VOLUME (MIX_MAX_VOLUME / 2)
#define AUDIO_DEFAULT_NUM_CHANNELS 16
#define AUDIO_ANY_CHANNEL -1
#define AUDIO_LOOP -1
#define AUDIO_ONE_SHOT 0

typedef struct {
    Mix_Music* music;
    Mix_Chunk* sound_effect;
    uint8_t music_volume;
} Audio;

static Audio _audio = {0};

static bool load_all_sounds(void) {
    bool all_loaded = true;

    _audio.music = Mix_LoadMUS("assets/sounds/music.ogg");
    if (_audio.music == NULL) {
        SDL_Log("Failed to load music, error: %s", Mix_GetError());
    }
    all_loaded &= (_audio.music != NULL);
    _audio.sound_effect = Mix_LoadWAV("assets/sounds/sound_effect.wav");
    all_loaded &= (_audio.sound_effect != NULL);

    if (!all_loaded) {
        SDL_Log("Failed to load audio. Exiting.");
        return false;
    }
    return true;;
}

bool audio_init(void) {
    _audio.music_volume = AUDIO_DEFAULT_VOLUME;
    Mix_AllocateChannels(AUDIO_DEFAULT_NUM_CHANNELS);
    Mix_VolumeMusic(_audio.music_volume);
    Mix_Volume(AUDIO_ANY_CHANNEL, _audio.music_volume);
    return load_all_sounds();
}

void audio_play_sound_effect(void) {
    Mix_PlayChannel(AUDIO_ANY_CHANNEL, _audio.sound_effect, AUDIO_ONE_SHOT);
}

void audio_play_pause_music(void) {
    if (!Mix_PlayingMusic()) {
        Mix_PlayMusic(_audio.music, AUDIO_LOOP);
    } else {
        if (Mix_PausedMusic()) {
            Mix_ResumeMusic();
        } else {
            Mix_PauseMusic();
        }
    }
}
