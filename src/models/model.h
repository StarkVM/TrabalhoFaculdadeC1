#ifndef MODEL_H
#define MODEL_H

typedef enum
{
    ACCOUNT_TYPE_NONE = 0,
    ACCOUNT_TYPE_RECEIVER = 1,
    ACCOUNT_TYPE_DONOR = 2,
    ACCOUNT_TYPE_PARTNER_STORE = 3
} AccountType;

typedef struct
{
    int account_id;
    AccountType account_type;
} UserSession;

int open_database(void);
void close_database(void);

int is_valid_cpf(const char *cpf);
int is_valid_cnpj(const char *cnpj);
int school_exists(const char *inep);

int create_receiver(
    const char *full_name,
    const char *birth_date,
    const char *inep,
    const char *cpf,
    const char *email,
    const char *password
);

int create_donor(
    const char *full_name,
    const char *birth_date,
    const char *cpf,
    const char *email,
    const char *password
);

int create_partner_store(
    const char *name,
    const char *cnpj,
    const char *email,
    const char *password,
    double discount
);

int authenticate_user(
    const char *email,
    const char *password,
    UserSession *session
);

int update_receiver_profile(
    int account_id,
    const char *full_name,
    const char *birth_date,
    const char *email,
    const char *inep
);

int update_donor_profile(
    int account_id,
    const char *full_name,
    const char *birth_date,
    const char *email
);

int update_partner_store_profile(
    int account_id,
    const char *name,
    const char *email,
    const char *address,
    double discount
);

int set_partner_store_active(
    int account_id,
    int is_active
);

#endif
