#include <stdio.h>

#include "../src/models/model.h"

int main(void)
{
    int failed_tests;

    failed_tests = 0;

    if (open_database() == 0)
    {
        printf("[FALHOU] Banco de dados não abriu.\n");
        return 1;
    }

    printf("[PASSOU] Banco de dados abriu.\n");

    if (school_exists("12345678"))
    {
        printf("[PASSOU] INEP existente foi encontrado.\n");
    }
    else
    {
        printf("[FALHOU] INEP existente não foi encontrado.\n");
        failed_tests++;
    }

    if (school_exists("99999999") == 0)
    {
        printf("[PASSOU] INEP inexistente foi recusado.\n");
    }
    else
    {
        printf("[FALHOU] INEP inexistente foi aceito.\n");
        failed_tests++;
    }

    if (is_valid_cpf("529.982.247-25"))
    {
        printf("[PASSOU] CPF válido foi aceito.\n");
    }
    else
    {
        printf("[FALHOU] CPF válido foi recusado.\n");
        failed_tests++;
    }

    if (is_valid_cpf("111.111.111-11") == 0)
    {
        printf("[PASSOU] CPF inválido foi recusado.\n");
    }
    else
    {
        printf("[FALHOU] CPF inválido foi aceito.\n");
        failed_tests++;
    }

    if (is_valid_cnpj("11.222.333/0001-81"))
    {
        printf("[PASSOU] CNPJ válido foi aceito.\n");
    }
    else
    {
        printf("[FALHOU] CNPJ válido foi recusado.\n");
        failed_tests++;
    }

    if (is_valid_cnpj("11.111.111/1111-11") == 0)
    {
        printf("[PASSOU] CNPJ inválido foi recusado.\n");
    }
    else
    {
        printf("[FALHOU] CNPJ inválido foi aceito.\n");
        failed_tests++;
    }

    close_database();

    printf("\n");

    if (failed_tests == 0)
    {
        printf("Todos os testes passaram.\n");
        return 0;
    }

    printf("%d teste(s) falharam.\n", failed_tests);

    return 1;
}
