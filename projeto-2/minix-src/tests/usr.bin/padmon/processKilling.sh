#! /bin/sh

# Usar "padmon -e <pid>" para encerrar um processo e verificar se ele realmente foi excluído

sleep 120 & export pid=$! # Começa um processo e guarda se PID
NUM_PROCESS=$(ps | grep $pid | wc -l)

padmon -e $pid # kill the process
NEW_NUM_PROCESS=$(ps | grep $pid | wc -l)

if [ $NEW_NUM_PROCESS -eq $NUM_PROCESS ]; then # Verifica se a quantidade de processos diminuiu
	echo "\"padmon -e <pid>\" nao finalizou o processo desejado"
	exit 1
fi
