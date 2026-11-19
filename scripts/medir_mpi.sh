#!/usr/bin/env bash
# Coleta de escalabilidade MPI - Joao Lucas Gomes da Silva
# Programacao Paralela (CCO085) - IESB
set -euo pipefail

N=${1:-50000000}
REPETICOES=${2:-3}
SAIDA="resultados/mpi_escalabilidade.csv"
BIN="codigos/02_mpi/03_coletivas_pi"

[ -x "$BIN" ] || { echo "Binario nao encontrado. Execute 'make mpi' antes."; exit 1; }
mkdir -p resultados

echo "processos,repeticao,tempo_s" > "$SAIDA"
echo "Executando medicoes MPI para N = $N..."

for P in 1 2 4; do
    for R in $(seq 1 "$REPETICOES"); do
        SAIDA_CMD=$(mpirun -np "$P" "$BIN" "$N")
        TEMPO=$(echo "$SAIDA_CMD" | grep 'Tempo decorrido' | sed -E 's/.*: ([0-9.]+) s/\1/')
        echo "$P,$R,$TEMPO" >> "$SAIDA"
    done
    echo "  $P processos: OK"
done

echo "Medicoes concluidas. Resultados gravados em $SAIDA"
