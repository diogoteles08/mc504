#! /bin/sh

# Verificar que a saída de “padmon -v -t $pid” é a mesma da de “padmon -vt $pid” e da de “padmon -t $pid -v”.
sleep 120 & export pid=$! # Begins a proccess and saves its PID on $pid
call1=$(padmon -v -e $pid)

sleep 120 & export pid=$! # Begins a proccess and saves its PID on $pid
call2=$(padmon -ve $pid)

sleep 120 & export pid=$! # Begins a proccess and saves its PID on $pid
call2=$(padmon -e $pid -v)
if [ $call1 != $call2 || $call2 != $call3 ]; then
	echo "Diferentes posicoes do \"-v\" no comando tras diferentes resultados"
	exit 1
fi
