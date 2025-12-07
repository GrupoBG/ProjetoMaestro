#ifndef EFFECTS_H
#define EFFECTS_H

#include <SDL2/SDL.h>
//#include <SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>


/*      Definicao de efeito     */

// enm com todos os tipos de efeito
typedef enum{
        CLICK_PERFEITO,
        CLICK_BOM,
        CLICK_RUIM,
        CLICK_EXPIRADO,
        CLICK_ERROU,


        EFFECT_NUMBER
} Effect_type;

// Caracteristicas de cada instancia criada por um evento mesmo
// ex: particulas de fumaca
typedef struct{
        int x, y;

        int starting_tick;
        int remaining_ticks;
        int base_ticks;

} Effect_display;

// Efeito com textura
typedef struct{
        Effect_type tag;

        int w, h;
        SDL_Texture* img;

        Effect_display* display_vector;
        int size;

} Effect;

// Cria efeito baseado em array de template de efeitos
void create_effect(Effect* effect_array, int* effect_array_begin, int* effect_array_end, Effect* effect_templates, int effect_template,
                int x, int y, int* now, int delay, int spawn_time_variation, int base_ticks);

// Destroi efeito
void destroy_effect(Effect* effect_array, int pos);



#endif
