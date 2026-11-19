# Programação Paralela (CCO085) — IESB 2026/2
# Projeto Acadêmico: Estimativa de Pi por Monte Carlo
# Integrantes:
#   Lucas G. Rocha       - Baseline e Perfilamento
#   Lucas F. Ribeiro     - OpenMP (Memória Compartilhada)
#   João Lucas G. Silva  - MPI (Memória Distribuída)

CXX      = g++
MPICXX   = mpicxx
CXXFLAGS = -O3 -Wall
OMPFLAG  = -fopenmp
LDLIBS   = -lm

BASE_DIR = codigos/00_base
OMP_DIR  = codigos/01_openmp
MPI_DIR  = codigos/02_mpi

all: $(BASE_DIR)/baseline_pi $(OMP_DIR)/02_pi_openmp $(MPI_DIR)/03_coletivas_pi

base: $(BASE_DIR)/baseline_pi
openmp: $(OMP_DIR)/02_pi_openmp
mpi: $(MPI_DIR)/03_coletivas_pi

benchmarks: $(OMP_DIR)/benchmark_escalonamento $(MPI_DIR)/comparativo_coletiva_vs_p2p

$(BASE_DIR)/baseline_pi: $(BASE_DIR)/baseline_pi.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDLIBS)

$(OMP_DIR)/02_pi_openmp: $(OMP_DIR)/02_pi_openmp.cpp
	$(CXX) $(CXXFLAGS) $(OMPFLAG) -o $@ $< $(LDLIBS)

$(MPI_DIR)/03_coletivas_pi: $(MPI_DIR)/03_coletivas_pi.cpp
	$(MPICXX) $(CXXFLAGS) -o $@ $< $(LDLIBS)

$(OMP_DIR)/benchmark_escalonamento: $(OMP_DIR)/benchmark_escalonamento.cpp
	$(CXX) $(CXXFLAGS) $(OMPFLAG) -o $@ $< $(LDLIBS)

$(MPI_DIR)/comparativo_coletiva_vs_p2p: $(MPI_DIR)/comparativo_coletiva_vs_p2p.cpp
	$(MPICXX) $(CXXFLAGS) -o $@ $< $(LDLIBS)

clean:
	rm -f $(BASE_DIR)/baseline_pi $(OMP_DIR)/02_pi_openmp $(MPI_DIR)/03_coletivas_pi $(OMP_DIR)/benchmark_escalonamento $(MPI_DIR)/comparativo_coletiva_vs_p2p

.PHONY: all base openmp mpi benchmarks clean
