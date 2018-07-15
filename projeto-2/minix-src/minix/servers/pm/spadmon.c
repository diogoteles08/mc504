#include "pm.h"
#include <lib.h>
#include <minix/callnr.h>
#include <minix/endpoint.h>
#include <limits.h>
#include <minix/com.h>
#include <signal.h>
#include <stdio.h>
#include <machine/archtypes.h>
#include "mproc.h"
#include "servers/vfs/const.h"
#include "servers/vfs/fproc.h"
#include "kernel/proc.h"

void clean_flags(int index);

/**
 * Syscall for swapping a process if given PID to a given state.
 *
 * @param	message m1_i1 the index of the process
 *			message m1_i2 char indicating the state to swap to
 *
 * @return  *Return on m_in.m1_i3*
 *          404 if pid couldn't be found on PM process table
 *			400 if an invalid option was passed as state
 *			0 if success
 */
int do_spadmon_swap(void) {
	int proc_index = m_in.m1_i1;
	if (proc_index < 0 || proc_index >= NR_PROCS) {
		// Invalid index
		m_in.m1_i3 = 404;
		return m_in.m1_i3;
	}

	switch (m_in.m1_i2) {
		case 'Z':
			clean_flags(proc_index);
			mproc[proc_index].mp_flags |= TRACE_ZOMBIE;
			break;

		case 'T':
			clean_flags(proc_index);
			mproc[proc_index].mp_flags |= TRACE_STOPPED;
			break;

		case 'S':
			clean_flags(proc_index);
			mproc[proc_index].mp_flags |= WAITING;
			break;

		case 'R':
			clean_flags(proc_index);
			break;

		case 'E':
			clean_flags(proc_index);			
			// Calls a PM function for exiting the given process
			exit_proc(&(mproc[proc_index]), 0, FALSE);
			break;

		default:
			// Swapping state not valid
			m_in.m1_i3 = 400;
			return m_in.m1_i3;
	}
	
	m_in.m1_i3 = 0;
	return m_in.m1_i3;
}

// Cleans all the flags of the mproc flags leaving only the one
// that define the slot of the table used
void clean_flags(int index) {
	mproc[index].mp_flags = IN_USE;
}
