#include <lib.h> // provides _syscall and message
#include <stdio.h>
#include <stdlib.h>
/* para getopt() */
#include <unistd.h>
#include <string.h>
#include <errno.h>

pid_t pids[30000];
char status[30000];

#define ERRO -1

void show_help(char *name, int retorno);

/* Mostra a ajuda */
void show_help(char *name, int retorno) {
    FILE *f = stdout;
    if (retorno == ERRO) f = stderr;
    fprintf(f, "\
            [uso] %s <opcoes>\n\
            -ps                mostra o estado de todos os processos.\n\
            -r NUM_PROCESSO    muda o processo de seu estado atual para executavel.\n\
            -s NUM_PROCESSO    muda o processo de seu estado atual para dormindo.\n\
            -t NUM_PROCESSO    muda o processo de seu estado atual para detido.\n\
            -z NUM_PROCESSO    muda o processo de seu estado atual para zumbi.\n\
            -e NUM_PROCESSO    encerra o processo.\n\
            -v                 habilita verbosidade.\n", name) ;
    if (retorno == ERRO) exit(ERRO);
}

int main(int argc, char **argv) {
    int opt ;
    int verbose = 0;
	message m;
  int flagerro = 1;

    int index;

    errno = 0;

    /* Chama ajuda. */
    if ( argc < 2 ) show_help(argv[0], ERRO) ;

    for (index = 1; index < argc; index++){
        if(strcmp(argv[index], "-ps") == 0){
            strcpy(argv[index], "-p");
            flagerro = 0;
        }
        else if(strcmp(argv[index], "-help") == 0){
            strcpy(argv[index], "-h");
            flagerro = 0;
        }
    }

	// Verifies if V flag was passed and sets verbose if so
    for (index = 1; index < argc; index++){
        if(argv[index][2] == 'v'){
          verbose = 1;
          argv[index][2] = '\0';
        }
        else if(argv[index][1] == 'v'){
          verbose = 1;
          if (argv[index][2] != '\0'){
            argv[index][1] = argv[index][2];
            argv[index][2] = '\0';
          }
          else
            argv[index][0] = '\0';
        }
    }
/* getopt() ira retornar uma opcao a cada
     * iteracao e -1 quando todas as opcoes foram
     * conferidas. */
    while( (opt = getopt(argc, argv, "phr:s:t:z:e:")) != -1 ) {
        switch ( opt ) {
            case 'h':
                if (flagerro){
                  fprintf(stderr, "Opcao inválida ou faltando argumento: `%c'\n", optopt) ;
                  return 400;
                }
                show_help(argv[0], 0);
                break;
            case 'p':
				// Pass pointer to an array with length equal to PM NR_PROCS, the
				// max number of processes
                if (flagerro){
                  fprintf(stderr, "Opcao inválida ou faltando argumento: `%c'\n", optopt) ;
                  return 400;
                }

				m.m_spadmon_processes.pid_array = (vir_bytes) pids;
				m.m_spadmon_processes.status_array = (vir_bytes) status;
				// Calls spadmon PS, interprets its return and prints meaning
				// for user
				_syscall(SPADMON_PROC_NR, SPADMON_PS, &m);
				switch (m.m_spadmon_processes.fun_return) {
				case 0:
					if (verbose) {
						printf("Número de processos encontrados: %d\n", m.m_spadmon_processes.length);
					}
					break;
				case 1:
					fprintf(stderr, "Erro: Não foi possível obter a tabela de processos\n");
					return 1;
				default:
					fprintf(stderr, "Erro desconhecido");
					return 500;
				}

				// Pretty print processes
				for (int i = 0; i < 21; i++) printf("-"); printf("\n");
				printf(" PID/EndPoint\tState\n");
				for (int i = 0; i < 21; i++) printf("-"); printf("\n");
				for (int i = 0; i < m.m_spadmon_processes.length; i++)
					printf(" %03d\t\t%c\n", pids[i], status[i]);
				for (int i = 0; i < 21; i++) printf("-"); printf("\n");

                break;
            case 'r':
				// Puts PID on message
                m.m1_i1 = atoi(optarg);
				// Calls spadmon R, interprets its return and prints meaning
				// for user
				_syscall(SPADMON_PROC_NR, SPADMON_R, &m);
				switch (m.m1_i3) {
				case 0:
					if (verbose) {
						printf("Processo de PID %d agora está executável ou sendo executado!\n", m.m1_i1);
					}
					break;
				case 400:
					fprintf(stderr, "Erro: Solicitada alteração para estado inexistente\n");
          break;
				case 404:
					fprintf(stderr, "Erro: Processo de PID %d não encontrado!\n", m.m1_i1);
					return 404;
				case 1:
					fprintf(stderr, "Erro: Não foi possível obter a tabela de processos\n");
					return 1;
				case 2:
					fprintf(stderr, "Erro: Processo já está sendo executado!\n");
					return 2;
				default:
					fprintf(stderr, "Erro desconhecido");
					return 500;
				}
                break;
            case 's':
				// Puts PID on message
                m.m1_i1 = atoi(optarg);
				// Calls spadmon S, interprets its return and prints meaning
				// for user
				_syscall(SPADMON_PROC_NR, SPADMON_S, &m);
				switch (m.m1_i3) {
				case 0:
					if (verbose) {
						printf("Processo de PID %d agora está dormindo!\n", m.m1_i1);
					}
					break;
				case 400:
					fprintf(stderr, "Erro: Solicitada alteração para estado inexistente\n");
          break;
				case 404:
					fprintf(stderr, "Erro: Processo de PID %d não encontrado!\n", m.m1_i1);
					return 404;
				case 1:
					fprintf(stderr, "Erro: Não foi possível obter a tabela de processos\n");
					return 1;
				case 2:
					fprintf(stderr, "Erro: Processo já está dormindo!\n");
					return 2;
				default:
					fprintf(stderr, "Erro desconhecido");
					return 500;
				}
                break;
            case 't':
				// Puts PID on message
                m.m1_i1 = atoi(optarg);
				// Calls spadmon T, interprets its return and prints meaning
				// for user
				_syscall(SPADMON_PROC_NR, SPADMON_T, &m);
				switch (m.m1_i3) {
				case 0:
					if (verbose) {
						printf("Processo de PID %d agora está detido!\n", m.m1_i1);
					}
					break;
				case 400:
					fprintf(stderr, "Erro: Solicitada alteração para estado inexistente\n");
          break;
				case 404:
					fprintf(stderr, "Erro: Processo de PID %d não encontrado!\n", m.m1_i1);
					return 404;
				case 1:
					fprintf(stderr, "Erro: Não foi possível obter a tabela de processos\n");
					return 1;
				case 2:
					fprintf(stderr, "Erro: Processo já está detido!\n");
					return 2;
				default:
					fprintf(stderr, "Erro desconhecido");
					return 500;
				}
                break;
            case 'z':
				// Puts PID on message
                m.m1_i1 = atoi(optarg);
				// Calls spadmon Z, interprets its return and prints meaning
				// for user
				_syscall(SPADMON_PROC_NR, SPADMON_Z, &m);
				switch (m.m1_i3) {
				case 0:
					if (verbose) {
						printf("Processo de PID %d agora é um zombie!\n", m.m1_i1);
					}
					break;
				case 400:
					fprintf(stderr, "Erro: Solicitada alteração para estado inexistente\n");
          break;
				case 404:
					fprintf(stderr, "Erro: Processo de PID %d não encontrado!\n", m.m1_i1);
					return 404;
				case 1:
					fprintf(stderr, "Erro: Não foi possível obter a tabela de processos\n");
					return 1;
				case 2:
					fprintf(stderr, "Erro: Processo já está no estado zombie!\n");
					return 2;
				default:
					fprintf(stderr, "Erro desconhecido");
					return 500;
				}
                break;
            case 'e':
				// Puts PID on message
                m.m1_i1 = atoi(optarg);
				// Calls spadmon E, interprets its return and prints meaning
				// for user
				_syscall(SPADMON_PROC_NR, SPADMON_E, &m);
				switch (m.m1_i3) {
				case 0:
					if (verbose) {
						printf("Processo de PID %d foi finalizado!\n", m.m1_i1);
					}
					break;
				case 400:
					fprintf(stderr, "Erro: Solicitada alteração para estado inexistente\n");
          break;
				case 404:
					fprintf(stderr, "Erro: Processo de PID %d não encontrado!\n", m.m1_i1);
					return 404;
				case 1:
					fprintf(stderr, "Erro: Não foi possível obter a tabela de processos\n");
					return 1;
				default:
					fprintf(stderr, "Erro desconhecido");
					return 500;
				}
                break;
            default:
                fprintf(stderr, "Opcao inválida ou faltando argumento: `%c'\n", optopt) ;
                return 400 ;
        }
    }
    return 0 ;
}
