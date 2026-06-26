chcp 65001 | Out-Null

Remove-Item `
    .\database\educacompartilha.db `
    -Force `
    -ErrorAction SilentlyContinue

sqlite3 `
    .\database\educacompartilha.db `
    ".read database/schema.sql"

sqlite3 `
    .\database\educacompartilha.db `
    ".read database/public_schools_cachoeiro.sql"

Write-Host "Banco de dados recriado com sucesso."
