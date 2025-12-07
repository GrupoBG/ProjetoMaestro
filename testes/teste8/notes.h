#ifndef NOTES_H
#define NOTES_H

#include <SDL2/SDL.h>
#include <SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>


/*      Definicao de tipos de nota      */

typedef struct {
        Sint16 radius;
} Circle;

typedef struct {
        Sint16 x2, y2;
} Straight_line;
typedef struct{
        Sint16 radius;
        Sint16 start, end;
} Arc;
typedef struct{
        Sint16 radius;
} Round;


/*      Definicao de nota       */

// Tipo de nota
typedef enum{
        CLICK,
        MULTI_CLICK,
        STRAIGHT_DRAG,
/*
        ARC_DRAG,
        ROUND_DRAG,
*/
        NOTE_NUMBER
} Note_type;


// Tipo de parte de nota
typedef enum{
        // Tipos que usam SDL_gfx
        CIRCLE,
        STRAIGHT_LINE,
/*
        ARC,
        ROUND,
        // Tipos que nao usam SDL_gfx
        IMAGE,

*/
        NOTE_DISPLAY_NUMBER
} Note_display_type;



// Cada parte da nota
typedef struct{
        Sint16 x, y;    // Posicao atual na tela

        Sint16 base_x, base_y;  // Posicao inicial na tela

        int partition;      // Diz a qual particao o circulo pertence. partition<0 -> invalido
        int starting_tick; // Diz quantos ticks de espera em relacao ao ultimo circulo

        int base_ticks; // Diz quantidade de tempo que circulo deve ficar na tela em ticks
        int remaining_ticks; // Diz quantos mais ticks o circulo tem na tela. remaining_ticks < 0 -> circulo expirado

        SDL_Color color;

        //tagged union definindo tipo de nota
        Note_display_type type;
        union {
                Circle c;
                Straight_line s;
/*
                Arc a;
                Round r;
*/
                SDL_Texture* img;
        };

} Note_display;


// Nota de fato
typedef struct{
        Note_type type;

        bool is_finished;

        int img_number;
        SDL_Texture** img;

        int size;
        Note_display* display_vector;

} Note



// Usado para gerar partitura aleatoria
//
// Futuramente, menu principal entrega partitura pronta para jogo
void generate_random_note(Note* note_template, Note* note);

// Checa se o mouse acertou a nota
//
// Se acertou, retorna x e y do centro do circulo acertado
int check_collision_note(int* x, int* y, int* pos);


// Lida com as notas de arrastar
//
// Se falhou, retorna INT_MIN, se não, retorna numero relativo a qualidade do arrasto
int check_drag_note(Note* note, int* x, int* y);

// Enche array com notas vazias
void initiate_note_array(Note* array, int array_size);

/*
// Enche array com notas validas aleatorias
void fill_note_array(Note* array, int array_size);
*/

// Pega nota vazia e inicia com valores reais
void initiate_note(Note* note);


// Desenha nota e atualiza tempo de vida da mesma
//
// Saida define qual tipo de nota foi expirada
int draw_note(SDL_Renderer* renderer, Note* note);


// Le partitura de um arquivo e retorna string constante
//
//      CUIDADO: garantir que partiture_string nao seja alocado dinamicamente antes da chamada. Senao, causa memory leak
//
void read_partiture(char* partiture_string, const char* partiture_name);

// Aloca partitura e enche usando uma string
// Salva tamanho da partitura em partiture_size
void parse_partiture(char* partiture_string, Note* partiture, int* partiture_size);


#endif
