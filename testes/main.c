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
    int partition;	// Diz a qual particao o circulo pertence. partition<0 -> invalido

    int starting_tick; // Diz quantos ticks de espera em relacao ao ultimo circulo

    int base_ticks; // Diz quantidade de tempo que circulo deve ficar na tela em ticks
    int remaining_ticks; // Diz quantos mais ticks o circulo tem na tela. remaining_ticks < 0 -> circulo expirado
    SDL_Color color;
} Circle;

SDL_mutex* mutex_circle_array = NULL;


SDL_mutex* mutex_state = NULL;
SDL_mutex* mutex_ticks_counter = NULL;

SDL_cond* conditional_tick_update = NULL;
SDL_cond* conditional_circle_expired = NULL;

SDL_sem* semaphore_circle_array_blank = NULL;


// Estado do jogo, usado por todas as threads
Uint32 tick_counter = 0;
int state = 1;

// Sequencia de todas as notas do jogo
Circle partiture[2000];
int partiture_next = 0;
int partiture_end = 2000;

// Array de circulos na memoria compartilhada, usando vetor circular
// usado para indicar quantos circulos estao presentes na tela
Circle circle_array[100];
int circle_array_begin = 0;
int circle_array_end = 0;



int AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms);

void generate_random_circle(Circle* circle);

int check_collision_circle();

// Enche array com circulos vazios
void initiate_circle_array(Circle* array, int array_size);

// Enche array com circulos validos aleatorios
void fill_circle_array(Circle* array, int array_size);

// Rotina da thread que lida com particoes e criacao de circulos no array
int produce_notes_routine(void* data);

int consume_notes_routine(void* data);

// Rotina da thread que lida com a musica (pauses, sincronizacao, etc)
int music_routine(void* data);



int main(int argc, char* argv[]) {

    int output = 0;

    int sdl_init_code = -1;

	/*	Declaracoes SDL		*/
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    	/*	Declaracoes de threads	*/

    // Cria thread que aloca circulos
    SDL_Thread* produce_notes = NULL;

    // Cria thread que desaloca circulos
    SDL_Thread* consume_notes = NULL;

    /*  TO IMPLEMENT
     *
    // Cria thread de musica
    SDL_Thread* music = NULL;
    */


	/*      Iniciacao de globais    */
    srand(time(NULL));  // Gera chave aleatoria

    // Inicia globais

    mutex_circle_array = SDL_CreateMutex();
    mutex_state = SDL_CreateMutex();
    mutex_ticks_counter = SDL_CreateMutex();

    conditional_tick_update = SDL_CreateCond();
    conditional_circle_expired = SDL_CreateCond();


    semaphore_circle_array_blank = SDL_CreateSemaphore(100);


    if(!mutex_circle_array){
	printf("Erro ao criar mutex de circulos: %s\n", SDL_GetError());
	output = -3;
        goto FIM;
    }
    if(!mutex_state){
	printf("Erro ao criar mutex de estados: %s\n", SDL_GetError());
	output = -3;
	goto FIM;
    }
    if(!mutex_ticks_counter){
        printf("Erro ao criar mutex de contagem de ticks: %s\n", SDL_GetError());
	output = -3;
        goto FIM;
    }



    if(!conditional_tick_update){
        printf("Erro ao criar condicional de contagem de ticks: %s\n", SDL_GetError());
        output = -5;
        goto FIM;
    }
    if(!conditional_circle_expired){
        printf("Erro ao criar condicional de circulos expirados no inicio do array: %s\n", SDL_GetError());
        output = -5;
        goto FIM;
    }


    if(!semaphore_circle_array_blank){
        printf("Erro ao criar semaforo de contagem de espacos vazios no buffer de circulos: %s\n", SDL_GetError());
        output = -6;
        goto FIM;
    }


    initiate_circle_array(circle_array, 100);
    fill_circle_array(partiture, partiture_end);

	/*	Iniciacao do SDL	*/


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

    	/*	Iniciacao de threads	*/
    // Inicia threads filhos
    produce_notes = SDL_CreateThread(produce_notes_routine, "Produz notas", (void*)"aaa");
    if (!produce_notes){
        printf("Erro ao criar thread de alocar circulos: %s\n", SDL_GetError());
        output = -2;
        goto FIM;
    }

    consume_notes = SDL_CreateThread(consume_notes_routine, "Consome notas", (void*)"bbb");
    if (!consume_notes){
        printf("Erro ao criar thread de desalocar circulos: %s\n", SDL_GetError());
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

    int score = 0;
    Uint32 tickTime = 17;


    SDL_Event event;

    	/*	Execucao	*/

    while (1) {

	SDL_LockMutex(mutex_state);
    	if (!state) {
        	SDL_UnlockMutex(mutex_state);
	        break;
	}
	SDL_UnlockMutex(mutex_state);

		/*      Desenho         */
	// Fundo
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

	// Circulos
	bool wake_consumer = false;

	SDL_LockMutex(mutex_circle_array);
	for(int i = circle_array_begin; i != circle_array_end; i=((i+1)%100)){

		//printf("Pai: Desenhando circulos!\n");

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

			if(circle_array[i].remaining_ticks >=0){
				filledCircleRGBA(renderer, circle_array[i].x, circle_array[i].y, circle_array[i].radius, circle_array[i].color.r,
                                        circle_array[i].color.g, circle_array[i].color.b, circle_array[i].color.a);

				--circle_array[i].remaining_ticks;
			}
			else{
				printf("Pai: Circulo expirado! (-1 ponto)\n");
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
		else{
			// Se primeiro elemento for vazio, habilita o consumidor
			if(i == circle_array_begin){
				wake_consumer = true;
			}
		}
	}
	SDL_UnlockMutex(mutex_circle_array);

	if(wake_consumer){
		SDL_CondBroadcast(conditional_circle_expired);
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

			/*	Checa eventos		*/
	int isevt = AUX_WaitEventTimeoutCount(&event, &tickTime);
        if (isevt) {
		switch(event.type){
			case SDL_QUIT:
				SDL_LockMutex(mutex_state);
	                	state = 0;

				SDL_SemPost(semaphore_circle_array_blank);

				SDL_CondBroadcast(conditional_tick_update);
				SDL_CondBroadcast(conditional_circle_expired);

				SDL_UnlockMutex(mutex_state);
				break;
	    		case SDL_MOUSEBUTTONDOWN:
		                // Verifica se acertou o clique no circulo
				SDL_LockMutex(mutex_circle_array);
                		int colision_result = check_collision_circle(&score);
				SDL_UnlockMutex(mutex_circle_array);
				switch(colision_result){
					case INT_MIN:
						printf("Pai: sei la oque deu\n");
						break;
					case -1:
						printf("Pai: Nao houve colisao! (-1 ponto)\n");
						if (score > 0){
							--score;
						}
						break;
					default:
						printf("Pai: Colisao bem-sucedida! (circulo na posicao: %d)\n", colision_result);
						++score;
				}
				break;
	
        	}
	}
	else{
		tickTime = 17;
		SDL_LockMutex(mutex_ticks_counter);
		++tick_counter;       // Incrementa ticks
		SDL_CondBroadcast(conditional_tick_update);	// Avisa que atualizou ticks
		SDL_UnlockMutex(mutex_ticks_counter);
	}

    }
    printf("Fim de Jogo! Sua pontuacao e: %d\n", score);
    printf("Total de circulos gerados: %d\n", partiture_next);


    //SDL_WaitThread(produce_notes, NULL);
    //SDL_WaitThread(consume_notes, NULL);

FIM:
    	/*	Desaloca tudo	*/
    if (produce_notes){
	SDL_DetachThread(produce_notes);
    }

    if (consume_notes){
        SDL_DetachThread(consume_notes);
    }

    /*		TO IMPLEMENT
    if (music){
	SDL_DetachThread(music);
    }
    */

    if(mutex_circle_array){
		SDL_DestroyMutex(mutex_circle_array);
    }
    if(mutex_state){
                SDL_DestroyMutex(mutex_state);
    }
    if(mutex_ticks_counter){
                SDL_DestroyMutex(mutex_ticks_counter);
    }



    if(conditional_tick_update){
                SDL_DestroyCond(conditional_tick_update);
    }
    if(conditional_circle_expired){
                SDL_DestroyCond(conditional_circle_expired);
    }



    if(semaphore_circle_array_blank){
                SDL_DestroySemaphore(semaphore_circle_array_blank);
    }


	/*	Finaliza SDL	*/
    if(renderer){
	    SDL_DestroyRenderer(renderer);
    }
    if(window){
	    SDL_DestroyWindow(window);
    }

     SDL_Quit();

    	/*	Retorna funcao */
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

void generate_random_circle(Circle* circle) {
    circle->partition = rand() % 3;	// Define particao


    circle->starting_tick = 10 + (rand() % 30);

    circle-> base_ticks = 180 + (rand() % 300); // Circulos no intervalo de [3s;8s]

    circle-> remaining_ticks = circle-> base_ticks;

    // Movido pra cima para ser usado em circle->x e circle->y
    circle->radius = rand() % 50 + 20;  // Raio entre 20 e 70 pixels

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

    if(	mouse_x == INT_MIN
     ||	mouse_y == INT_MIN	){

	printf("Erro! colisao inesperada!");
	return INT_MIN;
    }


    int output = -1;

    if (circle_array_end == circle_array_begin){
	return output;
    }



    for(int i = ( (circle_array_end-1)%100); i != ((circle_array_begin-1)%100); i=((i-1)%100)){
	int dx = mouse_x - circle_array[i].x;
	int dy = mouse_y - circle_array[i].y;

	if( 	( (dx * dx + dy * dy) <= (circle_array[i].radius * circle_array[i].radius) )
		&& (circle_array[i].remaining_ticks >= 0) ){

			circle_array[i].remaining_ticks = -1;	// Remove circulo
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

// Enche array com circulos validos aleatorios
void fill_circle_array(Circle* array, int array_size){
	for (int i = 0; i < array_size; i++){
		generate_random_circle(&array[i]);
        }
}


// Rotina da thread que lida com criacao de circulos no aray
int produce_notes_routine(void* data){

        // Guarda tempo em ticks antes da primeira execucao
        SDL_LockMutex(mutex_ticks_counter);
        Uint32 last_note_tick = tick_counter;
        SDL_UnlockMutex(mutex_ticks_counter);


        while(1){

		// Se acabou partitura, altera estado do jogo para fechar outras threads
		// Tambem acorda todos os semaforos/condicionais parados no momento
                if(partiture_next == partiture_end){
                        SDL_LockMutex(mutex_state);
                        state = 0;

			SDL_SemPost(semaphore_circle_array_blank);

                        SDL_CondBroadcast(conditional_tick_update);
			SDL_CondBroadcast(conditional_circle_expired);

                        SDL_UnlockMutex(mutex_state);

			break;
                }

		SDL_SemWait(semaphore_circle_array_blank);


		//printf("\033[0;32mFilho-produtor: achou espaco!\033[0m\n");

		//Se ainda estiver rodando, tenta criar circulo
                //Senao, sai do loop
                SDL_LockMutex(mutex_state);
                if (!state) {
                        SDL_UnlockMutex(mutex_state);
                        return 1;
                }
                SDL_UnlockMutex(mutex_state);


                // Checa se o tempo de criacao do proximo circulo ja passou
		// Se sim produz circulo
                SDL_LockMutex(mutex_ticks_counter);

		while(tick_counter < (last_note_tick + partiture[partiture_next].starting_tick)){

			SDL_LockMutex(mutex_state);
		        if (!state) {
                        	SDL_UnlockMutex(mutex_state);
				SDL_UnlockMutex(mutex_ticks_counter);
	                        return 1;
        	        }
                	SDL_UnlockMutex(mutex_state);


                        printf("\033[0;32mFilho-produtor: Esperando ticks da proxima nota! (ticks faltando: %d )\033[0m\n", (last_note_tick+partiture[partiture_next].starting_tick) - tick_counter);

                        SDL_CondWait(conditional_tick_update, mutex_ticks_counter);

                }

		// Atualiza proximo circulo na partitura e tempo da ultima nota
		last_note_tick = tick_counter;
                

		SDL_UnlockMutex(mutex_ticks_counter);



		// Cria circulo
		SDL_LockMutex(mutex_circle_array);

                circle_array[circle_array_end] = partiture[partiture_next];
                ++circle_array_end;
                circle_array_end %= 100;

		++partiture_next;


		printf("\033[0;32mFilho-produtor: criou circulo! (espacos possiveis para novos circulo possiveis: %d )\033[0m\n", SDL_SemValue(semaphore_circle_array_blank));

		SDL_UnlockMutex(mutex_circle_array);




        }
        return 0;
}

// Rotina de thread que consome circulos do buffer
int consume_notes_routine(void* data){

        while(1){

		// Se ainda estiver rodando, remove circulo
                // Senao, sai do loop
		SDL_LockMutex(mutex_state);
		if (!state) {
                        SDL_UnlockMutex(mutex_state);
                        return 1;
                }
		SDL_UnlockMutex(mutex_state);


		// Espera receber mensagem de circulos no inicio do array expirados
		SDL_LockMutex(mutex_circle_array);
		while(	(circle_array[circle_array_begin].remaining_ticks >= 0)
			|| (circle_array_begin == circle_array_end)){

	
			SDL_LockMutex(mutex_state);
	                if (!state) {
        	                SDL_UnlockMutex(mutex_state);
				SDL_UnlockMutex(mutex_circle_array);
                	        return 1;
	                }
        	        SDL_UnlockMutex(mutex_state);

			printf("\033[0;31mFilho-consumidor: Esperando circulos expirados no inicio do array...\033[0m\n");
			SDL_CondWait(conditional_circle_expired, mutex_circle_array);
		}

		/*
		// Agora que achou circulos expirados no inicio do buffer,
		// avanca inicio do buffer enquanto houverem circulos expirados
		while(  (circle_array[circle_array_begin].remaining_ticks < 0)
                        && (circle_array_begin != circle_array_end)){
*/
				SDL_LockMutex(mutex_state);
	                        if (!state) {
        	                        SDL_UnlockMutex(mutex_state);
                	                SDL_UnlockMutex(mutex_circle_array);
                        	        return 1;
	                        }
        	                SDL_UnlockMutex(mutex_state);

				++circle_array_begin;
		        	circle_array_begin %= 100;

				// Avisa na tela
                		printf("\033[0;31mFilho-consumidor: circulo consumido! (circulos restantes: %d)\033[0m\n", 100-SDL_SemValue(semaphore_circle_array_blank) );

		        	// Acrescenta posicao vazia ao array
	        		SDL_SemPost(semaphore_circle_array_blank);
//		}

		SDL_UnlockMutex(mutex_circle_array);

        }

        return 0;
}

/*
// Rotina da thread que lida com a musica (pauses, sincronizacao, etc)
int music_routine(){





}
*/

