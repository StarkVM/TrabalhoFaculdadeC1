#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/rand.h>

#include <sqlite3.h>

#include "bcrypt.h"
#include "ow-crypt.h"
#include "model.h"

#define DATABASE_PATH "database/educacompartilha.db"
#define CPF_LENGTH 11
#define CNPJ_LENGTH 14
#define HASH_LENGTH 65
#define BCRYPT_COST 12

static sqlite3 *database = NULL;


/* =========================================================
   FUNÇÕES AUXILIARES
   ========================================================= */

static int text_is_empty(const char *text)
{
    return text == NULL || text[0] == '\0';
}

static int password_is_valid(const char *password)
{
    int length;

    if (password == NULL)
    {
        return 0;
    }

    length = (int)strlen(password);

    return length >= 8 && length <= 72;
}

static int normalize_document(
    const char *document,
    char *normalized,
    int expected_length
)
{
    int source_position;
    int target_position;

    if (document == NULL)
    {
        return 0;
    }

    target_position = 0;

    for (
        source_position = 0;
        document[source_position] != '\0';
        source_position++
    )
    {
        if (isdigit((unsigned char)document[source_position]))
        {
            if (target_position >= expected_length)
            {
                return 0;
            }

            normalized[target_position] = document[source_position];
            target_position++;
        }
        else if (
            document[source_position] != '.' &&
            document[source_position] != '-' &&
            document[source_position] != '/' &&
            document[source_position] != ' '
        )
        {
            return 0;
        }
    }

    normalized[target_position] = '\0';

    return target_position == expected_length;
}

static int all_digits_are_equal(
    const char *digits,
    int length
)
{
    int position;

    for (position = 1; position < length; position++)
    {
        if (digits[position] != digits[0])
        {
            return 0;
        }
    }

    return 1;
}

static int execute_sql(const char *sql)
{
    return sqlite3_exec(
        database,
        sql,
        NULL,
        NULL,
        NULL
    ) == SQLITE_OK;
}

static int create_password_hash(
    const char *password,
    char *password_hash
)
{
    unsigned char random_bytes[16];
    char salt[BCRYPT_HASHSIZE];

    if (
        RAND_bytes(
            random_bytes,
            sizeof(random_bytes)
        ) != 1
    )
    {
        return 0;
    }

    if (
        crypt_gensalt_rn(
            "$2a$",
            BCRYPT_COST,
            (const char *)random_bytes,
            sizeof(random_bytes),
            salt,
            sizeof(salt)
        ) == NULL
    )
    {
        return 0;
    }

    if (
        bcrypt_hashpw(
            password,
            salt,
            password_hash
        ) != 0
    )
    {
        return 0;
    }

    return 1;
}

static int verify_password(
    const char *password,
    const char *password_hash
)
{
    return bcrypt_checkpw(password, password_hash) == 0;
}

static int create_cpf_hash(
    const char *cpf,
    char *cpf_hash
)
{
    char normalized_cpf[CPF_LENGTH + 1];
    const char *secret;
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned int result_length;
    unsigned int position;

    if (
        normalize_document(
            cpf,
            normalized_cpf,
            CPF_LENGTH
        ) == 0
    )
    {
        return 0;
    }

    secret = getenv("CPF_HASH_SECRET");

    if (secret == NULL || secret[0] == '\0')
    {
        return 0;
    }

    if (
        HMAC(
            EVP_sha256(),
            secret,
            (int)strlen(secret),
            (const unsigned char *)normalized_cpf,
            strlen(normalized_cpf),
            result,
            &result_length
        ) == NULL
    )
    {
        return 0;
    }

    for (position = 0; position < result_length; position++)
    {
        sprintf(
            &cpf_hash[position * 2],
            "%02x",
            result[position]
        );
    }

    cpf_hash[result_length * 2] = '\0';

    return 1;
}

static int find_school_id(const char *inep)
{
    const char *sql;
    sqlite3_stmt *statement;
    int school_id;

    sql = "SELECT id FROM schools WHERE inep = ?;";
    statement = NULL;
    school_id = -1;

    if (
        sqlite3_prepare_v2(
            database,
            sql,
            -1,
            &statement,
            NULL
        ) != SQLITE_OK
    )
    {
        return -1;
    }

    sqlite3_bind_text(
        statement,
        1,
        inep,
        -1,
        SQLITE_TRANSIENT
    );

    if (sqlite3_step(statement) == SQLITE_ROW)
    {
        school_id = sqlite3_column_int(statement, 0);
    }

    sqlite3_finalize(statement);

    return school_id;
}

static int insert_account(
    const char *email,
    const char *password,
    const char *account_type,
    const char *cpf_hash
)
{
    const char *sql;
    sqlite3_stmt *statement;
    char password_hash[BCRYPT_HASHSIZE];
    int account_id;

    if (create_password_hash(password, password_hash) == 0)
    {
        return -1;
    }

    sql =
        "INSERT INTO accounts "
        "(email, password_hash, account_type, cpf_hash) "
        "VALUES (?, ?, ?, ?);";

    statement = NULL;

    if (
        sqlite3_prepare_v2(
            database,
            sql,
            -1,
            &statement,
            NULL
        ) != SQLITE_OK
    )
    {
        return -1;
    }

    sqlite3_bind_text(statement, 1, email, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, password_hash, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, account_type, -1, SQLITE_TRANSIENT);

    if (cpf_hash == NULL)
    {
        sqlite3_bind_null(statement, 4);
    }
    else
    {
        sqlite3_bind_text(statement, 4, cpf_hash, -1, SQLITE_TRANSIENT);
    }

    if (sqlite3_step(statement) != SQLITE_DONE)
    {
        sqlite3_finalize(statement);
        return -1;
    }

    sqlite3_finalize(statement);

    account_id = (int)sqlite3_last_insert_rowid(database);

    return account_id;
}

static int update_account_email(
    int account_id,
    const char *email
)
{
    const char *sql;
    sqlite3_stmt *statement;
    int result;

    sql = "UPDATE accounts SET email = ? WHERE id = ?;";
    statement = NULL;

    if (
        sqlite3_prepare_v2(
            database,
            sql,
            -1,
            &statement,
            NULL
        ) != SQLITE_OK
    )
    {
        return 0;
    }

    sqlite3_bind_text(statement, 1, email, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 2, account_id);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    return result == SQLITE_DONE;
}

static AccountType convert_account_type(const char *account_type)
{
    if (strcmp(account_type, "RECEIVER") == 0)
    {
        return ACCOUNT_TYPE_RECEIVER;
    }

    if (strcmp(account_type, "DONOR") == 0)
    {
        return ACCOUNT_TYPE_DONOR;
    }

    if (strcmp(account_type, "PARTNER_STORE") == 0)
    {
        return ACCOUNT_TYPE_PARTNER_STORE;
    }

    return ACCOUNT_TYPE_NONE;
}


/* =========================================================
   BANCO DE DADOS
   ========================================================= */

int open_database(void)
{
    if (sqlite3_open(DATABASE_PATH, &database) != SQLITE_OK)
    {
        if (database != NULL)
        {
            sqlite3_close(database);
            database = NULL;
        }

        return 0;
    }

    if (execute_sql("PRAGMA foreign_keys = ON;") == 0)
    {
        close_database();
        return 0;
    }

    return 1;
}

void close_database(void)
{
    if (database != NULL)
    {
        sqlite3_close(database);
        database = NULL;
    }
}


/* =========================================================
   VALIDAÇÃO DE CPF E CNPJ
   ========================================================= */

int is_valid_cpf(const char *cpf)
{
    char digits[CPF_LENGTH + 1];
    int position;
    int sum;
    int first_digit;
    int second_digit;

    if (normalize_document(cpf, digits, CPF_LENGTH) == 0)
    {
        return 0;
    }

    if (all_digits_are_equal(digits, CPF_LENGTH))
    {
        return 0;
    }

    sum = 0;

    for (position = 0; position < 9; position++)
    {
        sum += (digits[position] - '0') * (10 - position);
    }

    first_digit = (sum * 10) % 11;

    if (first_digit == 10)
    {
        first_digit = 0;
    }

    if (first_digit != digits[9] - '0')
    {
        return 0;
    }

    sum = 0;

    for (position = 0; position < 10; position++)
    {
        sum += (digits[position] - '0') * (11 - position);
    }

    second_digit = (sum * 10) % 11;

    if (second_digit == 10)
    {
        second_digit = 0;
    }

    if (second_digit != digits[10] - '0')
    {
        return 0;
    }

    return 1;
}

int is_valid_cnpj(const char *cnpj)
{
    char digits[CNPJ_LENGTH + 1];

    int first_weights[12] =
    {
        5, 4, 3, 2, 9, 8,
        7, 6, 5, 4, 3, 2
    };

    int second_weights[13] =
    {
        6, 5, 4, 3, 2, 9, 8,
        7, 6, 5, 4, 3, 2
    };

    int position;
    int sum;
    int remainder;
    int first_digit;
    int second_digit;

    if (normalize_document(cnpj, digits, CNPJ_LENGTH) == 0)
    {
        return 0;
    }

    if (all_digits_are_equal(digits, CNPJ_LENGTH))
    {
        return 0;
    }

    sum = 0;

    for (position = 0; position < 12; position++)
    {
        sum += (digits[position] - '0') * first_weights[position];
    }

    remainder = sum % 11;

    if (remainder < 2)
    {
        first_digit = 0;
    }
    else
    {
        first_digit = 11 - remainder;
    }

    if (first_digit != digits[12] - '0')
    {
        return 0;
    }

    sum = 0;

    for (position = 0; position < 13; position++)
    {
        sum += (digits[position] - '0') * second_weights[position];
    }

    remainder = sum % 11;

    if (remainder < 2)
    {
        second_digit = 0;
    }
    else
    {
        second_digit = 11 - remainder;
    }

    if (second_digit != digits[13] - '0')
    {
        return 0;
    }

    return 1;
}

int school_exists(const char *inep)
{
    return find_school_id(inep) != -1;
}


/* =========================================================
   CADASTROS
   ========================================================= */

int create_receiver(
    const char *full_name,
    const char *birth_date,
    const char *inep,
    const char *cpf,
    const char *email,
    const char *password
)
{
    const char *sql;
    sqlite3_stmt *statement;
    char cpf_hash[HASH_LENGTH];
    int school_id;
    int account_id;
    int result;

    if (
        text_is_empty(full_name) ||
        text_is_empty(birth_date) ||
        text_is_empty(inep) ||
        text_is_empty(cpf) ||
        text_is_empty(email) ||
        password_is_valid(password) == 0
    )
    {
        return 0;
    }

    if (is_valid_cpf(cpf) == 0)
    {
        return 0;
    }

    school_id = find_school_id(inep);

    if (school_id == -1)
    {
        return 0;
    }

    if (create_cpf_hash(cpf, cpf_hash) == 0)
    {
        return 0;
    }

    if (execute_sql("BEGIN TRANSACTION;") == 0)
    {
        return 0;
    }

    account_id = insert_account(
        email,
        password,
        "RECEIVER",
        cpf_hash
    );

    if (account_id == -1)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    sql =
        "INSERT INTO receivers "
        "(account_id, school_id, full_name, birth_date) "
        "VALUES (?, ?, ?, ?);";

    statement = NULL;

    if (
        sqlite3_prepare_v2(
            database,
            sql,
            -1,
            &statement,
            NULL
        ) != SQLITE_OK
    )
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    sqlite3_bind_int(statement, 1, account_id);
    sqlite3_bind_int(statement, 2, school_id);
    sqlite3_bind_text(statement, 3, full_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 4, birth_date, -1, SQLITE_TRANSIENT);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    if (result != SQLITE_DONE)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    if (execute_sql("COMMIT;") == 0)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    return 1;
}

int create_donor(
    const char *full_name,
    const char *birth_date,
    const char *cpf,
    const char *email,
    const char *password
)
{
    const char *sql;
    sqlite3_stmt *statement;
    char cpf_hash[HASH_LENGTH];
    int account_id;
    int result;

    if (
        text_is_empty(full_name) ||
        text_is_empty(birth_date) ||
        text_is_empty(cpf) ||
        text_is_empty(email) ||
        password_is_valid(password) == 0
    )
    {
        return 0;
    }

    if (is_valid_cpf(cpf) == 0)
    {
        return 0;
    }

    if (create_cpf_hash(cpf, cpf_hash) == 0)
    {
        return 0;
    }

    if (execute_sql("BEGIN TRANSACTION;") == 0)
    {
        return 0;
    }

    account_id = insert_account(
        email,
        password,
        "DONOR",
        cpf_hash
    );

    if (account_id == -1)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    sql =
        "INSERT INTO donors "
        "(account_id, full_name, birth_date) "
        "VALUES (?, ?, ?);";

    statement = NULL;

    if (
        sqlite3_prepare_v2(
            database,
            sql,
            -1,
            &statement,
            NULL
        ) != SQLITE_OK
    )
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    sqlite3_bind_int(statement, 1, account_id);
    sqlite3_bind_text(statement, 2, full_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, birth_date, -1, SQLITE_TRANSIENT);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    if (result != SQLITE_DONE)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    if (execute_sql("COMMIT;") == 0)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    return 1;
}

int create_partner_store(
    const char *name,
    const char *cnpj,
    const char *email,
    const char *password,
    double discount
)
{
    const char *sql;
    sqlite3_stmt *statement;
    char normalized_cnpj[CNPJ_LENGTH + 1];
    int account_id;
    int result;

    if (
        text_is_empty(name) ||
        text_is_empty(cnpj) ||
        text_is_empty(email) ||
        password_is_valid(password) == 0
    )
    {
        return 0;
    }

    if (discount < 0 || discount > 100)
    {
        return 0;
    }

    if (is_valid_cnpj(cnpj) == 0)
    {
        return 0;
    }

    if (
        normalize_document(
            cnpj,
            normalized_cnpj,
            CNPJ_LENGTH
        ) == 0
    )
    {
        return 0;
    }

    if (execute_sql("BEGIN TRANSACTION;") == 0)
    {
        return 0;
    }

    account_id = insert_account(
        email,
        password,
        "PARTNER_STORE",
        NULL
    );

    if (account_id == -1)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    sql =
        "INSERT INTO partner_stores "
        "(account_id, name, cnpj, discount) "
        "VALUES (?, ?, ?, ?);";

    statement = NULL;

    if (
        sqlite3_prepare_v2(
            database,
            sql,
            -1,
            &statement,
            NULL
        ) != SQLITE_OK
    )
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    sqlite3_bind_int(statement, 1, account_id);
    sqlite3_bind_text(statement, 2, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 3, normalized_cnpj, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(statement, 4, discount);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    if (result != SQLITE_DONE)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    if (execute_sql("COMMIT;") == 0)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    return 1;
}


/* =========================================================
   LOGIN
   ========================================================= */

int authenticate_user(
    const char *email,
    const char *password,
    UserSession *session
)
{
    const char *sql;
    sqlite3_stmt *statement;
    const char *password_hash;
    const char *account_type;

    if (
        text_is_empty(email) ||
        text_is_empty(password) ||
        session == NULL
    )
    {
        return 0;
    }

    sql =
        "SELECT id, password_hash, account_type "
        "FROM accounts "
        "WHERE email = ?;";

    statement = NULL;

    if (
        sqlite3_prepare_v2(
            database,
            sql,
            -1,
            &statement,
            NULL
        ) != SQLITE_OK
    )
    {
        return 0;
    }

    sqlite3_bind_text(
        statement,
        1,
        email,
        -1,
        SQLITE_TRANSIENT
    );

    if (sqlite3_step(statement) != SQLITE_ROW)
    {
        sqlite3_finalize(statement);
        return 0;
    }

    password_hash =
        (const char *)sqlite3_column_text(statement, 1);

    account_type =
        (const char *)sqlite3_column_text(statement, 2);

    if (verify_password(password, password_hash) == 0)
    {
        sqlite3_finalize(statement);
        return 0;
    }

    session->account_id =
        sqlite3_column_int(statement, 0);

    session->account_type =
        convert_account_type(account_type);

    sqlite3_finalize(statement);

    return session->account_type != ACCOUNT_TYPE_NONE;
}


/* =========================================================
   EDIÇÃO DE PERFIL
   ========================================================= */

int update_receiver_profile(
    int account_id,
    const char *full_name,
    const char *birth_date,
    const char *email,
    const char *inep
)
{
    const char *sql;
    sqlite3_stmt *statement;
    int school_id;
    int result;

    if (
        text_is_empty(full_name) ||
        text_is_empty(birth_date) ||
        text_is_empty(email) ||
        text_is_empty(inep)
    )
    {
        return 0;
    }

    school_id = find_school_id(inep);

    if (school_id == -1)
    {
        return 0;
    }

    if (execute_sql("BEGIN TRANSACTION;") == 0)
    {
        return 0;
    }

    if (update_account_email(account_id, email) == 0)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    sql =
        "UPDATE receivers "
        "SET full_name = ?, birth_date = ?, school_id = ? "
        "WHERE account_id = ?;";

    statement = NULL;

    if (
        sqlite3_prepare_v2(
            database,
            sql,
            -1,
            &statement,
            NULL
        ) != SQLITE_OK
    )
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    sqlite3_bind_text(statement, 1, full_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, birth_date, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 3, school_id);
    sqlite3_bind_int(statement, 4, account_id);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    if (result != SQLITE_DONE)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    if (execute_sql("COMMIT;") == 0)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    return 1;
}

int update_donor_profile(
    int account_id,
    const char *full_name,
    const char *birth_date,
    const char *email
)
{
    const char *sql;
    sqlite3_stmt *statement;
    int result;

    if (
        text_is_empty(full_name) ||
        text_is_empty(birth_date) ||
        text_is_empty(email)
    )
    {
        return 0;
    }

    if (execute_sql("BEGIN TRANSACTION;") == 0)
    {
        return 0;
    }

    if (update_account_email(account_id, email) == 0)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    sql =
        "UPDATE donors "
        "SET full_name = ?, birth_date = ? "
        "WHERE account_id = ?;";

    statement = NULL;

    if (
        sqlite3_prepare_v2(
            database,
            sql,
            -1,
            &statement,
            NULL
        ) != SQLITE_OK
    )
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    sqlite3_bind_text(statement, 1, full_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, birth_date, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(statement, 3, account_id);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    if (result != SQLITE_DONE)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    if (execute_sql("COMMIT;") == 0)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    return 1;
}

int update_partner_store_profile(
    int account_id,
    const char *name,
    const char *email,
    const char *address,
    double discount
)
{
    const char *sql;
    sqlite3_stmt *statement;
    int result;

    if (
        text_is_empty(name) ||
        text_is_empty(email) ||
        text_is_empty(address)
    )
    {
        return 0;
    }

    if (discount < 0 || discount > 100)
    {
        return 0;
    }

    if (execute_sql("BEGIN TRANSACTION;") == 0)
    {
        return 0;
    }

    if (update_account_email(account_id, email) == 0)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    sql =
        "UPDATE partner_stores "
        "SET name = ?, address = ?, discount = ? "
        "WHERE account_id = ?;";

    statement = NULL;

    if (
        sqlite3_prepare_v2(
            database,
            sql,
            -1,
            &statement,
            NULL
        ) != SQLITE_OK
    )
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    sqlite3_bind_text(statement, 1, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(statement, 2, address, -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(statement, 3, discount);
    sqlite3_bind_int(statement, 4, account_id);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    if (result != SQLITE_DONE)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    if (execute_sql("COMMIT;") == 0)
    {
        execute_sql("ROLLBACK;");
        return 0;
    }

    return 1;
}

int set_partner_store_active(
    int account_id,
    int is_active
)
{
    const char *sql;
    sqlite3_stmt *statement;
    int result;

    if (is_active != 0 && is_active != 1)
    {
        return 0;
    }

    sql =
        "UPDATE partner_stores "
        "SET is_active = ? "
        "WHERE account_id = ?;";

    statement = NULL;

    if (
        sqlite3_prepare_v2(
            database,
            sql,
            -1,
            &statement,
            NULL
        ) != SQLITE_OK
    )
    {
        return 0;
    }

    sqlite3_bind_int(statement, 1, is_active);
    sqlite3_bind_int(statement, 2, account_id);

    result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    return result == SQLITE_DONE;
}
