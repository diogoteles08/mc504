#! /bin/sh

# Verificar que a saída de “padmon -zv $pid” é maior que a saída de “padmon -z $pid”, isso para todos os outros estados, e dps para  "padmon -e $pid".

## Para -r
sleep 120 & export pid=$! # Inicia um processo e guarda seu PID
nchar_sem_verb=$(padmon -r $pid | wc -c)
padmon -e $pid # Mata o processo em definitivo

sleep 120 & export pid=$! # Inicia um processo e guarda seu PID
nchar_com_verb=$(padmon -vr $pid | wc -c)
padmon -e $pid

if [ $nchar_com_verb -eq $nchar_sem_verb ];then
	echo "Nao houve diferenca com \"-v\""
	exit 1
fi

## Para -s
sleep 120 & export pid=$! # Inicia um processo e guarda seu PID
nchar_sem_verb=$(padmon -s $pid | wc -c)
padmon -e $pid # Mata o processo em definitivo

sleep 120 & export pid=$! # Inicia um processo e guarda seu PID
nchar_com_verb=$(padmon -vs $pid | wc -c)
padmon -e $pid

if [ $nchar_com_verb -eq $nchar_sem_verb ];then
	echo "Nao houve diferenca com \"-v\""
	exit 1
fi

## Para -t
sleep 120 & export pid=$! # Inicia um processo e guarda seu PID
nchar_sem_verb=$(padmon -t $pid | wc -c)
padmon -e $pid # Mata o processo em definitivo

sleep 120 & export pid=$! # Inicia um processo e guarda seu PID
nchar_com_verb=$(padmon -vt $pid | wc -c)
padmon -e $pid

if [ $nchar_com_verb -eq $nchar_sem_verb ];then
	echo "Nao houve diferenca com \"-v\""
	exit 1
fi

## Para -z
sleep 120 & export pid=$! # Inicia um processo e guarda seu PID
nchar_sem_verb=$(padmon -z $pid | wc -c)
padmon -e $pid # Mata o processo em definitivo

sleep 120 & export pid=$! # Inicia um processo e guarda seu PID
nchar_com_verb=$(padmon -vz $pid | wc -c)
padmon -e $pid

if [ $nchar_com_verb -eq $nchar_sem_verb ];then
	echo "Nao houve diferenca com \"-v\""
	exit 1
fi

## Para -e
sleep 120 & export pid=$! # Inicia um processo e guarda seu PID
nchar_sem_verb=$(padmon -e $pid | wc -c)
padmon -e $pid # Mata o processo em definitivo

sleep 120 & export pid=$! # Inicia um processo e guarda seu PID
nchar_com_verb=$(padmon -ve $pid | wc -c)
padmon -e $pid

if [ $nchar_com_verb -eq $nchar_sem_verb ];then
	echo "Nao houve diferenca com \"-v\""
	exit 1
fi
