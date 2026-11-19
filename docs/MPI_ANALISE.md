# Analise Tecnica da Paralelizacao em Memoria Distribuida (MPI)

**Responsavel:** Joao Lucas Gomes da Silva  
**Disciplina:** Programacao Paralela (CCO085) - IESB  
**Problema:** Estimativa de Pi por Monte Carlo  

---

## 1. Fundamentos da Arquitetura de Memoria Distribuida

Diferente do OpenMP, onde todas as threads acessam o mesmo barramento e espaco de enderecamento de memoria RAM, o modelo MPI (*Message Passing Interface*) baseia-se em **processos completamente isolados**.

Cada processo possui:
- Seu proprio espaco de enderecamento de memoria privada virtual.
- Sua propria tabela de descritores de arquivos e ponteiros.
- Identificador exclusivo dentro do comunicador global (`rank`), variando de $0$ ate $P - 1$, onde $P$ e o numero total de processos (`size`).

Nenhum processo pode ler ou gravar diretamente na memoria de outro processo. Toda troca de informacoes e sincronizacao deve ocorrer explicitamente atraves de passagem de mensagens via rede ou mecanismos de IPC (*Inter-Process Communication*), gerenciados pelo daemon do MPI (`mpirun`/`mpiexec`).

---

## 2. Estrategia de Decomposicao de Dominio

O problema de Monte Carlo e classificado na literatura de computacao de alto desempenho como *embarrassingly parallel* (paralelismo embaraçoso ou quase perfeito). Isso ocorre porque a amostragem de cada ponto $(x_i, y_i)$ e estritamente independente de qualquer outro ponto.

### Particionamento de Carga (Data Parallelism):
Dado um total de $N$ amostras a avaliar e $P$ processos disponiveis, a carga e particionada homogeneamente:

$$N_{\text{local}} = \left\lfloor \frac{N}{P} \right\rfloor$$

Para garantir a **corretude estrita** e evitar a perda de pontos por divisao inteira, o processo de maior rank ($rank = P - 1$) absorve o resto da divisao:

```cpp
long long N_local = N / size;
if (rank == size - 1) {
    N_local += N % size; // Garante que a soma total seja exatamente N
}
```

Isso garante que para $N = 50.000.000$ e $P = 4$, os ranks 0, 1 e 2 executem exatamente 12.500.000 iteracoes e o rank 3 execute 12.500.000 iteracoes, mantendo balanceamento de carga de 100%.

---

## 3. Isolamento e Independencia Estatistica dos Processos

Em memoria distribuida, como cada processo e independente, a utilizacao de geradores pseudoaleatorios globais e naturalmente evitada. Contudo, se todos os processos utilizassem a mesma semente estatica (`seed = 42`), todos eles gerariam a **mesma sequencia identica de pontos**, invalidando a amostragem estatistica e calculando a mesma porcao do espaco repetidamente.

### Estrategia de Sementes por Rank:
Para assegurar trajetorias estatisticas independentes e nao correlacionadas:
```cpp
std::mt19937 gen(42 + rank); // Semente parametrizada pelo identificador do processo
std::uniform_real_distribution<double> dis(0.0, 1.0);
```
Dessa forma, os 4 processos cobrem regioes estatisticas distintas do espaco de probabilidades no quadrado unitario, convergindo rigorosamente para o gabarito de $\pi$.

---

## 4. Analise de Comunicacao: Coletiva vs Ponto a Ponto

Ao final da fase de calculo local, cada rank possui uma variavel `pontos_locais` acumulada. E necessario consolidar essas contagens parciais no processo mestre (rank 0) para o calculo da razao global de acertos.

### 4.1 Abordagem Ponto a Ponto (`MPI_Send` / `MPI_Recv`):
Na abordagem ingênua, cada processo trabalhador ($rank > 0$) envia sua contagem via `MPI_Send` e o rank 0 recebe uma a uma via `MPI_Recv`:
- **Complexidade de tempo:** $O(P)$ mensagens recebidas sequencialmente no rank 0.
- **Gargalo:** O rank 0 se torna um ponto central de congestionamento (gargalo de barramento/rede).

### 4.2 Abordagem Coletiva com `MPI_Reduce` (Adotada no Projeto):
Utilizamos a operacao coletiva:
```cpp
MPI_Reduce(&pontos_locais, &pontos_globais, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
```
- **Topologia de arvore binaria:** A biblioteca OpenMPI implementa o `MPI_Reduce` utilizando uma arvore de reducao balanceada.
- **Complexidade de tempo:** $O(\log P)$ etapas de comunicacao.

Diagrama do fluxo de comunicacao em arvore para 4 processos:

```text
Etapa 0 (Calculo local paralelo):
  [Rank 0]         [Rank 1]         [Rank 2]         [Rank 3]
 (12.5M pts)      (12.5M pts)      (12.5M pts)      (12.5M pts)

Etapa 1 (Primeiro nivel de reducao em arvore - 2 pares paralelos):
  [Rank 0] <==== [Rank 1]           [Rank 2] <==== [Rank 3]
  (Soma R0+R1)                     (Soma R2+R3)

Etapa 2 (Nivel final de consolidacao no mestre):
  [Rank 0] <===================== [Rank 2]
  (Total Global = R0+R1+R2+R3)
```

- Para $P = 4$ processos, sao necessarios apenas $\log_2(4) = 2$ passos de troca de mensagens.
- O volume de dados transmitido por processo e infinitesimal (apenas um unico inteiro de 8 bytes, `MPI_LONG_LONG`). Portanto, o tempo de comunicacao e da ordem de microssegundos, praticamente desprezivel frente ao tempo de calculo ($~0.19$ s).

---

## 5. Comparativo de Desempenho: MPI vs OpenMP

Ao confrontarmos os resultados medidos no ambiente do laboratorio com $N = 50.000.000$:

| Metrica | Baseline Sequencial | OpenMP (4 threads) | MPI (4 processos) |
| :--- | :--- | :--- | :--- |
| **Tempo de Execucao** | 0.427 s | **0.126 s** | 0.196 s |
| **Speedup** | 1.00x | **3.49x** | 2.18x |
| **Eficiencia** | 100.0% | **87.2%** | 54.5% |

### Por que o OpenMP superou o MPI nesta medicao local?
1. **Natureza dos Processos do Sistema Operacional:** No MPI, cada unidade de execucao e um processo independente do Linux com espaco de enderecamento proprio, tabela de paginas e controle de memoria isolado. A instanciacao desses processos via `mpirun` e muito mais custosa que a criacao de threads leves no OpenMP.
2. **Custo de Inicializacao (`MPI_Init`):** A inicializacao do ambiente MPI realiza conexao de sockets locais, alocacao de estruturas de roteamento e comunicadores internos, introduzindo overhead de boot.
3. **Comunicacao Inter-Processos (IPC):** Mesmo rodando em uma unica maquina fisica, o MPI realiza comunicacao via memoria compartilhada intermediada por canais IPC do SO, enquanto o OpenMP acessa variaveis privadas alocadas diretamente em registradores e pilha das threads.

### Vantagem Incomparavel do MPI:
Embora o OpenMP apresente menor overhead em uma unica maquina multicore (escalabilidade vertical), o MPI e **a unica tecnologia capaz de escalar horizontalmente atraves de multiplos nos e servidores em cluster**. Em um supercomputador com milhares de nos interconectados por InfiniBand, apenas o MPI permite paralelismo em larga escala.

---

## 6. Lei de Amdahl aplicada ao MPI

Com $p = 4$ processos e speedup medido de $S = 2.18$:

$$2.18 = \frac{1}{f + \frac{1 - f}{4}} \implies f \approx 0.278 \quad (27.8\%)$$

A fracao "serial efetiva" de $27.8\%$ no MPI nao significa que 27% do codigo e sequencial, mas sim que o tempo de inicializacao dos processos pelo sistema operacional e as barreiras de sincronizacao atuam como penalidade constante independente do tamanho do laço.
