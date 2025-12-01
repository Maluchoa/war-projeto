/*
  
  Detective Quest - exploração de mansão (árvore binária), coleta de pistas (BST)
  e associação pista -> suspeito (hash). Julgamento final com verificação automática.
  Compilar: gcc -o detective_quest detective_quest.c -std=c11
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_NOME 64
#define HASH_SIZE 101

/* ---------- ESTRUTURAS ---------- */

/* Sala na mansão (nó da árvore binária) */
typedef struct Sala {
    char nome[MAX_NOME];
    struct Sala *esq;
    struct Sala *dir;
    struct Sala *pai; /* para permitir voltar */
} Sala;

/* Nó da BST de pistas */
typedef struct PistaNode {
    char *texto;                /* dinamicamente alocado */
    struct PistaNode *esq;
    struct PistaNode *dir;
} PistaNode;

/* Nó da tabela hash (encadeamento separado) */
typedef struct HashNode {
    char *pista;       /* chave */
    char *suspeito;    /* valor */
    struct HashNode *next;
} HashNode;

/* ---------- PROTÓTIPOS (funções exigidas e utilitárias) ---------- */

/* criarSala() – cria dinamicamente um cômodo. */
Sala* criarSala(const char *nome);

/* explorarSalas() – navega pela árvore e ativa o sistema de pistas. */
void explorarSalas(Sala *raiz, PistaNode **bstPistas, HashNode *hash[]);

/* inserirPista() / adicionarPista() – insere a pista coletada na árvore BST. */
PistaNode* inserirPista(PistaNode *root, const char *texto);
void adicionarPista(PistaNode **root, const char *texto);

/* inserirNaHash() – insere associação pista/suspeito na tabela hash. */
unsigned int hashFunction(const char *s);
void inserirNaHash(HashNode *hash[], const char *pista, const char *suspeito);

/* encontrarSuspeito() – consulta o suspeito correspondente a uma pista. */
const char* encontrarSuspeito(HashNode *hash[], const char *pista);

/* verificarSuspeitoFinal() – conduz à fase de julgamento final. */
void verificarSuspeitoFinal(PistaNode *root, HashNode *hash[]);

/* funções utilitárias */
const char* pistaAssociadaASala(const char *nomeSala); /* define pistas estáticas por sala */
void listarPistasBST(PistaNode *root);
void liberarBST(PistaNode *root);
void liberarHash(HashNode *hash[]);
void montarMapa(Sala **raiz); /* constrói mapa fixo no main() */
void mostrarSalaOpcoes(Sala *atual);
void limpaNewline(char *s);

/* ---------- IMPLEMENTAÇÃO ---------- */

/* cria dinamicamente uma sala com nome (criarSala) */
Sala* criarSala(const char *nome) {
    Sala *s = (Sala*) malloc(sizeof(Sala));
    if (!s) {
        perror("malloc criarSala");
        exit(1);
    }
    strncpy(s->nome, nome, MAX_NOME-1);
    s->nome[MAX_NOME-1] = '\0';
    s->esq = s->dir = s->pai = NULL;
    return s;
}

/* Hash simples (djb2 mod HASH_SIZE) */
unsigned int hashFunction(const char *s) {
    unsigned long hash = 5381;
    int c;
    while ((c = *s++))
        hash = ((hash << 5) + hash) + (unsigned char)c;
    return (unsigned int)(hash % HASH_SIZE);
}

/* inserirNaHash: insere cópia da chave e valor na tabela */
void inserirNaHash(HashNode *hash[], const char *pista, const char *suspeito) {
    unsigned int idx = hashFunction(pista);
    HashNode *node = (HashNode*) malloc(sizeof(HashNode));
    if (!node) { perror("malloc inserirNaHash"); exit(1); }
    node->pista = strdup(pista);
    node->suspeito = strdup(suspeito);
    node->next = hash[idx];
    hash[idx] = node;
}

/* encontrarSuspeito: retorna ponteiro para string do suspeito (ou NULL se não existir) */
const char* encontrarSuspeito(HashNode *hash[], const char *pista) {
    unsigned int idx = hashFunction(pista);
    HashNode *cur = hash[idx];
    while (cur) {
        if (strcmp(cur->pista, pista) == 0) return cur->suspeito;
        cur = cur->next;
    }
    return NULL;
}

/* inserirPista: insere texto unicamente numa BST (evita duplicatas) */
PistaNode* inserirPista(PistaNode *root, const char *texto) {
    if (!root) {
        PistaNode *n = (PistaNode*) malloc(sizeof(PistaNode));
        if (!n) { perror("malloc inserirPista"); exit(1); }
        n->texto = strdup(texto);
        n->esq = n->dir = NULL;
        return n;
    }
    int cmp = strcmp(texto, root->texto);
    if (cmp == 0) {
        /* já existe: não duplicar */
        return root;
    } else if (cmp < 0) {
        root->esq = inserirPista(root->esq, texto);
    } else {
        root->dir = inserirPista(root->dir, texto);
    }
    return root;
}

/* adicionarPista: wrapper que atualiza ponteiro raiz */
void adicionarPista(PistaNode **root, const char *texto) {
    *root = inserirPista(*root, texto);
}

/* pistaAssociadaASala: define as pistas estáticas para cada sala (regra codificada) */
const char* pistaAssociadaASala(const char *nomeSala) {
    /* exemplo: mapeamento fixo */
    if (strcmp(nomeSala, "Entrada") == 0) return "pegada molhada";
    if (strcmp(nomeSala, "Sala de Estar") == 0) return "cinzeiro quebrado";
    if (strcmp(nomeSala, "Biblioteca") == 0) return "marcador de livro rasgado";
    if (strcmp(nomeSala, "Cozinha") == 0) return "panela com restos";
    if (strcmp(nomeSala, "Escritorio") == 0) return "bilhete amassado";
    if (strcmp(nomeSala, "Quarto") == 0) return "fio de cabelo loiro";
    if (strcmp(nomeSala, "Banheiro") == 0) return "detergente derramado";
    if (strcmp(nomeSala, "Jardim") == 0) return "pegada de sola distinta";
    if (strcmp(nomeSala, "Sotao") == 0) return "ferramenta enferrujada";
    if (strcmp(nomeSala, "Garagem") == 0) return "marca de pneu";
    /* salas sem pista retornam NULL */
    return NULL;
}

/* explorarSalas: navegação interativa com coleta de pistas.
   - Parametros: raiz atual da exploração, ponteiro para raiz da BST de pistas coletadas,
     tabela hash (já populada com associações pista->suspeito).
   - Navegação: 'e' esquerda, 'd' direita, 'v' voltar ao pai (usamos pai para permitir voltar),
     's' sair. Em cada visita, exibe sala e pista (se existir) e armazena a pista na BST.
*/
void explorarSalas(Sala *atual, PistaNode **bstPistas, HashNode *hash[]) {
    if (!atual) return;
    Sala *pos = atual;
    char buffer[64];
    while (1) {
        printf("\nVocê está na sala: %s\n", pos->nome);
        const char *pista = pistaAssociadaASala(pos->nome);
        if (pista) {
            printf("  -> Você encontrou uma pista: \"%s\"\n", pista);
            /* adiciona na BST (não duplica) */
            adicionarPista(bstPistas, pista);
        } else {
            printf("  -> Nenhuma pista aparente nesta sala.\n");
        }

        mostrarSalaOpcoes(pos);
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        limpaNewline(buffer);
        if (strlen(buffer) == 0) { printf("Opção vazia. Tente novamente.\n"); continue; }
        char cmd = buffer[0];

        if (cmd == 's') {
            printf("Saindo da exploração.\n");
            return;
        } else if (cmd == 'e') {
            if (pos->esq) {
                pos = pos->esq;
            } else {
                printf("Não há sala à esquerda.\n");
            }
        } else if (cmd == 'd') {
            if (pos->dir) {
                pos = pos->dir;
            } else {
                printf("Não há sala à direita.\n");
            }
        } else if (cmd == 'v') {
            if (pos->pai) pos = pos->pai;
            else printf("Você está na raiz; não há sala pai.\n");
        } else {
            printf("Comando desconhecido. Use 'e' (esq), 'd' (dir), 'v' (voltar), 's' (sair).\n");
        }
    }
}

/* listarPistasBST: percorre em-ordem e mostra pistas coletadas */
void listarPistasBST(PistaNode *root) {
    if (!root) return;
    listarPistasBST(root->esq);
    printf("  - %s\n", root->texto);
    listarPistasBST(root->dir);
}

/* verificarSuspeitoFinal: percorre a BST de pistas, consulta hash para cada pista e conta quantas apontam
   para o suspeito acusado. Se >=2, acusação é bem-sucedida. */
void verificarSuspeitoFinal(PistaNode *root, HashNode *hash[]) {
    if (!root) {
        printf("Nenhuma pista coletada. Impossível julgar.\n");
        return;
    }
    char acusado[64];
    printf("\nDigite o nome do suspeito que deseja acusar: ");
    if (!fgets(acusado, sizeof(acusado), stdin)) return;
    limpaNewline(acusado);
    if (strlen(acusado) == 0) {
        printf("Nome vazio. Acusação cancelada.\n");
        return;
    }

    /* Percorre as pistas e conta quantas mapeiam para 'acusado' */
    int contador = 0;
    /* usar pilha recursiva: lambda-like auxiliar */
    /* Implementamos uma função interna via ponteiro recursivo (definida aqui mesmo) */
    void contarRec(PistaNode *n) {
        if (!n) return;
        contarRec(n->esq);
        const char *s = encontrarSuspeito(hash, n->texto);
        if (s && strcmp(s, acusado) == 0) {
            contador++;
        }
        contarRec(n->dir);
    }
    contarRec(root);

    printf("\nResultado da acuação contra '%s':\n", acusado);
    if (contador >= 2) {
        printf("  Sucesso! %d pista(s) suportam a acusação. O culpado foi preso.\n", contador);
    } else if (contador == 1) {
        printf("  Há apenas 1 pista apontando para '%s'. Não é suficiente para condenar.\n", acusado);
    } else {
        printf("  Nenhuma pista aponta para '%s'. Acusação infundada.\n", acusado);
    }
}

/* liberarBST: liberar memória da BST de pistas */
void liberarBST(PistaNode *root) {
    if (!root) return;
    liberarBST(root->esq);
    liberarBST(root->dir);
    free(root->texto);
    free(root);
}

/* liberarHash: libera cada cadeia na tabela hash */
void liberarHash(HashNode *hash[]) {
    for (int i = 0; i < HASH_SIZE; ++i) {
        HashNode *cur = hash[i];
        while (cur) {
            HashNode *next = cur->next;
            free(cur->pista);
            free(cur->suspeito);
            free(cur);
            cur = next;
        }
        hash[i] = NULL;
    }
}

/* montarMapa: cria manualmente o mapa (árvore de salas) e conecta pais */
void montarMapa(Sala **raiz) {
    /* Exemplo de mansão fixa:
               Entrada
              /       \
       Sala de Estar  Cozinha
         /     \         \
     Biblioteca Quarto   Garagem
      /                /
    Sotao           Jardim
    (ajuste conforme desejar)
    */
    Sala *entrada = criarSala("Entrada");
    Sala *estar = criarSala("Sala de Estar");
    Sala *cozinha = criarSala("Cozinha");
    Sala *biblioteca = criarSala("Biblioteca");
    Sala *quarto = criarSala("Quarto");
    Sala *garagem = criarSala("Garagem");
    Sala *sotao = criarSala("Sotao");
    Sala *jardim = criarSala("Jardim");
    Sala *escritorio = criarSala("Escritorio");
    Sala *banheiro = criarSala("Banheiro");

    /* ligações */
    entrada->esq = estar; estar->pai = entrada;
    entrada->dir = cozinha; cozinha->pai = entrada;

    estar->esq = biblioteca; biblioteca->pai = estar;
    estar->dir = quarto; quarto->pai = estar;

    biblioteca->esq = sotao; sotao->pai = biblioteca;
    cozinha->dir = garagem; garagem->pai = cozinha;

    garagem->esq = jardim; jardim->pai = garagem;

    quarto->esq = escritorio; escritorio->pai = quarto;
    quarto->dir = banheiro; banheiro->pai = quarto;

    *raiz = entrada;
}

/* mostrarSalaOpcoes: mostra as opções disponíveis na exploração (inclui voltar para usabilidade) */
void mostrarSalaOpcoes(Sala *atual) {
    printf("\nOpções de navegação:\n");
    if (atual->esq) printf("  e - ir para a esquerda (%s)\n", atual->esq->nome);
    if (atual->dir) printf("  d - ir para a direita (%s)\n", atual->dir->nome);
    if (atual->pai) printf("  v - voltar para a sala (%s)\n", atual->pai->nome);
    printf("  s - sair da exploração\n");
    printf("Escolha (e/d/v/s): ");
}

/* limpaNewline: remove '\n' deixado por fgets */
void limpaNewline(char *s) {
    if (!s) return;
    size_t l = strlen(s);
    if (l > 0 && s[l-1] == '\n') s[l-1] = '\0';
}

/* ---------- MAIN (montagem do hash, mapa e loop principal) ---------- */
int main(void) {
    /* seed aleatória para eventuais expansões (não estritamente necessária aqui) */
    srand((unsigned) time(NULL));

    /* tabela hash vazia */
    HashNode *hash[HASH_SIZE];
    for (int i = 0; i < HASH_SIZE; ++i) hash[i] = NULL;

    /* Preenchemos a tabela hash com associações pista -> suspeito
       (regras codificadas; pode ser ajustado). */
    inserirNaHash(hash, "pegada molhada", "Sr. Verde");
    inserirNaHash(hash, "cinzeiro quebrado", "Sra. Azul");
    inserirNaHash(hash, "marcador de livro rasgado", "Prof. Roxo");
    inserirNaHash(hash, "panela com restos", "Sra. Azul");
    inserirNaHash(hash, "bilhete amassado", "Sr. Verde");
    inserirNaHash(hash, "fio de cabelo loiro", "Sra. Rosa");
    inserirNaHash(hash, "detergente derramado", "Sr. Amarelo");
    inserirNaHash(hash, "pegada de sola distinta", "Sr. Verde");
    inserirNaHash(hash, "ferramenta enferrujada", "Prof. Roxo");
    inserirNaHash(hash, "marca de pneu", "Sr. Amarelo");
    /* se desejar, mais pistas podem ser adicionadas */

    /* monta mapa fixo */
    Sala *raiz = NULL;
    montarMapa(&raiz);

    /* BST de pistas coletadas (inicialmente vazia) */
    PistaNode *bstPistas = NULL;

    /* iniciar exploração a partir da raiz */
    printf("Bem-vindo(a) a Detective Quest - Explore a mansão e colete pistas!\n");
    printf("Navegue pelas salas e ao terminar faremos o julgamento.\n");

    explorarSalas(raiz, &bstPistas, hash);

    /* Fim da exploração: listamos pistas e pedimos acusação */
    printf("\n--- Pistas coletadas (ordenadas) ---\n");
    if (!bstPistas) {
        printf("Nenhuma pista coletada.\n");
    } else {
        listarPistasBST(bstPistas);
    }

    verificarSuspeitoFinal(bstPistas, hash);

    /* liberar memória (BST e hash). Observação: salas não são liberadas aqui para simplificar;
       em código de produção você liberaria todas as salas também. */
    liberarBST(bstPistas);
    liberarHash(hash);

    printf("\nObrigado por jogar Detective Quest!\n");

    return 0;
}
