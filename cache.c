#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

typedef struct {
    int tag;
    int dirty;
    int lru;
    int valido;
} LinhaCache;

int conta_bits(int n) {
    int bits = 0;
    while(n > 1) {
        n = n / 2;
        bits++;
    }
    return bits;
}

int eh_potencia_2(int n) {
    return n > 0 && (n & (n - 1)) == 0;
}

int compara_texto(char a[], char b[]) {
    int i = 0;
    while(a[i] != '\0' && b[i] != '\0') {
        if(toupper((unsigned char)a[i]) != toupper((unsigned char)b[i])) return 0;
        i++;
    }
    return a[i] == '\0' && b[i] == '\0';
}

int politica_substituicao_valida(char subs[]) {
    return compara_texto(subs, "LRU") || compara_texto(subs, "Aleatoria");
}

void imprime_uso(char nome[]) {
    printf("Uso com arquivo: %s <arquivo> <politica_escrita> <tam_linha> <num_linhas> <assoc> <hit_time> <LRU|Aleatoria> <tempo_mem>\n", nome);
    printf("Uso padrao:      %s <politica_escrita> <tam_linha> <num_linhas> <assoc> <hit_time> <LRU|Aleatoria> <tempo_mem>\n", nome);
    printf("Politica de escrita: 0 - write-through, 1 - write-back.\n");
}

void memoria_cache(FILE *arq, int escrita, int tam_linha, int num_linhas, int assoc, int hit_time, char subs[], int tempo_mem) {
    int num_conjuntos = num_linhas / assoc;
    LinhaCache **cache = (LinhaCache**) malloc(num_conjuntos * sizeof(LinhaCache*));
    char endereco[32], op;
    unsigned int num;
    int linha, tag, hit, i, j;
    int op_escrita = 0, op_leitura = 0, hit_total = 0;
    int hit_escrita = 0, hit_leitura = 0;
    int mem_escrita = 0, mem_leitura = 0;
    int tempo = 0;
    int tempo_total = 0;
    int bits_offset = conta_bits(tam_linha);
    int bits_indice = conta_bits(num_conjuntos);
    srand(time(NULL));
    if(cache == NULL) {
        printf("Erro ao alocar memoria para a cache.\n");
        return;
    }
    for(i = 0; i < num_conjuntos; i++) {
        cache[i] = (LinhaCache*) malloc(assoc * sizeof(LinhaCache));
        if(cache[i] == NULL) {
            printf("Erro ao alocar memoria para a cache.\n");
            while(i > 0) free(cache[--i]);
            free(cache);
            return;
        }
        for(j = 0; j < assoc; j++) {
            cache[i][j].tag = -1;
            cache[i][j].dirty = 0;
            cache[i][j].lru = -1;
            cache[i][j].valido = 0;
        }
    }
    int total_ops = 0;
    while(fscanf(arq, "%31s %c", endereco, &op) != EOF) {
        total_ops++;
        if(op == 'W') op_escrita++;
        else op_leitura++;
        tempo_total += hit_time;
        num = strtoul(endereco, NULL, 16);
        linha = (num >> bits_offset) & ((1 << bits_indice) - 1);
        tag = num >> (bits_offset + bits_indice);
        hit = 0;
        for(i = 0; i < assoc; i++) {
            if(cache[linha][i].valido && cache[linha][i].tag == tag) {
                hit = 1;
                hit_total++;
                if(op == 'W') hit_escrita++;
                else hit_leitura++;
                cache[linha][i].lru = tempo++;
                if(op == 'W') {
                    if(escrita == 0) {
                        mem_escrita++;
                        tempo_total += tempo_mem;
                    } else {
                        cache[linha][i].dirty = 1;
                    }
                }
                break;
            }
        }
        if(hit == 0) {
            if(op == 'W' && escrita == 0) {
                mem_escrita++;
                tempo_total += tempo_mem;
                continue;
            }

            mem_leitura++;
            tempo_total += tempo_mem;
            int pos = -1;
            for(i = 0; i < assoc; i++) {
                if(cache[linha][i].valido == 0) {
                    pos = i;
                    break;
                }
            }
            if(pos == -1) {
                if(compara_texto(subs, "Aleatoria")) {
                    pos = rand() % assoc;
                } else {
                    int menor = cache[linha][0].lru;
                    pos = 0;
                    for(i = 1; i < assoc; i++) {
                        if(cache[linha][i].lru < menor) {
                            menor = cache[linha][i].lru;
                            pos = i;
                        }
                    }
                }
                if(escrita == 1 && cache[linha][pos].dirty) {
                    mem_escrita++;
                    tempo_total += tempo_mem;
                }
            }
            cache[linha][pos].valido = 1;
            cache[linha][pos].tag = tag;
            cache[linha][pos].lru = tempo++;
            cache[linha][pos].dirty = 0;
            if(op == 'W') {
                cache[linha][pos].dirty = 1;
            }
        }
    }
    if(escrita == 1) {
        for(i = 0; i < num_conjuntos; i++) {
            for(j = 0; j < assoc; j++) {
                if(cache[i][j].valido && cache[i][j].dirty) {
                    mem_escrita++;
                    tempo_total += tempo_mem;
                }
            }
        }
    }
    float taxa_hit_leitura = op_leitura > 0 ? (float)hit_leitura / op_leitura * 100 : 0;
    float taxa_hit_escrita = op_escrita > 0 ? (float)hit_escrita / op_escrita * 100 : 0;
    float taxa_hit_global = total_ops > 0 ? (float)hit_total / total_ops * 100 : 0;
    float tempo_acesso = total_ops > 0 ? hit_time + (1 - (float)hit_total / total_ops) * tempo_mem : 0;
    printf("\nINFORMACOES DE ENTRADA -----------------------\n");
    printf("Politica de Escrita: %s.\n", escrita==1 ? "Write-Back" : "Write-Through");
    printf("Tamanho da Linha: %d.\n", tam_linha);
    printf("Numero de Linhas: %d.\n", num_linhas);
    printf("Associatividade por Conjunto: %d.\n", assoc);
    printf("Tempo de Acesso (Hit): %d.\n", hit_time);
    printf("Politica de Substituicao: %s.\n", subs);
    printf("Tempo de Memoria: %d.\n\n", tempo_mem);
    printf("INFORMACOES DO ARQUIVO -----------------------\n");
    printf("Escritas: %d.\n", op_escrita);
    printf("Leituras: %d.\n", op_leitura);
    printf("Total: %d.\n\n", total_ops);
    printf("INFORMACOES DA SIMULACAO -----------------------\n");
    printf("Memoria Principal Leitura: %d.\n", mem_leitura);
    printf("Memoria Principal Escrita: %d.\n", mem_escrita);
    printf("Hit Rate Leitura: %.4f%% (%d)\n", taxa_hit_leitura, hit_leitura);
    printf("Hit Rate Escrita: %.4f%% (%d)\n", taxa_hit_escrita, hit_escrita);
    printf("Hit Rate Global: %.4f%% (%d)\n", taxa_hit_global, hit_total);
    printf("Tempo Medio de Acesso: %.4f ns\n", tempo_acesso);
    for(i = 0; i < num_conjuntos; i++) free(cache[i]);
    free(cache);
}

int main(int argc, char *argv[]) {
    FILE *arq;
    char *arquivo = "teste_curto.txt";
    char *subs;
    int inicio = 1;
    int escrita, tam_linha, num_linhas, assoc, hit_time, tempo_mem;

    if(argc == 9) {
        arquivo = argv[1];
        inicio = 2;
    } else if(argc != 8) {
        imprime_uso(argv[0]);
        return 1;
    }

    escrita = atoi(argv[inicio]);
    tam_linha = atoi(argv[inicio + 1]);
    num_linhas = atoi(argv[inicio + 2]);
    assoc = atoi(argv[inicio + 3]);
    hit_time = atoi(argv[inicio + 4]);
    subs = argv[inicio + 5];
    tempo_mem = atoi(argv[inicio + 6]);

    if(escrita != 0 && escrita != 1) {
        printf("Erro: politica de escrita deve ser 0 (write-through) ou 1 (write-back).\n");
        return 1;
    }
    if(!eh_potencia_2(tam_linha) || !eh_potencia_2(num_linhas) || !eh_potencia_2(assoc)) {
        printf("Erro: tamanho da linha, numero de linhas e associatividade devem ser potencias de 2.\n");
        return 1;
    }
    if(assoc < 1 || assoc > num_linhas || num_linhas % assoc != 0) {
        printf("Erro: associatividade deve ser no minimo 1 e no maximo igual ao numero de linhas.\n");
        return 1;
    }
    if(hit_time < 0 || tempo_mem < 0) {
        printf("Erro: tempos de acesso devem ser maiores ou iguais a zero.\n");
        return 1;
    }
    if(!politica_substituicao_valida(subs)) {
        printf("Erro: politica de substituicao deve ser LRU ou Aleatoria.\n");
        return 1;
    }

    arq = fopen(arquivo, "rt");
    if(arq == NULL) {
        printf("Erro ao abrir o arquivo %s.\n", arquivo);
        return 1;
    }

    memoria_cache(arq, escrita, tam_linha, num_linhas, assoc, hit_time, subs, tempo_mem);
    fclose(arq);
    return 0;
}
