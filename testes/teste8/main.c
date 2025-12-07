#include <SDL2/SDL.h>
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


// Trocar path para local com a fonte,
// ao reexecutar codigo
#define FONT_PATH_ITALIC "Libre_Baskerville/LibreBaskerville-Italic.ttf"

#define MAX_SCORE 9999999999
#define MAX_MULTIPLIER 99

/* TODO 
 *
 *	- Implementar notas de arrasto circular e em arco
 *
 *	- Pensar em arte para colocar como background
 *
 */

/*	Definicao de evento	*/

// Enum com todos os tipos de evento
//
typedef enum{
	NOTE_CLICK,
	NOTE_DRAG,
	TIME,
	KEYBOARD,

	EVENTS_NUMBER
} Event_types;


/*	Definicao de HUD	*/
typedef struct{
	SDL_Texture* score;
	SDL_Texture* multiplier;
	int w, h;
	int score_value;
	short multiplier_value;
} HUD;

/*	Globais		*/


// Hard coded no momento,
// mas no futuro, menu selecionara texto dependendo de opcao clicada
char* music_name = "musics/Bach-Overture_No.1-Passepied.mp3";

char* partiture_name = "partitures/Bach-Overture_No.1-Passepied.csv";

// Ponteiro alocado dinamicamente com todas as notas usadas
Note* partiture = NULL;
int partiture_next = 0;
int partiture_size = -1;

// Total de particoes de instrumentos existentes na musica
int total_partitions = -1;

// Array de notas, usando vetor circular
// usado para indicar quantas notas estao presentes na tela no momento
Note note_array[NOTE_ARRAY_SIZE];
int note_array_begin = 0;
int note_array_end = 0;

Note note_template[NOTE_NUMBER];


// Lida com evento
int AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms);

// Cria texto em ponteiro de textura
void create_text(SDL_Renderer* ren, SDL_Texture** text, char* text_content, SDL_Color color, TTF_Font* fnt);

// Cria imagem em ponteiro de textura
/*
	void create_image(SDL_Renderer* ren, SDL_Texture** text, char* text_content, SDL_Color color, TTF_Font* fnt);
*/

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

	srand(time(NULL));  // Gera chave aleatoria

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
	window = SDL_CreateWindow("Prototipo 6",
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
	SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

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


	/*	Inicia templates de notas	*/
	for(int i = 0; i < NOTE_NUMBER; i++){
		// Nullify array
		note_template[i].type = i;
		note_template[i].size = 0;
		note_template[i].img = NULL;
		note_template[i].display_vector = NULL;

		// Fill array
		initiate_note(&note_template[i]);
	}


	/*	Inicia partitura	*/
	char* partiture_string = read_partiture(partiture_name);

	partiture = parse_partiture(partiture_string, &partiture_size);


	Uint32 next_note_tick = partiture[0].display_vector[0].starting_tick;

	/*	Inicia templates de efeitos	*/

	Effect effect_template[EFFECT_NUMBER];

	for(int i = 0; i < EFFECT_NUMBER; i++){
		effect_template[i].tag = i;
		effect_template[i].img = NULL;
		effect_template[i].w = 100;
		effect_template[i].h = 50;
		effect_template[i].display_vector = NULL;
		effect_template[i].size = 0;

		char* string;
		// Seta string (caso tenha)
		switch(i){
			case CLICK_PERFEITO: 
				string = "Perfeito!";
				break;
			case CLICK_BOM:
				string = "Bom";
				break;
			case CLICK_RUIM:
				string = "Ruim";
				break;
			case CLICK_EXPIRADO:
				string = "Expirado";
				break;
			case CLICK_ERROU:
                                string = "Errou!";
                                break;
		}
		// Inicia efeito no vetor
		SDL_Color effect_color = {0xFF, 0xFF, 0xFF, 0xFF};
		switch(i){
			// Eventos de texto com 1 de tamanho
			case CLICK_PERFEITO:
			case CLICK_BOM:
			case CLICK_RUIM:
			case CLICK_EXPIRADO:
			case CLICK_ERROU:
				effect_template[i].size = 1;
				create_text(renderer, &effect_template[i].img, string, effect_color, fnt);
				break;
		}
	}
	/*	Inicia buffer circular de efeitos	*/

	Effect effects_array[EFFECTS_ARRAY_SIZE];
	int effects_array_begin = 0;
	int effects_array_end = 0;

	for(int i = 0; i < EFFECTS_ARRAY_SIZE; i++){
		effects_array[i].size = 0;
	}

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

	int last_note_position[3] = {INT_MIN, INT_MIN, INT_MIN};

	while (state) {

		// Checa se acabou a musica
		if(partiture_next >= partiture_size){
			state = 0;

			break;
		}


		// Tenta gerar novas notas
		if ( (next_note_tick <= tick_counter)
				&& (((note_array_end + 1)%NOTE_ARRAY_SIZE) != note_array_begin) ){

			// Cria circulo
			note_array[note_array_end] = partiture[partiture_next];
			// Aloca memoria
			note_array[note_array_end].display_vector = NULL;
			note_array[note_array_end].display_vector = (Note_display*) malloc(note_array[note_array_end].size * sizeof(Note_display));

			// Copia malloc
			for(int i = 0; i < note_array[note_array_end].size; i++){
				note_array[note_array_end].display_vector[i] = partiture[partiture_next].display_vector[i];
			}

			// Atualiza array
			++note_array_end;
			note_array_end %= NOTE_ARRAY_SIZE;

			// Atualiza proximo circulo na partitura
			++partiture_next;

			// Atualiza proximo circulo na partitura e tempo da ultima nota
			next_note_tick = tick_counter + partiture[partiture_next].display_vector[0].starting_tick;


			printf("\033[0;32mCirculo criado! (espacos possiveis para novos circulo: %d )\033[0m\n",
					((note_array_end-note_array_begin)>=0)?(NOTE_ARRAY_SIZE-(note_array_end-note_array_begin)):NOTE_ARRAY_SIZE-(NOTE_ARRAY_SIZE-note_array_begin+note_array_end));
		}


		// Tenta limpar notas expiradas do buffer
		while (note_array_begin != note_array_end){

			// Checa se primeira nota no vetor NAO acabou
			if (note_array[note_array_begin].display_vector[0].remaining_ticks >= 0){
				break;
			}

			// Desaloca memoria alocada
			if(note_array[note_array_begin].display_vector){
				free(note_array[note_array_begin].display_vector);
				note_array[note_array_begin].display_vector = NULL;
			}

			// Anula ponteiro de imagem
			if(note_array[note_array_begin].img){
                                note_array[note_array_begin].img = NULL;
                        }

			// Se a nota acabou, fica no loop e atualiza inicio
			++note_array_begin;
			note_array_begin %= NOTE_ARRAY_SIZE;
			// Avisa na tela
			printf("\033[0;33mCirculo consumido do buffer! (circulos restantes: %d)\033[0m\n",
					((note_array_end-note_array_begin)>=0)?(note_array_end-note_array_begin):(NOTE_ARRAY_SIZE-note_array_begin+note_array_begin) );

		}

		// Tenta limpar efeitos expirados do buffer
		while( (!effects_array[effects_array_begin].display_vector) && (effects_array_begin != effects_array_end)){
			++effects_array_begin;
			effects_array_begin %= EFFECTS_ARRAY_SIZE;

			// Avisa na tela
			printf("\033[0;33mEfeito consumido do buffer!\033[0m\n"
					//, ((note_array_end-note_array_begin)>=0)?(note_array_end-note_array_begin):(NOTE_ARRAY_SIZE-note_array_begin+note_array_begin)
			      );
		}

		// Atualiza HUD
		if (	score != game_HUD.score_value
				||	multiplier != game_HUD.multiplier_value){

			// Desaloca texturas
			SDL_DestroyTexture(game_HUD.score); game_HUD.score= NULL;
			SDL_DestroyTexture(game_HUD.multiplier); game_HUD.multiplier= NULL;

			// Cria nova pontuacao
			snprintf(score_buffer, sizeof(score_buffer), "%10d", score);
			create_text(renderer, &game_HUD.score, strncat(score_label, score_buffer, sizeof(score_label) - strlen(score_label) - 1), HUD_color, fnt);
			strcpy(score_label, "Pontos: ");

			// Cria novo multiplicador
			snprintf(multiplier_buffer, sizeof(multiplier_buffer), "%3d", multiplier);
			create_text(renderer, &game_HUD.multiplier, strncat(multiplier_label, multiplier_buffer, sizeof(multiplier_label) - strlen(multiplier_label) - 1), HUD_color, fnt);
			strcpy(multiplier_label, "Multiplicador: ");

			// Salva pontos e multiplicador atuais para comparar futuramente
			game_HUD.score_value = score;
			game_HUD.multiplier_value = multiplier;
		}

		/*      Desenho         */
		// Fundo
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		// Notas
		for(int i = note_array_begin; i != note_array_end; i=((i+1)%NOTE_ARRAY_SIZE)){
			int draw_note_result = draw_note(renderer, &note_array[i]);

			int expired_position[2] = {note_array[i].display_vector[0].x, note_array[i].display_vector[0].y};

			switch (draw_note_result){
				case INT_MIN: // Nao expirou
					break;

				case 0: // Clique expirou
					create_event(&event, custom_events_start, NOTE_CLICK, CLICK_EXPIRADO, expired_position);
                                        if (score > 0){
                                                --score;
                                        }
					multiplier = 1;
					break;

				case 1:	// Arrastar expirou
					if(i == last_note_position[2]){
						state = 1;
					}

					create_event(&event, custom_events_start, NOTE_DRAG, CLICK_EXPIRADO, expired_position);
					if (score > 0){
                                                --score;
                                        }
					multiplier = 1;
					break;
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

		/*	Checa eventos		*/

		SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

		int isevt = -1;
		switch(state){
			case 1:
				isevt = AUX_WaitEventTimeoutCount(&event, &tickTime);
				if (isevt) {
					switch(event.type){
						case SDL_QUIT:
							state = 0;
							break;
						case SDL_MOUSEBUTTONDOWN:

							// Verifica se acertou o clique no circulo

							int colision_result = check_collision_note(&last_note_position[0], &last_note_position[1], &last_note_position[2]);
							switch(colision_result){
								case INT_MIN:
									printf("sei la oque deu\n");
									break;
								case -1:
									state = 2;
									break;
								case CLICK_PERFEITO:
									printf("Colisao perfeita!\n");
									score+= (multiplier* 9);
									multiplier +=2;
									create_event(&event, custom_events_start, NOTE_CLICK, colision_result, last_note_position);
									break;
								case CLICK_BOM:
									printf("Colisao boa!\n");
									score+= (multiplier* 3);
									multiplier +=1;
									create_event(&event, custom_events_start, NOTE_CLICK, colision_result, last_note_position);
									break;
								case CLICK_RUIM:
									printf("Colisao ruim!\n");
									score+= multiplier;
									create_event(&event, custom_events_start, NOTE_CLICK, colision_result, last_note_position);
									break;
								case CLICK_ERROU:
									printf("Nao houve colisao! (-1 ponto)\n");
									if (score > 0){
										--score;
									}
									multiplier = 1;
									create_event(&event, custom_events_start, NOTE_CLICK, colision_result, last_note_position);
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

							//printf("Tipo de efeito %d\n", event.user.code-custom_events_start);
							int delay = -1;
                                                        int spawn_time_variation = -1;
                                                        int base_ticks = -1;
							switch( (event.user.code-custom_events_start) ){
								case NOTE_CLICK:
									switch( *(Effect_type*)event.user.data2 ){
										case CLICK_PERFEITO:
										case CLICK_BOM:
										case CLICK_RUIM:
										case CLICK_EXPIRADO:
										case CLICK_ERROU:
											// Mensagem referente a clique/falta de clique no circulo
											delay = 5;
											spawn_time_variation = 0;
											base_ticks = 100;
											break;
									}
									create_effect(effects_array, &effects_array_begin, &effects_array_end, effect_template, *(Effect_type*)event.user.data2,
											((int*)event.user.data1)[0], ((int*)event.user.data1)[1], &tick_counter, delay, spawn_time_variation, base_ticks);
									break;
								/*
								 *
								 *
								 *
								 *
								 *
								 *	Por enquanto, drag pega efeito emprestado de click
								 *
								 *
								 *
								 *
								 *
								 *
								 *
								 */

								case NOTE_DRAG:
                                                                        switch( *(Effect_type*)event.user.data2 ){
                                                                                case CLICK_PERFEITO:
                                                                                case CLICK_BOM:
                                                                                case CLICK_RUIM:
                                                                                case CLICK_EXPIRADO:
                                                                                case CLICK_ERROU:
                                                                                        // Mensagem referente a clique/falta de clique no circulo
                                                                                        delay = 5;
                                                                                        spawn_time_variation = 0;
                                                                                        base_ticks = 100;
                                                                                        break;
                                                                        }
                                                                        create_effect(effects_array, &effects_array_begin, &effects_array_end, effect_template, *(Effect_type*)event.user.data2,
                                                                                        ((int*)event.user.data1)[0], ((int*)event.user.data1)[1], &tick_counter, delay, spawn_time_variation, base_ticks);
                                                                        break;
							}

							free(event.user.data1); event.user.data1 = NULL;
							free(event.user.data2); event.user.data2 = NULL;
					}
				}
				else{
					tickTime = 17;
					++tick_counter;       // Incrementa ticks
					//printf("incrementa tick_counter! (tick_counter: %d. next_note_tick: %d)\n", tick_counter, next_note_tick);
				}
				break;

			case 2:
				isevt = AUX_WaitEventTimeoutCount(&event, &tickTime);
                                if (isevt) {
					 switch(event.type){
                                                case SDL_QUIT:
                                                        state = 0;
                                                        break;
						case SDL_MOUSEBUTTONUP:
							printf("Errou arrastar! (-1 ponto)\n");
                                                        if (score > 0){
                                                                --score;
                                                        }
                                                        multiplier = 1;

                                                        state = 1;

                                                        note_array[last_note_position[2]].display_vector[0].remaining_ticks = -1;
                                                        create_event(&event, custom_events_start, NOTE_DRAG, CLICK_ERROU, last_note_position);
							break;
					}
				}
				else{
					tickTime = 17;
                                        ++tick_counter;       // Incrementa ticks
					//printf("incrementa tick_counter! (tick_counter: %d. next_note_tick: %d)\n", tick_counter, next_note_tick);
					

					int drag_result = check_drag_note( &note_array[last_note_position[2]], &last_note_position[0], &last_note_position[1]);
					switch(drag_result){
						case INT_MIN:
							break;
						case CLICK_PERFEITO:
							printf("Arrasto perfeito!\n");
							score+= (multiplier* 9);
							multiplier +=2;

							state = 1;

							note_array[last_note_position[2]].display_vector[0].remaining_ticks = -1;
							create_event(&event, custom_events_start, NOTE_DRAG, drag_result, last_note_position);
							break;
						case CLICK_BOM:
							printf("Bom arrasto!\n");
							score+= (multiplier* 3);
							multiplier +=1;

							state = 1;

							note_array[last_note_position[2]].display_vector[0].remaining_ticks = -1;
							create_event(&event, custom_events_start, NOTE_DRAG, drag_result, last_note_position);
							break;
						case CLICK_RUIM:
							printf("Arrastar ruim!\n");
							score+= multiplier;

							state = 1;

							note_array[last_note_position[2]].display_vector[0].remaining_ticks = -1;
							create_event(&event, custom_events_start, NOTE_DRAG, drag_result, last_note_position);
							break;
						case CLICK_ERROU:
							printf("Errou arrastar! (-1 ponto)\n");
							if (score > 0){
								--score;
							}
							multiplier = 1;

							state = 1;

							note_array[last_note_position[2]].display_vector[0].remaining_ticks = -1;
							create_event(&event, custom_events_start, NOTE_DRAG, drag_result, last_note_position);
							break;
					}
					if (multiplier > MAX_MULTIPLIER){
						multiplier = MAX_MULTIPLIER;
					}
				}
				break;

		}
	}
	printf("\033[36mFim de Jogo! Sua pontuacao e: %d\033[0m\n", score);
	printf("\033[36mTotal de circulos gerados: %d\033[0m\n", partiture_next);

	while (note_array_begin != note_array_end){
		printf("\033[36mTipo de nota[%d]: %d\033[0m\n", note_array_begin, note_array[note_array_begin].type);
		note_array_begin++;
		note_array_begin %= NOTE_ARRAY_SIZE;
	}


	printf("\n\n\033[36minicio do buffer: %d\nfim do buffer: %d\033[0m\n", note_array_begin, note_array_end);

	printf("\n\n\033[36mtempo proximo circulo: %d\ntempo final: %d\033[0m\n", next_note_tick, tick_counter);


FIM:
	/*	Desaloca tudo	*/
	// Desaloca partitura
	for(int i = 0; i < partiture_size; i++){
		if (partiture[i].display_vector){
			free(partiture[i].display_vector);
			partiture[i].display_vector = NULL;
		}
	}
	if(partiture){
		free(partiture);
		partiture = NULL;
	}

	// Desaloca todas as notas
	for(int i = note_array_begin; i != note_array_end; i=((i+1)% NOTE_ARRAY_SIZE )){
		if (note_array[i].display_vector){
			free(note_array[i].display_vector);
			note_array[i].display_vector = NULL;
		}
	}

	// Desaloca template de notas
	for(int i = 0; i < NOTE_NUMBER; i++){
		if (note_template[i].display_vector){
			free(note_template[i].display_vector);
			note_template[i].display_vector = NULL;
		}
		if (note_template[i].img){
			for(int j = 0; j < note_template[i].img_number; j++){
				SDL_DestroyTexture(note_template[i].img[j]);
				note_template[i].img[j] = NULL;
			}

			free(note_template[i].img);
			note_template[i].img = NULL;
		}
	}

	// Desaloca todos os efeitos
	for(int i = effects_array_begin; i != effects_array_end; i=((i+1)% EFFECTS_ARRAY_SIZE )){
		if(effects_array[i].display_vector){
			destroy_effect(effects_array, i);
		}
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

int create_event(SDL_Event* evt, Uint32 custom_events_start, int code, Effect_type event_effect, int position[2]){

	evt->user.type = SDL_USEREVENT;
	evt->user.code = custom_events_start + code;

	int result = -1;
	switch(code){
		case NOTE_CLICK: // Evento que cria efeito ao clicar
		case NOTE_DRAG:	// Evento que cria efeito ao arrastar

			// Salva posicao do mouse
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

