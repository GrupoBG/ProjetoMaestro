#include <SDL2/SDL.h>
//#include <SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>

#define PARTITION_WIDTH 500
#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 900


typedef struct {
    int x, y;
    int radius;
    int partition;      // Diz a qual particao o circulo pertence. partition<0 -> invalido

    int starting_tick; // Diz quantos ticks de espera em relacao ao ultimo circulo

    int base_ticks; // Diz quantidade de tempo que circulo deve ficar na tela em ticks
    int remaining_ticks; // Diz quantos mais ticks o circulo tem na tela. remaining_ticks < 0 -> circulo expirado
    SDL_Color color;
} Circle;


// Sequencia de todas as notas do jogo
Circle partiture[2000];
int partiture_next = 0;
int partiture_end = 1999;

// Array de circulos na memoria compartilhada, usando vetor circular
// usado para indicar quantos circulos estao presentes na tela
Circle circle_array[100];
int circle_array_begin = 0;
int circle_array_end = 0;


// Lida com evento
int AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms);


// Driver que produz partitura
//
// Futuramente, menu principal entrega partitura para jogo
void generate_random_circle(Circle* circle);

// Checa se o mouse acertou o circulo
int check_collision_circle();

// Enche array com circulos vazios
void initiate_circle_array(Circle* array, int array_size);

// Enche array com circulos validos aleatorios
void fill_circle_array(Circle* array, int array_size);



int main(int argc, char* argv[]) {

    int output = 0;

    int sdl_init_code = -1;

    int state = 1;
    int score = 0;

    Uint32 tick_counter = 0;
    Uint32 tickTime = 17;
    SDL_Event event;

    Uint32 next_note_tick;

        /*      Declaracoes SDL         */
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;


        /*      Iniciacao de globais    */
    srand(time(NULL));  // Gera chave aleatoria

    // Inicia globais

    initiate_circle_array(circle_array, 100);
    fill_circle_array(partiture, partiture_end);

    next_note_tick = partiture[0].starting_tick;


        /*      Iniciacao do SDL        */


    sdl_init_code = SDL_Init(SDL_INIT_VIDEO);
    if (sdl_init_code < 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        output = -1;
        goto FIM;
    }

    // Cria a janela
    window = SDL_CreateWindow("Prototipo",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Erro ao criar janela: %s\n", SDL_GetError());
        output = -1;
        goto FIM;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erro ao criar renderizador: %s\n", SDL_GetError());
        output = -1;
        goto FIM;
    }

        /*      Execucao        */

    while (state) {

        // Checa se acabou a musica
        if(partiture_next > partiture_end){
                state = 0;

                break;
        }


        // Tenta gerar novos circulos
        if (next_note_tick <= tick_counter
        && ((circle_array_end + 1)%100) != circle_array_begin){

                // Cria circulo
                circle_array[circle_array_end] = partiture[partiture_next];
                ++circle_array_end;
                circle_array_end %= 100;

                // Atualiza proximo circulo na partitura
                ++partiture_next;

                // Atualiza proximo circulo na partitura e tempo da ultima nota
                next_note_tick = tick_counter + partiture[partiture_next].starting_tick;


                printf("\033[0;32mCirculo criado! (espacos possiveis para novos circulo: %d )\033[0m\n",
        ((circle_array_begin-circle_array_end)>=0)?(100-(100-circle_array_begin+circle_array_end)):(100-circle_array_end-circle_array_begin));
        }


        // Tenta limpar circulos expirados do buffer
        while (  (circle_array[circle_array_begin].remaining_ticks < 0)
        && (circle_array_begin != circle_array_end)){
                ++circle_array_begin;
                circle_array_begin %= 100;

                // Avisa na tela
                printf("\033[0;33mCirculo consumido do buffer! (circulos restantes: %d)\033[0m\n",
        ((circle_array_end-circle_array_begin)>=0)?(circle_array_end-circle_array_begin):100-(circle_array_end-circle_array_begin) );

        }


                /*      Desenho         */
        // Fundo
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Circulos
        for(int i = circle_array_begin; i != circle_array_end; i=((i+1)%100)){

                //printf("Desenhando circulos!\n");

                // Atualiza circulos com ticks
                if(circle_array[i].remaining_ticks >=0){

                        // Efeito de fade in
                        if((circle_array[i].remaining_ticks) > (circle_array[i].base_ticks/2)){

                                if( 255 < (circle_array[i].color.a + ( 255/(circle_array[i].base_ticks/2) ) ) ){
                                        circle_array[i].color.a = 255;
                                }
                                else{
                                        circle_array[i].color.a += ( 255/(circle_array[i].base_ticks/2) );
                                }
                        }

                        // Efeito de fade out
                        else{
                                if(circle_array[i].color.a > ( 255/(circle_array[i].base_ticks/2))){
                                        circle_array[i].color.a -= ( 255/(circle_array[i].base_ticks/2) );
                                }
                                else{
                                        circle_array[i].color.a = 0;
                                }
                        }

                        --circle_array[i].remaining_ticks;

                        if(circle_array[i].remaining_ticks >=0){
                                filledCircleRGBA(renderer, circle_array[i].x, circle_array[i].y, circle_array[i].radius,
                        circle_array[i].color.r, circle_array[i].color.g, circle_array[i].color.b, circle_array[i].color.a);

                        }
                        else{
                                printf("\033[34mCirculo expirado! (-1 ponto)\033[0m\n");
                                /*
                                 *
                                 *      Criar efeito de mensagem de circulo nao clicado aqui!
                                 *
                                 *
                                 */
                                if (score > 0){
                                        --score;
                                }
                        }
                }
        }

        // Linhas
        SDL_SetRenderDrawColor(renderer, 0xFF,0xFF,0xFF,0xFF);

        SDL_Rect line1 = {PARTITION_WIDTH, 0, 1, WINDOW_HEIGHT};
        SDL_Rect line2 = {2*PARTITION_WIDTH, 0, 1, WINDOW_HEIGHT};

        SDL_RenderFillRect(renderer, &line1);
        SDL_RenderFillRect(renderer, &line2);

        // Mostra desenho
        SDL_RenderPresent(renderer);

        //while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION));
        SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

                        /*      Checa eventos           */
        int isevt = AUX_WaitEventTimeoutCount(&event, &tickTime);
        if (isevt) {
                switch(event.type){
                        case SDL_QUIT:
                                state = 0;
                                break;
                        case SDL_MOUSEBUTTONDOWN:
                                // Verifica se acertou o clique no circulo
                                int colision_result = check_collision_circle(&score);
                                switch(colision_result){
                                        case INT_MIN:
                                                printf("sei la oque deu\n");
                                                break;
                                        case -1:
                                                printf("Nao houve colisao! (-1 ponto)\n");
                                                if (score > 0){
                                                        --score;
                                                }
                                                break;
                                        default:
                                                printf("Colisao bem-sucedida! (circulo na posicao: %d)\n", colision_result);
                                                ++score;
                                }
                                break;

                }
        }
        else{
                tickTime = 17;
                ++tick_counter;       // Incrementa ticks
        }

    }
    printf("\033[36mFim de Jogo! Sua pontuacao e: %d\033[0m\n", score);
    printf("\033[36mTotal de circulos gerados: %d\033[0m\n", partiture_next);


    printf("\n\n\033[36minicio do buffer: %d\nfim do buffer: %d\033[0m\n", circle_array_begin, circle_array_end);

    printf("\n\n\033[36mtempo proximo circulo: %d\ntempo final: %d\033[0m\n", next_note_tick, tick_counter);


FIM:
        /*      Desaloca tudo   */
        /*      Finaliza SDL    */
    if(renderer){
            SDL_DestroyRenderer(renderer);
    }
    if(window){
            SDL_DestroyWindow(window);
    }

     SDL_Quit();

        /*      Retorna funcao */
    // 0-> sucesso
    // <0 -> erro
    return output;
}


//Funcao auxiliar que esconde o uso de "SDL_WaitEventTimeout"
//
//
//Atualizada -> Agora nao precisa de getTicks() anterior a chamada
int AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms){

        //Pega tempo anterior
        Uint32 antes = SDL_GetTicks();

        int isevt =  SDL_WaitEventTimeout( evt , *ms );

        //Lida com atualizacao do valor de espera
        (*ms) = (isevt)? ((*ms)-( SDL_GetTicks()- antes )):(*ms);

        return isevt;
}

// Driver
//
// Gera circulo aleatorio
void generate_random_circle(Circle* circle) {
    circle->partition = rand() % 3;     // Define particao


    circle->starting_tick = 15 + (rand() % 46); // Circulo inicia entre [0,25s;1s]

    circle-> base_ticks = 180 + (rand() % 301); // Circulos no intervalo de [3s;8s]

    circle-> remaining_ticks = circle->base_ticks;

    // Movido pra cima para ser usado em circle->x e circle->y
    circle->radius = rand() % 51 + 20;  // Raio entre 20 e 70 pixels

    circle->x = circle->radius + (rand() % (PARTITION_WIDTH - 2*circle->radius)) + ( circle->partition* PARTITION_WIDTH);
    circle->y = circle->radius + (rand() % (WINDOW_HEIGHT - 2*circle->radius));

    circle->color.r = rand() % 256;
    circle->color.g = rand() % 256;
    circle->color.b = rand() % 256;
    circle->color.a = 0;
}

int check_collision_circle() {

    int mouse_x = INT_MIN;
    int mouse_y = INT_MIN;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    if( mouse_x == INT_MIN
     || mouse_y == INT_MIN      ){
        printf("Erro! colisao inesperada!");
        return INT_MIN;
    }

    int output = -1;

    if (circle_array_end == circle_array_begin){
        return output;
    }

    for(int i = circle_array_begin; i != circle_array_end; i=((i+1)%100)){
        int dx = mouse_x - circle_array[i].x;
        int dy = mouse_y - circle_array[i].y;

        if(     ( (dx * dx + dy * dy) <= (circle_array[i].radius * circle_array[i].radius) )
                && (circle_array[i].remaining_ticks >= 0) ){

                        circle_array[i].remaining_ticks = -1;   // Remove circulo
                        output = i;
                        break;
        }
    }

    return output;
}

// Enche array com circulos vazios
void initiate_circle_array(Circle* array, int array_size){
        for (int i = 0; i < array_size; i++){

                circle_array[i].x = circle_array[i].y = circle_array[i].radius =
                                circle_array[i].partition = circle_array[i].remaining_ticks = circle_array[i].base_ticks = circle_array[i].starting_tick = -1;

                circle_array[i].color.r = circle_array[i].color.g = circle_array[i].color.b = circle_array[i].color.a = 0x00;
        }
}

// Driver
//
// Enche array com circulos validos aleatorios
void fill_circle_array(Circle* array, int array_size){
        for (int i = 0; i <= array_size; i++){
                generate_random_circle(&array[i]);
        }
}



/*
int music_routine(){





}
*/
