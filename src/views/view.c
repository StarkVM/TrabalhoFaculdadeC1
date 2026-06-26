#include <stdio.h>
#include <stdlib.h>

#include "view.h"
#include "../controllers/controller.h"

#define NAME_SIZE 101
#define DATE_SIZE 11
#define INEP_SIZE 20
#define DOCUMENT_SIZE 30
#define EMAIL_SIZE 101
#define PASSWORD_SIZE 101
#define ADDRESS_SIZE 201


/* =========================================================
   FUNÇÕES AUXILIARES
   ========================================================= */

static void clear_screen(void)
{
    system("cls");
}

static void clear_input_buffer(void)
{
    int character;

    while (
        (character = getchar()) != '\n' &&
        character != EOF
    )
    {
    }
}

static void remove_line_break(char *text)
{
    int position;

    for (position = 0; text[position] != '\0'; position++)
    {
        if (text[position] == '\n')
        {
            text[position] = '\0';
            return;
        }
    }

    clear_input_buffer();
}

static void read_text(
    const char *message,
    char *text,
    int size
)
{
    printf("%s", message);

    if (fgets(text, size, stdin) == NULL)
    {
        text[0] = '\0';
        return;
    }

    remove_line_break(text);
}

static int read_integer(const char *message)
{
    int value;

    printf("%s", message);

    if (scanf("%d", &value) != 1)
    {
        clear_input_buffer();
        return -1;
    }

    clear_input_buffer();

    return value;
}

static double read_decimal(const char *message)
{
    double value;

    printf("%s", message);

    if (scanf("%lf", &value) != 1)
    {
        clear_input_buffer();
        return -1;
    }

    clear_input_buffer();

    return value;
}

static void wait_for_enter(void)
{
    printf("\nPressione Enter para continuar...");
    getchar();
}


/* =========================================================
   MENU PRINCIPAL
   ========================================================= */

void show_main_menu(void)
{
    clear_screen();

    printf("====================================\n");
    printf("         EDUCACOMPARTILHA\n");
    printf("====================================\n");
    printf("1 - Entrar\n");
    printf("2 - Cadastrar recebedor\n");
    printf("3 - Cadastrar doador\n");
    printf("4 - Cadastrar papelaria parceira\n");
    printf("0 - Sair\n");
    printf("====================================\n");
}

void run_application_view(void)
{
    int option;

    do
    {
        show_main_menu();

        option = read_integer("Escolha uma opção: ");

        switch (option)
        {
            case 1:
                show_login_view();
                break;

            case 2:
                show_receiver_registration_view();
                break;

            case 3:
                show_donor_registration_view();
                break;

            case 4:
                show_partner_store_registration_view();
                break;

            case 0:
                clear_screen();
                printf("Encerrando o sistema...\n");
                break;

            default:
                printf("\nOpção inválida.\n");
                wait_for_enter();
                break;
        }

    } while (option != 0);
}


/* =========================================================
   LOGIN
   ========================================================= */

void show_login_view(void)
{
    char email[EMAIL_SIZE];
    char password[PASSWORD_SIZE];

    clear_screen();

    printf("========== LOGIN ==========\n");

    read_text("E-mail: ", email, EMAIL_SIZE);
    read_text("Senha: ", password, PASSWORD_SIZE);

    if (login_controller(email, password) == 0)
    {
        printf("\nE-mail ou senha inválidos.\n");
        wait_for_enter();
        return;
    }

    switch (get_logged_account_type_controller())
    {
        case ACCOUNT_TYPE_RECEIVER:
            show_receiver_menu();
            break;

        case ACCOUNT_TYPE_DONOR:
            show_donor_menu();
            break;

        case ACCOUNT_TYPE_PARTNER_STORE:
            show_partner_store_menu();
            break;

        default:
            printf("\nTipo de conta inválido.\n");
            logout_controller();
            wait_for_enter();
            break;
    }
}


/* =========================================================
   CADASTROS
   ========================================================= */

void show_receiver_registration_view(void)
{
    char full_name[NAME_SIZE];
    char birth_date[DATE_SIZE];
    char inep[INEP_SIZE];
    char cpf[DOCUMENT_SIZE];
    char email[EMAIL_SIZE];
    char password[PASSWORD_SIZE];

    clear_screen();

    printf("===== CADASTRO DE RECEBEDOR =====\n");

    read_text("Nome completo: ", full_name, NAME_SIZE);
    read_text(
        "Data de nascimento (AAAA-MM-DD): ",
        birth_date,
        DATE_SIZE
    );
    read_text("Código INEP da escola: ", inep, INEP_SIZE);
    read_text("CPF: ", cpf, DOCUMENT_SIZE);
    read_text("E-mail: ", email, EMAIL_SIZE);
    read_text(
        "Senha com pelo menos 8 caracteres: ",
        password,
        PASSWORD_SIZE
    );

    if (
        register_receiver_controller(
            full_name,
            birth_date,
            inep,
            cpf,
            email,
            password
        )
    )
    {
        printf("\nRecebedor cadastrado com sucesso.\n");
    }
    else
    {
        printf("\nNão foi possível realizar o cadastro.\n");
        printf("Verifique o CPF, o INEP, o e-mail e a senha.\n");
    }

    wait_for_enter();
}

void show_donor_registration_view(void)
{
    char full_name[NAME_SIZE];
    char birth_date[DATE_SIZE];
    char cpf[DOCUMENT_SIZE];
    char email[EMAIL_SIZE];
    char password[PASSWORD_SIZE];

    clear_screen();

    printf("======= CADASTRO DE DOADOR =======\n");

    read_text("Nome completo: ", full_name, NAME_SIZE);
    read_text(
        "Data de nascimento (AAAA-MM-DD): ",
        birth_date,
        DATE_SIZE
    );
    read_text("CPF: ", cpf, DOCUMENT_SIZE);
    read_text("E-mail: ", email, EMAIL_SIZE);
    read_text(
        "Senha com pelo menos 8 caracteres: ",
        password,
        PASSWORD_SIZE
    );

    if (
        register_donor_controller(
            full_name,
            birth_date,
            cpf,
            email,
            password
        )
    )
    {
        printf("\nDoador cadastrado com sucesso.\n");
    }
    else
    {
        printf("\nNão foi possível realizar o cadastro.\n");
        printf("Verifique o CPF, o e-mail e a senha.\n");
    }

    wait_for_enter();
}

void show_partner_store_registration_view(void)
{
    char name[NAME_SIZE];
    char cnpj[DOCUMENT_SIZE];
    char email[EMAIL_SIZE];
    char password[PASSWORD_SIZE];
    double discount;

    clear_screen();

    printf("=== CADASTRO DE PAPELARIA PARCEIRA ===\n");

    read_text("Nome da papelaria: ", name, NAME_SIZE);
    read_text("CNPJ: ", cnpj, DOCUMENT_SIZE);
    read_text("E-mail: ", email, EMAIL_SIZE);
    read_text(
        "Senha com pelo menos 8 caracteres: ",
        password,
        PASSWORD_SIZE
    );

    discount = read_decimal(
        "Porcentagem de desconto oferecida: "
    );

    if (
        register_partner_store_controller(
            name,
            cnpj,
            email,
            password,
            discount
        )
    )
    {
        printf("\nPapelaria cadastrada com sucesso.\n");
    }
    else
    {
        printf("\nNão foi possível realizar o cadastro.\n");
        printf("Verifique o CNPJ, o e-mail, a senha e o desconto.\n");
    }

    wait_for_enter();
}


/* =========================================================
   MENUS DOS USUÁRIOS
   ========================================================= */

void show_receiver_menu(void)
{
    int option;

    do
    {
        clear_screen();

        printf("======= MENU DO RECEBEDOR =======\n");
        printf("1 - Editar perfil\n");
        printf("2 - Cadastrar demanda\n");
        printf("3 - Listar recebimentos\n");
        printf("0 - Sair da conta\n");
        printf("=================================\n");

        option = read_integer("Escolha uma opção: ");

        switch (option)
        {
            case 1:
                show_receiver_profile_edit_view();
                break;

            case 2:
                clear_screen();
                printf("Cadastro de demanda será implementado depois.\n");
                wait_for_enter();
                break;

            case 3:
                clear_screen();
                printf("Listagem de recebimentos será implementada depois.\n");
                wait_for_enter();
                break;

            case 0:
                logout_controller();
                break;

            default:
                printf("\nOpção inválida.\n");
                wait_for_enter();
                break;
        }

    } while (option != 0);
}

void show_donor_menu(void)
{
    int option;

    do
    {
        clear_screen();

        printf("========= MENU DO DOADOR =========\n");
        printf("1 - Editar perfil\n");
        printf("2 - Listar demandas\n");
        printf("0 - Sair da conta\n");
        printf("==================================\n");

        option = read_integer("Escolha uma opção: ");

        switch (option)
        {
            case 1:
                show_donor_profile_edit_view();
                break;

            case 2:
                clear_screen();
                printf("Listagem de demandas será implementada depois.\n");
                wait_for_enter();
                break;

            case 0:
                logout_controller();
                break;

            default:
                printf("\nOpção inválida.\n");
                wait_for_enter();
                break;
        }

    } while (option != 0);
}

void show_partner_store_menu(void)
{
    int option;

    do
    {
        clear_screen();

        printf("===== MENU DA PAPELARIA PARCEIRA =====\n");
        printf("1 - Editar perfil\n");
        printf("2 - Pausar parceria\n");
        printf("3 - Reativar parceria\n");
        printf("0 - Sair da conta\n");
        printf("======================================\n");

        option = read_integer("Escolha uma opção: ");

        switch (option)
        {
            case 1:
                show_partner_store_profile_edit_view();
                break;

            case 2:
                clear_screen();

                if (pause_partner_store_controller())
                {
                    printf("Parceria pausada com sucesso.\n");
                }
                else
                {
                    printf("Não foi possível pausar a parceria.\n");
                }

                wait_for_enter();
                break;

            case 3:
                clear_screen();

                if (reactivate_partner_store_controller())
                {
                    printf("Parceria reativada com sucesso.\n");
                }
                else
                {
                    printf("Não foi possível reativar a parceria.\n");
                }

                wait_for_enter();
                break;

            case 0:
                logout_controller();
                break;

            default:
                printf("\nOpção inválida.\n");
                wait_for_enter();
                break;
        }

    } while (option != 0);
}


/* =========================================================
   EDIÇÃO DE PERFIL
   ========================================================= */

void show_receiver_profile_edit_view(void)
{
    char full_name[NAME_SIZE];
    char birth_date[DATE_SIZE];
    char email[EMAIL_SIZE];
    char inep[INEP_SIZE];

    clear_screen();

    printf("===== EDITAR PERFIL DO RECEBEDOR =====\n");

    read_text("Novo nome completo: ", full_name, NAME_SIZE);
    read_text(
        "Nova data de nascimento (AAAA-MM-DD): ",
        birth_date,
        DATE_SIZE
    );
    read_text("Novo e-mail: ", email, EMAIL_SIZE);
    read_text("Novo código INEP: ", inep, INEP_SIZE);

    if (
        update_receiver_profile_controller(
            full_name,
            birth_date,
            email,
            inep
        )
    )
    {
        printf("\nPerfil atualizado com sucesso.\n");
    }
    else
    {
        printf("\nNão foi possível atualizar o perfil.\n");
        printf("O novo INEP deve existir no sistema.\n");
    }

    wait_for_enter();
}

void show_donor_profile_edit_view(void)
{
    char full_name[NAME_SIZE];
    char birth_date[DATE_SIZE];
    char email[EMAIL_SIZE];

    clear_screen();

    printf("======= EDITAR PERFIL DO DOADOR =======\n");

    read_text("Novo nome completo: ", full_name, NAME_SIZE);
    read_text(
        "Nova data de nascimento (AAAA-MM-DD): ",
        birth_date,
        DATE_SIZE
    );
    read_text("Novo e-mail: ", email, EMAIL_SIZE);

    if (
        update_donor_profile_controller(
            full_name,
            birth_date,
            email
        )
    )
    {
        printf("\nPerfil atualizado com sucesso.\n");
    }
    else
    {
        printf("\nNão foi possível atualizar o perfil.\n");
    }

    wait_for_enter();
}

void show_partner_store_profile_edit_view(void)
{
    char name[NAME_SIZE];
    char email[EMAIL_SIZE];
    char address[ADDRESS_SIZE];
    double discount;

    clear_screen();

    printf("===== EDITAR PERFIL DA PAPELARIA =====\n");

    read_text("Novo nome da papelaria: ", name, NAME_SIZE);
    read_text("Novo e-mail: ", email, EMAIL_SIZE);
    read_text("Novo endereço: ", address, ADDRESS_SIZE);

    discount = read_decimal(
        "Nova porcentagem de desconto: "
    );

    if (
        update_partner_store_profile_controller(
            name,
            email,
            address,
            discount
        )
    )
    {
        printf("\nPerfil atualizado com sucesso.\n");
    }
    else
    {
        printf("\nNão foi possível atualizar o perfil.\n");
        printf("O desconto deve estar entre 0 e 100.\n");
    }

    wait_for_enter();
}
