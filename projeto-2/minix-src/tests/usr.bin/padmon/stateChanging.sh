#! /bin/sh

# Pegar o pid de algum processo, executar “padmon -z $pid”(provavelmente o estado inicial do processo nao era zombie), executar novamente “padmon -ps” e verificar se é indicado que o processo está no modo zombie. Executar isso para os outros estados (r, s, t), deixando o argumento -e por último, verificando dps se o pid deixou a lista de processos.
sleep 120 & export pid=$! # Begins a proccess and saves its PID on var

#### Para -t
padmon -t $pid
text="$(padmon -ps | grep $pid)" # Seleciona  informação do processo
t="$(echo $text | sed 's/[[:blank:]]*$//')" # Remove os espaços
status="$(echo -n $t | tail -c 1)" # Seleciona o ultimo caracter
if [ $status != "T" ];then
	echo "Mudanda para estado \"T\" nao reconhecida"
	exit 1
fi

#### Para -r
padmon -r $pid
text="$(padmon -ps | grep $pid)" # Seleciona  informação do processo
t="$(echo $text | sed 's/[[:blank:]]*$//')" # Remove os espaços
status="$(echo -n $t | tail -c 1)" # Seleciona o ultimo caracter
if [ $status != "R" ];then
	echo "Mudanda para estado \"R\" nao reconhecida"
	exit 1
fi

#### Para -s
padmon -s $pid
text="$(padmon -ps | grep $pid)" # Seleciona  informação do processo
t="$(echo $text | sed 's/[[:blank:]]*$//')" # Remove os espaços
status="$(echo -n $t | tail -c 1)" # Seleciona o ultimo caracter
if [ $status != "S" ];then
	echo "Mudanda para estado \"S\" nao reconhecida"
	exit 1
fi

#### Para -z
padmon -z $pid
text="$(padmon -ps | grep $pid)" # Seleciona  informação do processo
t="$(echo $text | sed 's/[[:blank:]]*$//')" # Remove os espaços
status="$(echo -n $t | tail -c 1)" # Seleciona o ultimo caracter
if [ $status != "Z" ];then
	echo "Mudanda para estado \"Z\" nao reconhecida"
	exit 1
fi
