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


// Trocar path para local com a fonte,
// ao reexecutar codigo

#define FONT_PATH_ITALIC "Libre_Baskerville/LibreBaskerville-Italic.ttf"


// Configuracao de tela
#define PARTITION_WIDTH 500
#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 900

// Configuracao de array
#define EFFECTS_ARRAY_SIZE 1000
#define CIRCLE_ARRAY_SIZE 100

#define MAX_SCORE 9999999999
#define MAX_MULTIPLIER 99

/* DONE
 *
 * - Criar diferenciacao de pontuacao e display ao lidar com cliques/falhas (Ótimo, Bom, Ruim, Expirou e Errou)
 *
 * - Desenhar efeitos criados na tela.
 * - Usar logica similar aos circulos para criar e destruir efeitos
 * - HUD basica
 * - Melhoria no sistema de pontuacao para clicks
 * 
 *
 */

typedef struct {
    int x, y;
    int radius;
    int partition;	// Diz a qual particao o circulo pertence. partition<0 -> invalido

    int starting_tick; // Diz quantos ticks de espera em relacao ao ultimo circulo

    int base_ticks; // Diz quantidade de tempo que circulo deve ficar na tela em ticks
    int remaining_ticks; // Diz quantos mais ticks o circulo tem na tela. remaining_ticks < 0 -> circulo expirado
    SDL_Color color;
} Circle;


// Enum com todos os tipos de evento
//
typedef enum{
	CLIQUE,
	TIME,
	KEYBOARD,

	EVENTS_NUMBER
} Event_types;



// enum com todos os tipos de efeito
typedef enum{
	CLIQUE_PERFEITO, 
	CLIQUE_BOM,
	CLIQUE_RUIM,
	CLIQUE_EXPIRADO,
	CLIQUE_ERROU,


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

typedef struct{
	SDL_Texture* score;
	SDL_Texture* multiplier;
	int w, h;
	int score_value;
	short multiplier_value;
} HUD;


// Sequencia de todas as notas do jogo
Circle partiture[2000];
int partiture_next = 0;
int partiture_size = 2000;

// Array de circulos na memoria compartilhada, usando vetor circular
// usado para indicar quantos circulos estao presentes na tela
Circle circle_array[CIRCLE_ARRAY_SIZE];
int circle_array_begin = 0;
int circle_array_end = 0;

// Lida com evento
int AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms);

// Driver que produz partitura
//
// Futuramente, menu principal entrega partitura para jogo
void generate_random_circle(Circle* circle);

// Checa se o mouse acertou o circulo
//
// Se acertou, retorna x e y do centro do circulo acertado
int check_collision_circle(int* x, int* y);

// Enche array com circulos vazios
void initiate_circle_array(Circle* array, int array_size);

// Enche array com circulos validos aleatorios
void fill_circle_array(Circle* array, int array_size);

// Cria texto em ponteiro de textura
void create_text(SDL_Renderer* ren, SDL_Texture** text, char* text_content, SDL_Color color, TTF_Font* fnt);

// Cria imagem em ponteiro de textura
void create_image(SDL_Renderer* ren, SDL_Texture** text, char* text_content, SDL_Color color, TTF_Font* fnt);

// Cria efeito baseado em array de template de efeitos
void create_effect(Effect* effect_array, int* effect_array_begin, int* effect_array_end, Effect* effect_templates, int effect_template,
                        int x, int y, int* now, int delay, int spawn_time_variation, int base_ticks);

// Destroi efeito
void destroy_effect(Effect* effect_array, int pos);


// Cria evento com codigo 'code' e informacao "data".
int create_event(SDL_Event* evt, Uint32 custom_events_start, int code, Effect_type event_effect, int position[2]);


int main(int argc, char* argv[]) {

	/*      Iniciacao de globais    */
    int output = 0;

    int sdl_init_code = -1;
    int ttf_init_code = -1;

    int state = 1;
    int score = 0;
    short multiplier = 1;

    HUD game_HUD;

    game_HUD.w = 200;
    game_HUD.h = 100;

    game_HUD.score = NULL;
    game_HUD.multiplier = NULL;

    game_HUD.score_value = score;
    game_HUD.multiplier_value = multiplier;


    // Lida com eventos
    Uint32 tick_counter = 0;
    Uint32 tickTime = 17;
    SDL_Event event;

    Uint32 next_note_tick = partiture[0].starting_tick;

    // Buffer circular de efeitos
    Effect effects_array[EFFECTS_ARRAY_SIZE];
    int effects_array_begin = 0;
    int effects_array_end = 0;

    for(int i = 0; i < EFFECTS_ARRAY_SIZE; i++){
	effects_array[i].size = 0;
    }

    srand(time(NULL));  // Gera chave aleatoria

    initiate_circle_array(circle_array, CIRCLE_ARRAY_SIZE);
    fill_circle_array(partiture, partiture_size);


	/*	Declaracoes SDL		*/
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    TTF_Font* fnt = NULL;

	/*	Iniciacao do SDL	*/
    sdl_init_code = SDL_Init(SDL_INIT_VIDEO);
    if (sdl_init_code < 0) {
        printf("Erro ao inicializar SDL: %s\n", SDL_GetError());
	output = -1;
        goto FIM;
    }
    // Cria a janela
    window = SDL_CreateWindow("Prototipo 5",
                                          SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Erro ao criar janela: %s\n", SDL_GetError());
        output = -1;
        goto FIM;
    }
    // Cria renderizador
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("Erro ao criar renderizador: %s\n", SDL_GetError());
        output = -1;
        goto FIM;
    }
    // Inicia ttf
    ttf_init_code = TTF_Init();
    if (ttf_init_code <0) {
        printf("Erro ao iniciar TTF: %s\n", SDL_GetError());
        output = -1;
        goto FIM;
    }
    // Cria fonte
    fnt = TTF_OpenFont(FONT_PATH_ITALIC, 20);
    if (!fnt) {
        printf("Erro ao criar fonte: %s\n", SDL_GetError());
        output = -1;
        goto FIM;
    }
    	/* Inicia templates de efeitos */

    Effect effect_template[EFFECT_NUMBER];

    for(int i = 0; i < EFFECT_NUMBER; i++){
            effect_template[i].tag = i;
            effect_template[i].img = NULL;
	    effect_template[i].w = 100;
	    effect_template[i].h = 50;
            effect_template[i].display_vector = NULL;
            effect_template[i].size = 0;
    }

    SDL_Color effect_color = {0xFF, 0xFF, 0xFF, 0xFF};

    
    // Efeito clique perfeito
    create_text(renderer, &effect_template[CLIQUE_PERFEITO].img, "Perfeito!", effect_color, fnt);
    effect_template[CLIQUE_PERFEITO].size = 1;
    // Efeito clique bom
    create_text(renderer, &effect_template[CLIQUE_BOM].img, "Bom", effect_color, fnt);
    effect_template[CLIQUE_BOM].size = 1;
    // Efeito clique ruim
    create_text(renderer, &effect_template[CLIQUE_RUIM].img, "Ruim", effect_color, fnt);
    effect_template[CLIQUE_RUIM].size = 1;
    // Efeito clique expirado
    create_text(renderer, &effect_template[CLIQUE_EXPIRADO].img, "Expirado", effect_color, fnt);
    effect_template[CLIQUE_EXPIRADO].size = 1;
    // Efeito clique errado
    create_text(renderer, &effect_template[CLIQUE_ERROU].img, "Errou", effect_color, fnt);
    effect_template[CLIQUE_ERROU].size = 1;

    	/*	Inicia HUD	*/

    SDL_Color HUD_color = {0xFF, 0xFF, 0xFF, 0xFF};

    char score_buffer[20];
    snprintf(score_buffer, sizeof(score_buffer), "%10d", score);

    char score_label[50] = "Pontos: ";
    create_text(renderer, &game_HUD.score, strncat(score_label, score_buffer, sizeof(score_label) - strlen(score_label) - 1), HUD_color, fnt);
    strcpy(score_label, "Pontos: ");

    if (!game_HUD.score){
	printf("Erro ao iniciar pontuacao: %s\n", SDL_GetError());
        output = -1;
        goto FIM;
    }

    char multiplier_buffer[10];
    snprintf(multiplier_buffer, sizeof(multiplier_buffer), "%3d", multiplier);

    char multiplier_label[50] = "Multiplicador: ";
    create_text(renderer, &game_HUD.multiplier, strncat(multiplier_label, multiplier_buffer, sizeof(multiplier_label) - strlen(multiplier_label) - 1), HUD_color, fnt);
    strcpy(multiplier_label, "Multiplicador: ");

    if (!game_HUD.multiplier){
	printf("Erro ao iniciar multiplicador: %s\n", SDL_GetError());
        output = -1;
        goto FIM;
    }

    	/*	Eventos custom	*/

    Uint32 custom_events_start = SDL_RegisterEvents(EVENTS_NUMBER);

    	/*	Execucao	*/

    while (state) {

	// Checa se acabou a musica
	if(partiture_next >= partiture_size){
                state = 0;

        	break;
        }


	// Tenta gerar novos circulos
	if ( (next_note_tick <= tick_counter)
	&& (((circle_array_end + 1)%CIRCLE_ARRAY_SIZE) != circle_array_begin) ){

                // Cria circulo
                circle_array[circle_array_end] = partiture[partiture_next];
                ++circle_array_end;
                circle_array_end %= CIRCLE_ARRAY_SIZE;

		// Atualiza proximo circulo na partitura
                ++partiture_next;

		// Atualiza proximo circulo na partitura e tempo da ultima nota
                next_note_tick = tick_counter + partiture[partiture_next].starting_tick;


                printf("\033[0;32mCirculo criado! (espacos possiveis para novos circulo: %d )\033[0m\n",
	((circle_array_end-circle_array_begin)>=0)?(CIRCLE_ARRAY_SIZE-(circle_array_end-circle_array_begin)):CIRCLE_ARRAY_SIZE-(CIRCLE_ARRAY_SIZE-circle_array_begin+circle_array_end));
	}


	// Tenta limpar circulos expirados do buffer
	while (  (circle_array[circle_array_begin].remaining_ticks < 0) && (circle_array_begin != circle_array_end)){

                ++circle_array_begin;
                circle_array_begin %= CIRCLE_ARRAY_SIZE;

                // Avisa na tela
                printf("\033[0;33mCirculo consumido do buffer! (circulos restantes: %d)\033[0m\n",
        ((circle_array_end-circle_array_begin)>=0)?(circle_array_end-circle_array_begin):(CIRCLE_ARRAY_SIZE-circle_array_begin+circle_array_begin) );

        }

	// Tenta limpar efeitos expirados do buffer
	while( (!effects_array[effects_array_begin].display_vector) && (effects_array_begin != effects_array_end)){
		++effects_array_begin;
                effects_array_begin %= EFFECTS_ARRAY_SIZE;

                // Avisa na tela
                printf("\033[0;33mEfeito consumido do buffer!\033[0m\n"
        	//, ((circle_array_end-circle_array_begin)>=0)?(circle_array_end-circle_array_begin):(CIRCLE_ARRAY_SIZE-circle_array_begin+circle_array_begin)
		);
	}

	// Atualiza HUD
	
		// Desaloca texturas
	if (	score != game_HUD.score_value
	||	multiplier != game_HUD.multiplier_value){

		SDL_DestroyTexture(game_HUD.score); game_HUD.score= NULL;
		SDL_DestroyTexture(game_HUD.multiplier); game_HUD.multiplier= NULL;

			// Cria nova pontuacao
		snprintf(score_buffer, sizeof(score_buffer), "%10d", score);
		create_text(renderer, &game_HUD.score, strncat(score_label, score_buffer, sizeof(score_label) - strlen(score_label) - 1), HUD_color, fnt);
		strcpy(score_label, "Pontos: ");

			// Cira novo multiplicador
		snprintf(multiplier_buffer, sizeof(multiplier_buffer), "%3d", multiplier);
		create_text(renderer, &game_HUD.multiplier, strncat(multiplier_label, multiplier_buffer, sizeof(multiplier_label) - strlen(multiplier_label) - 1), HUD_color, fnt);
		strcpy(multiplier_label, "Multiplicador: ");

		game_HUD.score_value = score;
		game_HUD.multiplier_value = multiplier;
	}

		/*      Desenho         */
	// Fundo
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

	// Circulos
	for(int i = circle_array_begin; i != circle_array_end; i=((i+1)%CIRCLE_ARRAY_SIZE)){

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

			// Cria circulo exterior
			int click_timing = abs( (circle_array[i].base_ticks/2)-circle_array[i].remaining_ticks );

			if ( click_timing <= (circle_array[i].base_ticks/20) ){  // 10% do tempo

				int external_circle_radius = circle_array[i].radius + click_timing;

                                aacircleRGBA(renderer, circle_array[i].x, circle_array[i].y, external_circle_radius,
                        0xFF, 0xFF, 0xFF, 0xFF);
                        }
			else if ( click_timing < (circle_array[i].base_ticks/5) ){ // (40-10)% do tempo
				
				int external_circle_radius = circle_array[i].radius + click_timing;

                                aacircleRGBA(renderer, circle_array[i].x, circle_array[i].y, external_circle_radius,
                        0x00, 0x00, 0xFF, 0xFF);
                        }
                        else{
				int external_circle_radius = circle_array[i].radius + click_timing;

                                aacircleRGBA(renderer, circle_array[i].x, circle_array[i].y, external_circle_radius,
                        0xFF, 0x00, 0x00, 0xFF);
                        }

			--circle_array[i].remaining_ticks;


			if(circle_array[i].remaining_ticks >=0){
				filledCircleRGBA(renderer, circle_array[i].x, circle_array[i].y, circle_array[i].radius,
			circle_array[i].color.r, circle_array[i].color.g, circle_array[i].color.b, circle_array[i].color.a);

			}
			else{
				printf("\033[34mCirculo expirado! (-1 ponto)\033[0m\n");

				multiplier = 1;
				int expired_circle_position[2] = {circle_array[i].x, circle_array[i].y};

				create_event(&event, custom_events_start, CLIQUE, CLIQUE_EXPIRADO, expired_circle_position);
                                if (score > 0){
                                        --score;
                                }
			}
		}
	}


	// Efeitos
	for(int i = effects_array_begin; i != effects_array_end; i=((i+1)%EFFECTS_ARRAY_SIZE)){
		assert(effects_array[i].display_vector);

		// Checa se todas as particulas do efeito foram expiradas
		bool expired_effect = true;
		for(int j = 0; j < effects_array[i].size; j++){
			// Desenha particulas validas para o efeito
			if (effects_array[i].display_vector[j].remaining_ticks >=0){
				expired_effect = false;
				
				SDL_Rect effect_framing = {effects_array[i].display_vector[j].x, effects_array[i].display_vector[j].y, effects_array[i].w, effects_array[i].h};


				double fade_status = ((double) effects_array[i].display_vector[j].remaining_ticks)/( (double)effects_array[i].display_vector[j].base_ticks);

				SDL_SetTextureAlphaMod(effects_array[i].img, ((int) (255 * fade_status)));
				SDL_RenderCopy(renderer, effects_array[i].img, NULL, &effect_framing);
				SDL_SetTextureAlphaMod(effects_array[i].img, 0xFF);

				--effects_array[i].display_vector[j].remaining_ticks;
			}
		}
		// destroi efeito expirado
		if(expired_effect){
			destroy_effect(effects_array, i);
			effects_array[i].size = 0;
		}
	}


	// Linhas
	SDL_SetRenderDrawColor(renderer, 0xFF,0xFF,0xFF,0xFF);

	SDL_Rect line1 = {PARTITION_WIDTH, 0, 1, WINDOW_HEIGHT};
	SDL_Rect line2 = {2*PARTITION_WIDTH, 0, 1, WINDOW_HEIGHT};

	SDL_RenderFillRect(renderer, &line1);
	SDL_RenderFillRect(renderer, &line2);


	//HUD
	SDL_Rect score_framing = {(WINDOW_WIDTH-game_HUD.w)-20, 0, game_HUD.w, (game_HUD.h/2)};
	SDL_Rect multiplier_framing = {(WINDOW_WIDTH-game_HUD.w)-20, (game_HUD.h/2), game_HUD.w, (game_HUD.h/2)};

	SDL_RenderCopy(renderer, game_HUD.score, NULL, &score_framing);
	SDL_RenderCopy(renderer, game_HUD.multiplier, NULL, &multiplier_framing);

	// Mostra desenho
	SDL_RenderPresent(renderer);

	//while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_MOUSEMOTION, SDL_MOUSEMOTION));
	SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

			/*	Checa eventos		*/
	int isevt = AUX_WaitEventTimeoutCount(&event, &tickTime);
        if (isevt) {
		switch(event.type){
			case SDL_QUIT:
	                	state = 0;
				break;
	    		case SDL_MOUSEBUTTONDOWN:
		                // Verifica se acertou o clique no circulo
				int circle_position[2] = {INT_MIN, INT_MIN};

                		int colision_result = check_collision_circle(&circle_position[0], &circle_position[1]);
				switch(colision_result){
					case INT_MIN:
						printf("sei la oque deu\n");
						break;
					case CLIQUE_PERFEITO:
						printf("Colisao perfeita!\n");
						score+= (multiplier* 9);
						multiplier +=2;
						create_event(&event, custom_events_start, CLIQUE, colision_result, circle_position);
						break;
					case CLIQUE_BOM:
						printf("Colisao boa!\n");
                                                score+= (multiplier* 3);
						multiplier +=1;
						create_event(&event, custom_events_start, CLIQUE, colision_result, circle_position);
                                                break;
					case CLIQUE_RUIM:
						printf("Colisao ruim!\n");
                                                score+= multiplier;
						create_event(&event, custom_events_start, CLIQUE, colision_result, circle_position);
                                                break;
					case CLIQUE_ERROU:
                                                printf("Nao houve colisao! (-1 ponto)\n");
                                                if (score > 0){
                                                        --score;
                                                }
						multiplier = 1;
						create_event(&event, custom_events_start, CLIQUE, colision_result, circle_position);
                                                break;
				}
				if (multiplier > MAX_MULTIPLIER){
					multiplier = MAX_MULTIPLIER;
				}
				break;
			case SDL_USEREVENT:
				if (!event.user.data1){
                                        printf("Erro ao ler posicao do custom event!\n");
                                        break;
                                }
				if (!event.user.data2){
					printf("Erro ao ler efeito de custom event!\n");
					break;
				}
				int delay = -1;
				int spawn_time_variation = -1;
				int base_ticks = -1;

				switch( *(Effect_type*)event.user.data2 ){
					case CLIQUE_PERFEITO:
					case CLIQUE_BOM:
                                        case CLIQUE_RUIM:
                                        case CLIQUE_EXPIRADO:
					case CLIQUE_ERROU:
						// Mensagem referente a clique/falta de clique no circulo
						delay = 5;
		                                spawn_time_variation = 0;
		                                base_ticks = 100;
						break;
				}

				create_effect(effects_array, &effects_array_begin, &effects_array_end, effect_template, *(Effect_type*)event.user.data2,
                        ((int*)event.user.data1)[0], ((int*)event.user.data1)[1], &tick_counter, delay, spawn_time_variation, base_ticks);

				free(event.user.data1); event.user.data1 = NULL;
				free(event.user.data2); event.user.data2 = NULL;
	
        	}
	}
	else{
		tickTime = 17;
		++tick_counter;       // Incrementa ticks
		//printf("incrementa tick_counter! (tick_counter: %d. next_note_tick: %d)\n", tick_counter, next_note_tick);
	}

    }
    printf("\033[36mFim de Jogo! Sua pontuacao e: %d\033[0m\n", score);
    printf("\033[36mTotal de circulos gerados: %d\033[0m\n", partiture_next);


    printf("\n\n\033[36minicio do buffer: %d\nfim do buffer: %d\033[0m\n", circle_array_begin, circle_array_end);

    printf("\n\n\033[36mtempo proximo circulo: %d\ntempo final: %d\033[0m\n", next_note_tick, tick_counter);


FIM:
    	/*	Desaloca tudo	*/


    // Desaloca todos os efeitos
    for(int i = effects_array_begin; i != effects_array_end; i=((i+1)% EFFECTS_ARRAY_SIZE )){
	destroy_effect(effects_array, i);
    }

    // Desaloca template dos efeitos
    for(int i = 0; i < EFFECT_NUMBER; i++){
            SDL_DestroyTexture(effect_template[i].img);
    }

    if (game_HUD.score){
	    SDL_DestroyTexture(game_HUD.score);
    }

    if (game_HUD.multiplier){
	    SDL_DestroyTexture(game_HUD.multiplier);
    }

    // Desaloca fonte
    if (fnt) {
	    TTF_CloseFont(fnt);
    }

    if (ttf_init_code>=0) {
        TTF_Quit();
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
// Atualizada -> Sempre tentar reduzir tempo, nao somente quando houver evento
// parece ter consertado problemas de travamento
int AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms){

        //Pega tempo anterior
        Uint32 antes = SDL_GetTicks();

        int isevt =  SDL_WaitEventTimeout( evt , *ms );

	Uint32 decorrido = SDL_GetTicks() - antes;

        //Lida com atualizacao do valor de espera
        *ms = (*ms > decorrido) ? (*ms - decorrido) : 0;

        return isevt;
}

// Driver
//
// Gera circulo aleatorio
void generate_random_circle(Circle* circle) {
    circle->partition = rand() % 3;	// Define particao


    circle->starting_tick = 15 + (rand() % 46);	// Circulo inicia entre [0,25s;1s]

    circle-> base_ticks = 30 + (rand() % 31); // Circulos no intervalo de [0,5s;1s]

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

// Checa colisao de circulo
//
// retorna qualidade do clique
// 0 - perfeito
// 1
//
// INT_MIN -> a
int check_collision_circle(int* x, int* y) {

    SDL_GetMouseState(x, y);

    if(	(*x) == INT_MIN
     ||	(*y) == INT_MIN	){
	printf("Erro! Falha ao obter informacoes do mouse!\n");
	return INT_MIN;
    }

    int output = CLIQUE_ERROU;


    for(int i = ((circle_array_end-1 + CIRCLE_ARRAY_SIZE)%CIRCLE_ARRAY_SIZE);
		    			i != ((circle_array_begin-1 + CIRCLE_ARRAY_SIZE )%CIRCLE_ARRAY_SIZE);
										i=((i-1+ CIRCLE_ARRAY_SIZE)%CIRCLE_ARRAY_SIZE)){
	int dx = (*x) - circle_array[i].x;
	int dy = (*y) - circle_array[i].y;

	if( 	( (dx * dx + dy * dy) <= (circle_array[i].radius * circle_array[i].radius) )
		&& (circle_array[i].remaining_ticks >= 0) ){

			int click_timing = abs( (circle_array[i].base_ticks/2)-circle_array[i].remaining_ticks );

			circle_array[i].remaining_ticks = -1;	// Remove circulo

			if ( click_timing < (circle_array[i].base_ticks/20) ){	// 10% do tempo
				output = CLIQUE_PERFEITO;
			}

			else if ( click_timing < (circle_array[i].base_ticks/5) ){ // (40-10)% do tempo
                                output = CLIQUE_BOM;
                        }

			else{
				output = CLIQUE_RUIM;
			}

			(*x) = circle_array[i].x;
			(*y) = circle_array[i].y;

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
	for (int i = 0; i < array_size; i++){
		generate_random_circle(&array[i]);
        }
}


void create_text(SDL_Renderer* ren, SDL_Texture** text, char* text_content, SDL_Color color, TTF_Font* fnt){

	assert(fnt);

	SDL_Surface* sfc = TTF_RenderText_Blended(fnt, text_content, color);
	assert(sfc);

	(*text) = SDL_CreateTextureFromSurface(ren, sfc);
	assert(text);

	SDL_FreeSurface(sfc);
}

/*
void create_image(SDL_Renderer* ren, SDL_Texture** text, char* text_content, SDL_Color color, TTF_Font* fnt){

        assert(fnt);

        SDL_Surface* sfc = TTF_RenderText_Blended(fnt, text_content, color);
        assert(sfc);

        (*text) = SDL_CreateTextureFromSurface(ren, sfc);
        assert(text);

        SDL_FreeSurface(sfc);

}
*/

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


int create_event(SDL_Event* evt, Uint32 custom_events_start, int code, Effect_type event_effect, int position[2]){

	evt->user.type = SDL_USEREVENT;
	evt->user.code = custom_events_start + code;

	int result = -1;
	switch(code){
		case CLIQUE: // Evento que cria efeito ao clicar

			// Salva posicao do mouse em data1
			evt->user.data1 = (int*) malloc(2* sizeof(int));
			((int*)evt->user.data1)[0] = position[0];
			((int*)evt->user.data1)[1] = position[1];

			// Salva tag do efeito a ser realizado apos evento em data2
			evt->user.data2 = (Effect_type*) malloc(sizeof(Effect_type));
		        (*(Effect_type*)evt->user.data2) = event_effect;

			// Tenta jogar evento na fila
			result = SDL_PushEvent(evt);

			break;

	}

	// Se falhar, diz que falhou e desaloca memoria (seta para NULL para evitar dangling pointer)
	if (result < 0){
		fprintf(stderr, "SDL_PushEvent falhou: %s\n", SDL_GetError());
	
		if(evt->user.data1){
			free(evt->user.data1); evt->user.data1 = NULL;
		}
		if(evt->user.data2){
                        free(evt->user.data2); evt->user.data2 = NULL;
                }

	}

	// Retorna se o evento foi enviado ou nao para a fila
	return result;

}



/*
int music_routine(){


}
*/
