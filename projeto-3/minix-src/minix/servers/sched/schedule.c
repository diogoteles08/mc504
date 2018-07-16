/* This file contains the scheduling policy for SCHED
 *
 * The entry points are:
 *   do_noquantum:        Called on behalf of process' that run out of quantum
 *   do_start_scheduling  Request to start scheduling a proc
 *   do_stop_scheduling   Request to stop scheduling a proc
 *   do_nice		  Request to change the nice level on a proc
 *   init_scheduling      Called from main.c to set up/prepare scheduling
 */
#include "sched.h"
#include "schedproc.h"
#include <assert.h>
#include <minix/com.h>
#include <machine/archtypes.h>

#include <stdlib.h>

static unsigned balance_timeout;

// Declara as cabecas das listas ligadas
static child* head_init_children; // Lista dos filhos de INIT (shell processes)
static child* head_parents; // Lista dos filhos do shell (tomados como pais)
static child* head_children; // Lista dos filhos de filhos de shell, postos em FCFS

#define BALANCE_TIMEOUT	5 /* how often to balance queues in seconds */

static int schedule_process(struct schedproc * rmp, unsigned flags);

int is_parent(struct schedproc* rmp);
int is_childd(struct schedproc* rmp);

#define SCHEDULE_CHANGE_PRIO	0x1
#define SCHEDULE_CHANGE_QUANTUM	0x2
#define SCHEDULE_CHANGE_CPU	0x4

#define SCHEDULE_CHANGE_ALL	(	\
		SCHEDULE_CHANGE_PRIO	|	\
		SCHEDULE_CHANGE_QUANTUM	|	\
		SCHEDULE_CHANGE_CPU		\
		)

#define schedule_process_local(p)	\
	schedule_process(p, SCHEDULE_CHANGE_PRIO | SCHEDULE_CHANGE_QUANTUM)
#define schedule_process_migrate(p)	\
	schedule_process(p, SCHEDULE_CHANGE_CPU)

#define CPU_DEAD	-1

#define cpu_is_available(c)	(cpu_proc[c] >= 0)

#define DEFAULT_USER_TIME_SLICE 200

/* if not child of INIT or RS */
#define is_child(p) (((p)->parent != INIT_PROC_NR) && ((p)->parent != RS_PROC_NR))

#define is_init_child(p) ((p)->parent == INIT_PROC_NR)

/* processes created by RS are sysytem processes */
#define is_system_proc(p)	((p)->parent == RS_PROC_NR)

static unsigned cpu_proc[CONFIG_MAX_CPUS];

static void pick_cpu(struct schedproc * proc)
{
#ifdef CONFIG_SMP
	unsigned cpu, c;
	unsigned cpu_load = (unsigned) -1;

	if (machine.processors_count == 1) {
		proc->cpu = machine.bsp_id;
		return;
	}

	/* schedule sysytem processes only on the boot cpu */
	if (is_system_proc(proc)) {
		proc->cpu = machine.bsp_id;
		return;
	}

	/* if no other cpu available, try BSP */
	cpu = machine.bsp_id;
	for (c = 0; c < machine.processors_count; c++) {
		/* skip dead cpus */
		if (!cpu_is_available(c))
			continue;
		if (c != machine.bsp_id && cpu_load > cpu_proc[c]) {
			cpu_load = cpu_proc[c];
			cpu = c;
		}
	}
	proc->cpu = cpu;
	cpu_proc[cpu]++;
#else
	proc->cpu = 0;
#endif
}

/*===========================================================================*
 *				do_noquantum				     *
 *===========================================================================*/

int do_noquantum(message *m_ptr)
{
	register struct schedproc *rmp;
	int rv, proc_nr_n;

	if (sched_isokendpt(m_ptr->m_source, &proc_nr_n) != OK) {
		printf("SCHED: WARNING: got an invalid endpoint in OOQ msg %u.\n",
		m_ptr->m_source);
		return EBADEPT;
	}

	rmp = &schedproc[proc_nr_n];

	printf("Processo sem quantum ID: %d\n", rmp->endpoint);

	if ((rv = schedule_process_local(rmp)) != OK) {
		return rv;
	}
	return OK;
}

/*===========================================================================*
 *				do_stop_scheduling			     *
 *===========================================================================*/
int do_stop_scheduling(message *m_ptr)
{
	register struct schedproc *rmp;
	int proc_nr_n;
	int rv;

	/* check who can send you requests */
	if (!accept_message(m_ptr))
		return EPERM;

	if (sched_isokendpt(m_ptr->m_lsys_sched_scheduling_stop.endpoint,
		    &proc_nr_n) != OK) {
		printf("SCHED: WARNING: got an invalid endpoint in OOQ msg "
		"%d\n", m_ptr->m_lsys_sched_scheduling_stop.endpoint);
		return EBADEPT;
	}

	rmp = &schedproc[proc_nr_n];

	printf("Imprimindo fila\n");
	for (int i = fcfs.inicio; i<fcfs.fim; i++){
		printf("pos: %d endpoint: %d\n", i, fcfs.filhos[i]->endpoint);
	}

	printf("Processo parou ID: %d\n", rmp->endpoint);
#ifdef CONFIG_SMP
	cpu_proc[rmp->cpu]--;
#endif
	rmp->flags = 0; /*&= ~IN_USE;*/

	if (is_child(rmp)) {
		printf("Processo eh filho! ID pai: %d\n", rmp->parent);

		if (fcfs.filhos[fcfs.inicio] != NULL && rmp->endpoint == (fcfs.filhos[fcfs.inicio])->endpoint) {
			printf("Processo filho que terminou era o primeiro da fila! BELEZINHA\n");
		}

		// Diminui tamanho da fila
		fcfs.inicio = (fcfs.inicio + 1) % NR_PROCS;

		// Se a fila nao estava vazia
		if (fcfs.inicio != fcfs.fim){

			printf("Fila nao esta vazia!\n");
			 // Coloca o prox processo da fila para executar
			 rmp = fcfs.filhos[fcfs.inicio];
			 printf("Prox processo FCFS ID: %d. Posicao: %d\n", rmp->endpoint, fcfs.inicio);

			 /* Schedule the process, giving it some quantum */
			 pick_cpu(rmp);
			 while ((rv = schedule_process(rmp, SCHEDULE_CHANGE_ALL)) == EBADCPU) {
				 /* don't try this CPU ever again */
				 cpu_proc[rmp->cpu] = CPU_DEAD;
				 pick_cpu(rmp);
			 }
			 if (rv != OK) {
				 printf("Sched: Error while scheduling process, kernel replied %d\n",
					 rv);
				 return rv;
			 }

			// mark ourselves as the new scheduler.
			// by default, processes are scheduled by the parents scheduler. in case
				// this scheduler would want to delegate scheduling to another
				// scheduler, it could do so and then write the endpoint of that
				// scheduler into the "scheduler" field.
			m_ptr->m_sched_lsys_scheduling_start.scheduler = SCHED_PROC_NR;

			 printf("Processo seguinte escalonado!\n");

			 // Reafirma que tem algum processo rodando
			 fcfs.rodando = 1;
		 } else {
			 // Nao tem nenhum processo rodando
			 printf("Nao tinha nenhum processo na fila\n");
			 fcfs.rodando = 0;
		 }
	} else {
		printf("Processo que terminou nao nos interessa!\n");
	}

	return OK;
}

/*===========================================================================*
 *				do_start_scheduling			     *
 *===========================================================================*/
int do_start_scheduling(message *m_ptr)
{
	register struct schedproc *rmp;
	int rv, proc_nr_n, parent_nr_n;

	/* we can handle two kinds of messages here */
	assert(m_ptr->m_type == SCHEDULING_START ||
		m_ptr->m_type == SCHEDULING_INHERIT);

	/* check who can send you requests */
	if (!accept_message(m_ptr))
		return EPERM;

	/* Resolve endpoint to proc slot. */
	if ((rv = sched_isemtyendpt(m_ptr->m_lsys_sched_scheduling_start.endpoint,
			&proc_nr_n)) != OK) {
		return rv;
	}
	rmp = &schedproc[proc_nr_n];

	/* Populate process slot */
	rmp->endpoint     = m_ptr->m_lsys_sched_scheduling_start.endpoint;
	rmp->parent       = m_ptr->m_lsys_sched_scheduling_start.parent;
	rmp->max_priority = m_ptr->m_lsys_sched_scheduling_start.maxprio;
	if (rmp->max_priority >= NR_SCHED_QUEUES) {
		return EINVAL;
   	}

	printf("Novo processo iniciado, ID: %d\n", rmp->endpoint);

	/* Inherit current priority and time slice from parent. Since there
	 * is currently only one scheduler scheduling the whole system, this
	 * value is local and we assert that the parent endpoint is valid */
	if (rmp->endpoint == rmp->parent) {
		/* We have a special case here for init, which is the first
		   process scheduled, and the parent of itself. */
		rmp->priority   = USER_Q;
		rmp->time_slice = DEFAULT_USER_TIME_SLICE;

		/*
		 * Since kernel never changes the cpu of a process, all are
		 * started on the BSP and the userspace scheduling hasn't
		 * changed that yet either, we can be sure that BSP is the
		 * processor where the processes run now.
		 */
#ifdef CONFIG_SMP
		rmp->cpu = machine.bsp_id;
		/* FIXME set the cpu mask */
#endif
	}

	switch (m_ptr->m_type) {

	case SCHEDULING_START:
		/* We have a special case here for system processes, for which
		 * quanum and priority are set explicitly rather than inherited
		 * from the parent */
		rmp->priority   = rmp->max_priority;
		rmp->time_slice = m_ptr->m_lsys_sched_scheduling_start.quantum;
		break;

	case SCHEDULING_INHERIT:
		/* Inherit current priority and time slice from parent. Since there
		 * is currently only one scheduler scheduling the whole system, this
		 * value is local and we assert that the parent endpoint is valid */
		if ((rv = sched_isokendpt(m_ptr->m_lsys_sched_scheduling_start.parent,
				&parent_nr_n)) != OK)
			return rv;

		rmp->priority = schedproc[parent_nr_n].priority;
		rmp->time_slice = schedproc[parent_nr_n].time_slice;
		break;

	default:
		/* not reachable */
		assert(0);
	}

	/* Take over scheduling the process. The kernel reply message populates
	 * the processes current priority and its time slice */
	if ((rv = sys_schedctl(0, rmp->endpoint, 0, 0, 0)) != OK) {
		printf("Sched: Error taking over scheduling for %d, kernel said %d\n",
			rmp->endpoint, rv);
		return rv;
	}
	rmp->flags = IN_USE;

	if (is_child(rmp)) {
	// Verifica se o processo é filho de um parent, ou seja, filho de um filho de um shell
	// Entao escalonamos esse processo no FCFS
		printf("Processo eh filho, filho de: %d. Posicao na fila: %d\n", rmp->parent, fcfs.fim);
		printf("Prioridade: %d\n", rmp->priority);

		// Coloca novo processo filho na fila fcfs
		fcfs.filhos[fcfs.fim] = rmp;

		// Aumenta tamanho da fila
		fcfs.fim = (fcfs.fim + 1) % NR_PROCS;

		// Se o fcfs nao esta rodando, ja roda o novo processo
		if (fcfs.rodando == 0) {
			fcfs.rodando = 1;

			printf("Processo filho chegou rodando: %d\n", rmp->endpoint);

			// Schedule the process, giving it some quantum
			pick_cpu(rmp);
			while ((rv = schedule_process(rmp, SCHEDULE_CHANGE_ALL)) == EBADCPU) {
				// don't try this CPU ever again
				cpu_proc[rmp->cpu] = CPU_DEAD;
				pick_cpu(rmp);
			}

			if (rv != OK) {
				printf("Sched: Error while scheduling process, kernel replied %d\n",
					rv);
				return rv;
			}

			// mark ourselves as the new scheduler.
			// by default, processes are scheduled by the parents scheduler. in case
			// this scheduler would want to delegate scheduling to another
			// scheduler, it could do so and then write the endpoint of that
			// scheduler into the "scheduler" field.

			m_ptr->m_sched_lsys_scheduling_start.scheduler = SCHED_PROC_NR;
		}
	} else {

		// Salvamos o processo na nossa lista de filhos de INIT, e escalonamos o processo normalmente
		printf("Processo eh filho de INIT\n");

		pick_cpu(rmp);
		while ((rv = schedule_process(rmp, SCHEDULE_CHANGE_ALL)) == EBADCPU) {
			/* don't try this CPU ever again */
			cpu_proc[rmp->cpu] = CPU_DEAD;
			pick_cpu(rmp);
		}

		if (rv != OK) {
			printf("Sched: Error while scheduling process, kernel replied %d\n",
				rv);
			return rv;
		}

		/* Mark ourselves as the new scheduler.
		 * By default, processes are scheduled by the parents scheduler. In case
		 * this scheduler would want to delegate scheduling to another
		 * scheduler, it could do so and then write the endpoint of that
		 * scheduler into the "scheduler" field.
		 */
		m_ptr->m_sched_lsys_scheduling_start.scheduler = SCHED_PROC_NR;

	}

	return OK;
}

/*===========================================================================*
 *				do_nice					     *
 *===========================================================================*/
int do_nice(message *m_ptr)
{
	struct schedproc *rmp;
	int rv;
	int proc_nr_n;
	unsigned new_q, old_q, old_max_q;

	/* check who can send you requests */
	if (!accept_message(m_ptr))
		return EPERM;

	if (sched_isokendpt(m_ptr->m_pm_sched_scheduling_set_nice.endpoint, &proc_nr_n) != OK) {
		printf("SCHED: WARNING: got an invalid endpoint in OoQ msg "
		"%d\n", m_ptr->m_pm_sched_scheduling_set_nice.endpoint);
		return EBADEPT;
	}

	rmp = &schedproc[proc_nr_n];
	new_q = m_ptr->m_pm_sched_scheduling_set_nice.maxprio;
	if (new_q >= NR_SCHED_QUEUES) {
		return EINVAL;
	}

	/* Store old values, in case we need to roll back the changes */
	old_q     = rmp->priority;
	old_max_q = rmp->max_priority;

	/* Update the proc entry and reschedule the process */
	rmp->max_priority = rmp->priority = new_q;

	if ((rv = schedule_process_local(rmp)) != OK) {
		/* Something went wrong when rescheduling the process, roll
		 * back the changes to proc struct */
		rmp->priority     = old_q;
		rmp->max_priority = old_max_q;
	}

	return rv;
}

/*===========================================================================*
 *				schedule_process			     *
 *===========================================================================*/
static int schedule_process(struct schedproc * rmp, unsigned flags)
{
	int err;
	int new_prio, new_quantum, new_cpu, niced;

	printf("imprimindo fila\n");
	for (int i = fcfs.inicio; i<fcfs.fim; i++){
		printf("pos: %d endpoint: %d\n", i, fcfs.filhos[i]->endpoint);
	}

	pick_cpu(rmp);

	if (flags & SCHEDULE_CHANGE_PRIO)
		new_prio = rmp->priority;
	else
		new_prio = -1;

	if (flags & SCHEDULE_CHANGE_QUANTUM)
		new_quantum = rmp->time_slice;
	else
		new_quantum = -1;

	if (flags & SCHEDULE_CHANGE_CPU)
		new_cpu = rmp->cpu;
	else
		new_cpu = -1;

	niced = (rmp->max_priority > USER_Q);

	/* Give high quantum to child */
	//if (is_child(rmp)){
		//new_quantum = UINT_MAX;
		//new_prio = 15;
	//}

		/* Deactivate Multi-priority system */
		new_prio = 14;


	if ((err = sys_schedule(rmp->endpoint, new_prio,
		new_quantum, new_cpu, niced)) != OK) {
		printf("PM: An error occurred when trying to schedule %d: %d\n",
		rmp->endpoint, err);
	}

	return err;
}


/*===========================================================================*
 *				init_scheduling				     *
 *===========================================================================*/
void init_scheduling(void)
{
	int r;

	// Inicializa variaveis da fila
	fcfs.rodando = 0;
	fcfs.inicio = 0;
	fcfs.fim = 0;

	// Constroi e inicializa a cabeca das listas ligadas
	head_init_children = malloc(sizeof(child));
	head_init_children->next = NULL;
	head_parents = malloc(sizeof(child));
	head_parents->next = NULL;
	head_children = malloc(sizeof(child));
	head_children->next = NULL;

	balance_timeout = BALANCE_TIMEOUT * sys_hz();

	if ((r = sys_setalarm(balance_timeout, 0)) != OK)
		panic("sys_setalarm failed: %d", r);
}

/*===========================================================================*
 *				balance_queues				     *
 *===========================================================================*/

/* This function in called every N ticks to rebalance the queues. The current
 * scheduler bumps processes down one priority when ever they run out of
 * quantum. This function will find all proccesses that have been bumped down,
 * and pulls them back up. This default policy will soon be changed.
 */
void balance_queues(void)
{
	struct schedproc *rmp;
	int r, proc_nr;

	for (proc_nr=0, rmp=schedproc; proc_nr < NR_PROCS; proc_nr++, rmp++) {
		if (rmp->flags & IN_USE) {
			if (rmp->priority > rmp->max_priority) {
				rmp->priority -= 1; /* increase priority */
				schedule_process_local(rmp);
			}
		}
	}

	if ((r = sys_setalarm(balance_timeout, 0)) != OK)
		panic("sys_setalarm failed: %d", r);
}

int is_parent(struct schedproc* rmp) {
	printf("Verificando se o processo eh um pai\n");
	child* child_it = head_init_children;

	// Busca o pai de rmp na lista de filhos de INIT
	while (child_it->next != NULL) {
		child_it = child_it->next;
		if (child_it->endpt == rmp->parent) {
			return 1;
		}
	}
	return 0;
}

int is_childd(struct schedproc* rmp) {
	printf("Verificando se o processo eh um filho\n");

	// Busca o pai de rmp na lista de pais, ou seja,
	// filhos de filhos de INIT, ou filhos do shell
	child* child_it = head_parents;
	while (child_it->next != NULL) {
		child_it = child_it->next;
		if (child_it->endpt == rmp->parent) {
			return 1;
		}
	}
	return 0;
}
