# Verifica se -help realmente retorna alguma coisa

sleep 120 & export pid=$!
num=0
num=$(padmon -help | wc -c)
if [ $num -eq 0 ]; then
	echo "\"padmon -help\" nao retornou ajuda"
	exit 1
fi
