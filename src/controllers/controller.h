#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "../models/model.h"

int start_system_controller(void);
void stop_system_controller(void);

int register_receiver_controller(
    const char *full_name,
    const char *birth_date,
    const char *inep,
    const char *cpf,
    const char *email,
    const char *password
);

int register_donor_controller(
    const char *full_name,
    const char *birth_date,
    const char *cpf,
    const char *email,
    const char *password
);

int register_partner_store_controller(
    const char *name,
    const char *cnpj,
    const char *email,
    const char *password,
    double discount
);

int login_controller(
    const char *email,
    const char *password
);

void logout_controller(void);

AccountType get_logged_account_type_controller(void);

int update_receiver_profile_controller(
    const char *full_name,
    const char *birth_date,
    const char *email,
    const char *inep
);

int update_donor_profile_controller(
    const char *full_name,
    const char *birth_date,
    const char *email
);

int update_partner_store_profile_controller(
    const char *name,
    const char *email,
    const char *address,
    double discount
);

int pause_partner_store_controller(void);
int reactivate_partner_store_controller(void);

#endif
