#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <sys/wait.h> 
#include <time.h> 
#include <fcntl.h> 
#include <sys/stat.h> 
#include <string.h> 
#include <stddef.h>
#include <signal.h>

//for colors:
#define RED "\033[31;1m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define WHITE "\033[37m"

int status_sigusr1=0, status_sigusr2=0, status_sigterm=0, status_alarm=0;

void handler(int signum) {
	if (signum == SIGUSR1)
    {
        status_sigusr1=1;
    }
    if (signum == SIGUSR2)
    {
        status_sigusr2=1;
    }
    if (signum == SIGTERM)
    {
   		status_sigterm=1;
    }
     if (signum == SIGALRM)
    {
   		status_alarm=1;
    }
}

int main(int argc, char **argv)
{
	
	struct sigaction action; 
	action.sa_handler = handler; 
	sigaction(SIGALRM, &action, NULL);
	sigaction(SIGUSR1, &action, NULL);
	sigaction(SIGUSR2, &action, NULL);
	sigaction(SIGTERM, &action, NULL);
 	
 	int i,seconds=0, father_pid;
 	char state[120]; 
 	time_t start, end;
    int elapsed;

 	i=atoi(argv[2]);
	state[i]=*argv[1];
	
    alarm(5);

    time(&start);  // start the timer 

 	while(1)
 	{ 
 		pause();

 		//for SIGALARM -------------------------------------------------------------------------	
 		if(status_alarm==1)
 		{
	 		alarm(5);		
			time(&end);
			elapsed = difftime(end, start);
			
			if(state[i]== 't')
			{
				printf(GREEN"[ID=%d/PID=%d/TIME=%ds] The gates are open!\n", i, getpid(), elapsed); 
			}
		    else
		    {
				printf(RED"[ID=%d/PID=%d/TIME=%ds] The gates are closed!\n", i, getpid(), elapsed);
		    }
		}

		status_alarm=0;
		//for SIGUSR1 -------------------------------------------------------------------------	
		if(status_sigusr1==1)
		{
			time(&end);
			elapsed = difftime(end, start);
			
			if(state[i]== 't')
			{
				printf(GREEN"[ID=%d/PID=%d/TIME=%ds] state:'%c'!\n", i, getpid(), elapsed, state[i]); 
			}
		    else
		    {
				printf(RED"[ID=%d/PID=%d/TIME=%ds] state: '%c'!\n", i, getpid(), elapsed, state[i]);
		    }
		}	
		
		status_sigusr1=0;
		//-------------------------------------------------------------------------------------

		//for SIGUSR2 -------------------------------------------------------------------------	
		if(status_sigusr2==1)
		{
			time(&end);
			elapsed = difftime(end, start);
			if(state[i]== 't')
			{
				state[i]='f';
				printf(RED"[ID=%d/PID=%d/TIME=%ds] The gates are closed!\n", i, getpid(), elapsed);
			}
			else 
			{
				state[i]='t';
				printf(GREEN"[ID=%d/PID=%d/TIME=%ds] The gates are open!\n", i, getpid(), elapsed);
			}
		}

		status_sigusr2=0;
		//-------------------------------------------------------------------------------------

		//for SIGTERM -------------------------------------------------------------------------	
		if (status_sigterm==1)
		{	
	    	raise(SIGKILL);	
		}

		status_sigterm=0;
		//-------------------------------------------------------------------------------------
	}

    return 0;
}
 