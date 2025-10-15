#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdbool.h>

#ifdef __linux__
#include <unistd.h>
#endif

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BUTTON_WIDTH 200
#define BUTTON_HEIGHT 50
#define BUTTON_SPACING 20

typedef struct {
    SDL_Rect rect;
    SDL_Texture* texture;
    const char* text;
} Button;


int AUX_WaitEventTimeout(SDL_Event* ev, int timeout) {
    Uint32 start = SDL_GetTicks();
    while (1) {
        if (SDL_WaitEventTimeout(ev, timeout))
            return 1;
        if ((SDL_GetTicks() - start) >= timeout)
            return 0;
    }
}

void create_button(Button* button, SDL_Renderer* renderer, TTF_Font* font, 
                  const char* text, int y_position) {
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, white);
    button->texture = SDL_CreateTextureFromSurface(renderer, surface);
    button->text = text;
    
    button->rect.x = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
    button->rect.y = y_position;
    button->rect.w = BUTTON_WIDTH;
    button->rect.h = BUTTON_HEIGHT;
    
    SDL_FreeSurface(surface);
}

bool point_in_rect(int x, int y, SDL_Rect* rect) {
    return (x >= rect->x && x <= rect->x + rect->w &&
            y >= rect->y && y <= rect->y + rect->h);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    SDL_Window* window = SDL_CreateWindow("Projeto Maestro",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("Roboto-Regular.ttf", 32);
    if (!font) {
        printf("Nao carregou fonte: %s\n", TTF_GetError());
        return 1;
    }

    TTF_SetFontHinting(font, TTF_HINTING_LIGHT);

    Button buttons[4];
    create_button(&buttons[0], renderer, font, "Jogar", 200);
    create_button(&buttons[1], renderer, font, "Opcoes", 200 + BUTTON_HEIGHT + BUTTON_SPACING);
    create_button(&buttons[2], renderer, font, "Creditos", 200 + 2 * (BUTTON_HEIGHT + BUTTON_SPACING));
    create_button(&buttons[3], renderer, font, "Sair", 200 + 3 * (BUTTON_HEIGHT + BUTTON_SPACING));

    bool running = true;
    while (running) {
        SDL_Event event;
        if (AUX_WaitEventTimeout(&event, 16)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouse_x = event.button.x;
                int mouse_y = event.button.y;

                if (point_in_rect(mouse_x, mouse_y, &buttons[0].rect)) {
                    #ifdef __linux__
                    pid_t pid = fork();
                    if (pid == 0) { 
                        execl("./main", "./main", NULL);
                        exit(1);
                    }
                    #endif
                }
                else if (point_in_rect(mouse_x, mouse_y, &buttons[3].rect)) {
                    running = false;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < 4; i++) {
            SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
            SDL_RenderFillRect(renderer, &buttons[i].rect);
            
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
            SDL_RenderDrawRect(renderer, &buttons[i].rect);
            
            SDL_RenderCopy(renderer, buttons[i].texture, NULL, &buttons[i].rect);
        }

        SDL_RenderPresent(renderer);
    }

    for (int i = 0; i < 4; i++) {
        SDL_DestroyTexture(buttons[i].texture);
    }
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}