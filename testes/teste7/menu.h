//  gcc main.c -o main -lSDL2 -lSDL2_mixer -lSDL2_ttf -lSDL2_image -lSDL2_gfx -lm

#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>

extern int global_volume;
extern void update_audio_volumes(Mix_Chunk* hit_sound);

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

typedef enum {
    RES_800x600,
    RES_1024x768,
    RES_1280x960,
    RES_1600x1200,
    RES_1920x1080,
    RES_COUNT
} ResolutionOption;

typedef struct {
    int width;
    int height;
} Resolution;

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

typedef struct {
    SDL_Rect rect;
    char* text;
    SDL_Texture* texture;
    SDL_Texture* texture_hover;
    ResolutionOption action;
} ResolutionButton;


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

static inline SDL_Texture* menu_load_background(SDL_Renderer* ren, const char* filepath) {
    SDL_Surface* sfc = IMG_Load(filepath);
    if (!sfc) {
        printf("Erro ao carregar imagem de fundo '%s': %s\n", filepath, IMG_GetError());
        return NULL;
    }
    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, sfc);
    if (!tex) {
        printf("Erro ao criar textura do fundo para '%s': %s\n", filepath, SDL_GetError());
    }
    SDL_FreeSurface(sfc);
    return tex;
}

static inline void menu_render_background(SDL_Renderer* ren, SDL_Texture* bg_tex, int window_w, int window_h) {
    if (bg_tex) {
        SDL_Rect bg_rect = {0, 0, window_w, window_h};
        SDL_RenderCopy(ren, bg_tex, NULL, &bg_rect);
    }
}


static inline Resolution get_resolution(ResolutionOption opt) {
    Resolution resolutions[RES_COUNT] = {
        {896, 504},
        {1024, 576},
        {1280, 720},
        {1920, 1080}
    };
    return resolutions[opt];
}

typedef struct {
    int scroll_offset;
    int max_scroll;
} ScrollState;

// Função para exibir o menu de seleção de música
static inline GameState run_music_select_menu(SDL_Renderer* renderer, SDL_Window* window, TTF_Font* fnt, int window_w, int window_h) {
    
    MusicButton buttons[MUSIC_ITEM_COUNT];
    const char* labels[MUSIC_ITEM_COUNT] = {
        "Musica Principal",
        "Musica 2",
        "Musica 3",
        "Musica 4",
        "Musica 5",
        "Voltar"
    };
    
    SDL_Color color_normal = {0xFF, 0xFF, 0xFF, 0xFF};
    SDL_Color color_hover = {0xFF, 0xFF, 0x00, 0xFF};
    
    ScrollState scroll = {0, 0};
    
    // Carrega o background
    SDL_Texture* bg_tex = menu_load_background(renderer, "background.jpeg");
    
    // Inicializa e cria as texturas dos botões
    for (int i = 0; i < MUSIC_ITEM_COUNT; i++) {
        buttons[i].text = (char*)labels[i];
        buttons[i].action = (MusicMenuItem)i;

        // Criar texturas
        menu_create_text(renderer, &buttons[i].texture, buttons[i].text, color_normal, fnt);
        menu_create_text(renderer, &buttons[i].texture_hover, buttons[i].text, color_hover, fnt);

        if (!buttons[i].texture || !buttons[i].texture_hover) {
            printf("Erro ao criar textura para o botao %s: %s\n", labels[i], SDL_GetError());
            if (bg_tex) SDL_DestroyTexture(bg_tex);
            return STATE_QUIT;
        }
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
        
        // Obtém as dimensões atuais da janela
        SDL_GetWindowSize(window, &window_w, &window_h);
        
        // Recalcula posições baseado no tamanho atual
        int button_w = (window_w > 500) ? 400 : 250;
        int button_h = 60;
        int spacing = 20;
        int title_y = 50;
        int start_y = 150;
        int start_x = (window_w - button_w) / 2;
        
        // Atualiza scroll máximo
        int total_height = MUSIC_ITEM_COUNT * (button_h + spacing) + 100;
        scroll.max_scroll = (total_height > window_h) ? (total_height - window_h + 100) : 0;

        if (isevt) {
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    next_state = STATE_QUIT;
                    break;

                case SDL_MOUSEWHEEL:
                    if (event.wheel.y > 0) {
                        scroll.scroll_offset = (scroll.scroll_offset > 20) ? scroll.scroll_offset - 20 : 0;
                    } else if (event.wheel.y < 0) {
                        scroll.scroll_offset = (scroll.scroll_offset < scroll.max_scroll) ? scroll.scroll_offset + 20 : scroll.max_scroll;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        SDL_GetMouseState(&mouse_x, &mouse_y);
                        SDL_Point mouse_pos = {mouse_x, mouse_y};
                        
                        for (int i = 0; i < MUSIC_ITEM_COUNT; i++) {
                            buttons[i].rect.x = start_x;
                            buttons[i].rect.y = start_y + i * (button_h + spacing) - scroll.scroll_offset;
                            buttons[i].rect.w = button_w;
                            buttons[i].rect.h = button_h;
                            
                            if (SDL_PointInRect(&mouse_pos, &buttons[i].rect)) {
                                printf("Botão de música clicado: %s\n", buttons[i].text);
                                
                                switch (buttons[i].action) {
                                    case MUSIC_MOONLIGHT:
                                        printf("Iniciando jogo com Musica Principal...\n");
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
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        menu_render_background(renderer, bg_tex, window_w, window_h);

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

        // Desenhar botões com scroll
        for (int i = 0; i < MUSIC_ITEM_COUNT; i++) {
            buttons[i].rect.x = start_x;
            buttons[i].rect.y = start_y + i * (button_h + spacing) - scroll.scroll_offset;
            buttons[i].rect.w = button_w;
            buttons[i].rect.h = button_h;
            
            if (buttons[i].rect.y + button_h < 0 || buttons[i].rect.y > window_h) {
                continue;
            }
            
            SDL_Texture* tex_to_render;

            if (SDL_PointInRect(&mouse_pos, &buttons[i].rect)) {
                tex_to_render = buttons[i].texture_hover;
            } else {
                tex_to_render = buttons[i].texture;
            }

            int tex_w, tex_h;
            SDL_QueryTexture(tex_to_render, NULL, NULL, &tex_w, &tex_h);
            
            SDL_Rect dest_rect;
            dest_rect.w = tex_w;
            dest_rect.h = tex_h;
            dest_rect.x = buttons[i].rect.x + (buttons[i].rect.w - tex_w) / 2;
            dest_rect.y = buttons[i].rect.y + (buttons[i].rect.h - tex_h) / 2;

            SDL_RenderCopy(renderer, tex_to_render, NULL, &dest_rect);
        }

        // Desenha barra de scroll se necessário
        if (scroll.max_scroll > 0) {
            int scrollbar_w = 10;
            int scrollbar_x = window_w - scrollbar_w - 5;
            int scrollbar_h = window_h;
            int scrollbar_thumb_h = (window_h * window_h) / (window_h + scroll.max_scroll);
            int scrollbar_thumb_y = (scroll.scroll_offset * (window_h - scrollbar_thumb_h)) / scroll.max_scroll;
            
            SDL_Rect scrollbar_bg = {scrollbar_x, 0, scrollbar_w, scrollbar_h};
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &scrollbar_bg);
            
            SDL_Rect scrollbar_thumb = {scrollbar_x, scrollbar_thumb_y, scrollbar_w, scrollbar_thumb_h};
            SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
            SDL_RenderFillRect(renderer, &scrollbar_thumb);
        }

        SDL_RenderPresent(renderer);
    }
    
    SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);

    // Limpeza das texturas
    for (int i = 0; i < MUSIC_ITEM_COUNT; i++) {
        SDL_DestroyTexture(buttons[i].texture);
        SDL_DestroyTexture(buttons[i].texture_hover);
    }
    if (bg_tex) SDL_DestroyTexture(bg_tex);

    return next_state;
}


// Função para exibir o submenu de opções
// O parâmetro volume é um ponteiro para modificar o volume do jogo
static inline GameState run_options_menu(SDL_Renderer* renderer, TTF_Font* fnt, int window_w, int window_h, int* volume, SDL_Window* window, int* current_width, int* current_height) {
    
    OptionsButton buttons[OPTIONS_ITEM_COUNT];
    ControlButton control_buttons[2];
    ResolutionButton resolution_buttons[RES_COUNT];
    
    SDL_Color color_normal = {0xFF, 0xFF, 0xFF, 0xFF}; // Branco
    SDL_Color color_hover = {0xFF, 0xFF, 0x00, 0xFF};  // Amarelo para hover
    
    // Carrega o background
    SDL_Texture* bg_tex = menu_load_background(renderer, "background.jpeg");
    
    // Inicializa botão "Voltar" com texturas
    buttons[OPTIONS_BACK].text = "Voltar";
    buttons[OPTIONS_BACK].action = OPTIONS_BACK;
    
    menu_create_text(renderer, &buttons[OPTIONS_BACK].texture, "Voltar", color_normal, fnt);
    menu_create_text(renderer, &buttons[OPTIONS_BACK].texture_hover, "Voltar", color_hover, fnt);
    
    // Inicializa botão "-" (diminuir volume)
    control_buttons[0].text = "-";
    control_buttons[0].type = 0;
    
    menu_create_text(renderer, &control_buttons[0].texture, "-", color_normal, fnt);
    menu_create_text(renderer, &control_buttons[0].texture_hover, "-", color_hover, fnt);
    
    // Inicializa botão "+" (aumentar volume)
    control_buttons[1].text = "+";
    control_buttons[1].type = 1;
    
    menu_create_text(renderer, &control_buttons[1].texture, "+", color_normal, fnt);
    menu_create_text(renderer, &control_buttons[1].texture_hover, "+", color_hover, fnt);
    
    // Inicializa botões de resolução
    const char* resolution_labels[RES_COUNT] = {
        "800 x 600",
        "1024 x 768",
        "1280 x 960",
        "1600 x 1200",
        "1920 x 1080"
    };
    
    for (int i = 0; i < RES_COUNT; i++) {
        resolution_buttons[i].text = (char*)resolution_labels[i];
        resolution_buttons[i].action = (ResolutionOption)i;
        
        menu_create_text(renderer, &resolution_buttons[i].texture, resolution_buttons[i].text, color_normal, fnt);
        menu_create_text(renderer, &resolution_buttons[i].texture_hover, resolution_buttons[i].text, color_hover, fnt);
    }
    
    bool running = true;
    bool dropdown_open = false;
    GameState next_state = STATE_OPTIONS;
    int mouse_x = 0, mouse_y = 0;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    
    Uint32 tickTime = 17;
    SDL_Event event;
    
    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);
    
    while (running) {
        
        int isevt = menu_AUX_WaitEventTimeoutCount(&event, &tickTime);
        
        // Obtém as dimensões atuais da janela
        SDL_GetWindowSize(window, &window_w, &window_h);
        
        // Recalcula todas as posições baseado na janela atual
        int button_w = (window_w > 400) ? 300 : 200;
        int button_h = 70;
        int spacing = 30;
        
        int title_y = 30;
        
        int volume_y = (window_h > 400) ? 180 : 120;
        int control_button_w = 60;
        int control_button_h = 50;
        int volume_display_x = window_w / 2;
        
        int resolution_y = (window_h > 500) ? 340 : 250;
        int resolution_dropdown_w = (window_w > 400) ? 250 : 180;
        int resolution_dropdown_h = 50;
        int resolution_dropdown_x = (window_w - resolution_dropdown_w) / 2;
        
        int back_button_y = (window_h > 600) ? 550 : window_h - 100;
        int back_button_x = (window_w - button_w) / 2;
        
        // Atualiza posições dos botões
        buttons[OPTIONS_BACK].rect.x = back_button_x;
        buttons[OPTIONS_BACK].rect.y = back_button_y;
        buttons[OPTIONS_BACK].rect.w = button_w;
        buttons[OPTIONS_BACK].rect.h = button_h;
        
        control_buttons[0].rect.x = volume_display_x - 150;
        control_buttons[0].rect.y = volume_y;
        control_buttons[0].rect.w = control_button_w;
        control_buttons[0].rect.h = control_button_h;
        
        control_buttons[1].rect.x = volume_display_x + 90;
        control_buttons[1].rect.y = volume_y;
        control_buttons[1].rect.w = control_button_w;
        control_buttons[1].rect.h = control_button_h;
        
        for (int i = 0; i < RES_COUNT; i++) {
            resolution_buttons[i].rect.x = resolution_dropdown_x;
            resolution_buttons[i].rect.y = resolution_y + 60 + i * (resolution_dropdown_h + 5);
            resolution_buttons[i].rect.w = resolution_dropdown_w;
            resolution_buttons[i].rect.h = resolution_dropdown_h;
        }
        
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
                                    if (global_volume > 0) {
                                        global_volume--;
                                        update_audio_volumes(NULL);
                                        printf("Volume diminuído para: %d%%\n", global_volume * 10);
                                    }
                                } else if (control_buttons[i].type == 1) {
                                    if (global_volume < 10) {
                                        global_volume++;
                                        update_audio_volumes(NULL);
                                        printf("Volume aumentado para: %d%%\n", global_volume * 10);
                                    }
                                }
                            }
                        }
                        
                        // Verifica botão de dropdown de resolução
                        SDL_Rect dropdown_btn = {resolution_dropdown_x, resolution_y, resolution_dropdown_w, resolution_dropdown_h};
                        if (SDL_PointInRect(&mouse_pos, &dropdown_btn)) {
                            dropdown_open = !dropdown_open;
                        }
                        
                        // Verifica seleção de resolução se o dropdown estiver aberto
                        if (dropdown_open) {
                            for (int i = 0; i < RES_COUNT; i++) {
                                if (SDL_PointInRect(&mouse_pos, &resolution_buttons[i].rect)) {
                                    Resolution new_res = get_resolution(resolution_buttons[i].action);
                                    SDL_SetWindowSize(window, new_res.width, new_res.height);
                                    *current_width = new_res.width;
                                    *current_height = new_res.height;
                                    printf("Resolução alterada para: %d x %d\n", new_res.width, new_res.height);
                                    dropdown_open = false;
                                    break;
                                }
                            }
                        }
                        
                        // Verifica botão "Voltar" somente se dropdown não está aberto
                        if (!dropdown_open && SDL_PointInRect(&mouse_pos, &buttons[OPTIONS_BACK].rect)) {
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
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        menu_render_background(renderer, bg_tex, window_w, window_h);
        
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
        
        SDL_Rect bar_bg = {bar_x, bar_y, bar_width, bar_height};
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderFillRect(renderer, &bar_bg);
        
        int filled_width = (bar_width * (*volume)) / 10;
        SDL_Rect bar_fill = {bar_x, bar_y, filled_width, bar_height};
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &bar_fill);
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &bar_bg);
        
        // Desenha label "Resolução"
        SDL_Texture* resolution_label_tex = NULL;
        menu_create_text(renderer, &resolution_label_tex, "Resolucao", white, fnt);
        
        if (resolution_label_tex) {
            int tex_w, tex_h;
            SDL_QueryTexture(resolution_label_tex, NULL, NULL, &tex_w, &tex_h);
            SDL_Rect label_rect = {resolution_dropdown_x, resolution_y - 50, tex_w, tex_h};
            SDL_RenderCopy(renderer, resolution_label_tex, NULL, &label_rect);
            SDL_DestroyTexture(resolution_label_tex);
        }
        
        // Desenha botão do dropdown de resolução
        SDL_Rect dropdown_btn = {resolution_dropdown_x, resolution_y, resolution_dropdown_w, resolution_dropdown_h};
        SDL_Texture* dropdown_tex = NULL;
        char dropdown_label[50];
        snprintf(dropdown_label, sizeof(dropdown_label), "%d x %d", *current_width, *current_height);
        menu_create_text(renderer, &dropdown_tex, dropdown_label, white, fnt);
        
        if (dropdown_tex) {
            int tex_w, tex_h;
            SDL_QueryTexture(dropdown_tex, NULL, NULL, &tex_w, &tex_h);
            
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
            SDL_RenderFillRect(renderer, &dropdown_btn);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &dropdown_btn);
            
            SDL_Rect dest_rect;
            dest_rect.w = tex_w;
            dest_rect.h = tex_h;
            dest_rect.x = dropdown_btn.x + (dropdown_btn.w - tex_w) / 2;
            dest_rect.y = dropdown_btn.y + (dropdown_btn.h - tex_h) / 2;
            SDL_RenderCopy(renderer, dropdown_tex, NULL, &dest_rect);
            SDL_DestroyTexture(dropdown_tex);
        }
        
        // Desenha opções de resolução se dropdown estiver aberto
        if (dropdown_open) {
            for (int i = 0; i < RES_COUNT; i++) {
                SDL_Texture* tex_to_render;
                
                if (SDL_PointInRect(&mouse_pos, &resolution_buttons[i].rect)) {
                    tex_to_render = resolution_buttons[i].texture_hover;
                } else {
                    tex_to_render = resolution_buttons[i].texture;
                }
                
                int tex_w, tex_h;
                SDL_QueryTexture(tex_to_render, NULL, NULL, &tex_w, &tex_h);
                
                SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
                SDL_RenderFillRect(renderer, &resolution_buttons[i].rect);
                SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
                SDL_RenderDrawRect(renderer, &resolution_buttons[i].rect);
                
                SDL_Rect dest_rect;
                dest_rect.w = tex_w;
                dest_rect.h = tex_h;
                dest_rect.x = resolution_buttons[i].rect.x + (resolution_buttons[i].rect.w - tex_w) / 2;
                dest_rect.y = resolution_buttons[i].rect.y + (resolution_buttons[i].rect.h - tex_h) / 2;
                
                SDL_RenderCopy(renderer, tex_to_render, NULL, &dest_rect);
            }
        }
        
        // Desenha botão "Voltar" SOMENTE se o dropdown NÃO está aberto
        if (!dropdown_open && buttons[OPTIONS_BACK].texture && buttons[OPTIONS_BACK].texture_hover) {
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
    for (int i = 0; i < RES_COUNT; i++) {
        if (resolution_buttons[i].texture) {
            SDL_DestroyTexture(resolution_buttons[i].texture);
        }
        if (resolution_buttons[i].texture_hover) {
            SDL_DestroyTexture(resolution_buttons[i].texture_hover);
        }
    }
    if (bg_tex) SDL_DestroyTexture(bg_tex);
    
    return next_state;
}


// Função para exibir o menu de créditos
static inline GameState run_credits_menu(SDL_Renderer* renderer, SDL_Window* window, TTF_Font* fnt, int window_w, int window_h) {
    
    MenuButton back_button;
    
    SDL_Color color_normal = {0xFF, 0xFF, 0xFF, 0xFF};
    SDL_Color color_hover = {0xFF, 0xFF, 0x00, 0xFF};
    SDL_Color white = {0xFF, 0xFF, 0xFF, 0xFF};
    
    // Carrega o background
    SDL_Texture* bg_tex = menu_load_background(renderer, "background.jpeg");
    
    // Inicializa botão "Voltar"
    back_button.text = "Voltar";
    back_button.action = MENU_JOGAR;
    menu_create_text(renderer, &back_button.texture, "Voltar", color_normal, fnt);
    menu_create_text(renderer, &back_button.texture_hover, "Voltar", color_hover, fnt);
    
    if (!back_button.texture || !back_button.texture_hover) {
        printf("Erro ao criar textura para o botao Voltar\n");
        if (bg_tex) SDL_DestroyTexture(bg_tex);
        return STATE_QUIT;
    }

    bool running = true;
    GameState next_state = STATE_CREDITS;
    int mouse_x = 0, mouse_y = 0;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    Uint32 tickTime = 17;
    SDL_Event event;

    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

    while (running) {
        
        int isevt = menu_AUX_WaitEventTimeoutCount(&event, &tickTime);
        
        // Obtém as dimensões atuais da janela
        SDL_GetWindowSize(window, &window_w, &window_h);
        
        // Recalcula posições baseado no tamanho atual
        int button_w = (window_w > 400) ? 300 : 200;
        int button_h = 70;
        int back_button_x = (window_w - button_w) / 2;
        int back_button_y = window_h - 100;
        
        back_button.rect.x = back_button_x;
        back_button.rect.y = back_button_y;
        back_button.rect.w = button_w;
        back_button.rect.h = button_h;

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
                        
                        if (SDL_PointInRect(&mouse_pos, &back_button.rect)) {
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
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        menu_render_background(renderer, bg_tex, window_w, window_h);

        SDL_GetMouseState(&mouse_x, &mouse_y);
        SDL_Point mouse_pos = {mouse_x, mouse_y};

        // Desenha título "Créditos"
        SDL_Texture* title_tex = NULL;
        menu_create_text(renderer, &title_tex, "Creditos", white, fnt);
        
        if (title_tex) {
            int tex_w, tex_h;
            SDL_QueryTexture(title_tex, NULL, NULL, &tex_w, &tex_h);
            SDL_Rect title_rect = {(window_w - tex_w) / 2, 50, tex_w, tex_h};
            SDL_RenderCopy(renderer, title_tex, NULL, &title_rect);
            SDL_DestroyTexture(title_tex);
        }

        // Desenha nome 1 "Breno Rodrigues" - no meio da tela
        SDL_Texture* name1_tex = NULL;
        menu_create_text(renderer, &name1_tex, "Breno Rodrigues", white, fnt);
        
        if (name1_tex) {
            int tex_w, tex_h;
            SDL_QueryTexture(name1_tex, NULL, NULL, &tex_w, &tex_h);
            SDL_Rect name1_rect = {(window_w - tex_w) / 2, (window_h / 2) - 40, tex_w, tex_h};
            SDL_RenderCopy(renderer, name1_tex, NULL, &name1_rect);
            SDL_DestroyTexture(name1_tex);
        }

        // Desenha nome 2 "Gabriel Godoy" - no meio da tela
        SDL_Texture* name2_tex = NULL;
        menu_create_text(renderer, &name2_tex, "Gabriel Godoy", white, fnt);
        
        if (name2_tex) {
            int tex_w, tex_h;
            SDL_QueryTexture(name2_tex, NULL, NULL, &tex_w, &tex_h);
            SDL_Rect name2_rect = {(window_w - tex_w) / 2, (window_h / 2) + 20, tex_w, tex_h};
            SDL_RenderCopy(renderer, name2_tex, NULL, &name2_rect);
            SDL_DestroyTexture(name2_tex);
        }

        // Desenha botão "Voltar"
        SDL_Texture* back_tex_to_render;
        
        if (SDL_PointInRect(&mouse_pos, &back_button.rect)) {
            back_tex_to_render = back_button.texture_hover;
        } else {
            back_tex_to_render = back_button.texture;
        }
        
        int back_tex_w, back_tex_h;
        SDL_QueryTexture(back_tex_to_render, NULL, NULL, &back_tex_w, &back_tex_h);
        
        SDL_Rect back_dest_rect;
        back_dest_rect.w = back_tex_w;
        back_dest_rect.h = back_tex_h;
        back_dest_rect.x = back_button.rect.x + (back_button.rect.w - back_tex_w) / 2;
        back_dest_rect.y = back_button.rect.y + (back_button.rect.h - back_tex_h) / 2;
        
        SDL_RenderCopy(renderer, back_tex_to_render, NULL, &back_dest_rect);

        SDL_RenderPresent(renderer);
    }
    
    SDL_EventState(SDL_MOUSEMOTION, SDL_ENABLE);

    // Limpeza das texturas
    SDL_DestroyTexture(back_button.texture);
    SDL_DestroyTexture(back_button.texture_hover);
    if (bg_tex) SDL_DestroyTexture(bg_tex);

    return next_state;
}

static inline GameState run_menu(SDL_Renderer* renderer, TTF_Font* fnt, int window_w, int window_h, int* global_volume, SDL_Window* window, int* current_width, int* current_height) {

    MenuButton buttons[MENU_ITEM_COUNT];
    const char* labels[MENU_ITEM_COUNT] = {"Jogar", "Opcoes", "Creditos", "Sair"};

    SDL_Color color_normal = {0xFF, 0xFF, 0xFF, 0xFF};
    SDL_Color color_hover = {0xFF, 0xFF, 0x00, 0xFF};

    // Carrega o background
    SDL_Texture* bg_tex = menu_load_background(renderer, "background.jpeg");

    // Inicializa e cria as texturas dos botoes
    for (int i = 0; i < MENU_ITEM_COUNT; i++) {
        buttons[i].text = (char*)labels[i];
        buttons[i].action = (MenuItem)i;

        // Criar texturas
        menu_create_text(renderer, &buttons[i].texture, buttons[i].text, color_normal, fnt);
        menu_create_text(renderer, &buttons[i].texture_hover, buttons[i].text, color_hover, fnt);

        if (!buttons[i].texture || !buttons[i].texture_hover) {
            printf("Erro ao criar textura para o botao %s: %s\n", labels[i], SDL_GetError());
            if (bg_tex) SDL_DestroyTexture(bg_tex);
            return STATE_QUIT;
        }
    }

    bool running = true;
    GameState next_state = STATE_MENU;
    int mouse_x = 0, mouse_y = 0;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    Uint32 tickTime = 17;
    SDL_Event event;

    SDL_EventState(SDL_MOUSEMOTION, SDL_IGNORE);

    while (running) {
        
        int isevt = menu_AUX_WaitEventTimeoutCount(&event, &tickTime);
        
        // Obtém as dimensões atuais da janela
        SDL_GetWindowSize(window, &window_w, &window_h);
        
        // Recalcula posições baseado no tamanho atual
        int button_w = (window_w > 400) ? 300 : 200;
        int button_h = 70;
        int spacing = 30;
        int start_y = (window_h - (MENU_ITEM_COUNT * button_h + (MENU_ITEM_COUNT - 1) * spacing)) / 2;
        int start_x = (window_w - button_w) / 2;

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
                                        next_state = run_music_select_menu(renderer, window, fnt, window_w, window_h);
                                        if (next_state == STATE_GAME) {
                                            running = false;
                                        }
                                        break;
                                    case MENU_OPCOES:
                                        printf("Opcoes clicado\n");
                                        run_options_menu(renderer, fnt, window_w, window_h, global_volume, window, current_width, current_height);
                                        break;
                                    case MENU_CREDITOS:
                                        printf("Creditos clicado\n");
                                        run_credits_menu(renderer, window, fnt, window_w, window_h);
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

        // Renderiza o background
        menu_render_background(renderer, bg_tex, window_w, window_h);

        // Atualiza a posicao do mouse para o "hover"
        SDL_GetMouseState(&mouse_x, &mouse_y);
        SDL_Point mouse_pos = {mouse_x, mouse_y};

        // Desenha título principal "Projeto Maestro" - grande no topo
        SDL_Color title_color = {0xFF, 0xD7, 0x00, 0xFF}; // Ouro/Amarelo dourado
        SDL_Texture* main_title_tex = NULL;
        
        // Cria um texto grande usando a fonte existente (pode parecer maior com strings)
        menu_create_text(renderer, &main_title_tex, "PROJETO MAESTRO", title_color, fnt);
        
        if (main_title_tex) {
            int tex_w, tex_h;
            SDL_QueryTexture(main_title_tex, NULL, NULL, &tex_w, &tex_h);
            // Aumenta o tamanho visualmente escalando a renderização
            int scaled_w = (tex_w * 2) > window_w ? window_w - 40 : tex_w * 2;
            int scaled_h = (tex_h * 2);
            SDL_Rect title_rect = {(window_w - scaled_w) / 2, 20, scaled_w, scaled_h};
            SDL_RenderCopy(renderer, main_title_tex, NULL, &title_rect);
            SDL_DestroyTexture(main_title_tex);
        }

        // Recalcula posições dos botões a cada frame
        for (int i = 0; i < MENU_ITEM_COUNT; i++) {
            buttons[i].rect.x = start_x;
            buttons[i].rect.y = start_y + i * (button_h + spacing);
            buttons[i].rect.w = button_w;
            buttons[i].rect.h = button_h;
        }

        // Desenhar botoes
        for (int i = 0; i < MENU_ITEM_COUNT; i++) {
            SDL_Texture* tex_to_render;

            if (SDL_PointInRect(&mouse_pos, &buttons[i].rect)) {
                tex_to_render = buttons[i].texture_hover;
            } else {
                tex_to_render = buttons[i].texture;
            }

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
    if (bg_tex) SDL_DestroyTexture(bg_tex);

    return next_state;
}

#endif // MENU_H