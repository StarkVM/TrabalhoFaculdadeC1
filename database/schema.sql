PRAGMA foreign_keys = ON;

CREATE TABLE IF NOT EXISTS schools
(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    inep TEXT NOT NULL UNIQUE,
    name TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS accounts
(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    email TEXT NOT NULL UNIQUE,
    password_hash TEXT NOT NULL,
    account_type TEXT NOT NULL,
    cpf_hash TEXT UNIQUE,

    CHECK
    (
        account_type IN
        (
            'RECEIVER',
            'DONOR',
            'PARTNER_STORE'
        )
    )
);

CREATE TABLE IF NOT EXISTS receivers
(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    account_id INTEGER NOT NULL UNIQUE,
    school_id INTEGER NOT NULL,
    full_name TEXT NOT NULL,
    birth_date TEXT NOT NULL,

    FOREIGN KEY (account_id)
        REFERENCES accounts(id)
        ON DELETE CASCADE,

    FOREIGN KEY (school_id)
        REFERENCES schools(id)
);

CREATE TABLE IF NOT EXISTS donors
(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    account_id INTEGER NOT NULL UNIQUE,
    full_name TEXT NOT NULL,
    birth_date TEXT NOT NULL,

    FOREIGN KEY (account_id)
        REFERENCES accounts(id)
        ON DELETE CASCADE
);

CREATE TABLE IF NOT EXISTS partner_stores
(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    account_id INTEGER NOT NULL UNIQUE,
    name TEXT NOT NULL,
    cnpj TEXT NOT NULL UNIQUE,
    address TEXT NOT NULL DEFAULT '',
    discount REAL NOT NULL DEFAULT 0,
    is_active INTEGER NOT NULL DEFAULT 1,

    FOREIGN KEY (account_id)
        REFERENCES accounts(id)
        ON DELETE CASCADE,

    CHECK (discount >= 0 AND discount <= 100),
    CHECK (is_active IN (0, 1))
);
