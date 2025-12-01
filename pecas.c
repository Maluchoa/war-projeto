/*
  pecas.c
  Gerenciador de peças: fila circular (5) + pilha (3)
  - Peças: nome ('I','O','T','L') e id único (int)
  - Ações: jogar (dequeue), reservar (fila->pilha), usar (pop pilha),
           trocar frente da fila com topo da pilha, troca múltipla (3),
           exibição do estado e encerramento.
  - A fila é mantida cheia gerando novas peças automaticamente quando uma peça
    é removida ou enviada para a pilha (quando aplicável).
  - Compilar: gcc -o pecas pecas.c -std=c11
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define FILA_CAP 5
#define PILHA_CAP 3

/* --- Estrutura que representa uma peça --- */
typedef struct {
    char nome; /* 'I', 'O', 'T', 'L' */
    int id;    /* id único incremental */
} Peca;

/* --- Fila circular --- */
typedef struct {
    Peca dados[FILA_CAP];
    int inicio;  /* índice do elemento da frente */
    int tamanho; /* número de elementos presentes */
} Fila;

/* --- Pilha simples --- */
typedef struct {
    Peca dados[PILHA_CAP];
    int topo; /* índice do topo: -1 quando vazia */
} Pilha;

/* --- Variáveis globais auxiliares --- */
int proximoId = 0; /* gera ids únicos */

/* --- Prototipos --- */
Peca gerarPeca();
void inicializarFila(Fila *f);
void inicializarPilha(Pilha *p);
int filaVazia(Fila *f);
int filaCheia(Fila *f);
int pilhaVazia(Pilha *p);
int pilhaCheia(Pilha *p);
void enqueue(Fila *f, Peca px);
Peca dequeue(Fila *f);
void push(Pilha *p, Peca px);
Peca pop(Pilha *p);
void exibirEstado(const Fila *f, const Pilha *p);
void jogarPeca(Fila *f);
void reservarPeca(Fila *f, Pilha *p);
void usarPecaReservada(Pilha *p);
void trocarFrenteTopo(Fila *f, Pilha *p);
void trocarTres(Fila *f, Pilha *p);
void mostrarMenu();
void pausa();

/* --- Implementações --- */

/* Gera uma peça com nome aleatório ('I','O','T','L') e id incremental */
Peca gerarPeca() {
    Peca p;
    int r = rand() % 4;
    switch (r) {
        case 0: p.nome = 'I'; break;
        case 1: p.nome = 'O'; break;
        case 2: p.nome = 'T'; break;
        default: p.nome = 'L'; break;
    }
    p.id = proximoId++;
    return p;
}

/* Inicializa fila: coloca tamanho = 0 e inicio = 0
   Em seguida, enfileira FILA_CAP peças geradas para ficar cheia inicialmente */
void inicializarFila(Fila *f) {
    f->inicio = 0;
    f->tamanho = 0;
    for (int i = 0; i < FILA_CAP; ++i) {
        enqueue(f, gerarPeca());
    }
}

/* Inicializa pilha: topo = -1 (vazia) */
void inicializarPilha(Pilha *p) {
    p->topo = -1;
}

/* Verificações de estado */
int filaVazia(Fila *f) {
    return (f->tamanho == 0);
}
int filaCheia(Fila *f) {
    return (f->tamanho == FILA_CAP);
}
int pilhaVazia(Pilha *p) {
    return (p->topo == -1);
}
int pilhaCheia(Pilha *p) {
    return (p->topo == PILHA_CAP - 1);
}

/* Enfileira na fila circular (assume que não está cheia em uso regular) */
void enqueue(Fila *f, Peca px) {
    if (filaCheia(f)) {
        /* caso de segurança: se cheia, descarta (não deveria ocorrer se usado corretamente) */
        return;
    }
    int pos = (f->inicio + f->tamanho) % FILA_CAP;
    f->dados[pos] = px;
    f->tamanho++;
}

/* Desenfileira a frente e retorna a peça; assume que há elemento */
Peca dequeue(Fila *f) {
    Peca ret = { '?', -1 };
    if (filaVazia(f)) return ret;
    ret = f->dados[f->inicio];
    f->inicio = (f->inicio + 1) % FILA_CAP;
    f->tamanho--;
    return ret;
}

/* Push na pilha */
void push(Pilha *p, Peca px) {
    if (pilhaCheia(p)) return;
    p->topo++;
    p->dados[p->topo] = px;
}

/* Pop da pilha */
Peca pop(Pilha *p) {
    Peca ret = { '?', -1 };
    if (pilhaVazia(p)) return ret;
    ret = p->dados[p->topo];
    p->topo--;
    return ret;
}

/* Exibe uma peça no formato [X id] */
void exibirPeca(const Peca *px) {
    if (px->id < 0) {
        printf("[   ] ");
    } else {
        printf("[%c %d] ", px->nome, px->id);
    }
}

/* Exibe estado atual: fila e pilha formatadas */
void exibirEstado(const Fila *f, const Pilha *p) {
    printf("\nEstado atual:\n\n");
    /* Fila */
    printf("Fila de peças\t");
    if (filaVazia((Fila*)f)) {
        printf("(vazia)\n");
    } else {
        for (int i = 0; i < f->tamanho; ++i) {
            int idx = (f->inicio + i) % FILA_CAP;
            exibirPeca(&f->dados[idx]);
        }
        printf("\n");
    }
    /* Pilha */
    printf("Pilha de reserva\t(Topo -> base): ");
    if (pilhaVazia((Pilha*)p)) {
        printf("(vazia)\n");
    } else {
        for (int i = p->topo; i >= 0; --i) {
            exibirPeca(&p->dados[i]);
        }
        printf("\n");
    }
    printf("\n");
}

/* Jogar peça: remove da frente da fila (dequeue) e gera nova peça para manter fila cheia */
void jogarPeca(Fila *f) {
    if (filaVazia(f)) {
        printf("Fila vazia: nada para jogar.\n");
        return;
    }
    Peca jogada = dequeue(f);
    printf("Jogou a peça: [%c %d]\n", jogada.nome, jogada.id);
    /* Gerar nova peça e enfileirar (manter fila cheia quando possível) */
    Peca nova = gerarPeca();
    enqueue(f, nova);
}

/* Reservar peça: move a peça da frente da fila para o topo da pilha, se pilha tiver espaço.
   Após mover, gera nova peça para manter fila cheia */
void reservarPeca(Fila *f, Pilha *p) {
    if (filaVazia(f)) {
        printf("Fila vazia: nada para reservar.\n");
        return;
    }
    if (pilhaCheia(p)) {
        printf("Pilha cheia: não é possível reservar (pilha com capacidade máxima).\n");
        return;
    }
    Peca mover = dequeue(f);
    push(p, mover);
    printf("Reservou a peça [%c %d] para a pilha (topo).\n", mover.nome, mover.id);
    /* Gerar nova peça para manter fila cheia */
    Peca nova = gerarPeca();
    enqueue(f, nova);
}

/* Usar peça reservada (pop da pilha) */
void usarPecaReservada(Pilha *p) {
    if (pilhaVazia(p)) {
        printf("Pilha vazia: nenhuma peça reservada para usar.\n");
        return;
    }
    Peca usada = pop(p);
    printf("Usou a peça reservada: [%c %d]\n", usada.nome, usada.id);
    /* NÃO geramos nova peça aqui; somente remoções/transferências da fila geram novas peças. */
}

/* Trocar peça da frente da fila com o topo da pilha */
void trocarFrenteTopo(Fila *f, Pilha *p) {
    if (filaVazia(f)) {
        printf("Fila vazia: nada para trocar.\n");
        return;
    }
    if (pilhaVazia(p)) {
        printf("Pilha vazia: nada no topo para trocar.\n");
        return;
    }
    int idxFrente = f->inicio;
    Peca tmp = f->dados[idxFrente];
    f->dados[idxFrente] = p->dados[p->topo];
    p->dados[p->topo] = tmp;
    printf("Troca realizada entre frente da fila e topo da pilha.\n");
    /* Observação: não geramos novas peças aqui porque não ocorreu remoção da fila. */
}

/* Troca múltipla: alterna as três primeiras da fila com as três da pilha
   Condição: fila deve ter pelo menos 3 peças e pilha ter pelo menos 3 peças */
void trocarTres(Fila *f, Pilha *p) {
    if (f->tamanho < 3) {
        printf("A fila não tem 3 peças disponíveis para a troca.\n");
        return;
    }
    if (p->topo < 2) {
        printf("A pilha não tem 3 peças (pelo menos) para a troca.\n");
        return;
    }
    /* Captura as três primeiras da fila (ordem: frente -> frente+1 -> frente+2) */
    Peca q0 = f->dados[f->inicio];
    Peca q1 = f->dados[(f->inicio + 1) % FILA_CAP];
    Peca q2 = f->dados[(f->inicio + 2) % FILA_CAP];

    /* Captura as três do topo da pilha: top (s0), top-1 (s1), top-2 (s2) */
    Peca s0 = p->dados[p->topo];
    Peca s1 = p->dados[p->topo - 1];
    Peca s2 = p->dados[p->topo - 2];

    /* Coloca os elementos da pilha (top->down) nas três primeiras posições da fila */
    f->dados[f->inicio] = s0;
    f->dados[(f->inicio + 1) % FILA_CAP] = s1;
    f->dados[(f->inicio + 2) % FILA_CAP] = s2;

    /* Coloca os elementos da fila (frente->...) na pilha de forma que
       novo topo seja q2, depois q1, depois q0 (conforme exemplo de saída) */
    p->dados[p->topo] = q2;
    p->dados[p->topo - 1] = q1;
    p->dados[p->topo - 2] = q0;

    printf("Troca realizada entre os 3 primeiros da fila e as 3 peças do topo da pilha.\n");
}

/* Menu e interação */
void mostrarMenu() {
    printf("Opções disponíveis:\n\n");
    printf("Código\tAção\n");
    printf("1\tJogar peça da frente da fila\n");
    printf("2\tEnv
