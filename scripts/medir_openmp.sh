#!/usr/bin/env bash
# Coleta de escalabilidade OpenMP - Lucas Faria Ribeiro
# Programacao Paralela (CCO085) - IESB
set -euo pipefail

N=${1:-50000000}
REPETICOES=${2:-3}
SAIDA="resultados/openmp_escalabilidade.csv"
BIN="codigos/01_openmp/02_pi_openmp"

[ -x "$BIN" ] || { echo "Binario nao encontrado. Execute 'make openmp' antes."; exit 1; }
mkdir -p resultados

echo "threads,repeticao,tempo_s" > "$SAIDA"
echo "Executando medicoes OpenMP para N = $N..."

for T in 1 2 4 8; do
    for R in $(seq 1 "$REPETICOES"); do
        SAIDA_CMD=$("$BIN" "$N" "$T")
        TEMPO=$(echo "$SAIDA_CMD" | grep 'paralelo' | sed -E 's/.*t=([0-9.]+) s.*/\1/')
        echo "$T,$R,$TEMPO" >> "$SAIDA"
    done
    echo "  $T threads: OK"
done

echo "Medicoes concluidas. Resultados gravados em $SAIDA"
