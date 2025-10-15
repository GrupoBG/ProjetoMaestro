#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>


#define PARTITION_WIDTH 266      
#define WINDOW_WIDTH 800        
#define WINDOW_HEIGHT 600      
#define MAX_CIRCLES 100
#define GAME_DURATION 15000     // Duracao do jogo em milissegundos

// Estrutura para representar um circulo
typedef struct {
    int x, y;                   
    int radius;                 
    int partition;              
    int remaining_ticks;        // Tempo restante de vida
    int base_ticks;            // Tempo total de vida
    SDL_Color color;           
    int fade_in_ticks;         // Contador para efeito de fade in
} Circle;

// Estrutura para manter o estado do jogo
typedef struct {
    Circle circles[MAX_CIRCLES]; // Array de circulos
    int count;                  // Quantidade atual de circulos
    int score;                  // Pontuacao do jogador
    float spawn_timer;          // Temporizador para criar novos circulos
    Uint32 tick_counter;        // Contador de ticks do jogo
} GameState;


void generate_random_circle(Circle* circle) {
    circle->partition = rand() % 3;  // Escolhe uma das tres particoes
    circle->base_ticks = 180 + (rand() % 300);
    circle->remaining_ticks = circle->base_ticks;
    // Adjust circle size for smaller window
    circle->radius = rand() % 30 + 15; 
    
    // Posiciona o circulo dentro da particao escolhida
    circle->x = circle->radius + 
                (rand() % (PARTITION_WIDTH - 2*circle->radius)) + 
                (circle->partition * PARTITION_WIDTH);
    circle->y = circle->radius + 
                (rand() % (WINDOW_HEIGHT - 2*circle->radius));
    
    circle->color.r = rand() % 256;
    circle->color.g = rand() % 256;
    circle->color.b = rand() % 256;
    circle->color.a = 0;  // Comeca transparente para o fade in
    circle->fade_in_ticks = 60;  // 1 segundo de fade in
}

// Atualiza o estado de todos os circulos ativos
void update_circles(GameState* game) {
    for (int i = 0; i < game->count; i++) {
        Circle* circle = &game->circles[i];
        
        // Gerencia o fade in inicial
        if (circle->fade_in_ticks > 0) {
            circle->fade_in_ticks--;
            circle->color.a = 255 - (circle->fade_in_ticks * 255 / 60);
            continue;
        }
        
        // Gerencia o fade out normal
        if (circle->remaining_ticks > circle->base_ticks/2) {
            circle->color.a = 255;  // Totalmente visivel na primeira metade
        } else if (circle->remaining_ticks > 0) {
            // Fade out na segunda metade da vida
            circle->color.a = (255 * circle->remaining_ticks) / (circle->base_ticks/2);
        }

        // Atualiza tempo de vida e pontuacao
        if (circle->remaining_ticks >= 0) {
            circle->remaining_ticks--;
            if (circle->remaining_ticks < 0 && game->score > 0) {
                game->score--;
                printf("Circulo expirado! (-1 ponto)\n");
            }
        }
    }

    // Remove circulos expirados do array
    int write = 0;
    for (int read = 0; read < game->count; read++) {
        if (game->circles[read].remaining_ticks >= 0) {
            if (write != read) {
                game->circles[write] = game->circles[read];
            }
            write++;
        }
    }
    game->count = write;
}

// Verifica colisao do mouse com circulos
bool check_collision(GameState* game, int mouse_x, int mouse_y) {
    for (int i = game->count - 1; i >= 0; i--) {
        Circle* circle = &game->circles[i];
        if (circle->remaining_ticks >= 0) {
            int dx = mouse_x - circle->x;
            int dy = mouse_y - circle->y;
            if ((dx * dx + dy * dy) <= (circle->radius * circle->radius)) {
                circle->remaining_ticks = -1;
                game->score++;
                printf("Acerto! (+1 ponto)\n");
                return true;
            }
        }
    }
    if (game->score > 0) {
        game->score--;
        printf("Erro! (-1 ponto)\n");
    }
    return false;
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Prototipo", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Inicializa estado do jogo
    GameState game = {
        .count = 0,
        .score = 0,
        .spawn_timer = 0,
        .tick_counter = 0
    };

    srand(time(NULL));
    
    // Gera alguns circulos iniciais
    for (int i = 0; i < 5; i++) {
        if (game.count < MAX_CIRCLES) {
            generate_random_circle(&game.circles[game.count]);
            game.count++;
            printf("Círculo inicial gerado: x=%d, y=%d, r=%d, a=%d\n", 
                   game.circles[game.count-1].x,
                   game.circles[game.count-1].y,
                   game.circles[game.count-1].radius,
                   game.circles[game.count-1].color.a);
        }
    }

    bool running = true;
    Uint32 lastTick = SDL_GetTicks();
    Uint32 gameStartTime = lastTick;

    while (running) {
        // Verifica se o tempo de jogo acabou
        if (SDL_GetTicks() - gameStartTime >= GAME_DURATION) {
            running = false;
            continue;
        }

        // Processa eventos com timeout
        SDL_Event event;
        if (SDL_WaitEventTimeout(&event, 16)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouse_x, mouse_y;
                SDL_GetMouseState(&mouse_x, &mouse_y);
                check_collision(&game, mouse_x, mouse_y);
            }
        }

        // Mostra tempo restante
        printf("\rTempo restante: %.1f segundos", 
               (GAME_DURATION - (SDL_GetTicks() - gameStartTime)) / 1000.0f);
        fflush(stdout);

        // Atualiza estado do jogo
        Uint32 currentTick = SDL_GetTicks();
        float deltaTime = (currentTick - lastTick) / 1000.0f;
        lastTick = currentTick;

        game.tick_counter++;
        game.spawn_timer -= deltaTime;

        // Gera novo circulo quando necessario
        if (game.spawn_timer <= 0 && game.count < MAX_CIRCLES) {
            generate_random_circle(&game.circles[game.count]);
            game.count++;
            game.spawn_timer = 0.5f + (rand() % 1000) / 1000.0f;
            printf("Novo círculo gerado: x=%d, y=%d, r=%d, a=%d\n", 
                   game.circles[game.count-1].x,
                   game.circles[game.count-1].y,
                   game.circles[game.count-1].radius,
                   game.circles[game.count-1].color.a);
        }

        // Atualiza e renderiza
        update_circles(&game);

        // Limpa a tela
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Desenha linhas das particoes
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, PARTITION_WIDTH, 0, PARTITION_WIDTH, WINDOW_HEIGHT);
        SDL_RenderDrawLine(renderer, 2*PARTITION_WIDTH, 0, 2*PARTITION_WIDTH, WINDOW_HEIGHT);

        // Desenha todos os circulos ativos
        for (int i = 0; i < game.count; i++) {
            Circle* circle = &game.circles[i];
            if (circle->remaining_ticks >= 0) {
                filledCircleRGBA(renderer, circle->x, circle->y, circle->radius,
                    circle->color.r, circle->color.g, circle->color.b, circle->color.a);
            }
        }

        SDL_RenderPresent(renderer);
    }

    // Mostra pontuacao final
    printf("\nTempo acabou! Pontuacao final: %d\n", game.score);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
