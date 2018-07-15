#include "syslib.h"

int sys_spadmon_run_swap(int pos, int flag)
{
	message m;

	m.m1_i1 = pos; // Posicao no vetor
	m.m1_i2 = flag; // Flag a ser setada

	return(_kernel_call(SYS_SPADMON_RUN_SWAP, &m));
}
