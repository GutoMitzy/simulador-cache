#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

void memoria_cache(FILE *arq, int escrita, int tam_linha, int num_linhas, int assoc, int hit_time, char subs[], int tempo_mem) {
    int num_conjuntos = num_linhas / assoc;
    LinhaCache **cache = (LinhaCache**) malloc(num_conjuntos * sizeof(LinhaCache*));
    char endereco[32], op;
    unsigned int num;
    int palavra, linha, tag, hit, i, j;
    int op_escrita = 0, op_leitura = 0, hit_total = 0;
    int hit_escrita = 0, hit_leitura = 0;
    int mem_escrita = 0, mem_leitura = 0;
    int tempo = 0;
    int tempo_total = 0;
    int bits_offset = conta_bits(tam_linha);
    int bits_indice = conta_bits(num_conjuntos);
    srand(time(NULL));
    for(i = 0; i < num_conjuntos; i++) {
        cache[i] = (LinhaCache*) malloc(assoc * sizeof(LinhaCache));
        for(j = 0; j < assoc; j++) {
            cache[i][j].tag = -1;
            cache[i][j].dirty = 0;
            cache[i][j].lru = -1;
            cache[i][j].valido = 0;
        }
    }
    int total_ops = 0;
    while(fscanf(arq, "%s %c", endereco, &op) != EOF) {
        total_ops++;
        if(op == 'W') op_escrita++;
        else op_leitura++;
        tempo_total += hit_time;
        num = strtoul(endereco, NULL, 16);
        palavra = num & ((1 << bits_offset) - 1);
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
                if(strcmp(subs, "Aleatoria") == 0) {
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
                if(escrita == 0) {
                    mem_escrita++;
                    tempo_total += tempo_mem;
                } else {
                    cache[linha][pos].dirty = 1;
                }
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
    float tempo_acesso = hit_time + (1 - (float)hit_total / total_ops) * tempo_mem;
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
    printf("Hit Rate Leitura: %.4f%% (%d)\n", (float)hit_leitura / total_ops * 100, hit_leitura);
    printf("Hit Rate Escrita: %.4f%% (%d)\n", (float)hit_escrita / total_ops * 100, hit_escrita);
    printf("Hit Rate Global: %.4f%% (%d)\n", (float)hit_total / total_ops * 100, hit_total);
    printf("Tempo Medio de Acesso: %.4f ns\n", tempo_acesso);
    for(i = 0; i < num_conjuntos; i++) free(cache[i]);
    free(cache);
}

int main() {
    FILE *arq = fopen("teste_curto.txt", "rt");
    memoria_cache(arq, 1, 64, 4096, 2, 10, "Aleatoria", 80);
    fclose(arq);
    return 0;
}
