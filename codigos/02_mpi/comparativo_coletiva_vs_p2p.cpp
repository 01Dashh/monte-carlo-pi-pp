// codigos/02_mpi/comparativo_coletiva_vs_p2p.cpp
// Programacao Paralela (CCO085) - IESB
// Autor: Joao Lucas Gomes da Silva
// Comparativo de Comunicacao em Memoria Distribuida (MPI):
// - Operacao Coletiva: MPI_Reduce (reducao em arvore binaria, complexidade O(log P))
// - Comunicacao Ponto a Ponto: MPI_Send / MPI_Recv (recebimento serializado, complexidade O(P))
//
// Compilacao: mpicxx -O3 comparativo_coletiva_vs_p2p.cpp -o comparativo_coletiva_vs_p2p
// Execucao:   mpirun -np 4 ./comparativo_coletiva_vs_p2p 50000000

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

    // Decomposicao de dominio equilibrada
    long long N_local = N / size;
    if (rank == size - 1) {
        N_local += N % size; // Ultimo rank trata o resto da divisao
    }

    // Gerador de numeros aleatorios isolado por rank
    std::mt19937 gen(42 + rank);
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    // =========================================================================
    // FASE DE COMPUTACAO LOCAL (Identica para ambos os modelos de comunicacao)
    // =========================================================================
    MPI_Barrier(MPI_COMM_WORLD);
    double t_comp_ini = MPI_Wtime();

    long long pontos_locais = 0;
    for (long long i = 0; i < N_local; ++i) {
        double x = dis(gen);
        double y = dis(gen);
        if (x * x + y * y <= 1.0) {
            pontos_locais++;
        }
    }

    double t_comp_fim = MPI_Wtime();
    double tempo_computacao = t_comp_fim - t_comp_ini;

    // =========================================================================
    // TESTE 1: COMUNICACAO COLETIVA VIA MPI_Reduce (Arvore O(log P))
    // =========================================================================
    MPI_Barrier(MPI_COMM_WORLD);
    double t_col_ini = MPI_Wtime();

    long long total_coletiva = 0;
    MPI_Reduce(&pontos_locais, &total_coletiva, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    double t_col_fim = MPI_Wtime();
    double tempo_coletiva = t_col_fim - t_col_ini;

    // =========================================================================
    // TESTE 2: COMUNICACAO PONTO A PONTO VIA MPI_Send / MPI_Recv (Linear O(P))
    // =========================================================================
    MPI_Barrier(MPI_COMM_WORLD);
    double t_p2p_ini = MPI_Wtime();

    long long total_p2p = 0;
    if (rank != 0) {
        // Processos trabalhadores enviam seus resultados para o rank 0
        MPI_Send(&pontos_locais, 1, MPI_LONG_LONG, 0, 99, MPI_COMM_WORLD);
    } else {
        // Rank 0 contabiliza o seu proprio total
        total_p2p = pontos_locais;

        // E recebe individualmente a contagem de cada trabalhador
        for (int p = 1; p < size; ++p) {
            long long parcial = 0;
            MPI_Recv(&parcial, 1, MPI_LONG_LONG, p, 99, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_p2p += parcial;
        }
    }

    double t_p2p_fim = MPI_Wtime();
    double tempo_p2p = t_p2p_fim - t_p2p_ini;

    // =========================================================================
    // EXIBICAO DOS RESULTADOS NO RANK 0
    // =========================================================================
    if (rank == 0) {
        double pi_col = 4.0 * total_coletiva / static_cast<double>(N);
        double pi_p2p = 4.0 * total_p2p / static_cast<double>(N);

        std::cout << "===================================================================\n";
        std::cout << " BENCHMARK MPI: COLETIVA VS PONTO A PONTO - JOAO LUCAS G. DA SILVA\n";
        std::cout << " Processos: " << size << " | Amostras: " << N << "\n";
        std::cout << "===================================================================\n\n";

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "1. Fase de Computacao Pura:\n";
        std::cout << "   - Tempo medio de calculo local: " << tempo_computacao << " s\n\n";

        std::cout << "2. Comparativo de Comunicacao e Agregacao:\n";
        std::cout << "   - Operacao Coletiva (MPI_Reduce):      " << (tempo_coletiva * 1e6) << " microsegundos\n";
        std::cout << "   - Ponto a Ponto (MPI_Send / MPI_Recv): " << (tempo_p2p * 1e6) << " microsegundos\n";
        std::cout << "   - Pi Estimado (MPI_Reduce):            " << pi_col << "\n";
        std::cout << "   - Pi Estimado (Ponto a Ponto):         " << pi_p2p << "\n\n";

        std::cout << "3. Conclusao Tecnica de Comunicacao:\n";
        std::cout << "   A operacao MPI_Reduce utiliza algoritmo de reducao em arvore com\n";
        std::cout << "   complexidade O(log P), garantindo escalabilidade superior a escala\n";
        std::cout << "   linear O(P) do envio ponto a ponto serializado no processo mestre.\n";
        std::cout << "===================================================================\n";
    }

    MPI_Finalize();
    return 0;
}
