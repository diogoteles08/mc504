#include <lib.h> // provides _syscall and message
#include <machine/archtypes.h>
#include <errno.h>
#include "inc.h"
#include "servers/pm/const.h"
#include "servers/pm/mproc.h"
#include "servers/vfs/const.h"
#include "servers/vfs/fproc.h"
#include "kernel/proc.h"

static struct proc proc_cp[NR_TASKS + NR_PROCS];
static struct mproc mproc_cp[NR_PROCS];

int update_tables(void);
int find_pos(int pid);

int do_spadmon_ps(void) {
	if (update_tables()){
		// Error getting process tables
		m_spad.m_spadmon_processes.fun_return = 1;
		return 1;
	}

	// Crete auxiliar arrays for storing PIDs and status
	// for after be passed for user address
	pid_t pids[NR_PROCS];
	char status[NR_PROCS];

	// Iterate over the process tables checking slots of
	// the array in use
	int padmon_array_index = 0;
	for (int i = 0; i < NR_PROCS; i++) {
		if (mproc_cp[i].mp_flags & IN_USE) {
			// Insert PID of found process on auxiliar array
			pids[padmon_array_index] = mproc_cp[i].mp_pid;

			int blocked_by = P_BLOCKEDON(&proc_cp[i+NR_TASKS]);
			// Define states based on tables flags and saves // the state on auxiliar array
			if (mproc_cp[i].mp_flags & (TRACE_ZOMBIE | ZOMBIE)) {
				status[padmon_array_index] = 'Z';
			} else if (mproc_cp[i].mp_flags & (TRACE_STOPPED) || RTS_ISSET(&proc_cp[i+NR_TASKS], RTS_P_STOP)) {
				status[padmon_array_index] = 'T';
			} else if (proc_is_runnable(&proc_cp[i+NR_TASKS])){
				status[padmon_array_index] = 'R';
			} else if (blocked_by != 31744 && blocked_by != 31743 && blocked_by != 0 && blocked_by != 1) {
				status[padmon_array_index] = 'D';
			} else {
				status[padmon_array_index] = 'S';
			}
			padmon_array_index++;
		}
	}
	//
	// Using sys_datacopy to copying data from auxiliar
	// arrays to user address
	sys_datacopy(SELF, (vir_bytes) pids, m_spad.m_source,
			m_spad.m_spadmon_processes.pid_array,
			NR_PROCS * sizeof(pid_t)
	);
	sys_datacopy(SELF, (vir_bytes) status, m_spad.m_source,
			m_spad.m_spadmon_processes.status_array,
			NR_PROCS * sizeof(char)
	);

	m_spad.m_spadmon_processes.length = padmon_array_index;
	m_spad.m_spadmon_processes.fun_return = 0;
	return 0;
}

int do_spadmon_r(void) {
	if (update_tables()){
		// Error getting process tables
		m_spad.m1_i3 = 1;
		return 1;
	}

	int pos = find_pos(m_spad.m1_i1);
	if (pos < 0){
		// PID not found
		m_spad.m1_i3 = 404;
		return 404;
	}

	if (proc_is_runnable(&proc_cp[pos+NR_TASKS])){
		// Process already is runnable
		m_spad.m1_i3 = 2;
		return 2;
	}

	// Calls kernel call for setting process as runnable
	sys_spadmon_run_swap(pos+NR_TASKS, 0);

	// Calls PM syscall for cleaning other state flags
	message m;
	m.m1_i1 = pos;
	m.m1_i2 = 'R';
	_syscall(PM_PROC_NR, PM_SPADMON_SWAP, &m);
	return m.m1_i3;
}

int do_spadmon_s(void) {
	if (update_tables()){
		// Error getting process tables
		m_spad.m1_i3 = 1;
		return 1;
	}

	int pos = find_pos(m_spad.m1_i1);
	if (pos < 0){
		// PID not found
		m_spad.m1_i3 = 404;
		return 404;
	}

	if ((mproc_cp[pos].mp_flags & (WAITING | SIGSUSPENDED)) ||
			(P_BLOCKEDON(&proc_cp[pos+NR_TASKS]) == ANY)){
		// Process already sleeping
		m_spad.m1_i3 = 2;
		return 2;
	}

	// Calls kernel call for setting process as not runnable
	sys_spadmon_run_swap(pos+NR_TASKS, RTS_VMINHIBIT);

	// Calls PM syscall for cleaning other state flags
	// and setting sleeping flag
	message m;
	m.m1_i1 = pos;
	m.m1_i2 = 'S';
	_syscall(PM_PROC_NR, PM_SPADMON_SWAP, &m);
	return m.m1_i3;
}

int do_spadmon_t(void) {
	if (update_tables()){
		// Error getting process tables
		m_spad.m1_i3 = 1;
		return 1;
	}

	int pos = find_pos(m_spad.m1_i1);
	if (pos < 0){
		// PID not found
		m_spad.m1_i3 = 404;
		return 404;
	}

	if (mproc_cp[pos].mp_flags & (TRACE_STOPPED) ||
			RTS_ISSET(&proc_cp[pos+NR_TASKS], RTS_P_STOP)){
		// Process already is stopped
		m_spad.m1_i3 = 2;
		return 2;
	}

	// Calls kernel call for setting process as stopped
	sys_spadmon_run_swap(pos+NR_TASKS, RTS_PROC_STOP);

	// Calls PM syscall for cleaning other state flags
	// and setting stopped flag
	message m;
	m.m1_i1 = pos;
	m.m1_i2 = 'T';
	_syscall(PM_PROC_NR, PM_SPADMON_SWAP, &m);
	return m.m1_i3;
}

int do_spadmon_z(void) {
	if (update_tables()){
		// Error getting process tables
		m_spad.m1_i3 = 1;
		return 1;
	}

	int pos = find_pos(m_spad.m1_i1);
	if (pos < 0){
		// PID not found
		m_spad.m1_i3 = 404;
		return 404;
	}

	if (mproc_cp[pos].mp_flags & (TRACE_ZOMBIE | ZOMBIE)){
		// Process already is a zombie
		m_spad.m1_i3 = 2;
		return 2;
	}

	// Calls kernel call for setting process as non runnable
	sys_spadmon_run_swap(pos+NR_TASKS, RTS_PROC_STOP);

	// Calls PM syscall for cleaning other state flags
	// and setting zombie flags
	message m;
	m.m1_i1 = pos;
	m.m1_i2 = 'Z';
	_syscall(PM_PROC_NR, PM_SPADMON_SWAP, &m);
	return m.m1_i3;
}

int do_spadmon_e(void) {
	if (update_tables()){
		// Error getting process tables
		m_spad.m1_i3 = 1;
		return 1;
	}

	int pos = find_pos(m_spad.m1_i1);
	if (pos < 0){
		// PID not found
		m_spad.m1_i3 = 404;
		return 404;
	}

	// Calls PM syscall for exit process
	message m;
	m.m1_i1 = pos;
	m.m1_i2 = 'E';
	_syscall(PM_PROC_NR, PM_SPADMON_SWAP, &m);
	return m.m1_i3;
}

/**
 *	Iterate over process table searching for process of given
 *	pid
 *
 *  @returns the index of the process if it was found
 *			 -1 if the given PID wasn't found
 */
int find_pos(int pid){
	for (int i = 0; i < NR_PROCS; i++) {
		if(mproc_cp[i].mp_pid == pid && (mproc_cp[i].mp_flags & IN_USE)){
			return i;
		}
	}
	return -1;
}

/**
 *	Updates all three process tables copying the PM, the VFS and the
 *	kernel process tables in its respective static array
 *
 *	@returns 0 for sucess update
 *			 1 if some process table could not be copied
 */
int update_tables(void){
	int r;
	/* Retrieve and check the PM process table. */
	r = getsysinfo(PM_PROC_NR, SI_PROC_TAB, mproc_cp, sizeof(mproc_cp));
	if (r != 0) {
		printf("SPADMON: unable to obtain PM process table (%d)\n", r);
		return 1;
	}

	/* Retrieve and check the KERNEL process table. */
	if ((r = sys_getproctab(proc_cp)) != 0) {
		printf("SPADMON: unable to obtain kernel process table (%d)\n", r);
		return 1;
	}

	return 0;
}
