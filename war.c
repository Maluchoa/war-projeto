
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_TERR 10
#define MAX_MISSOES 6

/* Estrutura de território */
typedef struct {
    char nome[30];
    char cor[10];  /* dono: "vermelho", "azul", "neutro" */
    int tropas;
} Territorio;

/* --- Prototipos --- */
void atribuirMissao(char* destino, char* missoes[], int totalMissoes);
int verificarMissao(char* missao, Territorio* mapa, int tamanho);
void exibirMissao(const char* missao); /* passagem por valor (pointer passado por valor) */
void atacar(Territorio* atacante, Territorio* defensor);
void exibirMapa(Territorio* mapa, int tamanho);
void liberarMemoria(Territorio* mapa, char** missoesJogadores, int numPlayers);

/* Variável global que informa a cor do jogador atual para a função verificarMissao
   (seguindo a assinatura pedida verificarMissao(char*, Territorio*, int)). */
char jogadorAtualCor[10];

/* --- Implementações --- */

/* Escolhe aleatoriamente uma missão do vetor e copia para destino (destino deve estar alocado) */
void atribuirMissao(char* destino, char* missoes[], int totalMissoes) {
    int idx = rand() % totalMissoes;
    strcpy(destino, missoes[idx]); /* cópia conforme requisitado */
}

/* Exibe a missão (passagem por valor no sentido de não modificar) */
void exibirMissao(const char* missao) {
    printf("  Missao: %s\n", missao);
}

/* Simples verificação de missão para o jogador cuja cor está em jogadorAtualCor.
   As missões são frases fixas e a função detecta qual condição aplicar. */
int verificarMissao(char* missao, Territorio* mapa, int tamanho) {
    int i, count;
    /* 1) Conquistar 3 territorios seguidos */
    if (strstr(missao, "Conquistar 3 territorios seguidos") != NULL) {
        int consec = 0;
        for (i = 0; i < tamanho; ++i) {
            if (strcmp(mapa[i].cor, jogadorAtualCor) == 0) {
                consec++;
                if (consec >= 3) return 1;
            } else consec = 0;
        }
        return 0;
    }

    /* 2) Controlar 5 territorios */
    if (strstr(missao, "Controlar 5 territorios") != NULL) {
        count = 0;
        for (i = 0; i < tamanho; ++i)
            if (strcmp(mapa[i].cor, jogadorAtualCor) == 0) count++;
        return (count >= 5) ? 1 : 0;
    }

    /* 3) Eliminar todas as tropas da cor vermelho/azul */
    if (strstr(missao, "Eliminar todas as tropas da cor") != NULL) {
        /* extrair cor alvo da string (última palavra) */
        char copia[80];
        strcpy(copia, missao);
        char* token = strrchr(copia, ' ');
        if (token) {
            char corAlvo[10];
            strcpy(corAlvo, token + 1); /* cor alvo */
            /* somar tropas da cor alvo */
            int soma = 0;
            for (i = 0; i < tamanho; ++i)
                if (strcmp(mapa[i].cor, corAlvo) == 0) soma += mapa[i].tropas;
            return (soma == 0) ? 1 : 0;
        }
        return 0;
    }

    /* 4) Ter ao menos 10 tropas no total (somar tropas de sua cor) */
    if (strstr(missao, "Ter ao menos 10 tropas no total") != NULL) {
        int soma = 0;
        for (i = 0; i < tamanho; ++i)
            if (strcmp(mapa[i].cor, jogadorAtualCor) == 0) soma += mapa[i].tropas;
        return (soma >= 10) ? 1 : 0;
    }

    /* 5) Controlar 8 territorios (exemplo) */
    if (strstr(missao, "Controlar 8 territorios") != NULL) {
        count = 0;
        for (i = 0; i < tamanho; ++i)
            if (strcmp(mapa[i].cor, jogadorAtualCor) == 0) count++;
        return (count >= 8) ? 1 : 0;
    }

    /* se missão desconhecida, retorna 0 (não cumprida) */
    return 0;
}

/* Função de ataque:
   - valida que cores são diferentes antes de atacar
   - rolagens rand() entre 1 e 6
   - se atacante vence: defensor muda de cor e recebe metade das tropas do atacante
   - se atacante perde: atacante perde 1 tropa (se possível)
*/
void atacar(Territorio* atacante, Territorio* defensor) {
    if (!atacante || !defensor) return;
    if (strcmp(atacante->cor, defensor->cor) == 0) {
        printf("  Ataque inválido: mesmo dono.\n");
        return;
    }
    if (atacante->tropas <= 0) {
        printf("  Ataque inválido: atacante sem tropas.\n");
        return;
    }

    int atRoll = (rand() % 6) + 1;
    int defRoll = (rand() % 6) + 1;
    printf("  %s (%s, tropas=%d) rola %d vs %s (%s, tropas=%d) rola %d\n",
           atacante->nome, atacante->cor, atacante->tropas, atRoll,
           defensor->nome, defensor->cor, defensor->tropas, defRoll);

    if (atRoll > defRoll) {
        /* atacante vence */
        int transfer = atacante->tropas / 2;
        if (transfer < 1) transfer = 1;
        strcpy(defensor->cor, atacante->cor);
        defensor->tropas = transfer;
        atacante->tropas -= transfer;
        if (atacante->tropas < 0) atacante->tropas = 0;
        printf("  Ataque bem sucedido! %s agora pertence a %s com %d tropas (transferidas).\n",
               defensor->nome, defensor->cor, defensor->tropas);
    } else {
        /* atacante perde 1 tropa */
        if (atacante->tropas > 0) atacante->tropas -= 1;
        printf("  Ataque falhou. %s perde 1 tropa (tropas agora: %d).\n",
               atacante->nome, atacante->tropas);
    }
}

/* Exibe mapa de forma simples */
void exibirMapa(Territorio* mapa, int tamanho) {
    int i;
    printf("Mapa atual:\n");
    for (i = 0; i < tamanho; ++i) {
        printf("  %2d: %-10s | dono: %-8s | tropas: %2d\n",
               i+1, mapa[i].nome, mapa[i].cor, mapa[i].tropas);
    }
}

/* Libera memória alocada (territorios e as strings de missao de cada jogador) */
void liberarMemoria(Territorio* mapa, char** missoesJogadores, int numPlayers) {
    if (mapa) free(mapa);
    if (missoesJogadores) {
        for (int i = 0; i < numPlayers; ++i)
            if (missoesJogadores[i]) free(missoesJogadores[i]);
        free(missoesJogadores);
    }
}

/* --- MAIN: inicializa, atribui missões, simula turnos e verifica missões --- */
int main() {
    srand((unsigned)time(NULL));

    /* 1) vetor de missoes (pelo menos 5) */
    char* missoesPadrao[MAX_MISSOES] = {
        "Conquistar 3 territorios seguidos",
        "Controlar 5 territorios",
        "Eliminar todas as tropas da cor vermelho",
        "Ter ao menos 10 tropas no total",
        "Eliminar todas as tropas da cor azul",
        "Controlar 8 territorios"
    };

    int totalMissoes = MAX_MISSOES;

    /* 2) criar mapa dinamicamente */
    Territorio* mapa = (Territorio*) calloc(MAX_TERR, sizeof(Territorio));
    if (!mapa) {
        perror("calloc mapa");
        return 1;
    }

    /* inicializa alguns territorios com donos alternados e tropas aleatorias */
    for (int i = 0; i < MAX_TERR; ++i) {
        snprintf(mapa[i].nome, sizeof(mapa[i].nome), "Terr-%02d", i+1);
        if (i % 2 == 0) strcpy(mapa[i].cor, "vermelho");
        else strcpy(mapa[i].cor, "azul");
        mapa[i].tropas = (rand() % 6) + 1; /* 1..6 tropas iniciais */
    }

    /* 3) jogadores e suas missões (armazenadas dinamicamente) */
    const int numPlayers = 2;
    char* coresJogadores[numPlayers];
    coresJogadores[0] = "vermelho";
    coresJogadores[1] = "azul";

    /* aloca array de ponteiros para missões e cada missão recebe malloc */
    char** missoesJogadores = (char**) malloc(numPlayers * sizeof(char*));
    if (!missoesJogadores) {
        perror("malloc missoesJogadores");
        free(mapa);
        return 1;
    }

    for (int p = 0; p < numPlayers; ++p) {
        missoesJogadores[p] = (char*) malloc(100 * sizeof(char)); /* espaço suficiente */
        if (!missoesJogadores[p]) {
            perror("malloc missao");
            /* liberar já alocado */
            for (int k = 0; k < p; ++k) free(missoesJogadores[k]);
            free(missoesJogadores);
            free(mapa);
            return 1;
        }
        atribuirMissao(missoesJogadores[p], missoesPadrao, totalMissoes);
    }

    /* exibe missões (apenas uma vez, conforme requisito) */
    printf("Missões sorteadas (mostradas apenas uma vez):\n");
    for (int p = 0; p < numPlayers; ++p) {
        printf("Jogador %d (%s):\n", p+1, coresJogadores[p]);
        exibirMissao(missoesJogadores[p]);
    }
    printf("\n");

    /* exibe mapa inicial */
    exibirMapa(mapa, MAX_TERR);
    printf("\n");

    /* 4) laço de jogo simplificado: alterna turnos, realiza 1 ataque por turno com escolhas aleatórias */
    int vencedor = -1;
    int maxTurnos = 200;
    for (int turno = 1; turno <= maxTurnos && vencedor == -1; ++turno) {
        printf("----- Turno %d -----\n", turno);
        for (int p = 0; p < numPlayers && vencedor == -1; ++p) {
            printf("> Vez do jogador %d (%s)\n", p+1, coresJogadores[p]);
            /* escolhe um territorio atacante aleatorio do jogador que tenha tropas > 0 */
            int attackerIdx = -1;
            int defenderIdx = -1;
            int attempts = 0;
            while (attempts < 50) { /* tenta achar um atacante válido */
                int idx = rand() % MAX_TERR;
                if (strcmp(mapa[idx].cor, coresJogadores[p]) == 0 && mapa[idx].tropas > 0) {
                    attackerIdx = idx;
                    break;
                }
                attempts++;
            }
            /* escolhe defensor: territorio de inimigo */
            attempts = 0;
            while (attempts < 50) {
                int idx = rand() % MAX_TERR;
                if (strcmp(mapa[idx].cor, coresJogadores[p]) != 0) {
                    defenderIdx = idx;
                    break;
                }
                attempts++;
            }

            if (attackerIdx != -1 && defenderIdx != -1) {
                printf("Jogador %s ataca: %s -> %s\n",
                       coresJogadores[p], mapa[attackerIdx].nome, mapa[defenderIdx].nome);
                atacar(&mapa[attackerIdx], &mapa[defenderIdx]);
            } else {
                printf("  Sem ataques possiveis neste turno.\n");
            }

            /* verificar missão do jogador silenciosamente ao fim do turno */
            strcpy(jogadorAtualCor, coresJogadores[p]); /* informa qual jogador estamos testando */
            if (verificarMissao(missoesJogadores[p], mapa, MAX_TERR)) {
                vencedor = p;
                printf("\n*** Jogador %d (%s) cumpriu a missao: %s ***\n",
                       p+1, coresJogadores[p], missoesJogadores[p]);
                break;
            }
        }

        exibirMapa(mapa, MAX_TERR);
        printf("\n");
    }

    if (vencedor == -1) {
        printf("Fim do jogo: nenhum jogador cumpriu sua missao em %d turnos.\n", maxTurnos);
    } else {
        printf("Vencedor: Jogador %d (%s)\n", vencedor+1, coresJogadores[vencedor]);
    }

    /* liberar memoria */
    liberarMemoria(mapa, missoesJogadores, numPlayers);

    return 0;
}
