    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    typedef struct {
        int tag;        // referência pra memória
        int dirty;      // 1 se a linha foi modificada (para write-back)
    } LinhaCache;



    void memoria_cache(FILE *arq, int escrita, int tam_linha, int num_linhas, int assoc, int hit_time, char subs[], int tempo_mem) {
        LinhaCache *cache = (LinhaCache*)malloc(num_linhas*sizeof(LinhaCache));
        char endereco[32], letra, *p, op;
        int palavra, linha, tag;
        unsigned int binario;

        while(fscanf(arq, "%s %c", endereco, &op) != EOF) {
            binario = strtoul(endereco, NULL, 16);
            printf("%d\n", binario);
        }




        printf("INFORMAÇÕES DE ENTRADA -----------------------\n");
        printf("Política de Escrita: %s\n", escrita==1 ? "Write-Back" : "Write-Through");
        printf("Tamanho da Linha: %d\n", tam_linha);
        printf("Número de Linhas: %d\n", num_linhas);
        printf("Associatividade por Conjunto: %d\n", assoc);
        printf("Tempo de Acesso (Hit): %d\n", hit_time);
        printf("Política de Substituição: %s\n", subs);
        printf("Tempo de Leitura/Escrita (em ns): %d\n", tempo_mem);

        /*
        printf("INFORMAÇÕES DO ARQUIVO -----------------------\n");
        printf("Total de endereços de escrita: %d\n", );
        printf("Total de endereços de leitura: %d\n", );
        printf("Total de endereços: %d\n", );
        */

        /*
        printf("INFORMAÇÕES DA SIMULAÇÃO -----------------------\n");
        printf("Total de escritas/leitura na memória principal: %d\n", );
        printf("Taxa de acerto (hit rate): %d\n", );
        printf("Tempo medio de acesso da cache (em ns): %d\n", );
        */
    }

    int main ()
    {
        FILE *arq = fopen("teste_curto.txt", "rt");

        memoria_cache(arq, 1, 64, 4096, 2, 10, "LRU", 80);

        return 0;
    }
