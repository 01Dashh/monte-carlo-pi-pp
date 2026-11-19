// 02_openmp_pi.cpp
// Compilação: g++ -O3 -fopenmp 02_openmp_pi.cpp -o 02_openmp_pi
#include <iostream>
#include <random>
#include <chrono>
#include <string>
#include <omp.h>

int main(int argc, char* argv[]) {
    long long N = 100000000;
    int num_threads = 4;
    
    if (argc > 1) N = std::stoll(argv[1]);
    if (argc > 2) num_threads = std::stoi(argv[2]);

    omp_set_num_threads(num_threads);

    auto start = std::chrono::high_resolution_clock::now();
    long long pontos_no_circulo = 0;

    // Paralelização do laço. 
    // reduction(+:pontos_no_circulo) cria cópias privadas locais por thread 
    // e soma os resultados no final, evitando condição de corrida.
    #pragma omp parallel reduction(+:pontos_no_circulo)
    {
        int thread_id = omp_get_thread_num();
        
        // CRÍTICO: PRNG deve ser instanciado localmente por thread.
        // Se usássemos um gerador global, teríamos condições de corrida ou contenção de locks.
        // A seed varia pelo thread_id para garantir independência estatística.
        std::mt19937 gen(42 + thread_id); 
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

    double pi_estimado = 4.0 * pontos_no_circulo / N;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "Estimativa de Pi (OpenMP): " << pi_estimado << "\n";
    std::cout << "Tempo decorrido: " << diff.count() << " s\n";

    return 0;
}