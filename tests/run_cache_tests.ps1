Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$Root = Split-Path -Parent $PSScriptRoot
$TempDir = Join-Path ([System.IO.Path]::GetTempPath()) ("cache_tests_" + [System.Guid]::NewGuid().ToString())
$Exe = Join-Path $TempDir "cache_test.exe"
$InputFile = Join-Path $TempDir "wt_write_miss.txt"

New-Item -ItemType Directory -Path $TempDir | Out-Null

try {
    Set-Content -Path $InputFile -Value @(
        "00000000 W"
        "00000000 R"
    )

    gcc (Join-Path $Root "cache.c") -o $Exe

    function Assert-Contains {
        param(
            [string]$Output,
            [string]$Expected
        )

        if ($Output -notlike "*$Expected*") {
            Write-Error "Esperado encontrar: $Expected`nSaida obtida:`n$Output"
        }
    }

    $saidaWt = & $Exe $InputFile 0 4 1 1 5 LRU 70
    $textoWt = $saidaWt -join "`n"

    Assert-Contains $textoWt "Escritas: 1."
    Assert-Contains $textoWt "Leituras: 1."
    Assert-Contains $textoWt "Memoria Principal Leitura: 1."
    Assert-Contains $textoWt "Memoria Principal Escrita: 1."
    Assert-Contains $textoWt "Hit Rate Leitura: 0.0000% (0)"
    Assert-Contains $textoWt "Hit Rate Escrita: 0.0000% (0)"
    Assert-Contains $textoWt "Hit Rate Global: 0.0000% (0)"

    $saidaDefaultFile = & $Exe 0 4 1 1 5 LRU 70
    $textoDefaultFile = $saidaDefaultFile -join "`n"

    Assert-Contains $textoDefaultFile "Politica de Escrita: Write-Through."
    Assert-Contains $textoDefaultFile "Tamanho da Linha: 4."

    Write-Host "Todos os testes passaram."
} finally {
    Remove-Item -LiteralPath $TempDir -Recurse -Force -ErrorAction SilentlyContinue
}
