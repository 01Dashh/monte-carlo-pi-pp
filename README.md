# Projeto: Estimativa de $\pi$ por Monte Carlo em Paralelo

**Disciplina:** Programação Paralela (CCO085) — IESB 2026/2  
**Professor:** Rodrigo Gonçalves Pinto  
**Problema 2:** Estimativa de $\pi$ por Monte Carlo (Paralelismo quase perfeito)

---

## 👥 Integrantes e Frentes

- **Lucas Gonçalves Rocha** — Baseline e Perfilamento
- **Lucas Faria Ribeiro** — OpenMP (Memória Compartilhada)
- **João Lucas Gomes Da Silva** — MPI (Memória Distribuída)

---

## 📁 Estrutura do Repositório

```text
├── codigos/
│   ├── 00_base/
│   │   └── baseline_pi.cpp        # Algoritmo sequencial de referência
│   ├── 01_openmp/
│   │   └── 02_pi_openmp.cpp       # Paralelização OpenMP com reduction
│   └── 02_mpi/
│       └── 03_coletivas_pi.cpp    # Paralelização MPI com MPI_Reduce
├── scripts/
│   └── medir_speedup.sh           # Script de coleta automatizada
├── resultados/                    # Arquivos CSV e gráficos gerados
├── AUTORES.md                     # Atribuição formal de responsabilidades
├── Makefile                       # Regras de compilação automatizada
└── README.md                      # Instruções de execução e replicação
```

---

## 🚀 Como Compilar e Rodar do Zero

Dentro do contêiner Docker/Dev Container da disciplina:

### 1. Compilação
```bash
make clean
make
```
O comando compila todas as três versões com otimização `-O3`.

### 2. Execuções de Demonstração (N = 50.000.000)

#### Passo 1 — Baseline Sequencial (Lucas G.)
```bash
./codigos/00_base/baseline_pi 50000000
```

#### Passo 2 — OpenMP (Lucas F.)
```bash
./codigos/01_openmp/02_pi_openmp 50000000 4
```

#### Passo 3 — MPI (João Lucas)
```bash
mpirun -np 4 ./codigos/02_mpi/03_coletivas_pi 50000000
```

---

## 🔬 Metodologia e Decisões Técnicas

1. **Hotspot e Amdahl:**
   - O laço de sorteio de coordenadas $(x, y)$ e teste $x^2 + y^2 \le 1$ representa mais de 99,8% do tempo de execução sequencial ($O(N)$), permitindo alto teto teórico de speedup.
2. **Independência do PRNG:**
   - Para evitar contenção de cache e garantir validade estatística, cada thread (no OpenMP) e cada processo (no MPI) possui sua própria instância de `std::mt19937` inicializada com sementes exclusivas (`seed + id`).
3. **Redução sem Race Condition:**
   - No OpenMP, a cláusula `reduction(+:pontos_no_circulo)` aloca contadores em registradores/pilha privada por thread, eliminando condição de corrida e evitando *false sharing*.
   - No MPI, a agregação global é realizada ao final via `MPI_Reduce` para o rank 0, com overhead de comunicação mínimo de complexidade $O(\log P)$.
