// 01_baseline_pi.cpp
// Compilação: g++ -O3 01_baseline_pi.cpp -o 01_baseline_pi
#include <iostream>
#include <random>
#include <chrono>
#include <string>

int main(int argc, char* argv[]) {
    // Parâmetro de entrada: número de amostras (pontos)
    long long N = 100000000;
    if (argc > 1) {
        N = std::stoll(argv[1]);
    }

    auto start = std::chrono::high_resolution_clock::now();

    long long pontos_no_circulo = 0;
    
    // Geração de números aleatórios com seed fixa para reprodutibilidade no baseline
    std::mt19937 gen(42); 
    std::uniform_real_distribution<double> dis(0.0, 1.0);

    // Loop principal (Hotspot)
    for (long long i = 0; i < N; ++i) {
        double x = dis(gen);
        double y = dis(gen);
        
        // Verifica se o ponto caiu dentro do círculo unitário
        if (x * x + y * y <= 1.0) {
            pontos_no_circulo++;
        }
    }

    double pi_estimado = 4.0 * pontos_no_circulo / N;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "Estimativa de Pi (Sequencial): " << pi_estimado << "\n";
    std::cout << "Tempo decorrido: " << diff.count() << " s\n";

    return 0;
}