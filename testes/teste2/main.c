#include <SDL2/SDL.h>
//#include <SDL_mixer.h>
#include <SDL2/SDL2_gfxPrimitives.h>


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>



#define PARTITION_WIDTH 500
#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 1000
#define GAME_DURATION 15000  // Tempo de duracao do jogo em milissegundos


typedef struct {
    int x, y;
    int radius;
    int partition;	// Diz a qual particao o circulo pertence. partition<0 -> invalido

    int starting_tick; // Diz quantos ticks de espera em relacao ao ultimo circulo

    int remaining_ticks; // Diz quantos mais ticks o circulo tem na tela. remaining_ticks < 0 -> circulo expirado
    SDL_Color color;
} Circle;

SDL_mutex* mutex_circle_array = NULL; 
SDL_mutex* mutex_state = NULL;
SDL_mutex* mutex_ticks_counter = NULL;


// Estado do jogo, usado por todas as threads
int tick_counter = 0;
int state = 1;

// Sequencia de todas as notas do jogo
Circle partiture[2000];
int partiture_next = 0;
int partiture_end = 1999;

// Array de circulos na memoria compartilhada, usando vetor circular
// usado para indicar quantos circulos estao presentes na tela
Circle circle_array[10];
int circle_array_begin = 0;
int circle_array_end = 0;



int AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms);

void generate_random_circle(Circle* circle);

int check_collision_circle(int* circle);

// Enche array com circulos vazios
void initiate_circle_array(Circle* array, int array_size);

// Enche array com circulos validos aleatorios
void fill_circle_array(Circle* array, int array_size);

// Rotina da thread que lida com particoes e criacao de circulos no array
int partitions_routine();
// Rotina da thread que lida com a musica (pauses, sincronizacao, etc)
int music_routine();



int main(int argc, char* argv[]) {
	/*	Iniciacao de globais	*/
    
    srand(time(NULL));  // Gera chave aleatoria

    // Inicia globais

    mutex_circle_array = SDL_CreateMutex();
    mutex_state = SDL_CreateMutex();
    mutex_ticks_counter = SDL_CreateMutex();

    if(!mutex_circle_array){
	printf("Erro ao criar mutex de circulos: %s\n", SDL_GetError());
        return -3;
    }
    if(!mutex_state){
	printf("Erro ao criar mutex de estados: %s\n", SDL_GetError());
	SDL_DestroyMutex(mutex_circle_array);
        return -3;
    }
    if(!mutex_ticks_counter){
        printf("Erro ao criar mutex de contagem de ticks: %s\n", SDL_GetError());
        SDL_DestroyMutex(mutex_circle_array);
	SDL_DestroyMutex(mutex_state);
        return -3;
    }

    initiate_circle_array(circle_array, 10);

	/*	Iniciacao do SDL	*/

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
        return -1;
    }

    // Cria a janela
    SDL_Window* window = SDL_CreateWindow("Prototipo",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Erro ao criar janela: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erro ao criar renderizador: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }
	int output = 0;

	/*	Threads		*/

    // Cria thread de particoes 
    SDL_Thread* partitions = NULL;

    /*          TO IMPLEMENT
     *
    // Cria thread de musica
    SDL_Thread* music = NULL;
    */

    // Inicia threads filhos
    partitions = SDL_CreateThread(partitions_routine, "Particoes", (void*)"bbbbbbbb");
    if (!partitions){
        printf("Erro ao criar thread de particoes: %s\n", SDL_GetError());
        output = -2;
        goto FIM;
    }

    /*          TO IMPLEMENT
    music = SDL_CreateThread(music_routine, "Music", (void*)"aaaaaa");
    if (!music){
        printf("Erro ao criar thread de musica: %s\n", SDL_GetError());
        output = -2;

        goto FIM;
    }
    */

    	/*	Preparacao para execucao	*/

    srand(time(NULL));  // Geracao aleatoria

    int score = 0;
    Uint32 tickTime = 17;


    SDL_Event event;

    	/*	Execucao	*/

    SDL_LockMutex(mutex_state);
    while (state) {
	SDL_UnlockMutex(mutex_state);

	/*
		Conta tempo
        Uint32 current_time = SDL_GetTicks();
        if (current_time - start_time >= GAME_DURATION) {
		SDL_LockMutex(mutex_state);
        	state = 0;  // Termina o jogo apos X segundos
		SDL_UnlockMutex(mutex_state);
        }
	*/

			/*      Desenho         */
	// Fundo
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

	// Circulos
	SDL_LockMutex(mutex_circle_array);
	for(int i = circle_array_begin; i != circle_array_end; i=((i+1)%10)){

		// Atualiza circulos com ticks
		if(circle_array[i].remaining_ticks >=0){

			--circle_array[i].remaining_ticks;
		
			// Efeito de fadeout
			--circle_array[i].color.a;

			filledCircleRGBA(renderer, circle_array[i].x, circle_array[i].y, circle_array[i].radius, circle_array[i].color.r,
                                        circle_array[i].color.g, circle_array[i].color.b, circle_array[i].color.a);
		}
		else{
			// Atualiza inicio do vetor circular, caso inicio nao tenha mais ticks
			if (i == circle_array_begin){
				++circle_array_begin;
			}
		}
	}
	SDL_UnlockMutex(mutex_circle_array);


	// Linhas
	SDL_SetRenderDrawColor(renderer, 0xFF,0xFF,0xFF,0xFF);

	SDL_Rect line1 = {PARTITION_WIDTH, 0, 1, WINDOW_HEIGHT};
	SDL_Rect line2 = {2*PARTITION_WIDTH, 0, 1, WINDOW_HEIGHT};

	SDL_RenderFillRect(renderer, &line1);
	SDL_RenderFillRect(renderer, &line2);

	// Mostra desenho
	SDL_RenderPresent(renderer);


	while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION));

			/*	Checa eventos		*/
	int isevt = AUX_WaitEventTimeoutCount(&event, &tickTime);
        if (isevt) {
		switch(event.type){
			case SDL_QUIT:
				SDL_LockMutex(mutex_state);
	                	state = 0;
				SDL_UnlockMutex(mutex_state);
				break;
	    		case SDL_MOUSEBUTTONDOWN:
		                // Verifica se acertou o clique no circulo
                		int colision_result = check_collision_circle(&score);
				switch(colision_result){
					case INT_MIN:
						printf("sei la oque deu\n");
						break;
					case 0:
						printf("Nao houve colisao!\n");
						break;
					default:
						printf("Colisao bem-sucedida!\n");
				}
				break;
	
        	}
	}
	else{
		tickTime = 17;
		SDL_LockMutex(mutex_ticks_counter);
		++tick_counter;       // Incrementa ticks
		SDL_UnlockMutex(mutex_ticks_counter);
	}

	SDL_LockMutex(mutex_state);
    }
    SDL_UnlockMutex(mutex_state);
    printf("Fim de Jogo! Sua pontuacao e: %d\n", score);

    //SDL_WaitThread(partitions, NULL);

FIM:
    if (partitions){
	SDL_DetachThread(partitions);
    }

    /*		TO IMPLEMENT
    if (music){
	SDL_DetachThread(music);
    }
    */

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

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

void generate_random_circle(Circle* circle) {
    circle->partition = rand() % 3;	// Define particao


    circle->starting_tick = 30 + (rand() % 30);

    circle-> remaining_ticks = 60 + (rand() % 60); // Circulos no intervalo de [1s;2s]

    // Movido pra cima para ser usado em circle->x e circle->y
    circle->radius = rand() % 50 + 20;  // Raio entre 20 e 70 pixels


    circle->x = (rand() % (PARTITION_WIDTH - 2 * circle->radius) + circle->radius) + ( circle->partition* PARTITION_WIDTH);
    circle->y = rand() % (WINDOW_HEIGHT - 2 * circle->radius) + circle->radius;

    circle->color.r = rand() % 256;
    circle->color.g = rand() % 256;
    circle->color.b = rand() % 256;
    circle->color.a = 255;
}

int check_collision_circle(int* score) {

    int mouse_x = INT_MIN;
    int mouse_y = INT_MIN;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    if(	mouse_x == INT_MIN
     ||	mouse_y == INT_MIN	){

	printf("Erro! colisao inesperada!");
	return INT_MIN;
    }


    SDL_LockMutex(mutex_circle_array);
    for(int i = circle_array_begin; i != circle_array_end; i=((i+1)%10)){
	int dx = mouse_x - circle_array[i].x;
	int dy = mouse_y - circle_array[i].y;

	if((dx * dx + dy * dy) <= (circle_array[i].radius * circle_array[i].radius) && (circle_array[i].remaining_ticks >= 0) ){
		circle_array[i].remaining_ticks = INT_MIN;	// Remove circulo
		++(*score); // Aumenta pontuacao

		SDL_UnlockMutex(mutex_circle_array);
		return 1;
	}

    }
    SDL_UnlockMutex(mutex_circle_array);

    return 0;
}

// Enche array com circulos vazios
void initiate_circle_array(Circle* array, int array_size){
	for (int i = 0; i < array_size; i++){

                circle_array[i].x = circle_array[i].y = circle_array[i].radius =
							circle_array[i].partition = circle_array[i].remaining_ticks = INT_MIN;

                circle_array[i].color.r = circle_array[i].color.g = circle_array[i].color.b = circle_array[i].color.a = 0x00;
        }
}

// Enche array com circulos validos aleatorios
void fill_circle_array(Circle* array, int array_size){
	for (int i = 0; i < array_size; i++){
		generate_random_circle(&array[i]);
        }
}

// Rotina da thread que lida com particoes e criacao de circulos no array
int partitions_routine(void* data){

	fill_circle_array(partiture, 2000);

	SDL_LockMutex(mutex_ticks_counter);
        int now_tick = tick_counter;
        SDL_UnlockMutex(mutex_ticks_counter);

	SDL_LockMutex(mutex_state);
        while(state){
		SDL_UnlockMutex(mutex_state);

		// Se array de circulos nao estiver cheio
		SDL_LockMutex(mutex_circle_array);
                if ( ((circle_array_end +1) %10) != circle_array_begin){

			SDL_LockMutex(mutex_ticks_counter);
			if( tick_counter >= (now_tick + partiture[partiture_next].starting_tick)){
				now_tick = tick_counter;
				SDL_UnlockMutex(mutex_ticks_counter);

				circle_array[circle_array_end] = partiture[partiture_next];
				++partiture_next;
				++circle_array_end;
				circle_array_end %= 10;
			}
			SDL_UnlockMutex(mutex_ticks_counter);
                }
                SDL_UnlockMutex(mutex_circle_array);

		if(partiture_next > partiture_end){
			SDL_LockMutex(mutex_state);
			state = 0;
			SDL_UnlockMutex(mutex_state);
		}

		SDL_LockMutex(mutex_state);
        }
	SDL_UnlockMutex(mutex_state);

	return 0;
}

/*
// Rotina da thread que lida com a musica (pauses, sincronizacao, etc)
int music_routine(){





}
*/

