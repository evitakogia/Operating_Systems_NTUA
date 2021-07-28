#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <stddef.h>

//for colors:
#define RED "\033[31;1m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define WHITE "\033[37m"


int status_sigusr1=0, status_sigchld=0, status_sigterm=0;

void handler(int signum)
{
    if (signum == SIGUSR1)
    {
        status_sigusr1=1;
    }
    if (signum == SIGTERM)
    {
   		status_sigterm=1;
    }
    if (signum == SIGCHLD)
    {
        status_sigchld=1;
    }
}

void check_neg(int ret, const char *msg) 
{
	if (ret < 0) 
	{
		perror(msg);
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	//check if input is correct
	if( argc != 2)
    {
        printf(WHITE"Please input a keyword composing of letters only\n");
        return EXIT_FAILURE;
    }
	else
    {

		int leng = strlen(argv[1]);
		pid_t child[120],  pid;
	    int i, child_status, father_pid, n, j, status_execv;
	    char buffer1[32], buffer2[32], a[120];
	    
	    struct sigaction action; 
	    action.sa_handler = handler; 
	    sigaction(SIGUSR1, &action, NULL);
	    sigaction(SIGTERM, &action, NULL);
	    sigaction(SIGCHLD, &action, NULL);

	    //check if argn[1] has only f/t
		for (j = 0; j < strlen(argv[1]); j++){
			a[j] = argv[1][j];
	        if ((a[j] != 'f') && (a[j] != 't'))
	        {
	        	printf("only f or t\n");
	        	return EXIT_FAILURE;
	        }
	    }
	    //create children
		for (i = 0; i < leng; i++)
		{
			father_pid=getpid();
			child[i] = fork();
			
			check_neg(child[i], "fork");

			if (child[i] == 0)
			{
				snprintf(buffer1,32,"%d",i);
				
			    if(a[i]=='f')
			    {			    
				    char *const argv[] = {"./child", "f", buffer1,"closed", NULL};
				    status_execv=execv(argv[0],argv);
				    check_neg(status_execv, "execv failed");
				}

			    if(a[i]=='t')
			    {		   
				    char *const argv[] = {"./child", "t", buffer1, "open", NULL};
				    status_execv=execv(argv[0],argv);
				    check_neg(status_execv, "execv failed");
				}
			    exit(0);
			}
			else
			{
				printf(WHITE"[PARENT/PID=%d] Created child %d (PID=%d) and initial state '%c'\n", father_pid, i, child[i], a[i]);
			}
		}
		while(1)
		{
			//for SIGUSR1 -------------------------------------------------------------------------
			if (status_sigusr1==1){
				for (int i = 0; i < leng; i++)
				{
					kill (child[i], SIGUSR1);
				}
			}
			status_sigusr1=0;
			//-------------------------------------------------------------------------------------

			//for SIGTERM -------------------------------------------------------------------------
			if (status_sigterm==1){
				for (i = 0; i < leng; i++)
				{
					printf(WHITE"[PARENT/PID=%d] Waiting for %d children to exit \n", father_pid, (leng-i));
					kill (child[i], SIGTERM);
					pid=waitpid(child[i], &child_status, 0);
					printf(WHITE"Child with PID %d  terminated successfully with exit status code %d\n", pid, WEXITSTATUS(child_status));
				}
				printf(WHITE"[PARENT/PID=%d] All children exited, terminating as well\n", father_pid);
				return 0;
			}
			
			//-------------------------------------------------------------------------------------
			
			//for SIGCHLD -------------------------------------------------------------------------
			if (status_sigchld==1){		
				pid=waitpid(child[i], &child_status, 0);
				if(pid != -1)
				{
					for (i = 0; i < leng; i++)
					{
						if(child[i]==pid)
						{
							n = i;
						}
					}
					printf(WHITE"[PARENT/PID=%d] Child %d with PID=%d exited \n", getpid(), n, pid);
					child[n]=fork();
	
				    check_neg(child[n], "fork");
	
					if (child[n] == 0)
					{
						printf("[PARENT/PID=%d] Created new child for gate %d (PID %d) and initial state %c\n", getppid(), n,  getpid(), a[n]);
						snprintf(buffer1,32,"%d",n);
						if(a[n]=='f')
			    		{			    
					    	char *const argv[] = {"./child", "f", buffer1,"closed", NULL}; // the execv() only return if error occured the return value is -1
					    	status_execv=execv(argv[0],argv);
					    	check_neg(status_execv, "execv failed");
						}
					    if(a[n]=='t')
					    {		   
						    char *const argv[] = {"./child", "t", buffer1, "open", NULL}; // the execv() only return if error occured the return value is -1
						    status_execv=execv(argv[0],argv);
						    check_neg(status_execv, "execv failed");
						}
					    exit(0);
					}
				}			
			}	
			status_sigchld=0;
			//-------------------------------------------------------------------------------------	
		}
	    return 0;
    }
}