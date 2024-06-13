#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_ERRORS 100
#define MAX_ERROR_LENGTH 256

typedef struct {
    long dia, mes, ano;
    double qtd;
    char nome_arquivo[11];
} reg;


char error_messages[MAX_ERRORS][MAX_ERROR_LENGTH];
int error_count = 0;

void add_error_message(const char *message) {
    if (error_count < MAX_ERRORS) {
        strncpy(error_messages[error_count], message, MAX_ERROR_LENGTH - 1);
        error_messages[error_count][MAX_ERROR_LENGTH - 1] = '\0'; // Garante que a string seja terminada corretamente
        error_count++;
    } else {
        printf("Erro: excedido o limite máximo de mensagens de erro.\n");
    }
}

void display_error_messages() {
    printf("\nMensagens de erro:\n");
    for (int i = 0; i < error_count; i++) {
        printf("%s\n", error_messages[i]);
    }
    error_count = 0; // Limpa as mensagens de erro após a exibição
}

void limpar_tela() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int eh_data_valida(long dia, long mes, long ano) {
    if (ano < 1) return 0;
    if (mes < 1 || mes > 12) return 0;

    int dias_no_mes[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if (ano % 400 == 0 || (ano % 100 != 0 && ano % 4 == 0)) {
        dias_no_mes[1] = 29;
    }

    if (dia < 1 || dia > dias_no_mes[mes - 1]) return 0;

    return 1;
}

int eh_numero_valido(const char *str) {
    int ponto_decimal_encontrado = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit(str[i])) {
            if (str[i] == '.' && !ponto_decimal_encontrado) {
                ponto_decimal_encontrado = 1;
            } else {
                return 0;
            }
        }
    }
    return 1;
}

const char *nomes_dos_meses[] = {
    "janeiro.txt", "fevereiro.txt", "março.txt", "abril.txt", "maio.txt", "junho.txt",
    "julho.txt", "agosto.txt", "setembro.txt", "outubro.txt", "novembro.txt", "dezembro.txt"
};

int arquivo_ja_existe(const char *nome_arquivo, FILE *ptr_arquivo_bin) {
    reg registro;
    rewind(ptr_arquivo_bin);
    while (fread(&registro, sizeof(reg), 1, ptr_arquivo_bin)) {
        if (strcmp(registro.nome_arquivo, nome_arquivo) == 0) {
            return 1; // Arquivo já existe
        }
    }
    return 0; // Arquivo não encontrado
}

int transfere_txt_array_reg(char *nome_arquivo, FILE *ptr_arquivo_bin) {
    int erro = 0;
    // Verifica se o nome do arquivo corresponde a um mês válido
    int valido = 0;
    for (int i = 0; i < 12; i++) {
        if (strcmp(nome_arquivo, nomes_dos_meses[i]) == 0) {
            valido = 1;
            break;
        }
    }
    if (!valido) {
        add_error_message("Erro: nome de arquivo invalido.");
        return 1;
    }
    // Verifica se o arquivo já existe no arquivo binário

    if (arquivo_ja_existe(nome_arquivo, ptr_arquivo_bin)) {
        add_error_message("Erro: já existe um registro para este mês.");
        return 1;
    }

    FILE *ptr_arquivo_txt = fopen(nome_arquivo, "r");
    if (ptr_arquivo_txt == NULL) {
        char error_message[MAX_ERROR_LENGTH];
        sprintf(error_message, "Erro ao abrir o arquivo de texto: %s", nome_arquivo);
        add_error_message(error_message);
        return 1;
    }

    reg registro_unico;
    char linha[100];
    char string_data[11], string_litros[10];
    char *token;


    while (fgets(linha, sizeof(linha), ptr_arquivo_txt) != NULL) {
        linha[strcspn(linha, "\n")] = '\0'; // Remove o caractere de nova linha

        // Extrai a data
        token = strtok(linha, " ");
        if (token == NULL) {
            char error_message[MAX_ERROR_LENGTH];
            sprintf(error_message, "Erro: linha sem dados de data. Linha: %s\n", linha);
            add_error_message(error_message);
            erro = 1;
            continue;
        }
        strncpy(string_data, token, sizeof(string_data) - 1);
        string_data[sizeof(string_data) - 1] = '\0';

        // Extrai a quantidade de litros
        token = strtok(NULL, " ");
        if (token == NULL) {
            char error_message[MAX_ERROR_LENGTH];
            sprintf(error_message, "Erro: linha sem dados de litros. Linha: %s\n", linha);
            add_error_message(error_message);
            erro = 1;
            continue;
        }
        strncpy(string_litros, token, sizeof(string_litros) - 1);
        string_litros[sizeof(string_litros) - 1] = '\0';

        // Valida o formato da data
        if (sscanf(string_data, "%2ld/%2ld/%4ld", &registro_unico.dia, &registro_unico.mes, &registro_unico.ano) != 3) {
            char error_message[MAX_ERROR_LENGTH];
            sprintf(error_message, "Erro: formato de data inválido. Linha: %s\n", linha);
            add_error_message(error_message);
            erro = 1;
            continue;
        }

        // Verifica se a data é válida
        if (!eh_data_valida(registro_unico.dia, registro_unico.mes, registro_unico.ano)) {
            char error_message[MAX_ERROR_LENGTH];
            sprintf(error_message, "Erro: data inválida. Linha: %s\n", linha);
            add_error_message(error_message);
            erro = 1;
            continue;
        }

        // Verifica se o valor de litros é válido
        if (!eh_numero_valido(string_litros)) {
            char error_message[MAX_ERROR_LENGTH];
            sprintf(error_message, "Erro: valor de litros inválido. Linha: %s\n", linha);
            add_error_message(error_message);
            erro = 1;
            continue;
        }

        // Converte o valor de litros para double e atribui ao registro
        registro_unico.qtd = atof(string_litros);

        // Copia o nome do arquivo para o registro
        strncpy(registro_unico.nome_arquivo, nome_arquivo, sizeof(registro_unico.nome_arquivo) - 1);
        registro_unico.nome_arquivo[sizeof(registro_unico.nome_arquivo) - 1] = '\0';

        // Grava o registro no arquivo binário, apenas se não houver erro
        if (erro == 0) {
            fwrite(&registro_unico, sizeof(reg), 1, ptr_arquivo_bin);
        }
    }

    fclose(ptr_arquivo_txt);
    return erro;
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
        if (error_count > 0) { // Verifica se há mensagens de erro
            display_error_messages(); // Exibe as mensagens de erro antes do menu
        }
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
                } else {
                    printf("\n===============================\n");
                    printf("Dados adicionados com sucesso!");
                    printf("\n===============================\n\n");
                }
                break;
            }
            case 2:
                exibir_registros(ptr_arquivo_bin);
                break;
            case 3: {
                char nome_arquivo[100];
                printf("Digite o nome do arquivo cujos registros devem ser removidos: ");
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
