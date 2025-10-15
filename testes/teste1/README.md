# Explicação do Código


## Estrutura do Código

### Estrutura do Círculo
```c
typedef struct {
    int x, y;           // Posição do círculo
    int radius;         // Raio do círculo
    SDL_Color color;    // Cor do círculo (RGBA)
} Circle;
```

### Funções Principais

#### 1. Geração de Círculo Aleatório
```c
void generate_random_circle(Circle* circle)
```
- Gera posição aleatória (x,y) dentro dos limites da janela
- Define um raio aleatório entre 20 e 70 pixels
- Atribui uma cor aleatória RGB

#### 2. Verificação de Colisão
```c
int check_collision(int mouse_x, int mouse_y, Circle* circle)
```
- Verifica se o clique do mouse está dentro da área do círculo

### Loop Principal do Jogo

1. **Inicialização**
   - Configura SDL
   - Cria janela e renderer
   - Inicializa gerador de números aleatórios
   - Define pontuação inicial

2. **Game Loop**
   - Verifica tempo de jogo
   - Limpa a tela
   - Desenha o círculo atual
   - Processa eventos (cliques do mouse)
   - Atualiza a pontuação
   - Renderiza a tela

3. **Finalização**
   - Mostra pontuação final
   - Libera recursos SDL
   - Encerra o programa

## Funcionalidades Principais

- **Tempo Limitado**: O jogo dura 15 segundos
- **Sistema de Pontuação**: +1 ponto para cada círculo clicado
- **Círculos Dinâmicos**: 
  - Tamanhos variados (20-70 pixels de raio)
  - Cores aleatórias
  - Posições aleatórias
- **Detecção de Colisão**: Verifica precisamente se o clique foi dentro do círculo
