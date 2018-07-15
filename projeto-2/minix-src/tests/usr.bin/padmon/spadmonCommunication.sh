#! /bin/sh

# Verificar se padmon está chamando a syscall do spadmon
SPADMON_TRACE=$(strace padmon -ps 2>&1 | grep spadmon)
if [ -z $SPADMON_TRACE ]; then
	echo "Padmon não está chamando spadmon"
	exit 1
fi
