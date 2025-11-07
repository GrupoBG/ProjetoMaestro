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


// Trocar path para local com a fonte,
// ao reexecutar codigo

#define FONT_PATH_ITALIC "Libre_Baskerville/LibreBaskerville-Italic.ttf"

// Configuracao de tela
#define PARTITION_WIDTH 300
#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 600

// Configuracao de array
#define EFFECTS_ARRAY_SIZE 1000
#define NOTE_ARRAY_SIZE 100

#define MAX_SCORE 9999999999
#define MAX_MULTIPLIER 99

/* TODO 
 *
 *	- Implementar logica de notas de fato (tirando o stub)
 *
 *	- Pensar em arte para colocar como background
 *
 * 
 *
 */

/*	Definicao de tipos de nota	*/

typedef struct {
	Sint16 radius;
} Circle;

typedef struct {
	Sint16 x2, y2;
	double coeficient_1;
	double coeficient_0;
} Straight_line;
typedef struct{
	Sint16 radius;
	Sint16 start, end;
} Arc;

/*	Definicao de nota	*/

// Tipo de nota
typedef enum{
	CLICK,
	MULTI_CLICK,
	DRAG,


	NOTE_NUMBER
} Note_type;


// Tipo de parte de nota
typedef enum{
	// Tipos que usam SDL_gfx
	CIRCLE,
	STRAIGHT_LINE,
	ARC,
	// Tipos que nao usao SDL_gfx
	IMAGE,


	NOTE_DISPLAY_NUMBER
} Note_display_type;



// Cada parte da nota
typedef struct{
	Sint16 x, y;	// Posicao atual na tela

	Sint16 base_x, base_y;	// Posicao inicial na tela

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
		Arc a;

		SDL_Texture* img;
	};

} Note_display;


// Nota a ser referenciada
typedef struct{
	Note_type type;

	int img_number;
	SDL_Texture** img;

	int size;
	Note_display* display_vector;

} Note;

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

/*	Definicao de efeito	*/

// enum com todos os tipos de efeito
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


/*	Definicao de HUD	*/
typedef struct{
	SDL_Texture* score;
	SDL_Texture* multiplier;
	int w, h;
	int score_value;
	short multiplier_value;
} HUD;

/*	Globais		*/

// Sequencia de todas as notas da partida
Note partiture[2000];
int partiture_next = 0;
int partiture_size = 2000;

// Total de particoes de instrumentos existentes na musica
int total_partitions = 3;

// Array de circulos na memoria compartilhada, usando vetor circular
// usado para indicar quantos circulos estao presentes na tela
Note note_array[NOTE_ARRAY_SIZE];
int note_array_begin = 0;
int note_array_end = 0;

// Lida com evento
int AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms);

// Futuramente, menu principal entrega partitura para jogo
void generate_random_note(Note* note_template, Note* note);

// Checa se o mouse acertou o circulo
//
// Se acertou, retorna x e y do centro do circulo acertado
int check_collision_note(int* x, int* y, int* pos);


// Lida com as notas de arrastar
//
// Se falhou, retorna INT_MIN, se não, retorna numero relativo a qualidade do arrasto
int check_drag_note(Note* note, int* x, int* y);

// Enche array com circulos vazios
void initiate_note_array(Note* array, int array_size);

/*
// Enche array com circulos validos aleatorios
void fill_note_array(Note* array, int array_size);
*/


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


// Pega nota vazia e inicia com valores reais
void initiate_note(Note* note);

// Desenha nota e atualiza tempo de vida da mesma
//
// Saida define qual tipo de nota foi expirada
int draw_note(SDL_Renderer* renderer, Note* note);


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
	Note note_template[NOTE_NUMBER];

	for(int i = 0; i < NOTE_NUMBER; i++){
		// Nullify array
		note_template[i].type = i;
		note_template[i].size = 0;
		note_template[i].img = NULL;
		note_template[i].display_vector = NULL;

		// Fill array
		initiate_note(&note_template[i]);
	}

	/*      Inicia partitura       */
	for(int i = 0; i < partiture_size; i++){
		generate_random_note(note_template, &partiture[i]);
	}

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

// Gera circulo aleatorio
void generate_random_note(Note* note_template, Note* note) {

	/*
	 *
	 *	No momento, testando sem imagens
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
			note->display_vector[0].c.radius = rand() % 71 + 20;
			break;
		case DRAG:
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

	if (note->type == DRAG){
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

	if(	(*x) == INT_MIN
			||	(*y) == INT_MIN	){
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
				 *	So funciona com circulos por enquanto
				 *
				 *
				 */
				dx = (*x) - note_array[i].display_vector[0].x;
				dy = (*y) - note_array[i].display_vector[0].y;

				if( 	( (dx * dx + dy * dy) <= (note_array[i].display_vector[0].c.radius * note_array[i].display_vector[0].c.radius) )
						&& (note_array[i].display_vector[0].remaining_ticks >= 0) ){

					int click_timing = abs( (note_array[i].display_vector[0].base_ticks/2)-note_array[i].display_vector[0].remaining_ticks );

					note_array[i].display_vector[0].remaining_ticks = -1;	// Remove circulo

					if ( click_timing < (note_array[i].display_vector[0].base_ticks/20) ){	// 10% do tempo
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

					note_array[i].display_vector[0].remaining_ticks = -1;     // Remove circulo

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
			case DRAG:

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
		}
	}

RETORNA_CLIQUE:
	//	printf("Tipo da nota (posicao: %d) clicada: %d\n", (*pos), note_array[*pos].type);
	return click_result;
}


int check_drag_note(Note* note, int* x, int* y){

	SDL_GetMouseState(x, y);

	double closest_coordinate_inline_y;
	double closest_coordinate_inline_x;

	double mouse_relative_x = (*x) - note->display_vector[1].x;
	double mouse_relative_y = (*y) - note->display_vector[1].base_y;

	double dx = note->display_vector[1].s.x2 - note->display_vector[1].x;
	double dy = note->display_vector[1].s.y2 - note->display_vector[1].y;

	double line_length_squared = dx * dx + dy * dy;

	// Calcula proporcao da linha navegada
	double position_inline = (mouse_relative_x * dx + mouse_relative_y * dy)/line_length_squared;


	// Mantem circulo na linha
	position_inline = fmax(0.0, fmin(1.0, position_inline));

	closest_coordinate_inline_x = ( (double) note->display_vector[1].x + position_inline * dx);
	closest_coordinate_inline_y = ( (double) note->display_vector[1].y + position_inline * dy);

	// Passa coordenada achada para o circulo
        note->display_vector[0].x = closest_coordinate_inline_x;
        note->display_vector[0].y = closest_coordinate_inline_y;

	//	printf("x: %d | mousex %d\ny: %d | mousey: %d\nx1: %d\n", note->display_vector[0].x, *x, note->display_vector[0].y, *y, note->display_vector[1].x);

	int drag_result = INT_MIN;

	// Checa se o mouse NAO esta dentro do cilindro formado pelos dois circulos
	if ( ( (((double) (*x) - closest_coordinate_inline_x) *  ( (double) (*x) - closest_coordinate_inline_x) + ( (double) (*y) - closest_coordinate_inline_y) * ( (double) (*y) - closest_coordinate_inline_y)) > (note->display_vector[0].c.radius * note->display_vector[0].c.radius ) )
	   ){
		// Se o mouse nao estiver dentro desta area, falha o drag
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
		case NOTE_CLICK: // Evento que cria efeito ao clicar

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
		case NOTE_DRAG:

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


void initiate_note(Note* note){
	// Coloca tamanho do display da nota
	switch(note->type){
		case CLICK:
		case MULTI_CLICK:
			note->size = 1;
			break;
		case DRAG:
			note->size = 3;
			break;
	}

	// Aloca vetor de display
	note->display_vector = (Note_display*) malloc( (note->size)* sizeof(Note_display));

	// Aloca imagens a serem usadas
	switch(note->type){
		case CLICK:
		case MULTI_CLICK:
		case DRAG:
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
		case DRAG:
			note->display_vector[0].type = CIRCLE;	// Circulo inicial
			note->display_vector[1].type = STRAIGHT_LINE;	// Percurso
			note->display_vector[2].type = CIRCLE;	// Circulo final
			break;
	}
}

int draw_note(SDL_Renderer* renderer, Note* note){
	int draw_note_result = INT_MIN;
	if (!note->display_vector){
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
				if(note->display_vector[0].remaining_ticks < 0){
					printf("\033[34mNota expirada! (-1 ponto)\033[0m\n");
					draw_note_result = 0;
				}
			}
			break;
		case DRAG:
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

	}

RETORNA_DESENHO_NOTA:
	return draw_note_result;
}
