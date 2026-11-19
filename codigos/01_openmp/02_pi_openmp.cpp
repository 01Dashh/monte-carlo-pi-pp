// 01_openmp/02_pi_openmp.cpp
// Programação Paralela (CCO085) - IESB 2026/2
// Estimativa de Pi por Monte Carlo - Versão OpenMP (Memória Compartilhada)
#include <iostream>
#include <random>
#include <chrono>
#include <string>
#include <iomanip>
#include <omp.h>

static double pi_seq(long long N, double &tempo_seq) {
    auto start = std::chrono::high_resolution_clock::now();
    long long pontos_no_circulo = 0;
    std::mt19937 gen(42);
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    for (long long i = 0; i < N; ++i) {
        double x = dis(gen);
        double y = dis(gen);
        if (x * x + y * y <= 1.0) pontos_no_circulo++;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    tempo_seq = diff.count();
    return 4.0 * pontos_no_circulo / (double)N;
}

static double pi_par(long long N, int num_threads, double &tempo_par) {
    auto start = std::chrono::high_resolution_clock::now();
    long long pontos_no_circulo = 0;

    // reduction(+:pontos_no_circulo) cria contadores privados por thread e evita race condition
    // Otimizacao: escalonamento estatico padrao com carga balanceada e gerador mt19937 por thread    
    #pragma omp parallel num_threads(num_threads) reduction(+:pontos_no_circulo)
    {
        int tid = omp_get_thread_num();
        // Gerador pseudoaleatório local com seed independente por thread
        std::mt19937 gen(42 + tid);
        std::uniform_real_distribution<double> dis(0.0, 1.0);

        #pragma omp for
        for (long long i = 0; i < N; ++i) {
            double x = dis(gen);
            double y = dis(gen);
            if (x * x + y * y <= 1.0) {
                pontos_no_circulo++;
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;
    tempo_par = diff.count();
    return 4.0 * pontos_no_circulo / (double)N;
}

int main(int argc, char* argv[]) {
    long long N = 50000000;
    int num_threads = 4;
    
    if (argc > 1) N = std::stoll(argv[1]);
    if (argc > 2) num_threads = std::stoi(argv[2]);

    double t_seq = 0.0, t_par = 0.0;
    double pi_s = pi_seq(N, t_seq);
    double pi_p = pi_par(N, num_threads, t_par);

    double speedup = (t_par > 0.0) ? (t_seq / t_par) : 0.0;
    double eficiencia = (speedup / num_threads) * 100.0;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "sequencial : pi=" << pi_s << "  t=" << t_seq << " s\n";
    std::cout << "paralelo   : pi=" << pi_p << "  t=" << t_par << " s  (" << num_threads << " threads)\n";
    std::cout << std::setprecision(2);
    std::cout << "speedup    : " << speedup << "x\n";
    std::cout << "eficiencia : " << eficiencia << "%\n";

    return 0;
}
