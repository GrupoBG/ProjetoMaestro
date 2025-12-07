#include <SDL2/SDL.h>
//#include <SDL_mixer.h>
#include <SDL2/SDL2_gfxPriitives.h>
#include <SDL2/SDL_ttf.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include <math.h>

#include "notes.h"

// Gera nota aleatoria
void generate_random_note(Note* note_template, Note* note) {

        /*
         *
         *      No momento, testando sem imagens
         *
         */
        note->img_number = 0;


        // Seta valores referentes a nota como um todo
        int new_note_type = rand() % NOTE_NUMBER;
        (*note) = note_template[new_note_type];

        note->display_vector = NULL;
        note->display_vector = (Note_display*) malloc( (note->size)* sizeof(Note_display));

        int note_partition = rand() % total_partitions;
        int starting_tick = 15 + (rand() % 46);     // Circulo inicia entre [0,25s;1s]
        int base_ticks = 120 + (rand() % 51); // Intervalo em ticks de existencia do circulo
        int remaining_ticks = base_ticks;

        SDL_Color color = { rand() % 256, rand() % 256 , rand() % 256, 255};

        switch(note->type){
                case CLICK:
                        note->display_vector[0].c.radius = rand() % 51 + 20;
                        break;
                case MULTI_CLICK:

                        color.r = color.g = color.b = color.a = 255;

                        note->display_vector[0].c.radius = rand() % 71 + 20;
                        break;
                case STRAIGHT_DRAG:
                        // Cria circulo inicial
                        note->display_vector[0].c.radius = rand() % 51 + 20;
                        // Cria circulo final
                        note->display_vector[2].c.radius = note->display_vector[0].c.radius;
                        break;
        }

        // Seta valores de cada parte da nota
        for(int i = 0; i < note->size; i++){

                // Escolhe particao
                note->display_vector[i].partition = note_partition;

                // Escolhe tempo
                note->display_vector[i].starting_tick = starting_tick;
                note->display_vector[i].base_ticks = base_ticks;
                note->display_vector[i].remaining_ticks = remaining_ticks;


                // Escolhe coordenadas
                note->display_vector[i].x = note->display_vector[i].c.radius + (rand() % (PARTITION_WIDTH - 2*note->display_vector[i].c.radius)) + ( note->display_vector[i].partition* PARTITION_WIDTH);
                note->display_vector[i].y = note->display_vector[i].c.radius + (rand() % (WINDOW_HEIGHT - 2*note->display_vector[i].c.radius));

                // Copia coordenadas como as iniciais
                note->display_vector[i].base_x = note->display_vector[i].x;
                note->display_vector[i].base_y = note->display_vector[i].y;


                // Escolhe cor
                note->display_vector[i].color.r = color.r;
                note->display_vector[i].color.g = color.g;
                note->display_vector[i].color.b = color.b;
                note->display_vector[i].color.a = 255;

        }

        double dx, dy;
        switch(note->type){
                case STRAIGHT_DRAG:
                        // Cria linha reta entre os dois circulos
                        note->display_vector[1].x = note->display_vector[0].x;
                        note->display_vector[1].y = note->display_vector[0].y;
                        note->display_vector[1].s.x2 = note->display_vector[2].x;
                        note->display_vector[1].s.y2 = note->display_vector[2].y;

                        // Seta ponto inicial da reta
                        note->display_vector[1].base_x = note->display_vector[1].x;
                        note->display_vector[1].base_y = note->display_vector[1].y;

                        // Coloca alpha fixo para reta suporte
                        note->display_vector[1].color.a = 0x44;

                        // Coloca alpha fixo para circulo suporte
                        note->display_vector[2].color.a = 0xFF;
                        break;
/*
                case ARC_DRAG:
                        // Pega x e y intermediario entre inicio e fim
                        note->display_vector[1].x =  (int) ( (double) (note->display_vector[0].x + note->display_vector[2].x)/2.0);
                        note->display_vector[1].y =  (int) ( (double) (note->display_vector[0].y + note->display_vector[2].y)/2.0);

                        // Pega o raio
                        dx = note->display_vector[1].x - note->display_vector[2].x;
                        dy = note->display_vector[1].y - note->display_vector[2].y;

                        note->display_vector[1].a.radius = sqrt(dx * dx + dy * dy);

                        // Pega a angulatura dos pontos
                        double angle1 = atan2(note->display_vector[0].y - note->display_vector[1].y, note->display_vector[0].x - note->display_vector[1].x) * (180.0 / M_PI);
                        double angle2 = atan2(note->display_vector[0].y - note->display_vector[1].y, note->display_vector[0].x - note->display_vector[1].x) * (180.0 / M_PI);

                        angle1 = fmod((angle1 + 90.0 + 360.0), 360.0);
                        angle2 = fmod((angle2 + 90.0 + 360.0), 360.0);


                        // Define se arco vai pela direita ou esquerda
                        int arc_going_right = rand() %2;
                        if(arc_going_right){
                                note->display_vector[1].a.start = fmax(angle1, angle2);
                                note->display_vector[1].a.end = fmin(angle1, angle2);
                        }
                        else{
                                note->display_vector[1].a.start = fmin(angle1, angle2);
                                note->display_vector[1].a.end = fmax(angle1, angle2);
                        }


                        break;
                case ROUND_DRAG:
                        // Pega x e y intermediario entre inicio e fim
                        note->display_vector[1].x = (int) ( (double) (note->display_vector[0].x + note->display_vector[2].x)/2.0);
                        note->display_vector[1].y = (int) ( (double) (note->display_vector[0].y + note->display_vector[2].y)/2.0);

                        // Conserta posicao final
                        note->display_vector[2].x = note->display_vector[0].x;
                        note->display_vector[2].y = note->display_vector[0].y;

                        // Pega o raio
                        dx = note->display_vector[1].x - note->display_vector[2].x;
                        dy = note->display_vector[1].y - note->display_vector[2].y;

                        note->display_vector[1].a.radius = sqrt(dx * dx + dy * dy);
                        break;
*/
        }

}

// Checa colisao de circulo
//
// retorna qualidade do clique
// 0 - perfeito
// 1
//
// INT_MIN -> a
int check_collision_note(int* x, int* y, int* pos) {

        SDL_GetMouseState(x, y);

        if(     (*x) == INT_MIN
                        ||      (*y) == INT_MIN ){
                printf("Erro! Falha ao obter informacoes do mouse!\n");
                return INT_MIN;
        }

        int click_result = CLICK_ERROU;


        for(int i = ((note_array_end-1 + NOTE_ARRAY_SIZE)%NOTE_ARRAY_SIZE);
                        i != ((note_array_begin-1 + NOTE_ARRAY_SIZE )%NOTE_ARRAY_SIZE);
                        i=((i-1+ NOTE_ARRAY_SIZE)%NOTE_ARRAY_SIZE)){

                int dx, dy;
                dx = dy = INT_MIN;
                switch(note_array[i].type){
                        case CLICK:
                                /*
                                 *
                                 *      So funciona com circulos por enquanto
                                 *
                                 *
                                 */
                                dx = (*x) - note_array[i].display_vector[0].x;
                                dy = (*y) - note_array[i].display_vector[0].y;

                                if(     ( (dx * dx + dy * dy) <= (note_array[i].display_vector[0].c.radius * note_array[i].display_vector[0].c.radius) )
                                                && (note_array[i].display_vector[0].remaining_ticks >= 0) ){

                                        int click_timing = abs( (note_array[i].display_vector[0].base_ticks/2)-note_array[i].display_vector[0].remaining_ticks );

                                        note_array[i].display_vector[0].remaining_ticks = -1;   // Remove circulo
                                        note_array[i].is_finished = true;

                                        if ( click_timing < (note_array[i].display_vector[0].base_ticks/20) ){  // 10% do tempo
                                                click_result = CLICK_PERFEITO;
                                        }

                                        else if ( click_timing < (note_array[i].display_vector[0].base_ticks/5) ){ // (40-10)% do tempo
                                                click_result = CLICK_BOM;
                                        }

                                        else{
                                                click_result = CLICK_RUIM;
                                        }

                                        (*x) = note_array[i].display_vector[0].x;
                                        (*y) = note_array[i].display_vector[0].y;

                                        (*pos) = i;

                                        goto RETORNA_CLIQUE;
                                }
                                break;
                        case MULTI_CLICK:
                                dx = (*x) - note_array[i].display_vector[0].x;
                                dy = (*y) - note_array[i].display_vector[0].y;

                                if(     ( (dx * dx + dy * dy) <= (note_array[i].display_vector[0].c.radius * note_array[i].display_vector[0].c.radius) )
                                                && (note_array[i].display_vector[0].remaining_ticks >= 0) ){

                                        int click_timing = abs( (note_array[i].display_vector[0].base_ticks/2)-note_array[i].display_vector[0].remaining_ticks );

                                        // note_array[i].display_vector[0].remaining_ticks = -1;     // Remove circulo

                                        if ( click_timing < (note_array[i].display_vector[0].base_ticks/20) ){    // 10% do tempo
                                                click_result = CLICK_PERFEITO;
                                        }

                                        else if ( click_timing < (note_array[i].display_vector[0].base_ticks/5) ){ // (40-10)% do tempo
                                                click_result = CLICK_BOM;
                                        }

                                        else{
                                                click_result = CLICK_RUIM;
                                        }

                                        (*x) = note_array[i].display_vector[0].x;
                                        (*y) = note_array[i].display_vector[0].y;

                                        (*pos) = i;

                                        goto RETORNA_CLIQUE;
                                }
                                break;
                        case STRAIGHT_DRAG:

                                dx = (*x) - note_array[i].display_vector[0].x;
                                dy = (*y) - note_array[i].display_vector[0].y;

                                if(     ( (dx * dx + dy * dy) <= (note_array[i].display_vector[0].c.radius * note_array[i].display_vector[0].c.radius) )
                                                && (note_array[i].display_vector[0].remaining_ticks >= 0) ){

                                        (*x) = note_array[i].display_vector[0].x;
                                        (*y) = note_array[i].display_vector[0].y;

                                        (*pos) = i;

                                        click_result = -1;
                                        goto RETORNA_CLIQUE;
                                }
                                break;
/*
                        case ARC_DRAG:
                                dx = (*x) - note_array[i].display_vector[0].x;
                                dy = (*y) - note_array[i].display_vector[0].y;

                                if(     ( (dx * dx + dy * dy) <= (note_array[i].display_vector[0].c.radius * note_array[i].display_vector[0].c.radius) )
                                                && (note_array[i].display_vector[0].remaining_ticks >= 0) ){

                                        (*x) = note_array[i].display_vector[0].x;
                                        (*y) = note_array[i].display_vector[0].y;

                                        (*pos) = i;

                                        click_result = -1;
                                        goto RETORNA_CLIQUE;
                                }
                                break;
                        case ROUND_DRAG:
                                dx = (*x) - note_array[i].display_vector[0].x;
                                dy = (*y) - note_array[i].display_vector[0].y;

                                if(     ( (dx * dx + dy * dy) <= (note_array[i].display_vector[0].c.radius * note_array[i].display_vector[0].c.radius) )
                                                && (note_array[i].display_vector[0].remaining_ticks >= 0) ){

                                        (*x) = note_array[i].display_vector[0].x;
                                        (*y) = note_array[i].display_vector[0].y;

                                        (*pos) = i;

                                        click_result = -1;
                                        goto RETORNA_CLIQUE;
                                }
                                break;
*/
                }

        }

RETORNA_CLIQUE:
        //      printf("Tipo da nota (posicao: %d) clicada: %d\n", (*pos), note_array[*pos].type);
        return click_result;
}

int check_drag_note(Note* note, int* x, int* y){

        SDL_GetMouseState(x, y);

        double closest_coordinate_inline_x, closest_coordinate_inline_y;
        double mouse_relative_x, mouse_relative_y;
        double dx, dy;

        double position_inline;

        int drag_result;
        switch (note->type){
                case STRAIGHT_DRAG:
                        mouse_relative_x = (*x) - note->display_vector[1].x;
                        mouse_relative_y = (*y) - note->display_vector[1].base_y;

                        dx = note->display_vector[1].s.x2 - note->display_vector[1].x;
                        dy = note->display_vector[1].s.y2 - note->display_vector[1].y;

                        double line_length_squared = dx * dx + dy * dy;

                        // Calcula proporcao da linha navegada
                        position_inline = (mouse_relative_x * dx + mouse_relative_y * dy)/line_length_squared;


                        // Mantem circulo na linha
                        position_inline = fmax(0.0, fmin(1.0, position_inline));

                        closest_coordinate_inline_x = ( (double) note->display_vector[1].x + position_inline * dx);
                        closest_coordinate_inline_y = ( (double) note->display_vector[1].y + position_inline * dy);

                        // Passa coordenada achada para o circulo
                        note->display_vector[0].x = closest_coordinate_inline_x;
                        note->display_vector[0].y = closest_coordinate_inline_y;

                        //      printf("x: %d | mousex %d\ny: %d | mousey: %d\nx1: %d\n", note->display_vector[0].x, *x, note->display_vector[0].y, *y, note->display_vector[1].x);

                        drag_result = INT_MIN;

                        // Checa se o mouse NAO esta dentro do cilindro formado pelos dois circulos
                        if ( ( (((double) (*x) - closest_coordinate_inline_x) *  ( (double) (*x) - closest_coordinate_inline_x) + ( (double) (*y) - closest_coordinate_inline_y) * ( (double) (*y) - closest_coordinate_inline_y)) > (note->display_vector[0].c.radius * note->display_vector[0].c.radius ) )
                           ){
                                // Se o mouse nao estiver dentro desta area, falha o drag
                                note->is_finished = true;
                                for(int i = 0; i < note->size; i++){
                                        note->display_vector[i].remaining_ticks = -1;     // Remove nota
                                }
                                drag_result = CLICK_ERROU;

                                goto RETORNA_ARRASTO;
                        }

                        /*      Checa se chegou no outro circulo        */
                        if(     (note->display_vector[0].x == note->display_vector[2].x)
                                        &&      (note->display_vector[0].y == note->display_vector[2].y)){

                                int drag_timing = abs( (note->display_vector[2].base_ticks/2)-note->display_vector[2].remaining_ticks );

                                note->is_finished = true;
                                for(int i = 0; i < note->size; i++){
                                        note->display_vector[i].remaining_ticks = -1;     // Remove nota
                                }

                                if ( drag_timing < (note->display_vector[2].base_ticks/10) ){    // 20% do tempo
                                        drag_result = CLICK_PERFEITO;
                                }

                                else if ( drag_timing < (note->display_vector[2].base_ticks/3) ){ // (66.6-20)% do tempo
                                        drag_result = CLICK_BOM;
                                }

                                else{
                                        drag_result = CLICK_RUIM;
                                }
                        }
/*
                case ARC_DRAG:
                        mouse_relative_x = (*x) - note->display_vector[0].x;
                        mouse_relative_y = (*y) - note->display_vector[0].base_y;


                        *
                        *
                        *       Mexendo aqui
                        *
                        *
                        *

                        break;
                case ROUND_DRAG:
                        break;
*/
        }

RETORNA_ARRASTO:
        return drag_result;
}

/*
// Enche array com circulos validos aleatorios
void fill_note_array(Note* array, int array_size){
for (int i = 0; i < array_size; i++){
generate_random_note(&array[i]);
}
}
*/


void initiate_note(Note* note){
        // Coloca tamanho do display da nota
        switch(note->type){
                case CLICK:
                case MULTI_CLICK:
                        note->size = 1;
                        break;
                case STRAIGHT_DRAG:
                        note->size = 3;
                        break;
/*
                case ARC_DRAG:
                        note->size = 3;
                        break;
                case ROUND_DRAG:
                        note->size = 3;
                        break;
*/
        }

        // Aloca vetor de display
        note->is_finished = false;
        note->display_vector = (Note_display*) malloc( (note->size)* sizeof(Note_display));

        // Aloca imagens a serem usadas
        switch(note->type){
                case CLICK:
                case MULTI_CLICK:
                case STRAIGHT_DRAG:
/*
                case ARC_DRAG:
                case ROUND_DRAG:
*/
                        for(int i = 0; i < note->size; i++){
                                note->display_vector[i].img = NULL;
                        }
                        break;
        }

        // Anula vetor de display
        for(int i = 0; i < note->size; i++){
                note->display_vector[i].x = note->display_vector[i].y =
                        note->display_vector[i].partition = note->display_vector[i].remaining_ticks = note->display_vector[i].base_ticks = note->display_vector[i].starting_tick = -1;

                note->display_vector[i].color.r = note->display_vector[i].color.g = note->display_vector[i].color.b = note->display_vector[i].color.a = 0x00;

                note->display_vector[i].type = NOTE_DISPLAY_NUMBER; // nota nula = nota maxima
        }

        // Insere os tipos de display presentes
        switch(note->type){
                case CLICK:
                        note->display_vector[0].type = CIRCLE;
                        break;
                case MULTI_CLICK:
                        note->display_vector[0].type = CIRCLE;
                        break;
                case STRAIGHT_DRAG:
                        note->display_vector[0].type = CIRCLE;  // Circulo inicial
                        note->display_vector[1].type = STRAIGHT_LINE;   // Percurso
                        note->display_vector[2].type = CIRCLE;  // Circulo final
                        break;
/*
                case ARC_DRAG:
                        note->display_vector[0].type = CIRCLE;  // Circulo inicial
                        note->display_vector[1].type = ARC;   // Percurso
                        note->display_vector[2].type = CIRCLE;  // Circulo final
                        break;
                case ROUND_DRAG:
                        note->display_vector[0].type = CIRCLE;  // Circulo inicial
                        note->display_vector[1].type = ROUND;   // Percurso
                        note->display_vector[2].type = CIRCLE;  // Circulo final
                        break;
*/
        }
}


