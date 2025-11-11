import matplotlib.pyplot as plt
import csv
import os

# Configurações
INPUT_CSV = 'tabela_metrics.csv'
OUTPUT_MEM_IMG = 'comparacao_memoria.png'
OUTPUT_TIME_IMG = 'comparacao_tempo.png'

if not os.path.exists(INPUT_CSV):
    print(f"Erro: {INPUT_CSV} não encontrado. Rode o collect_metrics.sh primeiro.")
    exit(1)

# Dados
tabelas = []
memoria_kb = []
tempo_seg = []

# Lê o CSV
with open(INPUT_CSV, 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        tabelas.append(row['Tabela'])
        memoria_kb.append(int(row['MemoriaKB']))
        tempo_seg.append(float(row['TempoUsuarioSeg']))

# Cores para os gráficos
colors = ['#3498db', '#e74c3c', '#2ecc71', '#9b59b6']

# --- Gráfico 1: Consumo de Memória ---
plt.figure(figsize=(8, 6))
bars = plt.bar(tabelas, memoria_kb, color=colors)

# Adiciona os valores em cima das barras
for bar in bars:
    height = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2., height,
             f'{height/1024:.1f} MB',
             ha='center', va='bottom')

plt.title('Pico de Consumo de Memória por Tipo de Tabela')
plt.ylabel('Memória RAM (KB)')
plt.xlabel('Tipo de Tabela de Páginas')
plt.grid(axis='y', linestyle='--', alpha=0.7)

plt.savefig(OUTPUT_MEM_IMG, dpi=300)
print(f"Gráfico salvo: {OUTPUT_MEM_IMG}")
plt.close()

# --- Gráfico 2: Tempo de Execução ---
plt.figure(figsize=(8, 6))
bars = plt.bar(tabelas, tempo_seg, color=colors)

# Adiciona os valores em cima das barras
for bar in bars:
    height = bar.get_height()
    plt.text(bar.get_x() + bar.get_width()/2., height,
             f'{height:.2f} s',
             ha='center', va='bottom')

plt.title('Tempo de Execução (User CPU) por Tipo de Tabela')
plt.ylabel('Tempo (segundos)')
plt.xlabel('Tipo de Tabela de Páginas')
plt.grid(axis='y', linestyle='--', alpha=0.7)

plt.savefig(OUTPUT_TIME_IMG, dpi=300)
print(f"Gráfico salvo: {OUTPUT_TIME_IMG}")
plt.close()