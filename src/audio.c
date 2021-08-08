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
    Mix_Chunk* cursor_move;
    Mix_Chunk* starflower;
    uint8_t music_volume;
} Audio;

static Audio _audio = {0};

static Mix_Chunk* effect_to_mix_chunk(AudioSoundEffect effect) {
    if (effect == AUDIO_MOVE_CURSOR) {
        return _audio.cursor_move;
    } else if (effect == AUDIO_STARFLOWER) {
        return _audio.starflower;
    } else {
        return NULL;
    }
}

static bool load_all_sounds(void) {
    bool all_loaded = true;

    _audio.music = Mix_LoadMUS("assets/sounds/music.ogg");
    if (_audio.music == NULL) {
        SDL_Log("Failed to load music, error: %s", Mix_GetError());
    }
    all_loaded &= (_audio.music != NULL);

    _audio.cursor_move = Mix_LoadWAV("assets/sounds/cursor_move.wav");
    all_loaded &= (_audio.cursor_move != NULL);

    _audio.starflower = Mix_LoadWAV("assets/sounds/starflower.wav");
    all_loaded &= (_audio.starflower != NULL);

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

void audio_play_sound_effect(AudioSoundEffect effect) {
    if (effect >= AUDIO_NUM_SOUND_EFFECTS) {
        SDL_Log("Error, invalid sound effect: %d", effect);
        return;
    }

    Mix_PlayChannel(AUDIO_ANY_CHANNEL, effect_to_mix_chunk(effect), AUDIO_ONE_SHOT);
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
