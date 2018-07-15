#ifndef _DS_PROTO_H
#define _DS_PROTO_H

EXTERN message m_spad;		/* the incoming message itself is kept here. */

/* Function prototypes. */

/* table.c */
extern int (* const call_vec[])(void);

/* main.c */
int main(int argc, char **argv);

/* padmon.c */
int do_spadmon_ps(void);
int do_spadmon_r(void);
int do_spadmon_s(void);
int do_spadmon_t(void);
int do_spadmon_z(void);
int do_spadmon_e(void);

#endif
