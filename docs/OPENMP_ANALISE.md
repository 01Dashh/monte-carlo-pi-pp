# Analise Tecnica da Paralelizacao em Memoria Compartilhada (OpenMP)

**Responsavel:** Lucas Faria Ribeiro  
**Disciplina:** Programacao Paralela (CCO085) - IESB  
**Problema:** Estimativa de Pi por Monte Carlo  

---

## 1. Visao Geral da Arquitetura OpenMP

O modelo de programacao do OpenMP baseia-se em fork-join em memoria compartilhada. Todas as threads compartilham o mesmo espaco de enderecamento do processo pai, permitindo comunicacao direta atraves da leitura e escrita em variaveis comuns, mas exigindo gerenciamento rigoroso de concorrencia e coerencia de cache.

No calculo de Pi por Monte Carlo, avaliamos $N$ amostras aleatorias $(x, y) \in [0, 1]^2$. Cada ponto que satisfaz $x^2 + y^2 \le 1$ incrementa um contador global de acertos.

---

## 2. Ponto Critico: Condicao de Corrida (Race Condition)

Se multiplas threads executarem a instrucao de incremento sem sincronizacao:
```cpp
// Codigo com Race Condition (Incorreto)
#pragma omp parallel for
for (long long i = 0; i < N; ++i) {
    if (x*x + y*y <= 1.0) {
        pontos_no_circulo++; // Conflito de leitura/modificacao/escrita
    }
}
```
A operacao `pontos_no_circulo++` e decomposta pelo compilador e processador em tres instrucoes de maquina:
1. Ler o valor da memoria para um registrador local.
2. Incrementar o registrador em 1.
3. Escrever o valor atualizado de volta na memoria.

Quando duas ou mais threads executam essa sequencia simultaneamente sobre a mesma palavra de memoria, ocorre sobreposicao de ciclos e perda de incrementos. Como resultado, o valor final de $\pi$ diverge drasticamente do valor real e varia a cada execucao.

---

## 3. Avaliacao das Estrategias de Sincronizacao

Para resolver o conflito, o OpenMP oferece tres abordagens:

| Abordagem | Diretiva | Impacto no Desempenho | Viabilidade |
| :--- | :--- | :--- | :--- |
| **Secao Critica** | `#pragma omp critical` | Destrutivo. Serializa todas as threads com lock global em hardware/SO. | Inviavel no loop interno. |
| **Operacao Atomica** | `#pragma omp atomic` | Ruim. Usa instrucoes bloqueantes do processador (e.g., `LOCK XADD` em x86). | Lento para $N = 50.000.000$. |
| **Reducao** | `reduction(+:variavel)` | Otimo. Cada thread acumula em registrador/pilha privada e soma no final. | **Escolhida (Ideal)**. |

### Justificativa da Escolha da Reducao:
A clausula `reduction(+:pontos_no_circulo)` aloca uma copia privada da variavel para cada thread na memoria local. Durante todo o loop de amostragem ($O(N)$), as threads realizam somas em seus proprios registradores, sem qualquer lock ou sincronizacao. Apenas ao termino do laço paralelo o OpenMP combina os subtotais em uma unica operacao de soma paralela em arvore.

---

## 4. O Fenomeno do False Sharing e sua Mitigacao

False sharing (compartilhamento falso) ocorre quando threads em nucleos diferentes escrevem em variaveis distintas que residem na mesma linha de cache (tipicamente 64 bytes em processadores modernos).

### Como o False Sharing se manifestaria:
Se tivessemos implementado a contagem com um vetor compartilhado indexado pelo ID da thread:
```cpp
long long contadores_parciais[NUM_THREADS]; // Ruim: false sharing severo

#pragma omp parallel
{
    int tid = omp_get_thread_num();
    #pragma omp for
    for (long long i = 0; i < N; ++i) {
        if (dentro) contadores_parciais[tid]++; // Linha de cache invalidada o tempo todo
    }
}
```
Nessa situacao, quando a Thread 0 atualiza `contadores_parciais[0]`, o protocolo de coerencia de cache (MESI/MOESI) invalida a linha inteira de cache L1/L2 nos outros nucleos. A Thread 1, ao tentar escrever em `contadores_parciais[1]`, sofre um cache miss e precisa recarregar a linha da memoria principal, degradando brutalmente a performance.

### Como a nossa solucao mitiga:
A clausula `reduction` gera variaveis alocadas na pilha privada de cada thread ou em registradores de CPU. Com isso, os enderecos de memoria estao distantes entre si no espaco de enderecamento e em linhas de cache separadas, eliminando o false sharing.

---

## 5. Independencia do Gerador Pseudoaleatorio (PRNG)

A funcao padrao `rand()` da biblioteca C (`<cstdlib>`) compartilha uma semente estatica global protegida por mutex interno. Se chamada concorrentemente, ela gera dois problemas criticos:
1. **Contencao de lock:** todas as threads brigam pelo lock do gerador, destruindo o speedup.
2. **Correlacao estatistica:** a sequencia de numeros aleatorios fica corrompida.

### Solucao Adotada:
Instanciamos o gerador Mersenne Twister (`std::mt19937`) localmente no escopo privado de cada thread:
```cpp
#pragma omp parallel num_threads(num_threads) reduction(+:pontos_no_circulo)
{
    int tid = omp_get_thread_num();
    std::mt19937 gen(42 + tid); // Semente unica e independente por thread
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    // ...
}
```
Isso garante independencia estatistica de amostragem entre os fluxos e execucao sem chamadas bloqueantes de sistema.

---

## 6. Estudo Comparativo de Escalonamento (Scheduling)

O OpenMP permite definir a forma como as iteracoes do laço `for` sao distribuidas entre as threads:

1. **`schedule(static)` (Estatico):**
   - As iteracoes sao divididas em blocos iguais de tamanho $\lceil N / P \rceil$ no inicio da execucao.
   - Nao ha fila dinamica nem custo de sincronizacao em tempo de execucao.
   - Ideal para problemas onde o custo computacional de cada iteracao e constante e homogeneo.

2. **`schedule(dynamic, chunk)` (Dinamico):**
   - As threads retiram blocos de iteracoes de uma fila compartilhada sob demanda.
   - Util para problemas desbalanceados (e.g., Fractal de Mandelbrot).
   - Apresenta overhead de sincronizacao da fila.

3. **`schedule(guided, chunk)` (Guiado):**
   - Blocos comecam grandes e diminuem proporcionalmente ao numero de iteracoes restantes.
   - Reduz o overhead da fila mantendo certo balanceamento.

### Conclusao Experimental:
Como o teste do circulo unitario ($x^2 + y^2 \le 1$) possui custo fixo e identico para todo ponto avaliado, `schedule(static)` obteve o menor tempo e o maior speedup (3.49x com 4 threads), dispensando o custo extra de sincronizacao das politicas dinamicas.

---

## 7. Analise de Speedup e Eficiencia

- **Tempo Sequencial de Referencia ($T_1$):** 0.427 s
- **Tempo OpenMP 4 threads ($T_4$):** 0.126 s
- **Speedup Real ($S = T_1 / T_4$):** 3.49x
- **Eficiencia ($E = S / 4$):** 87.2%

Pela Lei de Amdahl:
$$S = \frac{1}{f + \frac{1 - f}{p}}$$

Para $S = 3.49$ e $p = 4$, a fracao serial efetiva $f$ e de apenas $4.9\%$. Essa pequena perda decorre da inicializacao dos geradores, alocacao da equipe de threads e sincronizacao da barreira final.
