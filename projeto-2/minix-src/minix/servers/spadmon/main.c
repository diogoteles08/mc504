/* Spadmon server. */

#include "inc.h"	/* include master header file */
#include <minix/endpoint.h>

/* Allocate space for the global variables. */
static endpoint_t who_e;	/* caller's proc number */
static int callnr;		/* system call number */
message m_spad;		/* the incoming message itself is kept here. */

/* Declare some local functions. */
static void get_work(message *m_ptr);
static void reply(endpoint_t whom, message *m_ptr);

/*===========================================================================*
 *				main                                         *
 *===========================================================================*/
int main(int argc, char **argv)
{
/* This is the main routine of this service. The main loop consists of
 * three major activities: getting new work, processing the work, and
 * sending the reply. The loop never terminates, unless a panic occurs.
 */

  int result;
  unsigned int call_index;

  /* Let SEF perform startup. */
  sef_startup();


  /* Main loop - get work and do it, forever. */
  while (TRUE) {

      /* Wait for incoming message, sets 'callnr' and 'who'. */
      get_work(&m_spad);

      call_index = (unsigned int) (callnr - SPADMON_BASE);
      result = (*call_vec[call_index])();

      if (is_notify(callnr)) {
          printf("SPADMON: warning, got illegal notify from: %d\n", m_spad.m_source);
          result = EINVAL;
          goto send_reply;
      }

send_reply:
      /* Finally send reply message, unless disabled. */
      if (result != EDONTREPLY) {
          //m.m_type = result;  		/* build reply message */
	  reply(who_e, &m_spad);		/* send it away */
      }
  }
  return(OK);				/* shouldn't come here */
}


/*===========================================================================*
 *				get_work                                     *
 *===========================================================================*/
static void get_work(
  message *m_ptr			/* message buffer */
)
{
    int status = sef_receive(ANY, m_ptr);   /* blocks until message arrives */
    if (OK != status)
        panic("failed to receive message!: %d", status);
    who_e = m_ptr->m_source;        /* message arrived! set sender */
    callnr = m_ptr->m_type;       /* set function call number */
}

/*===========================================================================*
 *				reply					     *
 *===========================================================================*/
static void reply(
  endpoint_t who_e,			/* destination */
  message *m_ptr			/* message buffer */
)
{
    int s = ipc_send(who_e, m_ptr);    /* send the message */
    if (OK != s)
        printf("SPADMON: unable to send reply to %d: %d\n", who_e, s);
}
