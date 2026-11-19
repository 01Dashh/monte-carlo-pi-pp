// 03_mpi_pi.cpp
// Compilação: mpicxx -O3 03_mpi_pi.cpp -o 03_mpi_pi
#include <iostream>
#include <random>
#include <string>
#include <mpi.h>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    long long N = 100000000;
    if (argc > 1) {
        N = std::stoll(argv[1]);
    }

    // Decomposição de domínio: Data Parallelism
    long long N_local = N / size;
    // O último processo assume o resto da divisão para garantir corretude total
    if (rank == size - 1) {
        N_local += N % size;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    long long pontos_locais = 0;
    
    // Cada processo inicializa seu próprio gerador com base no seu rank
    std::mt19937 gen(42 + rank);
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    for (long long i = 0; i < N_local; ++i) {
        double x = dis(gen);
        double y = dis(gen);
        if (x * x + y * y <= 1.0) {
            pontos_locais++;
        }
    }

    long long pontos_globais = 0;
    // Operação Coletiva: Consolida a soma de pontos_locais de todos os ranks no rank 0
    MPI_Reduce(&pontos_locais, &pontos_globais, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();

    if (rank == 0) {
        double pi_estimado = 4.0 * pontos_globais / N;
        std::cout << "Estimativa de Pi (MPI): " << pi_estimado << "\n";
        std::cout << "Tempo decorrido: " << (end_time - start_time) << " s\n";
    }

    MPI_Finalize();
    return 0;
}