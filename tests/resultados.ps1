$caches = @(8192,16384)
$blocos = @(64,128)
$assocs = @(2,4)
$politicas = @(0,1)

$resultados = @()

foreach ($cache_size in $caches)
{
    foreach ($bloco in $blocos)
    {
        $linhas = $cache_size / $bloco

        foreach ($assoc in $assocs)
        {
            foreach ($escrita in $politicas)
            {
                $saida = .\cache.exe teste_longo.txt $escrita $bloco $linhas $assoc 5 LRU 70

                $leituras = (($saida | Select-String "Memoria Principal Leitura").Line -replace '[^0-9]','')

                $escritas = (($saida | Select-String "Memoria Principal Escrita").Line -replace '[^0-9]','')

                $total = [int]$leituras + [int]$escritas

                $politica_nome = if($escrita -eq 0) {"Write-Through"} else {"Write-Back"}

                $resultados += [PSCustomObject]@{
                    "Politica" = $politica_nome
                    "Cache (Bytes)" = $cache_size
                    "Bloco (Bytes)" = $bloco
                    "Associatividade" = $assoc
                    "Leituras MP" = $leituras
                    "Escritas MP" = $escritas
                    "Total Trafego" = $total
                }
            }
        }
    }
}

$resultados | Export-Csv resultado_trafego_memoria.csv -NoTypeInformation -Delimiter ';'