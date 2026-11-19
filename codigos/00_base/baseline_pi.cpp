// 00_base/baseline_pi.cpp
// Programação Paralela (CCO085) - IESB 2026/2
// Estimativa de Pi por Monte Carlo - Versão Sequencial (Baseline)
#include <iostream>
#include <random>
#include <chrono>
#include <string>
#include <iomanip>

int main(int argc, char* argv[]) {
    long long N = 50000000;
    if (argc > 1) {
        N = std::stoll(argv[1]);
    }

    auto start = std::chrono::high_resolution_clock::now();

    long long pontos_no_circulo = 0;
    
    // Geração de números aleatórios com seed fixa para reprodutibilidade
    std::mt19937 gen(42); 
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    // Loop principal (Hotspot do programa: complexidade O(N))
    for (long long i = 0; i < N; ++i) {
        double x = dis(gen);
        double y = dis(gen);
        if (x * x + y * y <= 1.0) {
            pontos_no_circulo++;
        }
    }

    double pi_estimado = 4.0 * pontos_no_circulo / (double)N;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Estimativa de Pi (Sequencial): " << pi_estimado << "\n";
    std::cout << "Pontos avaliados: " << N << "\n";
    std::cout << "Tempo decorrido: " << diff.count() << " s\n";

    return 0;
}
