#!/usr/bin/env python3
"""
Gera os graficos de Speedup e Eficiencia do Projeto Academico CCO085
Uso: python3 python/gerar_grafico.py
"""
import os
import pandas as pd
import matplotlib.pyplot as plt

def main():
    csv_path = os.path.join("resultados", "medicoes_experimento.csv")
    out_path = os.path.join("resultados", "grafico_speedup.png")

    if not os.path.exists(csv_path):
        print(f"Arquivo CSV nao encontrado: {csv_path}")
        return

    df = pd.read_csv(csv_path)

    versoes = df["versao"].tolist()
    speedup = df["speedup"].tolist()
    eficiencias = df["eficiencia_pct"].tolist()

    cores = ["#94a3b8", "#14b8a6", "#06b6d4"]

    fig, ax = plt.subplots(figsize=(8, 4.5), dpi=150)
    bars = ax.bar(versoes, speedup, color=cores, width=0.45, edgecolor="none")

    ax.axhline(4.0, color="#f59e0b", linestyle="--", linewidth=1.5, label="Ideal: 4x")

    for bar, spd, ef in zip(bars, speedup, eficiencias):
        yval = bar.get_height()
        ax.text(bar.get_x() + bar.get_width() / 2, yval + 0.1, f"{spd:.2f}x", 
                ha="center", va="bottom", fontsize=11, fontweight="bold", color="#334155")
        ax.text(bar.get_x() + bar.get_width() / 2, -0.4, f"eficiência {ef:.1f}%", 
                ha="center", va="top", fontsize=9, color="#64748b")

    ax.set_ylim(0, 4.5)
    ax.set_ylabel("Speedup (x vezes mais rápido)", fontsize=11)
    ax.set_title("Speedup Medido com N = 50.000.000 (Ideal: 4x)", fontsize=13, pad=15)
    ax.legend(loc="upper right")
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)
    ax.grid(axis="y", linestyle=":", alpha=0.6)

    plt.tight_layout()
    plt.savefig(out_path)
    print(f"Grafico salvo com sucesso em: {out_path}")

if __name__ == "__main__":
    main()
