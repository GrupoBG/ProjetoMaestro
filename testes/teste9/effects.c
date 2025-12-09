#include <SDL2/SDL.h>
//#include <SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>

#include "effects.h"

void create_effect(Effect* effect_array, int* effect_array_begin, int* effect_array_end, Effect* effect_templates, int effect_template,
                int x, int y, int* now, int delay, int spawn_time_variation, int base_ticks){

        if( (( (*effect_array_end) +1)% EFFECTS_ARRAY_SIZE) != (*effect_array_begin) ){
                effect_array[*effect_array_end] = effect_templates[effect_template];

                effect_array[*effect_array_end].display_vector = (Effect_display*) malloc(effect_array[*effect_array_end].size*(sizeof(Effect_display)));

                int next_spawn = (*now) + delay;

                for (int i = 0; i < effect_array[*effect_array_end].size; i++){
                        effect_array[*effect_array_end].display_vector[i].x = x - (effect_array[*effect_array_end].w/2);
                        effect_array[*effect_array_end].display_vector[i].y = y - (effect_array[*effect_array_end].h/2);
                        effect_array[*effect_array_end].display_vector[i].starting_tick = next_spawn;
                        effect_array[*effect_array_end].display_vector[i].remaining_ticks = base_ticks;
                        effect_array[*effect_array_end].display_vector[i].base_ticks = base_ticks;

                        next_spawn += spawn_time_variation;
                }


                ++(*effect_array_end);
                (*effect_array_end) %= EFFECTS_ARRAY_SIZE;
        }
        else{
                printf("Erro! Muitos efeitos na tela\n");
        }

}

void destroy_effect(Effect* effects_array, int pos){
        assert(effects_array[pos].display_vector);
        free(effects_array[pos].display_vector);
        effects_array[pos].display_vector = NULL;
}
