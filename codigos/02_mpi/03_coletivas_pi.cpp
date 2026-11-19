// 02_mpi/03_coletivas_pi.cpp
// Programação Paralela (CCO085) - IESB 2026/2
// Estimativa de Pi por Monte Carlo - Versão MPI (Memória Distribuída)
#include <iostream>
#include <random>
#include <string>
#include <iomanip>
#include <mpi.h>

int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    long long N = 50000000;
    if (argc > 1) {
        N = std::stoll(argv[1]);
    }

    // Decomposição de domínio: Data Parallelism (N / size por processo)
    long long N_local = N / size;
    if (rank == size - 1) {
        N_local += N % size; // Último rank absorve o resto da divisão
    }

    // Sincronizacao de barreira para isolar o tempo estrito de amostragem e comunicacao

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    long long pontos_locais = 0;
    
    // Gerador de aleatórios local independente por rank (evita correlação estatística)
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
    // Operação Coletiva: Consolida a soma dos pontos locais no rank 0
    MPI_Reduce(&pontos_locais, &pontos_globais, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();

    if (rank == 0) {
        double pi_estimado = 4.0 * pontos_globais / (double)N;
        double tempo_mpi = end_time - start_time;
        std::cout << std::fixed << std::setprecision(6);
        std::cout << "Estimativa de Pi (MPI): " << pi_estimado << "\n";
        std::cout << "Pontos avaliados: " << N << "\n";
        std::cout << "Processos (ranks): " << size << "\n";
        std::cout << "Tempo decorrido: " << tempo_mpi << " s\n";
    }

    MPI_Finalize();
    return 0;
}
