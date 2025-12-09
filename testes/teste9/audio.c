#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>

#include "audio.h"

#define SOUND_PATH "osu-hit-sound.ogg"
#define MUSIC_PATH "Bach 07 - Overture No. 1 - Passepied.ogg"


// Variável global de volume (0-10, representando 0-100%)
int global_volume = 5; // Volume padrão em 50%


AudioResources* init_audio() {
    AudioResources* audio = malloc(sizeof(AudioResources));
    if (!audio) {
        printf("Erro ao alocar memoria para recursos de audio\n");
        return NULL;
    }

    audio->background_music = NULL;
    audio->hit_sound = NULL;

    // Não inicializa Mix_OpenAudio aqui - já foi inicializado em main()

    // Reduzir canais para melhor performance
    Mix_AllocateChannels(8);

    // Carregar efeitos sonoros
    audio->hit_sound = Mix_LoadWAV(SOUND_PATH);
    if (!audio->hit_sound) {
        printf("Erro ao carregar som: %s\n", Mix_GetError());
        return NULL;
    }

    // Ajustar volumes baseado na variável global de volume
    int mixer_volume = (global_volume * SDL_MIX_MAXVOLUME) / 10;
    Mix_Volume(-1, mixer_volume);
    Mix_VolumeChunk(audio->hit_sound, mixer_volume);

    audio->background_music = Mix_LoadMUS(MUSIC_PATH);
    if (!audio->background_music) {
        printf("Erro ao carregar musica: %s\n", Mix_GetError());
        Mix_FreeChunk(audio->hit_sound);
        return NULL;
    }

    int music_volume = (global_volume * SDL_MIX_MAXVOLUME) / 15;
    Mix_VolumeMusic(music_volume);

    return audio;
}


void cleanup_audio(AudioResources* audio) {
    if (audio) {
        if (audio->hit_sound) {
            Mix_FreeChunk(audio->hit_sound);
        }
        if (audio->background_music) {
            Mix_HaltMusic();
            Mix_FreeMusic(audio->background_music);
        }
        free(audio);
    }
}

// Função para atualizar volumes de áudio baseado na variável global
void update_audio_volumes(Mix_Chunk* hit_sound) {
    // Calcula o volume baseado na variável global (0-10 -> 0-MIX_MAX_VOLUME)
    int mixer_volume = (global_volume * SDL_MIX_MAXVOLUME) / 10;

    // Apenas atualiza volume, sem reinicializar
    Mix_Volume(-1, mixer_volume);

    // Garante que hit_sound tem o mesmo volume (se fornecido)
    if (hit_sound) {
        Mix_VolumeChunk(hit_sound, mixer_volume);
    }

    // Atualiza volume da musica de fundo
    int music_volume = (global_volume * SDL_MIX_MAXVOLUME) / 15;
    Mix_VolumeMusic(music_volume);

    printf("Volumes atualizados para: %d%% (hit_sound), %d%% (música)\n",
           (mixer_volume * 100) / SDL_MIX_MAXVOLUME,
           (music_volume * 100) / SDL_MIX_MAXVOLUME);
}

