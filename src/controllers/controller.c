#include "controller.h"

static UserSession current_session =
{
    0,
    ACCOUNT_TYPE_NONE
};

int start_system_controller(void)
{
    return open_database();
}

void stop_system_controller(void)
{
    close_database();
}

int register_receiver_controller(
    const char *full_name,
    const char *birth_date,
    const char *inep,
    const char *cpf,
    const char *email,
    const char *password
)
{
    return create_receiver(
        full_name,
        birth_date,
        inep,
        cpf,
        email,
        password
    );
}

int register_donor_controller(
    const char *full_name,
    const char *birth_date,
    const char *cpf,
    const char *email,
    const char *password
)
{
    return create_donor(
        full_name,
        birth_date,
        cpf,
        email,
        password
    );
}

int register_partner_store_controller(
    const char *name,
    const char *cnpj,
    const char *email,
    const char *password,
    double discount
)
{
    return create_partner_store(
        name,
        cnpj,
        email,
        password,
        discount
    );
}

int login_controller(
    const char *email,
    const char *password
)
{
    return authenticate_user(
        email,
        password,
        &current_session
    );
}

void logout_controller(void)
{
    current_session.account_id = 0;
    current_session.account_type = ACCOUNT_TYPE_NONE;
}

AccountType get_logged_account_type_controller(void)
{
    return current_session.account_type;
}

int update_receiver_profile_controller(
    const char *full_name,
    const char *birth_date,
    const char *email,
    const char *inep
)
{
    if (
        current_session.account_type !=
        ACCOUNT_TYPE_RECEIVER
    )
    {
        return 0;
    }

    return update_receiver_profile(
        current_session.account_id,
        full_name,
        birth_date,
        email,
        inep
    );
}

int update_donor_profile_controller(
    const char *full_name,
    const char *birth_date,
    const char *email
)
{
    if (
        current_session.account_type !=
        ACCOUNT_TYPE_DONOR
    )
    {
        return 0;
    }

    return update_donor_profile(
        current_session.account_id,
        full_name,
        birth_date,
        email
    );
}

int update_partner_store_profile_controller(
    const char *name,
    const char *email,
    const char *address,
    double discount
)
{
    if (
        current_session.account_type !=
        ACCOUNT_TYPE_PARTNER_STORE
    )
    {
        return 0;
    }

    return update_partner_store_profile(
        current_session.account_id,
        name,
        email,
        address,
        discount
    );
}

int pause_partner_store_controller(void)
{
    if (
        current_session.account_type !=
        ACCOUNT_TYPE_PARTNER_STORE
    )
    {
        return 0;
    }

    return set_partner_store_active(
        current_session.account_id,
        0
    );
}

int reactivate_partner_store_controller(void)
{
    if (
        current_session.account_type !=
        ACCOUNT_TYPE_PARTNER_STORE
    )
    {
        return 0;
    }

    return set_partner_store_active(
        current_session.account_id,
        1
    );
}
