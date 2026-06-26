chcp 65001 | Out-Null

[Console]::InputEncoding =
    [System.Text.UTF8Encoding]::new()

[Console]::OutputEncoding =
    [System.Text.UTF8Encoding]::new()

$OutputEncoding =
    [System.Text.UTF8Encoding]::new()

if ([string]::IsNullOrWhiteSpace($env:CPF_HASH_SECRET))
{
    Write-Host ""
    Write-Host "A variável CPF_HASH_SECRET não foi configurada."
    Write-Host 'Exemplo: $env:CPF_HASH_SECRET="uma-chave-secreta-grande"'
    exit 1
}

New-Item -ItemType Directory -Force build | Out-Null

if (-not (Test-Path ".\database\educacompartilha.db"))
{
    sqlite3 .\database\educacompartilha.db ".read database/schema.sql"
}

sqlite3 .\database\educacompartilha.db `
    "INSERT OR IGNORE INTO schools (inep, name) VALUES ('12345678', 'Escola Pública de Teste');"

Write-Host ""
Write-Host "Compilando os testes..."

gcc -Wall -Wextra -Wpedantic -std=c17 `
    -finput-charset=UTF-8 `
    -fexec-charset=UTF-8 `
    tests\test.c `
    src\models\model.c `
    libs\libbcrypt\bcrypt.c `
    libs\libbcrypt\crypt_blowfish\crypt_blowfish.c `
    libs\libbcrypt\crypt_blowfish\crypt_gensalt.c `
    libs\libbcrypt\crypt_blowfish\wrapper.c `
    libs\libbcrypt\crypt_blowfish\x86.S `
    -iquote libs\libbcrypt `
    -iquote libs\libbcrypt\crypt_blowfish `
    -lsqlite3 `
    -lcrypto `
    -o build\test.exe

if ($LASTEXITCODE -ne 0)
{
    Write-Host ""
    Write-Host "Erro ao compilar os testes."
    exit 1
}

Write-Host ""
Write-Host "Executando os testes..."

.\build\test.exe

if ($LASTEXITCODE -ne 0)
{
    Write-Host ""
    Write-Host "Um ou mais testes falharam."
    exit 1
}

Write-Host ""
Write-Host "Compilando o sistema..."

gcc -Wall -Wextra -Wpedantic -std=c17 `
    -finput-charset=UTF-8 `
    -fexec-charset=UTF-8 `
    src\main.c `
    src\views\view.c `
    src\controllers\controller.c `
    src\models\model.c `
    libs\libbcrypt\bcrypt.c `
    libs\libbcrypt\crypt_blowfish\crypt_blowfish.c `
    libs\libbcrypt\crypt_blowfish\crypt_gensalt.c `
    libs\libbcrypt\crypt_blowfish\wrapper.c `
    libs\libbcrypt\crypt_blowfish\x86.S `
    -iquote libs\libbcrypt `
    -iquote libs\libbcrypt\crypt_blowfish `
    -lsqlite3 `
    -lcrypto `
    -o build\educacompartilha.exe

if ($LASTEXITCODE -ne 0)
{
    Write-Host ""
    Write-Host "Erro ao compilar o sistema."
    exit 1
}

Write-Host ""
Write-Host "Iniciando o EducaCompartilha..."
Write-Host ""

.\build\educacompartilha.exe
