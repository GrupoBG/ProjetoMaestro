#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500
#define GAME_DURATION 15000  // Tempo de duracao do jogo em milissegundos

typedef struct {
    int x, y;
    int radius;
    SDL_Color color;
} Circle;

void generate_random_circle(Circle* circle) {
    circle->x = rand() % (WINDOW_WIDTH - 2 * circle->radius) + circle->radius;
    circle->y = rand() % (WINDOW_HEIGHT - 2 * circle->radius) + circle->radius;
    circle->radius = rand() % 50 + 20;  // Raio entre 20 e 70 pixels

    circle->color.r = rand() % 256;
    circle->color.g = rand() % 256;
    circle->color.b = rand() % 256;
    circle->color.a = 255;
}

int check_collision(int mouse_x, int mouse_y, Circle* circle) {
    int dx = mouse_x - circle->x;
    int dy = mouse_y - circle->y;
    return (dx * dx + dy * dy) <= (circle->radius * circle->radius);
}

int main(int argc, char* argv[]) {
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

    srand(time(NULL));  // Geracao aleatoria

    int score = 0;
    int running = 1;
    Uint32 start_time = SDL_GetTicks();
    Circle circle;
    generate_random_circle(&circle);

    SDL_Event event;
    while (running) {
        Uint32 current_time = SDL_GetTicks();
        if (current_time - start_time >= GAME_DURATION) {
            running = 0;  // Termina o jogo apos X segundos
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, circle.color.r, circle.color.g, circle.color.b, circle.color.a);
        for (int w = 0; w < circle.radius * 2; w++) {
            for (int h = 0; h < circle.radius * 2; h++) {
                int dx = circle.radius - w;
                int dy = circle.radius - h;
                if ((dx * dx + dy * dy) <= (circle.radius * circle.radius)) {
                    SDL_RenderDrawPoint(renderer, circle.x + dx, circle.y + dy);
                }
            }
        }

        if (SDL_WaitEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouse_x, mouse_y;
                SDL_GetMouseState(&mouse_x, &mouse_y);

                // Verifica se acertou o clique no circulo
                if (check_collision(mouse_x, mouse_y, &circle)) {
                    score++;  // Aumenta a pontuacao
                    generate_random_circle(&circle);  // Gera um novo circulo
                }
            }
        }

        SDL_RenderPresent(renderer);
    }

    printf("Fim de Jogo! Sua pontuacao e: %d\n", score);


    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    
    SDL_Quit();

    return 0;
}
