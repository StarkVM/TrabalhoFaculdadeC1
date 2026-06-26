#include <stdio.h>

#include "controllers/controller.h"
#include "views/view.h"

int main(void)
{
    //Vitor Mateus
    //Breno Ribeiro
    //Kaua Fernandes
    if (start_system_controller() == 0)
    {
        printf("Erro ao abrir o banco de dados.\n");
        return 1;
    }

    run_application_view();

    stop_system_controller();

    return 0;
}
