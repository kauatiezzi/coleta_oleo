#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    long int dia, mes, ano;
    double qtd;
    char nome_arquivo[11];
} reg;

// Função para adicionar registros de um arquivo de texto ao arquivo binário
int transfere_txt_array_reg(char *nome_arquivo, FILE *ptr_arquivo_bin) {
    FILE *ptr_arquivo_txt = fopen(nome_arquivo, "r");
    if (ptr_arquivo_txt == NULL) return 1;

    reg registro_unico;
    char linha[100];
    char string_data[11], string_litros[10];
    char *token;

    while (fgets(linha, sizeof(linha), ptr_arquivo_txt) != NULL) {
        token = strtok(linha, " ");
        strncpy(string_data, token, 10);
        token = strtok(NULL, " ");
        strncpy(string_litros, token, 10);

        sscanf(string_data, "%ld/%ld/%ld", &registro_unico.dia, &registro_unico.mes, &registro_unico.ano);
        registro_unico.qtd = atof(string_litros);

        // Copia o nome do arquivo para o registro
        strncpy(registro_unico.nome_arquivo, nome_arquivo, sizeof(registro_unico.nome_arquivo) - 1);
        registro_unico.nome_arquivo[sizeof(registro_unico.nome_arquivo) - 1] = '\0';

        fwrite(&registro_unico, sizeof(reg), 1, ptr_arquivo_bin);
    }

    fclose(ptr_arquivo_txt);
    return 0;
}

// Função para exibir os registros
void exibir_registros(FILE *ptr_arquivo_bin) {
    reg registro;

    printf("\n===============================\n");
    printf("Registros no arquivo Binario:");
    printf("\n===============================\n\n");
    rewind(ptr_arquivo_bin);
    while (fread(&registro, sizeof(reg), 1, ptr_arquivo_bin)) {
        printf("%02ld/%02ld/%04ld %.1lf\n", registro.dia, registro.mes, registro.ano, registro.qtd);
    }
    printf("\n===============================\n\n");
}

// Função para gerar um arquivo CSV com os registros do arquivo binário
void gerar_csv_registros(FILE *ptr_arquivo_bin) {
    FILE *ptr_arquivo_csv = fopen("todos_registros.csv", "w");
    if (ptr_arquivo_csv == NULL) {
        printf("Erro ao criar o arquivo CSV\n");
        return;
    }

    reg registro;
    fprintf(ptr_arquivo_csv, "Data,Litros\n");
    rewind(ptr_arquivo_bin);
    while (fread(&registro, sizeof(reg), 1, ptr_arquivo_bin)) {
        fprintf(ptr_arquivo_csv, "%02ld/%02ld/%04ld,%.1lf\n", registro.dia, registro.mes, registro.ano, registro.qtd);
    }

    fclose(ptr_arquivo_csv);
    printf("\n========================================\n");
    printf("Arquivo CSV detalhado gerado/atualizado.");
    printf("\n========================================\n");
}

// Função para remover registros de um determinado arquivo
int remover_registros(char *nome_arquivo) {
    FILE *ptr_arquivo_bin = fopen("todos_registros.bin", "rb");
    if (ptr_arquivo_bin == NULL) {
        printf("Erro ao abrir o arquivo binário\n");
        return 1;
    }

    FILE *ptr_arquivo_tmp = fopen("temporario.bin", "wb");
    if (ptr_arquivo_tmp == NULL) {
        fclose(ptr_arquivo_bin);
        printf("Erro ao abrir o arquivo temporário\n");
        return 1;
    }

    reg registro;
    int registros_removidos = 0;

    while (fread(&registro, sizeof(reg), 1, ptr_arquivo_bin)) {
        if (strcmp(registro.nome_arquivo, nome_arquivo) != 0) {
            fwrite(&registro, sizeof(reg), 1, ptr_arquivo_tmp);
        } else {
            registros_removidos++;
        }
    }

    fclose(ptr_arquivo_bin);
    fclose(ptr_arquivo_tmp);

    if (remove("todos_registros.bin") != 0) {
        printf("Erro ao remover o arquivo original\n");
        return 1;
    }

    if (rename("temporario.bin", "todos_registros.bin") != 0) {
        printf("Erro ao renomear o arquivo temporário\n");
        return 1;
    }
    printf("\n============================================\n");
    printf("Os dados do arquivo %s foram removidos.", nome_arquivo);
    printf("\n============================================\n");

    return 0;
}

// Função para gerar um arquivo CSV com a somatória mensal da quantidade coletada
void gerar_csv_somatoria_mensal(FILE *ptr_arquivo_bin) {
    double somas_mensais[12] = {0};
    reg registro;

    rewind(ptr_arquivo_bin);
    while (fread(&registro, sizeof(reg), 1, ptr_arquivo_bin)) {
        if (registro.mes >= 1 && registro.mes <= 12) {
            somas_mensais[registro.mes - 1] += registro.qtd;
        }
    }

    FILE *ptr_arquivo_csv = fopen("somatoria_mensal.csv", "w");
    if (ptr_arquivo_csv == NULL) {
        printf("Erro ao criar o arquivo CSV\n");
        return;
    }

    fprintf(ptr_arquivo_csv, "Mes,Total_Litros\n");
    for (int i = 0; i < 12; i++) {
        fprintf(ptr_arquivo_csv, "%02d,%.1lf\n", i + 1, somas_mensais[i]);
    }

    fclose(ptr_arquivo_csv);
    printf("\n====================================================\n");
    printf("Arquivo CSV com somatoria mensal gerado/atualizado.");
    printf("\n====================================================\n");
}

int main() {
    FILE *ptr_arquivo_bin = fopen("todos_registros.bin", "ab+");
    if (ptr_arquivo_bin == NULL) {
        printf("Erro ao abrir ou criar o arquivo binário\n");
        return 1;
    }

    int opcao;
    char input[100];

    do {
        printf("\n=======================================");
        printf("\n         Selecione uma opcao:\n");
        printf("=======================================\n");
        printf("1. Adicionar novo arquivo com dados.\n");
        printf("2. Exibir registros no arquivo binario.\n");
        printf("3. Remover arquivo com dados.\n");
        printf("4. Gerar arquivo CSV detalhado.\n");
        printf("5. Gerar somatoria mensal em CSV.\n");
        printf("6. Sair\n");
        printf("=======================================\n");
        fgets(input, sizeof(input), stdin);
        opcao = atoi(input);

        switch (opcao) {
            case 1: {
                char nome_arquivo[100];
                printf("Digite o nome do arquivo a ser processado: ");
                fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
                nome_arquivo[strcspn(nome_arquivo, "\n")] = 0;  // Remove newline

                if (transfere_txt_array_reg(nome_arquivo, ptr_arquivo_bin) > 0) {
                    printf("Erro ao abrir o arquivo\n");
                    return 1;
                }
                printf("\n===============================\n");
                printf("Dados adicionados com sucesso!");
                printf("\n===============================\n\n");
                break;
            }
            case 2:
                exibir_registros(ptr_arquivo_bin);
                break;
            case 3: {
                char nome_arquivo[100];
                printf("Digite o nome do arquivo cujos registros devem ser removidos (sem .txt no final): ");
                fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
                nome_arquivo[strcspn(nome_arquivo, "\n")] = 0;  // Remove newline

                fclose(ptr_arquivo_bin);
                remover_registros(nome_arquivo);
                ptr_arquivo_bin = fopen("todos_registros.bin", "ab+");
                if (ptr_arquivo_bin == NULL) {
                    printf("Erro ao reabrir o arquivo binário\n");
                    return 1;
                }
                break;
            }
            case 4:
                gerar_csv_registros(ptr_arquivo_bin);
                break;
            case 5:
                gerar_csv_somatoria_mensal(ptr_arquivo_bin);
                break;
            case 6:
                printf("Saindo...\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }

    } while (opcao != 6);

    fclose(ptr_arquivo_bin);
    return 0;
}
