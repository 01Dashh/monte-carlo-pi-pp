# Estimativa de Pi por Monte Carlo (Paralelismo em CPU)

Projeto da disciplina de Programacao Paralela (CCO085) - IESB 2026/2.
Problema 2: Estimativa de Pi via metodo de Monte Carlo.

## Integrantes e Frentes

- Lucas Goncalves Rocha: Baseline sequencial e perfilamento
- Lucas Faria Ribeiro: Paralelizacao em memoria compartilhada (OpenMP)
- Joao Lucas Gomes da Silva: Paralelizacao em memoria distribuida (MPI)

## Estrutura do Repositorio

- `codigos/00_base/baseline_pi.cpp`: Codigo sequencial de referencia.
- `codigos/01_openmp/02_pi_openmp.cpp`: Versao paralela com OpenMP e reduction.
- `codigos/02_mpi/03_coletivas_pi.cpp`: Versao distribuida com MPI e MPI_Reduce.
- `scripts/medir_openmp.sh`: Script de medicao automatizada para OpenMP.
- `scripts/medir_mpi.sh`: Script de medicao automatizada para MPI.
- `resultados/medicoes_experimento.csv`: Dados brutos coletados nos testes.
- `Makefile`: Script de compilacao de todos os binarios.
- `AUTORES.md`: Declaracao formal de responsabilidades do trio.

## Compilacao

Dentro do ambiente Linux (container Docker ou terminal com g++, OpenMP e OpenMPI):

```bash
make clean
make
```

O comando compila todas as versoes com otimizacao `-O3` e gera os executaveis correspondentes em cada subdiretorio de `codigos/`.

## Execucao dos Testes (N = 50.000.000)

### 1. Baseline Sequencial
```bash
./codigos/00_base/baseline_pi 50000000
```

### 2. Versao OpenMP (4 threads)
```bash
./codigos/01_openmp/02_pi_openmp 50000000 4
```
Tambem pode ser executado definindo a variavel de ambiente:
```bash
OMP_NUM_THREADS=4 ./codigos/01_openmp/02_pi_openmp 50000000
```

### 3. Versao MPI (4 processos)
```bash
mpirun -np 4 ./codigos/02_mpi/03_coletivas_pi 50000000
```

## Resultados Medidos em Laboratorio (N = 50.000.000)

| Versao | Unidades | Tempo (s) | Speedup | Eficiencia |
| :--- | :--- | :--- | :--- | :--- |
| Baseline | 1 nucleo | 0.427 s | 1.00x | 100.0% |
| OpenMP | 4 threads | 0.126 s | 3.49x | 87.2% |
| MPI | 4 processos | 0.196 s | 2.18x | 54.5% |

Observacao sobre os resultados: o OpenMP apresentou maior speedup que o MPI em um unico no devido ao menor overhead de criacao e sincronizacao de threads leves em memoria compartilhada em relacao a criacao de multiplos processos de sistema operacional via mpirun.
