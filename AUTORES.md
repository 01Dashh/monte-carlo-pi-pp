# Integrantes e Frentes de Atuação

**Disciplina:** Programação Paralela (CCO085) — IESB 2026/2  
**Projeto:** Paralelização de um Problema Computacionalmente Intensivo  
**Problema 2:** Estimativa de $\pi$ por Monte Carlo  

---

### Responsabilidades Individuais Formais:

1. **Lucas Gonçalves Rocha**
   - **Frente:** Baseline e Perfilamento
   - **Atribuições:** Análise de complexidade assintótica $O(N)$, identificação e perfilamento do hotspot (laço de amostragem), definição da metodologia de medição, estimativa da fração paralelizável e projeção de speedup pela Lei de Amdahl.

2. **Lucas Faria Ribeiro**
   - **Frente:** OpenMP (Memória Compartilhada)
   - **Atribuições:** Paralelização do laço com diretiva `#pragma omp parallel for reduction(+:pontos_no_circulo)`, mitigação de condições de corrida e eliminação de contenção de cache (*false sharing*), criação de geradores pseudoaleatórios independentes por thread (`std::mt19937`), medições de speedup e eficiência.

3. **João Lucas Gomes Da Silva**
   - **Frente:** MPI (Memória Distribuída)
   - **Atribuições:** Decomposição de domínio baseada em partição balanceada da carga ($N/P$), geradores pseudoaleatórios privados e independentes por processo com sementes exclusivas, agregação através da operação coletiva `MPI_Reduce`, análise de overhead de comunicação e escalabilidade.
