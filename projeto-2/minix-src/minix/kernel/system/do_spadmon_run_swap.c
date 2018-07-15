#include "kernel/system.h"
#include <minix/endpoint.h>

/*===========================================================================*
 *			          do_spadmon_run_swap			     *
 *===========================================================================*/
int do_spadmon_run_swap(struct proc * caller, message * m_ptr)
{
	// Verifica se posicao eh valida
	if (!isokprocn((m_ptr->m1_i1)-NR_TASKS)) {
		// Process not found
		m_ptr->m1_i3 = 404;
		return m_ptr->m1_i3;
	}

	// Seta as flags passadas em m1_i2
	proc[m_ptr->m1_i1].p_rts_flags = m_ptr->m1_i2;
	m_ptr->m1_i3 = 0;
	return m_ptr->m1_i3;
}
