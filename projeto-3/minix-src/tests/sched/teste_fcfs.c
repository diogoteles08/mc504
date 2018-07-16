#include<stdio.h>
#include<unistd.h>
#include <sys/wait.h>

int main(void){
	int pid_filho_1;
	int pid_filho_2;

	pid_filho_1 = fork();

	if (pid_filho_1 == 0) {
		for(long int x = 1000000; x > 0; x --){
			if (x % 100000 == 0)
					printf("Sou o processo A\n");
		}
	}else {
		pid_filho_2 = fork();

		if (pid_filho_2 == 0) {
			for(long int x = 10000; x > 0; x--){
				if(x % 2000 == 0)
					printf("Sou o processo B\n");
			}
		}
	}

	return 0;
}
