LOG_FILE="logs/compilador.log"
PAGE_SIZE=4
MEM_SIZE=1024
ALGO="lru"
OUTPUT_CSV="tabela_metrics.csv"

if [ ! -f "./bin/simulador" ]; then
    exit 1
fi

if [ ! -f "$LOG_FILE" ]; then
    echo "Erro: $LOG_FILE não encontrado."
    exit 1
fi

echo "Configuração: Log=$LOG_FILE, Pag=${PAGE_SIZE}KB, Mem=${MEM_SIZE}KB, Algo=$ALGO"
echo "----------------------------------------------------------------"

echo "Tabela,MemoriaKB,TempoUsuarioSeg" > $OUTPUT_CSV

for tabela in densa h2 h3 invertida; do
    echo -n "Executando para tabela '$tabela'... "

    /usr/bin/time -v -o temp_time.txt ./bin/simulador $ALGO $LOG_FILE $PAGE_SIZE $MEM_SIZE $tabela > /dev/null 2>&1

    MEM_KB=$(grep "Maximum resident set size" temp_time.txt | awk -F': ' '{print $2}')

    TIME_USR=$(grep "User time (seconds)" temp_time.txt | awk -F': ' '{print $2}')

    echo "$tabela,$MEM_KB,$TIME_USR" >> $OUTPUT_CSV
    echo "OK (Mem: ${MEM_KB}KB, Tempo: ${TIME_USR}s)"
done

rm temp_time.txt