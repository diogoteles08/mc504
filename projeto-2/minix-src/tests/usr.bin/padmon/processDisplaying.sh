#! /bin/sh

# Usar “padmon -ps”, contar o número de processos, algum processo, executar “padmon -ps” novamente e verificar se o número de processos foi incrementado
NUM_PROCESS=$(padmon -ps | wc -l) # NUM_PROCESS recebe o número de linhas printadas em padmon -ps

sleep 60 &

NEW_NUM_PROCESS=$(padmon -ps | wc -l)

if [ $NEW_NUM_PROCESS -ne $(($NUM_PROCESS+1)) ]; then
	echo "\"padmon -ps\" nao mostrou o novo processo"
	exit 1
fi

