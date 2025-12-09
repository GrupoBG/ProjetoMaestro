#ifndef AUDIO_H
#define AUDIO_H

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


/*      Definicao de audio      */
typedef struct {
    Mix_Music* background_music;
    Mix_Chunk* hit_sound;
} AudioResources;

extern int global_volume;

// Inicia audio
AudioResources* init_audio();

// Função para liberar recursos de audio
void cleanup_audio(AudioResources* audio);

// Função para atualizar volumes de áudio baseado na variável global
void update_audio_volumes(Mix_Chunk* hit_sound);



#endif
