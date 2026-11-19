// codigos/01_openmp/benchmark_escalonamento.cpp
// Programacao Paralela (CCO085) - IESB
// Autor: Lucas Faria Ribeiro
// Estudo comparativo de politicas de escalonamento no OpenMP para Monte Carlo:
// - schedule(static)
// - schedule(dynamic)
// - schedule(guided)
//
// Compilacao: g++ -O3 -fopenmp benchmark_escalonamento.cpp -o benchmark_escalonamento

#include <iostream>
#include <random>
#include <chrono>
#include <string>
#include <vector>
#include <iomanip>
#include <omp.h>

struct ResultadoEscalonamento {
    std::string politica;
    int chunk_size;
    double tempo_s;
    double pi_calculado;
    double speedup;
};

// Execucao com escalonamento estatico
ResultadoEscalonamento testar_static(long long N, int threads, int chunk, double tempo_base) {
    auto start = std::chrono::high_resolution_clock::now();
    long long acertos = 0;

    #pragma omp parallel num_threads(threads) reduction(+:acertos)
    {
        int tid = omp_get_thread_num();
        std::mt19937 gen(42 + tid);
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        if (chunk > 0) {
            #pragma omp for schedule(static, chunk)
            for (long long i = 0; i < N; ++i) {
                double x = dis(gen);
                double y = dis(gen);
                if (x * x + y * y <= 1.0) acertos++;
            }
        } else {
            #pragma omp for schedule(static)
            for (long long i = 0; i < N; ++i) {
                double x = dis(gen);
                double y = dis(gen);
                if (x * x + y * y <= 1.0) acertos++;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    double pi = 4.0 * acertos / static_cast<double>(N);

    return {"static", chunk, diff.count(), pi, tempo_base / diff.count()};
}

// Execucao com escalonamento dinamico
ResultadoEscalonamento testar_dynamic(long long N, int threads, int chunk, double tempo_base) {
    auto start = std::chrono::high_resolution_clock::now();
    long long acertos = 0;

    #pragma omp parallel num_threads(threads) reduction(+:acertos)
    {
        int tid = omp_get_thread_num();
        std::mt19937 gen(42 + tid);
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        #pragma omp for schedule(dynamic, chunk)
        for (long long i = 0; i < N; ++i) {
            double x = dis(gen);
            double y = dis(gen);
            if (x * x + y * y <= 1.0) acertos++;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    double pi = 4.0 * acertos / static_cast<double>(N);

    return {"dynamic", chunk, diff.count(), pi, tempo_base / diff.count()};
}

// Execucao com escalonamento guiado (guided)
ResultadoEscalonamento testar_guided(long long N, int threads, int chunk, double tempo_base) {
    auto start = std::chrono::high_resolution_clock::now();
    long long acertos = 0;

    #pragma omp parallel num_threads(threads) reduction(+:acertos)
    {
        int tid = omp_get_thread_num();
        std::mt19937 gen(42 + tid);
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        #pragma omp for schedule(guided, chunk)
        for (long long i = 0; i < N; ++i) {
            double x = dis(gen);
            double y = dis(gen);
            if (x * x + y * y <= 1.0) acertos++;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    double pi = 4.0 * acertos / static_cast<double>(N);

    return {"guided", chunk, diff.count(), pi, tempo_base / diff.count()};
}

int main(int argc, char* argv[]) {
    long long N = 20000000; // 20M para comparativo rapido
    int threads = 4;

    if (argc > 1) N = std::stoll(argv[1]);
    if (argc > 2) threads = std::stoi(argv[2]);

    std::cout << "===============================================================\n";
    std::cout << " BENCHMARK DE ESCALONAMENTO OPENMP - LUCAS FARIA RIBEIRO\n";
    std::cout << " Amostras (N): " << N << " | Threads: " << threads << "\n";
    std::cout << "===============================================================\n\n";

    // 1. Medicao Sequencial de Referencia
    std::cout << "Calculando tempo sequencial de referencia..." << std::flush;
    auto s_start = std::chrono::high_resolution_clock::now();
    long long s_acertos = 0;
    std::mt19937 s_gen(42);
    std::uniform_real_distribution<double> s_dis(0.0, 1.0);
    for (long long i = 0; i < N; ++i) {
        double x = s_dis(s_gen);
        double y = s_dis(s_gen);
        if (x * x + y * y <= 1.0) s_acertos++;
    }
    auto s_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> s_diff = s_end - s_start;
    double t_seq = s_diff.count();
    std::cout << " Concluido: " << t_seq << " s\n\n";

    std::vector<ResultadoEscalonamento> resultados;

    // Teste 1: Static padrao (sem chunk definido)
    resultados.push_back(testar_static(N, threads, 0, t_seq));

    // Teste 2: Static com chunk 10000
    resultados.push_back(testar_static(N, threads, 10000, t_seq));

    // Teste 3: Dynamic com chunk 10000
    resultados.push_back(testar_dynamic(N, threads, 10000, t_seq));

    // Teste 4: Dynamic com chunk 50000
    resultados.push_back(testar_dynamic(N, threads, 50000, t_seq));

    // Teste 5: Guided com chunk 10000
    resultados.push_back(testar_guided(N, threads, 10000, t_seq));

    // Exibicao da Tabela Formatada
    std::cout << std::left << std::setw(12) << "Politica"
              << std::setw(12) << "Chunk"
              << std::setw(14) << "Tempo (s)"
              << std::setw(12) << "Speedup"
              << std::setw(14) << "Pi Estimado" << "\n";
    std::cout << "---------------------------------------------------------------\n";

    for (const auto& r : resultados) {
        std::cout << std::left << std::setw(12) << r.politica
                  << std::setw(12) << (r.chunk_size == 0 ? "padrao" : std::to_string(r.chunk_size))
                  << std::fixed << std::setprecision(4)
                  << std::setw(14) << r.tempo_s
                  << std::setprecision(2)
                  << std::setw(10) << r.speedup << "x"
                  << std::setprecision(6)
                  << std::setw(14) << r.pi_calculado << "\n";
    }

    std::cout << "\nConclusao Tecnica: Para problemas com carga computacional homogenea\n"
              << "(como Monte Carlo, onde cada ponto tem custo O(1)), schedule(static)\n"
              << "apresenta o menor overhead por dispensar fila de trabalho sincronizada.\n";

    return 0;
}
