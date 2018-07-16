#include<stdio.h>
#include<unistd.h>
#include<string.h>

int main(int argc, char *argv[]){
	int pid_filho, enrola = 100;
	if (strcmp(argv[1], "A") == 0)
	    enrola = 1000000;
	    
	for(; enrola > 0; enrola --){
	    if(strcmp(argv[1], "A") == 0){
	        if(enrola % 1000 == 0)
		        printf("%s\n", argv[1]);
	    }
	    else 
	        printf("%s\n", argv[1]);
	}
return 0;
}