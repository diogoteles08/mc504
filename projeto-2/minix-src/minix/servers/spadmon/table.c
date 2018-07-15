/* This file contains the table used to map system call numbers onto the
 * routines that perform them.
 */

#define _TABLE

#include "inc.h"

#define CALL(n)	[((n) - SPADMON_BASE)]

int (* const call_vec[NR_SPADMON_CALLS])(void) = {
	CALL(SPADMON_PS)		= do_spadmon_ps,
	CALL(SPADMON_R)		= do_spadmon_r,
	CALL(SPADMON_S)		= do_spadmon_s,
	CALL(SPADMON_T)		= do_spadmon_t,
	CALL(SPADMON_Z)		= do_spadmon_z,
	CALL(SPADMON_E)		= do_spadmon_e,
};
