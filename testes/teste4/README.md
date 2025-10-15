
# Projeto Maestro

## Teste de Menu
Cria uma janela com quatro botões clicáveis e gerencia o lançamento do jogo principal.

### Estrutura do Menu
Quatro botões alinhados verticalmente:
1. **Jogar** - Inicia o jogo principal
2. **Opções** - Ainda não implementado
3. **Créditos** - Ainda não implementado
4. **Sair** - Fecha o programa

## Componentes Principais

### Estruturas

typedef struct {
    SDL_Rect rect;          
    SDL_Texture* texture;   
    const char* text;   
} Button;


### Funções Principais

- `AUX_WaitEventTimeout`: Gerencia temporização de eventos
- `create_button`: Cria e posiciona botões com texto
- `point_in_rect`: Detecção de colisão para cliques nos botões

## Fluxo do Programa

1. **Inicialização**
   - Sistemas SDL2 e TTF
   - Criação da janela e renderizador
   - Carregamento da fonte
   - Criação dos botões

2. **Loop Principal**
   - Processa eventos a cada 16ms
   - Trata cliques do mouse nos botões
   - Renderiza botões e texto
   - Atualiza display

3. **Ações dos Botões**
   - "Jogar": Inicia jogo principal em processo separado usando fork/exec
   - "Sair": Fecha o programa do menu
   - Outros botões: Sem ação implementada

4. **Limpeza**
   - Liberação adequada de recursos
   - Encerramento do sistema

## Compilação
gcc menu.c -o menu -lSDL2 -lSDL2_ttf -lSDL2_gfx

## Dependências
- SDL2
- SDL2_ttf
- SDL2_gfx
- Arquivo de fonte Roboto-Regular.ttf


## Teste do Jogo
Prototipo de um jogo rítmico onde círculos aparecem em três partições da tela. O jogador deve clicar nos círculos antes que desapareçam.


### Estruturas Principais

typedef struct {
    int x, y;              // Posição na tela
    int radius;            // Raio do círculo
    int partition;         // Partição (0-2)
    int remaining_ticks;   // Tempo de vida restante
    int base_ticks;        // Tempo total de vida
    SDL_Color color;       // Cor RGBA
    int fade_in_ticks;     // Contador para efeito fade
} Circle;

typedef struct {
    Circle circles[MAX_CIRCLES];  // Array de círculos
    int count;                    // Quantidade atual
    int score;                    // Pontuação
    float spawn_timer;           // Tempo para próximo círculo
    Uint32 tick_counter;         // Contador de ticks
} GameState;


## Mecânicas do Jogo

### Geração de Círculos
- Círculos aparecem aleatoriamente nas 3 partições
- Raio: 15-45 pixels
- Fade in suave ao aparecer
- Tempo entre círculos: 0.5-1.5 segundos

### Sistema de Pontuação
- +1 ponto: Acertar um círculo
- -1 ponto: Errar um clique ou deixar círculo expirar

### Efeitos Visuais
- Fade in ao aparecer
- Fade out ao expirar
- Cores aleatórias para cada círculo

## Funções Principais

1. **generate_random_circle**
   - Gera círculo com propriedades aleatórias
   - Define posição, tamanho e cor

2. **update_circles**
   - Atualiza estado de todos os círculos
   - Gerencia efeitos de fade
   - Remove círculos expirados

3. **check_collision**
   - Verifica cliques do mouse
   - Atualiza pontuação

## Loop Principal
1. Processa eventos (cliques, saída)
2. Atualiza temporizador do jogo
3. Gera novos círculos quando necessário
4. Atualiza estado dos círculos existentes
5. Renderiza:
   - Limpa tela
   - Desenha linhas das partições
   - Desenha círculos ativos
6. Repete até fim do tempo

## Compilação
gcc main.c -o main -lSDL2 -lSDL2_gfx

## Dependências
- SDL2
- SDL2_gfx

## Integração com Menu
- Iniciado pelo menu.c através de fork/exec
- Funciona independentemente após iniciado
- Fecha automaticamente após X segundos


## Estrutura de Arquivos (temporário)

/home/SDL/test/
    - main.c         # Jogo principal
    - menu.c         # Interface do menu
    - Roboto-Regular.ttf
    - Executáveis compilados
