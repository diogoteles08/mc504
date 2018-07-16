/* This table has one slot per process.  It contains scheduling information
 * for each process.
 */
#include <limits.h>

#include <minix/bitmap.h>

/* EXTERN should be extern except in main.c, where we want to keep the struct */
#ifdef _MAIN
#undef EXTERN
#define EXTERN
#endif

#ifndef CONFIG_SMP
#define CONFIG_MAX_CPUS 1
#endif

/**
 * We might later want to add more information to this table, such as the
 * process owner, process group or cpumask.
 */

EXTERN struct schedproc {
	endpoint_t endpoint;	/* process endpoint id */
	endpoint_t parent;	/* parent endpoint id */
	unsigned flags;		/* flag bits */

	/* User space scheduling */
	unsigned max_priority;	/* this process' highest allowed priority */
	unsigned priority;		/* the process' current priority */
	unsigned time_slice;		/* this process's time slice */
	unsigned cpu;		/* what CPU is the process running on */
	bitchunk_t cpu_mask[BITMAP_CHUNKS(CONFIG_MAX_CPUS)]; /* what CPUs is the
								process allowed
								to run on */
} schedproc[NR_PROCS];

EXTERN struct fila {
	int rodando; // se tem algum filho executando
	int inicio; // posicao do vetor com o 1 da fila
	int fim; // posicao do vetor com o ultimo da fila
	struct schedproc* filhos[NR_PROCS]; // tamanho maximo de filhos eh o tamanho total
} fcfs;

typedef struct child {
	endpoint_t endpt; // posicao do vetor com o ultimo da fila
	int is_grandchild; // sinaliza se é filho de filho
	struct child* next; // tamanho maximo de filhos eh o tamanho total
} child;

/* Flag values */
#define IN_USE		0x00001	/* set when 'schedproc' slot in use */
