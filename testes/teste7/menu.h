#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <assert.h>

typedef enum {
    STATE_MENU,
    STATE_GAME,
    STATE_OPTIONS,
    STATE_CREDITS,
    STATE_QUIT
} GameState;

typedef enum {
    MENU_JOGAR,
    MENU_OPCOES,
    MENU_CREDITOS,
    MENU_SAIR,
    MENU_ITEM_COUNT // Contador de itens
} MenuItem;

// Estrutura para um item do menu (botão)
typedef struct {
    SDL_Rect rect;          
    char* text;             
    SDL_Texture* texture;   // Textura para o estado normal
    SDL_Texture* texture_hover; // Textura para o estado "hover" (mouse em cima)
    MenuItem action;        // Ação que o botão dispara
} MenuButton;


static inline void menu_create_text(SDL_Renderer* ren, SDL_Texture** text, char* text_content, SDL_Color color, TTF_Font* fnt) {
    assert(fnt);
    SDL_Surface* sfc = TTF_RenderText_Blended(fnt, text_content, color);
    if (!sfc) {
        printf("Erro ao criar superfície TTF: %s\n", TTF_GetError());
        return;
    }
    (*text) = SDL_CreateTextureFromSurface(ren, sfc);
    if (!*text) {
        printf("Erro ao criar textura da superfície: %s\n", SDL_GetError());
    }
    SDL_FreeSurface(sfc);
}


static inline int menu_AUX_WaitEventTimeoutCount(SDL_Event* evt, Uint32* ms) {
    Uint32 antes = SDL_GetTicks();
    int isevt = SDL_WaitEventTimeout(evt, *ms);
    Uint32 decorrido = SDL_GetTicks() - antes;
    *ms = (*ms > decorrido) ? (*ms - decorrido) : 0;
    return isevt;
}


static inline GameState run_menu(SDL_Renderer* renderer, TTF_Font* fnt, int window_w, int window_h) {

    MenuButton buttons[MENU_ITEM_COUNT];
    const char* labels[MENU_ITEM_COUNT] = {"Jogar", "Opcoes", "Creditos", "Sair"};

    SDL_Color color_normal = {0xFF, 0xFF, 0xFF, 0xFF}; // Branco
    SDL_Color color_hover = {0xFF, 0xFF, 0x00, 0xFF};  // Amarelo para hover

    int button_w = 300;
    int button_h = 70;
    int spacing = 30;
    int start_y = (window_h - (MENU_ITEM_COUNT * button_h + (MENU_ITEM_COUNT - 1) * spacing)) / 2;
    int start_x = (window_w - button_w) / 2;

    // Inicializa e cria as texturas dos botoes
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        buttons[i].text = (char*)labels[i];
        buttons[i].action = (MenuItem)i;
        buttons[i].rect.x = start_x;
        buttons[i].rect.y = start_y + i * (button_h + spacing);
        buttons[i].rect.w = button_w;
        buttons[i].rect.h = button_h;

        // Criar texturas
        menu_create_text(renderer, &buttons[i].texture, buttons[i].text, color_normal, fnt);
        menu_create_text(renderer, &buttons[i].texture_hover, buttons[i].text, color_hover, fnt);

        if (!buttons[i].texture || !buttons[i].texture_hover) {
            printf("Erro ao criar textura para o botao %s: %s\n", labels[i], SDL_GetError());
            return STATE_QUIT; // Falha
        }
    }

    bool running = true;
    GameState next_state = STATE_MENU;
    int mouse_x = 0, mouse_y = 0;
    SDL_GetMouseState(&mouse_x, &mouse_y); // Pega o estado inicial do mouse

    Uint32 tickTime = 17;
    SDL_Event event;

    // Ignora eventos de movimento de mouse para nao acordar o loop desnecessariamente
    // O estado do mouse e pego manualmente
    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

    while (running) {
        
        int isevt = menu_AUX_WaitEventTimeoutCount(&event, &tickTime);

        if (isevt) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    next_state = STATE_QUIT;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        SDL_GetMouseState(&mouse_x, &mouse_y);
                        SDL_Point mouse_pos = {mouse_x, mouse_y};
                        
                        for (int i = 0; i < MENU_ITEM_COUNT; i++) {
                            if (SDL_PointInRect(&mouse_pos, &buttons[i].rect)) {
                                // Botão clicado
                                switch (buttons[i].action) {
                                    case MENU_JOGAR:
                                        next_state = STATE_GAME;
                                        running = false;
                                        break;
                                    case MENU_OPCOES:
                                        // Acao de "Opcoes" (nao implementada)
                                        printf("Opcoes clicado (nao implementado)\n");
                                        // next_state = STATE_OPTIONS;
                                        // running = false;
                                        break;
                                    case MENU_CREDITOS:
                                        // Acao de "Creditos" (nao implementada)
                                        printf("Creditos clicado (nao implementado)\n");
                                        // next_state = STATE_CREDITS;
                                        // running = false;
                                        break;
                                    case MENU_SAIR:
                                        next_state = STATE_QUIT;
                                        running = false;
                                        break;
                                    default:
                                        break;
                                }
                            }
                        }
                    }
                    break;
            }
        } else {
            // Timeout (sem evento)
            tickTime = 17; // Reinicia o tempo de espera
        }

        // --- Renderizacao ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fundo preto
        SDL_RenderClear(renderer);

        // Atualiza a posicao do mouse para o "hover"
        SDL_GetMouseState(&mouse_x, &mouse_y);
        SDL_Point mouse_pos = {mouse_x, mouse_y};

        // Desenhar botoes
        for (int i = 0; i < MENU_ITEM_COUNT; i++) {
            SDL_Texture* tex_to_render;

            // Verifica se o mouse esta sobre o botao
            if (SDL_PointInRect(&mouse_pos, &buttons[i].rect)) {
                tex_to_render = buttons[i].texture_hover;
            } else {
                tex_to_render = buttons[i].texture;
            }

            // Centraliza a textura dentro do retangulo do botao
            int tex_w, tex_h;
            SDL_QueryTexture(tex_to_render, NULL, NULL, &tex_w, &tex_h);
            
            SDL_Rect dest_rect;
            dest_rect.w = tex_w;
            dest_rect.h = tex_h;
            dest_rect.x = buttons[i].rect.x + (buttons[i].rect.w - tex_w) / 2;
            dest_rect.y = buttons[i].rect.y + (buttons[i].rect.h - tex_h) / 2;

            SDL_RenderCopy(renderer, tex_to_render, NULL, &dest_rect);
        }

        SDL_RenderPresent(renderer);
    }
    
    // Habilita eventos de mouse novamente para o jogo
    SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);

    // Limpeza das texturas do menu
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        SDL_DestroyTexture(buttons[i].texture);
        SDL_DestroyTexture(buttons[i].texture_hover);
    }

    return next_state;
}

#endif // MENU_H