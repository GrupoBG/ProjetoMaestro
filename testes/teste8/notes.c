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

#include "notes.h"
#include "effects.h"

// Gera nota aleatoria
void generate_random_note(Note* note_template, Note* note, int total_partitions) {

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

// Desenha nota e atualiza tempo de vida da mesma
//
// Saida define qual tipo de nota foi expirada
int draw_note(SDL_Renderer* renderer, Note* note){
        int draw_note_result = INT_MIN;
        if (!note->display_vector
        ||      note->is_finished){
                goto RETORNA_DESENHO_NOTA;
        }

        if(note->display_vector[0].remaining_ticks < 0){
                goto RETORNA_DESENHO_NOTA;
        }

        int click_timing;

        switch(note->type){

                case CLICK:
                case MULTI_CLICK:
                        // Efeito de fade in
                        if((note->display_vector[0].remaining_ticks) > (note->display_vector[0].base_ticks/2)){

                                if( 255 < (note->display_vector[0].color.a + ( 255/(note->display_vector[0].base_ticks/2) ) ) ){
                                        note->display_vector[0].color.a = 255;
                                }
                                else{
                                        note->display_vector[0].color.a += ( 255/(note->display_vector[0].base_ticks/2) );
                                }
                        }

                        // Efeito de fade out
                        else{
                                if(note->display_vector[0].color.a > ( 255/(note->display_vector[0].base_ticks/2))){
                                        note->display_vector[0].color.a -= ( 255/(note->display_vector[0].base_ticks/2) );
                                }
                                else{
                                        note->display_vector[0].color.a = 0;
                                }
                        }

                        // Cria circulo exterior
                        click_timing = abs( (note->display_vector[0].base_ticks/2)-note->display_vector[0].remaining_ticks );

                        if ( click_timing <= (note->display_vector[0].base_ticks/20) ){  // 10% do tempo

                                int external_circle_radius = note->display_vector[0].c.radius + click_timing;

                                aacircleRGBA(renderer, note->display_vector[0].x, note->display_vector[0].y, external_circle_radius,
                                                0xFF, 0xFF, 0xFF, 0xFF);
                        }
                        else if ( click_timing < (note->display_vector[0].base_ticks/5) ){ // (40-10)% do tempo

                                int external_circle_radius = note->display_vector[0].c.radius + click_timing;

                                aacircleRGBA(renderer, note->display_vector[0].x, note->display_vector[0].y, external_circle_radius,
                                                0x00, 0x00, 0xFF, 0xFF);
                        }
                        else{
                                int external_circle_radius = note->display_vector[0].c.radius + click_timing;

                                aacircleRGBA(renderer, note->display_vector[0].x, note->display_vector[0].y, external_circle_radius,
                                                0xFF, 0x00, 0x00, 0xFF);
                        }

                        // Se ainda tem tempo, desenha
                        if(note->display_vector[0].remaining_ticks >=0){

                                filledCircleRGBA(renderer, note->display_vector[0].x, note->display_vector[0].y, note->display_vector[0].c.radius,
                                                note->display_vector[0].color.r, note->display_vector[0].color.g, note->display_vector[0].color.b, note->display_vector[0].color.a);

                                // Atualiza tempo
                                --note->display_vector[0].remaining_ticks;
                                if(note->display_vector[0].remaining_ticks < 0 && note->type != MULTI_CLICK){
                                        printf("\033[34mNota expirada! (-1 ponto)\033[0m\n");
                                        draw_note_result = 0;
                                }
                        }
                        break;
                case STRAIGHT_DRAG:
                        // Efeito de fade in
                        if((note->display_vector[0].remaining_ticks) > (note->display_vector[0].base_ticks/2)){

                                if( 255 < (note->display_vector[0].color.a + ( 255/(note->display_vector[0].base_ticks/2) ) ) ){

                                        note->display_vector[0].color.a = 255;

                                        note->display_vector[1].color.a = 255;

                                        note->display_vector[2].color.a = 0x22;
                                }
                                else{
                                        note->display_vector[0].color.a += ( 255/(note->display_vector[0].base_ticks/2) );
                                        note->display_vector[1].color.a += ( 255/(note->display_vector[1].base_ticks/2) );
                                        note->display_vector[2].color.a += ( 0x22/(note->display_vector[2].base_ticks/2) );


                                }

                        }
                        // Efeito de fade out
                        else{
                                if(note->display_vector[0].color.a > ( 255/(note->display_vector[0].base_ticks/2))){

                                        note->display_vector[0].color.a -= ( 255/(note->display_vector[0].base_ticks/2) );

                                        note->display_vector[1].color.a -= ( 255/(note->display_vector[1].base_ticks/2) );
                                        note->display_vector[2].color.a -= ( 0x22/(note->display_vector[2].base_ticks/2) );
                                }
                                else{
                                        note->display_vector[0].color.a = note->display_vector[2].color.a = note->display_vector[2].color.a = 0;
                                }
                        }

                        // Cria circulo exterior
                        click_timing = abs( (note->display_vector[0].base_ticks/2)-note->display_vector[0].remaining_ticks );

                        if ( click_timing <= (note->display_vector[0].base_ticks/10) ){  // 20% do tempo

                                int external_circle_radius = note->display_vector[0].c.radius + click_timing;

                                aacircleRGBA(renderer, note->display_vector[0].x, note->display_vector[0].y, external_circle_radius,
                                                0xFF, 0xFF, 0xFF, 0xFF);
                        }
                        else if ( click_timing < (note->display_vector[0].base_ticks/3) ){ // (66.6-20)% do tempo

                                int external_circle_radius = note->display_vector[0].c.radius + click_timing;

                                aacircleRGBA(renderer, note->display_vector[0].x, note->display_vector[0].y, external_circle_radius,
                                                0x00, 0x00, 0xFF, 0xFF);
                        }
                        else{
                                int external_circle_radius = note->display_vector[0].c.radius + click_timing;

                                aacircleRGBA(renderer, note->display_vector[0].x, note->display_vector[0].y, external_circle_radius,
                                                0xFF, 0x00, 0x00, 0xFF);
                        }

                        // Se ainda tem tempo, desenha
                        if(note->display_vector[0].remaining_ticks >=0){

                                filledCircleRGBA(renderer, note->display_vector[0].x, note->display_vector[0].y, note->display_vector[0].c.radius,
                                                note->display_vector[0].color.r, note->display_vector[0].color.g, note->display_vector[0].color.b, note->display_vector[0].color.a);

                                aalineRGBA(renderer, note->display_vector[1].x, note->display_vector[1].y, note->display_vector[1].s.x2, note->display_vector[1].s.y2,
                                                note->display_vector[1].color.r, note->display_vector[1].color.g, note->display_vector[1].color.b, note->display_vector[1].color.a);

                                filledCircleRGBA(renderer, note->display_vector[2].x, note->display_vector[2].y, note->display_vector[2].c.radius,
                                                note->display_vector[2].color.r, note->display_vector[2].color.g, note->display_vector[2].color.b, note->display_vector[2].color.a);


                                // Atualiza tempo
                                --note->display_vector[0].remaining_ticks;
                                note->display_vector[1].remaining_ticks = note->display_vector[2].remaining_ticks = note->display_vector[0].remaining_ticks;
                                if(note->display_vector[0].remaining_ticks < 0){
                                        printf("\033[34mNota expirada! (-1 ponto)\033[0m\n");

                                        draw_note_result = 1;
                                }

                        }
                        break;
/*
                case ARC_DRAG:
                        *
                        *
                        *
                        *      Desenha Arco
                        *
                        *
                        *
                        break;
                case ROUND_DRAG:
                        *
                        *
                        *      Desenha circulo
                        *
                        *
                        *
                        break;
*/
        }

RETORNA_DESENHO_NOTA:
        return draw_note_result;
}

// Le partitura de um arquivo e retorna string constante
char* read_partiture(const char* partiture_name) {
    FILE* file = fopen(partiture_name, "r");
    if (!file) {
        fprintf(stderr, "Erro ao abrir partitura %s\n", partiture_name);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char* partiture_string = malloc(length + 1);
    if (!partiture_string) {
        perror("Erro ao alocar partitura");
        fclose(file);
        return NULL;
    }
    size_t read_length = fread(partiture_string, 1, length, file);
    partiture_string[read_length] = '\0';
    fclose(file);
    return partiture_string;
}


// Aloca partitura e enche usando uma string
// Salva tamanho da partitura em partiture_size
Note* parse_partiture(char* partiture_string, int* partiture_size){

	if (!partiture_string) return NULL;

	// Pega primeira linha
	char* line = strtok(partiture_string, "\n");
	if (!line) return NULL;

	// Deduz o numero de notas na partitura da primeira linha do csv
	char* partiture_size_string = strdup(line);
	*partiture_size = atoi(partiture_size_string);
	free(partiture_size_string);

	Note* partiture = (Note*) malloc(sizeof(Note) * (*partiture_size));
    	if (!partiture) return NULL;

        // Atualiza linha
        line = strtok(NULL, "\n");

        Note* current_note = &partiture[0];
        // Entre em loop
        while (*line){

                char* first_char = &line[0];
                char* next_camp = strchr(line, ',');

                // Pega campo contendo tipo de nota
                char type_string[next_camp-line+1];
                strncpy(type_string, line, next_camp-line);
                type_string[next_camp-line] = '\0';

                // Transforma tipo de nota de string para Note_type
                Note_type type_number = (Note_type) atoi(type_string);

                // Copia nota do array de templates
                (*current_note) = note_template[type_number];
                current_note->display_vector = NULL;
                current_note->display_vector = (Note_display*) malloc( (current_note->size)* sizeof(Note_display));

                ++next_camp;
                first_char = next_camp;

                // Ajuda a deduzir o campo atual sendo lido
                typedef enum{
                        TYPE,
                        PARTITION,
                        START_TIME,
                        LIFE_TIME,
                        POSITION_X,
                        POSITION_Y,
                        COLOR_R,
                        COLOR_G,
                        COLOR_B,
                        COLOR_A,
                        TYPE_SPECIFIC,

                        MAX_CAMP        // Conta numero de campos validos
                } Partiture_camp;

                Partiture_camp camp_position = PARTITION;
                int type_specific_camp_position;

                // Itera sobre cada campo presente na linha
                while(next_camp = strpbrk(next_camp, ",\n")){
                        // Pega proximo campo
                        char camp_string[next_camp-first_char+1];
                        strncpy(camp_string, first_char, next_camp-first_char);
                        camp_string[next_camp-first_char] = '\0';

                        int camp_integer = atoi(camp_string);

                        // Adiciona campo pego a nota, a partir da posicao do campo na linha lida
                        switch(camp_position){
                                case PARTITION:
					for(int i = 0; i < current_note->size; i++){
	                                        current_note->display_vector[i].partition = camp_integer;
					}
                                        break;
                                case START_TIME:
					for(int i = 0; i < current_note->size; i++){
                                                current_note->display_vector[i].starting_tick = camp_integer;
					}
                                        break;
                                case LIFE_TIME:
					for(int i = 0; i < current_note->size; i++){
	                                        current_note->display_vector[i].remaining_ticks = current_note->display_vector[i].base_ticks = camp_integer;
					}
                                        break;
                                case POSITION_X:
	                                current_note->display_vector[0].x = current_note->display_vector[0].base_x = camp_integer;
                                        break;
                                case POSITION_Y:
                                        current_note->display_vector[0].y = current_note->display_vector[0].base_y = camp_integer;
                                        break;
                                case COLOR_R:
					for(int i = 0; i < current_note->size; i++){
	                                        current_note->display_vector[i].color.r = camp_integer;
					}
                                        break;
                                case COLOR_G:
					for(int i = 0; i < current_note->size; i++){
	                                        current_note->display_vector[i].color.g = camp_integer;
					}
                                        break;
                                case COLOR_B:
					for(int i = 0; i < current_note->size; i++){
        	                                current_note->display_vector[i].color.b = camp_integer;
					}
                                        break;
                                case COLOR_A:
					for(int i = 0; i < current_note->size; i++){
	                                        current_note->display_vector[i].color.a = camp_integer;
					}
                                        break;
                                case TYPE_SPECIFIC:
                                        switch(type_number){
                                                case CLICK:
                                                case MULTI_CLICK:
                                                        // raio
                                                        current_note->display_vector[0].c.radius = camp_integer;
                                                        break;
                                                case STRAIGHT_DRAG:
                                                        switch(type_specific_camp_position){
                                                                case 0: // raio
                                                                        current_note->display_vector[0].c.radius = current_note->display_vector[2].c.radius = camp_integer;
                                                                case 1: // x2 (x final)
                                                                        current_note->display_vector[2].x = camp_integer;
                                                                        break;
                                                                case 2: // y2 (y final)
                                                                        current_note->display_vector[2].y = camp_integer;
                                                                        break;
                                                        }
                                        }
                        }

                        // Proximo campo
                        ++next_camp;
                        first_char = next_camp;

                        if (camp_position < TYPE_SPECIFIC){
                                ++camp_position;
                        }
                }

                // Conserta ultimos detalhes por tipo
                switch(type_number){
                        case CLICK:
                        case MULTI_CLICK:

                                /*
                                 *
                                 *      Ainda precisa truncar x e y caso passe dos limites da tela!
                                 *
                                 */

                                break;
                        case STRAIGHT_DRAG:
                                // Cria linha reta entre os dois circulos
                                current_note->display_vector[1].x =  current_note->display_vector[0].x;
                                current_note->display_vector[1].y =  current_note->display_vector[0].y;
                                current_note->display_vector[1].s.x2 =  current_note->display_vector[2].x;
                                current_note->display_vector[1].s.y2 =  current_note->display_vector[2].y;

                                // Seta ponto inicial da reta
                                current_note->display_vector[1].base_x =  current_note->display_vector[1].x;
                                current_note->display_vector[1].base_y =  current_note->display_vector[1].y;

                                // Coloca alpha fixo para reta suporte
                                current_note->display_vector[1].color.a = 0x44;

                                // Coloca alpha fixo para circulo suporte
                                current_note->display_vector[2].color.a = 0xFF;

                                /*
                                 *
                                 *      Ainda precisa truncar x e y caso passe dos limites da tela!
                                 *
                                 */

                                break;

                }

                // Proxima linha
                line = strtok(NULL, "\n");

                if (current_note < &partiture[*partiture_size]){
                        ++current_note;
                }
        }

	return partiture;
}

