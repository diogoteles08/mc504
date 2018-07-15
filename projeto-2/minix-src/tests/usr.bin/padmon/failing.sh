#! /bin/sh
### Tests for asserting padmon fails

feedback=$""    # Respostas de erro

######### Settar um processo de PID $pid para estado X, executar "padmon -x $pid" e esperar um erro
### Estado S
sleep 120 & export pid=$! # Começa um processo e guarda se PID
padmon -s "$pid"
text="$(padmon -ps | grep $pid)" # Seleciona  informação do processo
t="$(echo $text | sed 's/[[:blank:]]*$//')" # Remove os espaços
#status="$(echo "${t: -1}")" # Seleciona o ultimo caracter
status="$(echo -n $t | tail -c 1)" # Seleciona o ultimo caracter

#Converte para minúsculo
status=$(echo "$status" | tr '[:upper:]' '[:lower:]')

padmon -"$status $pid"
if [ $? -eq 0 ]; then
	 feedback="$feedback Padmon settando o mesmo estado e nao teve erro -> S"
fi

### Estado R
sleep 120 & export pid=$! # Começa um processo e guarda se PID
padmon -r "$pid"
text="$(padmon -ps | grep $pid)" # Seleciona  informação do processo
t="$(echo $text | sed 's/[[:blank:]]*$//')" # Remove os espaços
#status="$(echo "${t: -1}")" # Seleciona o ultimo caracter
status="$(echo -n $t | tail -c 1)" # Seleciona o ultimo caracter

#Converte para minúsculo
status=$(echo "$status" | tr '[:upper:]' '[:lower:]')

padmon -"$status $pid"
if [ $? -eq 0 ]; then
    if [ "$feedback" == "" ]; then
	    feedback="Padmon settando o mesmo estado e nao teve erro -> R"
	else
	    feedback="$feedback\nPadmon settando o mesmo estado e nao teve erro -> R"
	fi
fi

### Estado T
sleep 120 & export pid=$! # Começa um processo e guarda se PID
padmon -t "$pid"
text="$(padmon -ps | grep $pid)" # Seleciona  informação do processo
t="$(echo $text | sed 's/[[:blank:]]*$//')" # Remove os espaços
#status="$(echo "${t: -1}")" # Seleciona o ultimo caracter
status="$(echo -n $t | tail -c 1)" # Seleciona o ultimo caracter

#Converte para minúsculo
status=$(echo "$status" | tr '[:upper:]' '[:lower:]')

padmon -"$status $pid"
if [ $? -eq 0 ]; then
	if [ "$feedback" == "" ]; then
	    feedback="Padmon settando o mesmo estado e nao teve erro -> T"
	else
	    feedback="$feedback\nPadmon settando o mesmo estado e nao teve erro -> T"
	fi
fi

### Estado Z
sleep 120 & export pid=$! # Começa um processo e guarda se PID
padmon -z "$pid"
text="$(padmon -ps | grep $pid)" # Seleciona  informação do processo
t="$(echo $text | sed 's/[[:blank:]]*$//')" # Remove os espaços
#status="$(echo "${t: -1}")" # Seleciona o ultimo caracter
status="$(echo -n $t | tail -c 1)" # Seleciona o ultimo caracter

#Converte para minúsculo
status=$(echo "$status" | tr '[:upper:]' '[:lower:]')

padmon -"$status $pid"
if [ $? -eq 0 ]; then
	if [ "$feedback" == "" ]; then
	    feedback="Padmon settando o mesmo estado e nao teve erro -> Z"
	else
	    feedback="$feedback\nPadmon settando o mesmo estado e nao teve erro -> Z"
	fi
fi

######### Executar “padmon $pid” e esperar um erro
sleep 120 & export pid=$! # Começa um processo e guarda se PID

padmon $pid
if [ $? -eq 0 ]; then
    if [ "$feedback" == "" ]; then
	    feedback="Padmon sem nenhum argumento não teve nenhum erro"
	else
	    feedback="$feedback\nPadmon sem nenhum argumento não teve nenhum erro"
	fi
fi


######### Executar “padmon -y $pid” e esperar um erro
sleep 120 & export pid=$! # Começa um processo e guarda se PID

padmon -y $pid
if [ $? -eq 0 ]; then
	if [ "$feedback" == "" ]; then
	    feedback="Padmon com argumento invalido não teve erro"
	else
	    feedback="$feedback\nPadmon com argumento invalido não teve erro"
	fi
fi

if [ "$feedback" != ""]; then
    echo -e $feedback
    exit 1
fi
