#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

typedef enum {
    STATE_MENU,
    STATE_GAME,
    STATE_OPTIONS,
    STATE_MUSIC_SELECT,
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

typedef enum {
    OPTIONS_VOLUME,
    OPTIONS_BACK,
    OPTIONS_ITEM_COUNT // Contador de itens das opcoes
} OptionsMenuItem;

typedef enum {
    MUSIC_MOONLIGHT,
    MUSIC_2,
    MUSIC_3,
    MUSIC_4,
    MUSIC_5,
    MUSIC_BACK,
    MUSIC_ITEM_COUNT // Contador de itens do menu de música
} MusicMenuItem;

// Estrutura para um item do menu (botão)
typedef struct {
    SDL_Rect rect;          
    char* text;             
    SDL_Texture* texture;   // Textura para o estado normal
    SDL_Texture* texture_hover; // Textura para o estado "hover" (mouse em cima)
    MenuItem action;        // Ação que o botão dispara
} MenuButton;

// Estrutura para um item do submenu de opções
typedef struct {
    SDL_Rect rect;
    char* text;
    SDL_Texture* texture;
    SDL_Texture* texture_hover;
    OptionsMenuItem action;
} OptionsButton;

// Estrutura para um item do menu de seleção de música
typedef struct {
    SDL_Rect rect;
    char* text;
    SDL_Texture* texture;
    SDL_Texture* texture_hover;
    MusicMenuItem action;
} MusicButton;

// Estrutura para botões de controle (+ e -)
typedef struct {
    SDL_Rect rect;
    char* text;
    SDL_Texture* texture;
    SDL_Texture* texture_hover;
    int type; // 0 = menos, 1 = mais
} ControlButton;


static inline void menu_create_text(SDL_Renderer* ren, SDL_Texture** text, char* text_content, SDL_Color color, TTF_Font* fnt) {
    assert(fnt);
    assert(text_content != NULL);
    
    SDL_Surface* sfc = TTF_RenderText_Blended(fnt, text_content, color);
    if (!sfc) {
        printf("Erro ao criar superfície TTF para '%s': %s\n", text_content, TTF_GetError());
        return;
    }
    (*text) = SDL_CreateTextureFromSurface(ren, sfc);
    if (!*text) {
        printf("Erro ao criar textura da superfície para '%s': %s\n", text_content, SDL_GetError());
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


// Função para exibir o menu de seleção de música
static inline GameState run_music_select_menu(SDL_Renderer* renderer, TTF_Font* fnt, int window_w, int window_h) {
    
    MusicButton buttons[MUSIC_ITEM_COUNT];
    const char* labels[MUSIC_ITEM_COUNT] = {
        "Moonlight Sonata Aleatoria",
        "Musica 2",
        "Musica 3",
        "Musica 4",
        "Musica 5",
        "Voltar"
    };
    
    SDL_Color color_normal = {0xFF, 0xFF, 0xFF, 0xFF}; // Branco
    SDL_Color color_hover = {0xFF, 0xFF, 0x00, 0xFF};  // Amarelo para hover
    
    int button_w = 400;
    int button_h = 60;
    int spacing = 20;
    
    // Posição do título "Selecione uma Música"
    int title_y = 50;
    
    // Posição inicial dos botões de música
    int start_y = 150;
    int start_x = (window_w - button_w) / 2;
    
    // Inicializa e cria as texturas dos botões
    for (int i = 0; i < MUSIC_ITEM_COUNT; i++) {
        buttons[i].text = (char*)labels[i];
        buttons[i].action = (MusicMenuItem)i;
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
        
        printf("Botão música %d criado em Y=%d\n", i, buttons[i].rect.y);
    }

    bool running = true;
    GameState next_state = STATE_MUSIC_SELECT;
    int mouse_x = 0, mouse_y = 0;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    Uint32 tickTime = 17;
    SDL_Event event;

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
                        
                        for (int i = 0; i < MUSIC_ITEM_COUNT; i++) {
                            if (SDL_PointInRect(&mouse_pos, &buttons[i].rect)) {
                                // Botão clicado
                                printf("Botão de música clicado: %s\n", buttons[i].text);
                                
                                switch (buttons[i].action) {
                                    case MUSIC_MOONLIGHT:
                                        printf("Iniciando jogo com Moonlight Sonata...\n");
                                        next_state = STATE_GAME;
                                        running = false;
                                        break;
                                    case MUSIC_2:
                                    case MUSIC_3:
                                    case MUSIC_4:
                                    case MUSIC_5:
                                        printf("Musica %s nao foi implementada ainda!\n", buttons[i].text);
                                        break;
                                    case MUSIC_BACK:
                                        printf("Voltando ao menu principal...\n");
                                        next_state = STATE_MENU;
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
            tickTime = 17;
        }

        // --- Renderização ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fundo preto
        SDL_RenderClear(renderer);

        SDL_GetMouseState(&mouse_x, &mouse_y);
        SDL_Point mouse_pos = {mouse_x, mouse_y};

        // Desenha título "Selecione uma Música"
        SDL_Color white = {0xFF, 0xFF, 0xFF, 0xFF};
        SDL_Texture* title_tex = NULL;
        menu_create_text(renderer, &title_tex, "Selecione uma Musica", white, fnt);
        
        if (title_tex) {
            int tex_w, tex_h;
            SDL_QueryTexture(title_tex, NULL, NULL, &tex_w, &tex_h);
            SDL_Rect title_rect = {(window_w - tex_w) / 2, title_y, tex_w, tex_h};
            SDL_RenderCopy(renderer, title_tex, NULL, &title_rect);
            SDL_DestroyTexture(title_tex);
        }

        // Desenhar botões
        for (int i = 0; i < MUSIC_ITEM_COUNT; i++) {
            SDL_Texture* tex_to_render;

            // Verifica se o mouse está sobre o botão
            if (SDL_PointInRect(&mouse_pos, &buttons[i].rect)) {
                tex_to_render = buttons[i].texture_hover;
            } else {
                tex_to_render = buttons[i].texture;
            }

            // Centraliza a textura dentro do retângulo do botão
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
    
    SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);

    // Limpeza das texturas
    for (int i = 0; i < MUSIC_ITEM_COUNT; i++) {
        SDL_DestroyTexture(buttons[i].texture);
        SDL_DestroyTexture(buttons[i].texture_hover);
    }

    return next_state;
}


// Função para exibir o submenu de opções
// O parâmetro volume é um ponteiro para modificar o volume do jogo
static inline GameState run_options_menu(SDL_Renderer* renderer, TTF_Font* fnt, int window_w, int window_h, int* volume) {
    
    OptionsButton buttons[OPTIONS_ITEM_COUNT];
    
    ControlButton control_buttons[2]; // Botões + e -
    
    SDL_Color color_normal = {0xFF, 0xFF, 0xFF, 0xFF}; // Branco
    SDL_Color color_hover = {0xFF, 0xFF, 0x00, 0xFF};  // Amarelo para hover
    
    int button_w = 300;
    int button_h = 70;
    int spacing = 30;
    
    // Posição do título "Opções"
    int title_y = 50;
    
    // Posição do controle de volume (label + botões)
    int volume_y = 250;
    int control_button_w = 80;
    int control_button_h = 60;
    int volume_display_x = window_w / 2;
    
    // Posição do botão "Voltar" (bem embaixo da opção de volume)
    int back_button_y = volume_y + 150; // Posicionado abaixo da barra de volume
    int back_button_x = (window_w - button_w) / 2;
    
    // Inicializa botão "Voltar" - usando string direta
    buttons[OPTIONS_BACK].text = "Voltar";
    buttons[OPTIONS_BACK].action = OPTIONS_BACK;
    buttons[OPTIONS_BACK].rect.x = back_button_x;
    buttons[OPTIONS_BACK].rect.y = back_button_y;
    buttons[OPTIONS_BACK].rect.w = button_w;
    buttons[OPTIONS_BACK].rect.h = button_h;
    
    menu_create_text(renderer, &buttons[OPTIONS_BACK].texture, "Voltar", color_normal, fnt);
    menu_create_text(renderer, &buttons[OPTIONS_BACK].texture_hover, "Voltar", color_hover, fnt);
    
    printf("Posição do botão Voltar: X=%d, Y=%d, W=%d, H=%d\n", 
           buttons[OPTIONS_BACK].rect.x, 
           buttons[OPTIONS_BACK].rect.y, 
           buttons[OPTIONS_BACK].rect.w, 
           buttons[OPTIONS_BACK].rect.h);
    
    // Inicializa botão "-" (diminuir volume)
    control_buttons[0].rect.x = volume_display_x - 200;
    control_buttons[0].rect.y = volume_y;
    control_buttons[0].rect.w = control_button_w;
    control_buttons[0].rect.h = control_button_h;
    control_buttons[0].text = "-";
    control_buttons[0].type = 0;
    
    menu_create_text(renderer, &control_buttons[0].texture, "-", color_normal, fnt);
    menu_create_text(renderer, &control_buttons[0].texture_hover, "-", color_hover, fnt);
    
    // Inicializa botão "+" (aumentar volume)
    control_buttons[1].rect.x = volume_display_x + 120;
    control_buttons[1].rect.y = volume_y;
    control_buttons[1].rect.w = control_button_w;
    control_buttons[1].rect.h = control_button_h;
    control_buttons[1].text = "+";
    control_buttons[1].type = 1;
    
    menu_create_text(renderer, &control_buttons[1].texture, "+", color_normal, fnt);
    menu_create_text(renderer, &control_buttons[1].texture_hover, "+", color_hover, fnt);
    
    bool running = true;
    GameState next_state = STATE_OPTIONS;
    int mouse_x = 0, mouse_y = 0;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    
    Uint32 tickTime = 17;
    SDL_Event event;
    
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
                        
                        // Verifica botões de controle de volume
                        for (int i = 0; i < 2; i++) {
                            if (SDL_PointInRect(&mouse_pos, &control_buttons[i].rect)) {
                                if (control_buttons[i].type == 0) {
                                    // Botão "-" clicado - diminui o volume
                                    if (*volume > 0) {
                                        (*volume)--;
                                        // Aplica o volume ao mixer de áudio SDL
                                        int mixer_volume = ((*volume) * SDL_MIX_MAXVOLUME) / 10;
                                        Mix_MasterVolume(mixer_volume);
                                        printf("Volume diminuído para: %d%%\n", *volume * 10);
                                    }
                                } else if (control_buttons[i].type == 1) {
                                    // Botão "+" clicado - aumenta o volume
                                    if (*volume < 10) {
                                        (*volume)++;
                                        // Aplica o volume ao mixer de áudio SDL
                                        int mixer_volume = ((*volume) * SDL_MIX_MAXVOLUME) / 10;
                                        Mix_MasterVolume(mixer_volume);
                                        printf("Volume aumentado para: %d%%\n", *volume * 10);
                                    }
                                }
                            }
                        }
                        
                        // Verifica botão "Voltar"
                        if (SDL_PointInRect(&mouse_pos, &buttons[OPTIONS_BACK].rect)) {
                            printf("Voltando ao menu principal...\n");
                            next_state = STATE_MENU;
                            running = false;
                        }
                    }
                    break;
            }
        } else {
            tickTime = 17;
        }
        
        // --- Renderização ---
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Fundo preto
        SDL_RenderClear(renderer);
        
        SDL_GetMouseState(&mouse_x, &mouse_y);
        SDL_Point mouse_pos = {mouse_x, mouse_y};
        
        // Desenha título "Opções"
        SDL_Color white = {0xFF, 0xFF, 0xFF, 0xFF};
        SDL_Texture* title_tex = NULL;
        menu_create_text(renderer, &title_tex, "Opcoes", white, fnt);
        
        if (title_tex) {
            int tex_w, tex_h;
            SDL_QueryTexture(title_tex, NULL, NULL, &tex_w, &tex_h);
            SDL_Rect title_rect = {(window_w - tex_w) / 2, title_y, tex_w, tex_h};
            SDL_RenderCopy(renderer, title_tex, NULL, &title_rect);
            SDL_DestroyTexture(title_tex);
        }
        
        // Desenha label "Volume"
        SDL_Texture* volume_label_tex = NULL;
        menu_create_text(renderer, &volume_label_tex, "Volume", white, fnt);
        
        if (volume_label_tex) {
            int tex_w, tex_h;
            SDL_QueryTexture(volume_label_tex, NULL, NULL, &tex_w, &tex_h);
            SDL_Rect label_rect = {volume_display_x - tex_w / 2 - 100, volume_y - 60, tex_w, tex_h};
            SDL_RenderCopy(renderer, volume_label_tex, NULL, &label_rect);
            SDL_DestroyTexture(volume_label_tex);
        }
        
        // Desenha o valor atual do volume (0-100%)
        char volume_str[16];
        snprintf(volume_str, sizeof(volume_str), "%d%%", *volume * 10);
        SDL_Texture* volume_value_tex = NULL;
        menu_create_text(renderer, &volume_value_tex, volume_str, white, fnt);
        
        if (volume_value_tex) {
            int tex_w, tex_h;
            SDL_QueryTexture(volume_value_tex, NULL, NULL, &tex_w, &tex_h);
            SDL_Rect value_rect = {volume_display_x - tex_w / 2, volume_y, tex_w, tex_h};
            SDL_RenderCopy(renderer, volume_value_tex, NULL, &value_rect);
            SDL_DestroyTexture(volume_value_tex);
        }
        
        // Desenha botões de controle (- e +)
        for (int i = 0; i < 2; i++) {
            SDL_Texture* tex_to_render;
            
            if (SDL_PointInRect(&mouse_pos, &control_buttons[i].rect)) {
                tex_to_render = control_buttons[i].texture_hover;
            } else {
                tex_to_render = control_buttons[i].texture;
            }
            
            int tex_w, tex_h;
            SDL_QueryTexture(tex_to_render, NULL, NULL, &tex_w, &tex_h);
            
            SDL_Rect dest_rect;
            dest_rect.w = tex_w;
            dest_rect.h = tex_h;
            dest_rect.x = control_buttons[i].rect.x + (control_buttons[i].rect.w - tex_w) / 2;
            dest_rect.y = control_buttons[i].rect.y + (control_buttons[i].rect.h - tex_h) / 2;
            
            SDL_RenderCopy(renderer, tex_to_render, NULL, &dest_rect);
        }
        
        // Desenha barra visual de volume (retângulo)
        int bar_width = 200;
        int bar_height = 20;
        int bar_x = volume_display_x - bar_width / 2;
        int bar_y = volume_y + 80;
        
        // Fundo da barra (cinza)
        SDL_Rect bar_bg = {bar_x, bar_y, bar_width, bar_height};
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &bar_bg);
        
        // Barra de volume preenchida (verde)
        int filled_width = (bar_width * (*volume)) / 10;
        SDL_Rect bar_fill = {bar_x, bar_y, filled_width, bar_height};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &bar_fill);
        
        // Borda da barra
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &bar_bg);
        
        // Desenha botão "Voltar"
        if (buttons[OPTIONS_BACK].texture && buttons[OPTIONS_BACK].texture_hover) {
            SDL_Texture* back_tex_to_render;
            
            if (SDL_PointInRect(&mouse_pos, &buttons[OPTIONS_BACK].rect)) {
                back_tex_to_render = buttons[OPTIONS_BACK].texture_hover;
            } else {
                back_tex_to_render = buttons[OPTIONS_BACK].texture;
            }
            
            int back_tex_w, back_tex_h;
            SDL_QueryTexture(back_tex_to_render, NULL, NULL, &back_tex_w, &back_tex_h);
            
            SDL_Rect back_dest_rect;
            back_dest_rect.w = back_tex_w;
            back_dest_rect.h = back_tex_h;
            back_dest_rect.x = buttons[OPTIONS_BACK].rect.x + (buttons[OPTIONS_BACK].rect.w - back_tex_w) / 2;
            back_dest_rect.y = buttons[OPTIONS_BACK].rect.y + (buttons[OPTIONS_BACK].rect.h - back_tex_h) / 2;
            
            SDL_RenderCopy(renderer, back_tex_to_render, NULL, &back_dest_rect);
        }
        
        SDL_RenderPresent(renderer);
    }
    
    SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);
    
    // Limpeza das texturas do submenu de opções
    if (buttons[OPTIONS_BACK].texture) {
        SDL_DestroyTexture(buttons[OPTIONS_BACK].texture);
    }
    if (buttons[OPTIONS_BACK].texture_hover) {
        SDL_DestroyTexture(buttons[OPTIONS_BACK].texture_hover);
    }
    for (int i = 0; i < 2; i++) {
        if (control_buttons[i].texture) {
            SDL_DestroyTexture(control_buttons[i].texture);
        }
        if (control_buttons[i].texture_hover) {
            SDL_DestroyTexture(control_buttons[i].texture_hover);
        }
    }
    
    return next_state;
}


static inline GameState run_menu(SDL_Renderer* renderer, TTF_Font* fnt, int window_w, int window_h, int* global_volume) {

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
                                        printf("Jogar clicado - abrindo seleção de música\n");
                                        next_state = run_music_select_menu(renderer, fnt, window_w, window_h);
                                        if (next_state == STATE_GAME) {
                                            running = false;
                                        }
                                        break;
                                    case MENU_OPCOES:
                                        // Abre o submenu de opções, passando o ponteiro global de volume
                                        printf("Opcoes clicado\n");
                                        run_options_menu(renderer, fnt, window_w, window_h, global_volume);
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