#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

void check_neg(int ret, const char *msg) 
{
	if (ret < 0) 
	{
		perror(msg);
		exit(EXIT_FAILURE);
	}
}

int child_fn(int child_id, int in_fd, int out_fd, int k, int child_no)
{
	ssize_t w, r;
	unsigned long long int factorial, num;
	
	num = child_id;

	if (child_id == 0) // Only for the first child
	{
		num += child_no;
		factorial=1;
		w = write(out_fd, &factorial, sizeof(unsigned long long int));
		check_neg(w, "write");
	}
	while(num<=k)
	{
		r = read(in_fd, &factorial, sizeof(unsigned long long int));
		check_neg(r, "read");
		
		factorial *= num;
		
		w = write(out_fd, &factorial, sizeof(unsigned long long int));
		check_neg(w, "write");
			
		num += child_no;
	}
	if (num==(k+child_no))
	{
		printf("final factorial:%lld\n", factorial);
	}
}

int main(int argc, char **argv)
{
	if( argc != 3)
    {
        int status_argc = -1;
        check_neg(status_argc, "error argc");
    }
	else
	{
		int **pipes;
		pid_t *childpid;
		int child_no, c, i, k, pid, in_fd, out_fd, status, status_pipe;

		child_no = atoi(argv[1]);
		pipes = malloc(child_no*sizeof(int *));
		childpid = malloc(child_no*sizeof(int));
		k = atoi(argv[2]);

		for (i=0;i<child_no;i++) //create pipes
		{
			pipes[i] = malloc(2*sizeof(int));
			status_pipe = pipe(pipes[i]);
			check_neg(status_pipe, "error pipe");	
		}
		
		for (i=0;i<child_no;i++) //create child process
		{
			childpid[i] = fork();
			check_neg(childpid[i], "error pid");
			if (childpid[i] == 0) {
				if (i==0)
				{
					in_fd = pipes[child_no-1][0];
				}
				else
				{
					in_fd = pipes[i-1][0];
				}
				out_fd = pipes[i][1];
				
				child_fn(i, in_fd ,out_fd, k, child_no);
				exit(1);
			}
		}
		
		for (i=0;i<child_no;i++) //father's code
		{
			c = close(pipes[i][0]);
			check_neg(c, "error pipe[i][0]");
            
            c = close(pipes[i][1]); 
            check_neg(c, "error pipe[i][1]");
			
			pid = wait(&status);
			check_neg(pid, "error wait");
		}
	}
	return 0;
}