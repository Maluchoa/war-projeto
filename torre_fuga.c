/*
  Sistema de priorização e montagem de componentes da torre de fuga
  - Bubble sort por nome (string)
  - Insertion sort por tipo (string)
  - Selection sort por prioridade (int)
  - Busca binária por nome (aplicável após ordenar por nome)
  - Contagem de comparações e tempo de execução usando clock()
  - Menu interativo e entrada via fgets

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_COMPONENTES 20
#define TAM_NOME 30
#define TAM_TIPO 20

/* Estrutura do componente */
typedef struct {
    char nome[TAM_NOME];
    char tipo[TAM_TIPO];
    int prioridade; /* 1..10 */
} Componente;

/* Protótipos */
void cadastrarComponentes(Componente comps[], int *n);
void mostrarComponentes(const Componente comps[], int n);
void bubbleSortNome(Componente comps[], int n, long *comparacoes);
void insertionSortTipo(Componente comps[], int n, long *comparacoes);
void selectionSortPrioridade(Componente comps[], int n, long *comparacoes);
int buscaBinariaPorNome(const Componente comps[], int n, const char chave[], long *comparacoes);
void medirTempo(void (*algoritmo)(Componente*, int, long*), Componente comps[], int n, long *comparacoes, double *tempoSeg);
void limpaNewline(char *s);

/* --- Implementações --- */

/* Limpa '\n' deixado por fgets */
void limpaNewline(char *s) {
    if (!s) return;
    size_t len = strlen(s);
    if (len > 0 && s[len-1] == '\n') s[len-1] = '\0';
}

/* Cadastro de componentes (até MAX_COMPONENTES) usando fgets */
void cadastrarComponentes(Componente comps[], int *n) {
    char buffer[128];
    int quantidade = 0;
    printf("Quantos componentes deseja cadastrar? (1-%d): ", MAX_COMPONENTES);
    if (!fgets(buffer, sizeof(buffer), stdin)) return;
    quantidade = atoi(buffer);
    if (quantidade < 1) quantidade = 1;
    if (quantidade > MAX_COMPONENTES) quantidade = MAX_COMPONENTES;

    for (int i = 0; i < quantidade; ++i) {
        printf("\nComponente %d:\n", i+1);
        printf("  Nome (ex: chip central): ");
        if (!fgets(comps[i].nome, sizeof(comps[i].nome), stdin)) comps[i].nome[0] = '\0';
        limpaNewline(comps[i].nome);
        if (strlen(comps[i].nome) == 0) strncpy(comps[i].nome, "unnamed", TAM_NOME);

        printf("  Tipo (ex: controle, suporte, propulsao): ");
        if (!fgets(comps[i].tipo, sizeof(comps[i].tipo), stdin)) comps[i].tipo[0] = '\0';
        limpaNewline(comps[i].tipo);
        if (strlen(comps[i].tipo) == 0) strncpy(comps[i].tipo, "generico", TAM_TIPO);

        /* prioridade */
        int p = 0;
        while (1) {
            printf("  Prioridade (1-10): ");
            if (!fgets(buffer, sizeof(buffer), stdin)) break;
            p = atoi(buffer);
            if (p >= 1 && p <= 10) break;
            printf("    Prioridade invalida. Tente novamente.\n");
        }
        comps[i].prioridade = p;
    }
    *n = quantidade;
}

/* Exibe os componentes formatados */
void mostrarComponentes(const Componente comps[], int n) {
    printf("\n--- Componentes (%d) ---\n", n);
    printf("%-3s | %-28s | %-15s | %-9s\n", "No", "Nome", "Tipo", "Prioridade");
    printf("----+------------------------------+-----------------+----------\n");
    for (int i = 0; i < n; ++i) {
        printf("%-3d | %-28s | %-15s | %-9d\n",
               i+1, comps[i].nome, comps[i].tipo, comps[i].prioridade);
    }
    printf("------------------------------\n");
}

/* Bubble sort por nome (ordem lexicográfica crescente)
   Conta comparações: uma comparação é cada strcmp entre nomes */
void bubbleSortNome(Componente comps[], int n, long *comparacoes) {
    *comparacoes = 0;
    int trocou;
    for (int i = 0; i < n-1; ++i) {
        trocou = 0;
        for (int j = 0; j < n-1-i; ++j) {
            (*comparacoes)++;
            if (strcmp(comps[j].nome, comps[j+1].nome) > 0) {
                Componente tmp = comps[j];
                comps[j] = comps[j+1];
                comps[j+1] = tmp;
                trocou = 1;
            }
        }
        if (!trocou) break;
    }
}

/* Insertion sort por tipo (string) - crescente
   Conta comparações: cada comparação de tipo (strcmp) */
void insertionSortTipo(Componente comps[], int n, long *comparacoes) {
    *comparacoes = 0;
    for (int i = 1; i < n; ++i) {
        Componente chave = comps[i];
        int j = i - 1;
        /* primeiro teste: while j >=0 && comps[j].tipo > chave.tipo */
        while (j >= 0) {
            (*comparacoes)++;
            if (strcmp(comps[j].tipo, chave.tipo) > 0) {
                comps[j+1] = comps[j];
                j--;
            } else {
                break;
            }
        }
        comps[j+1] = chave;
    }
}

/* Selection sort por prioridade (int) - crescente (prioridade menor -> primeiro)
   Conta comparações: cada comparação entre prioridades */
void selectionSortPrioridade(Componente comps[], int n, long *comparacoes) {
    *comparacoes = 0;
    for (int i = 0; i < n-1; ++i) {
        int idxMin = i;
        for (int j = i+1; j < n; ++j) {
            (*comparacoes)++;
            if (comps[j].prioridade < comps[idxMin].prioridade) {
                idxMin = j;
            }
        }
        if (idxMin != i) {
            Componente tmp = comps[i];
            comps[i] = comps[idxMin];
            comps[idxMin] = tmp;
        }
    }
}

/* Busca binária por nome (assume vetor ordenado por nome, crescente)
   Retorna índice (0..n-1) se encontrado, -1 caso contrário.
   Conta comparações: cada strcmp com o elemento do meio conta como 1. */
int buscaBinariaPorNome(const Componente comps[], int n, const char chave[], long *comparacoes) {
    int left = 0, right = n - 1;
    *comparacoes = 0;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        (*comparacoes)++;
        int cmp = strcmp(comps[mid].nome, chave);
        if (cmp == 0) return mid;
        else if (cmp < 0) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

/* medirTempo: recebe uma função de ordenação (com assinatura (Componente*,int,long*))
   e executa medição de tempo em segundos e retorna comparações via ponteiro. */
void medirTempo(void (*algoritmo)(Componente*, int, long*), Componente comps[], int n, long *comparacoes, double *tempoSeg) {
    clock_t inicio = clock();
    algoritmo(comps, n, comparacoes);
    clock_t fim = clock();
    *tempoSeg = (double)(fim - inicio) / CLOCKS_PER_SEC;
}

/* --- Função principal com menu interativo --- */
int main(void) {
    Componente componentes[MAX_COMPONENTES];
    Componente copia[MAX_COMPONENTES]; /* cópia para permitir reordenações sem perder entrada original */
    int n = 0;
    char buffer[128];

    printf("=== Montagem da Torre de Fuga - Módulo de Priorização ===\n\n");

    /* Cadastro */
    cadastrarComponentes(componentes, &n);
    /* copia para estado original */
    for (int i = 0; i < n; ++i) copia[i] = componentes[i];

    int escolha;
    int ordenadoPorNome = 0; /* flag para permitir busca binária apenas se ordenado por nome */

    do {
        printf("\nMenu:\n");
        printf(" 1 - Mostrar componentes (estado atual)\n");
        printf(" 2 - Ordenar por NOME (Bubble Sort)\n");
        printf(" 3 - Ordenar por TIPO (Insertion Sort)\n");
        printf(" 4 - Ordenar por PRIORIDADE (Selection Sort)\n");
        printf(" 5 - Buscar componente-chave por NOME (busca binária) [requer ordenacao por NOME]\n");
        printf(" 6 - Resetar para entrada original\n");
        printf(" 0 - Sair\n");
        printf("Escolha uma opcao: ");
        if (!fgets(buffer, sizeof(buffer), stdin)) break;
        escolha = atoi(buffer);

        if (escolha == 1) {
            mostrarComponentes(componentes, n);
        } else if (escolha == 2) {
            /* ordena por nome usando bubble sort */
            /* faz cópia para operar sem perder a ordem anterior? aqui modificamos o vetor atual */
            long comps = 0;
            double tempo = 0.0;
            medirTempo(bubbleSortNome, componentes, n, &comps, &tempo);
            printf("\n[Bubble Sort por NOME] Comparacoes: %ld | Tempo: %.6f s\n", comps, tempo);
            mostrarComponentes(componentes, n);
            ordenadoPorNome = 1;
        } else if (escolha == 3) {
            long comps = 0;
            double tempo = 0.0;
            medirTempo(insertionSortTipo, componentes, n, &comps, &tempo);
            printf("\n[Insertion Sort por TIPO] Comparacoes: %ld | Tempo: %.6f s\n", comps, tempo);
            mostrarComponentes(componentes, n);
            ordenadoPorNome = 0;
        } else if (escolha == 4) {
            long comps = 0;
            double tempo = 0.0;
            medirTempo(selectionSortPrioridade, componentes, n, &comps, &tempo);
            printf("\n[Selection Sort por PRIORIDADE] Comparacoes: %ld | Tempo: %.6f s\n", comps, tempo);
            mostrarComponentes(componentes, n);
            ordenadoPorNome = 0;
        } else if (escolha == 5) {
            if (!ordenadoPorNome) {
                printf("\nERRO: A busca binaria por nome exige que os componentes estejam ordenados por NOME (Bubble Sort). Deseja ordenar agora? (s/n): ");
                if (!fgets(buffer, sizeof(buffer), stdin)) break;
                if (buffer[0] == 's' || buffer[0] == 'S') {
                    long comps = 0;
                    double tempo = 0.0;
                    medirTempo(bubbleSortNome, componentes, n, &comps, &tempo);
                    printf("\n[Bubble Sort por NOME] Comparacoes: %ld | Tempo: %.6f s\n", comps, tempo);
                    mostrarComponentes(componentes, n);
                    ordenadoPorNome = 1;
                } else {
                    printf("Busca cancelada. Ordene por nome antes de usar a busca binaria.\n");
                    continue;
                }
            }

            printf("Digite o NOME exato do componente a buscar: ");
            if (!fgets(buffer, sizeof(buffer), stdin)) break;
            limpaNewline(buffer);
            if (strlen(buffer) == 0) {
                printf("Nome vazio. Abortando busca.\n");
                continue;
            }
            long compsBusca = 0;
            int idx = buscaBinariaPorNome(componentes, n, buffer, &compsBusca);
            printf("Comparacoes na busca binaria: %ld\n", compsBusca);
            if (idx >= 0) {
                printf("Componente encontrado na posicao %d:\n", idx+1);
                printf("  Nome: %s\n  Tipo: %s\n  Prioridade: %d\n",
                       componentes[idx].nome, componentes[idx].tipo, componentes[idx].prioridade);
                printf("\n>> Componente-chave presente: ativacao possivel!\n");
            } else {
                printf("Componente '%s' NAO encontrado no vetor.\n", buffer);
            }
        } else if (escolha == 6) {
            /* restaurar do backup */
            for (int i = 0; i < n; ++i) componentes[i] = copia[i];
            ordenadoPorNome = 0;
            printf("Estado restaurado para os dados de entrada originais.\n");
        } else if (escolha == 0) {
            printf("Saindo...\n");
        } else {
            printf("Opcao invalida.\n");
        }

    } while (escolha != 0);

    return 0;
}
