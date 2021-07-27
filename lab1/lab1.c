#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

//for colors:
#define RED "\033[31;1m"
#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define WHITE "\033[37m"

int readFile(int fd_in) {
    char buffer[32];
    ssize_t count;
    for(;;) //while(true)
    {
    	count = read(fd_in, buffer, sizeof(buffer)-1); //return the number of characters  
    	if(count==0) return 0;
    	if(count==-1)
    	{
    		perror("read");
    		return 1;
    	}
    	buffer[count]='\0';
    	printf(WHITE"%s",buffer);
    }
}

int main(int argc, char **argv) 
{
    int num = atoi(argv[2]);//cast from char to int
    int status,i,fd_in;
    pid_t child1, child2;
    time_t seconds;
    char buffer[32];

    // Creat and open file for reading and writting
    fd_in = open(argv[1], O_CREAT | O_RDWR);

    child1 = fork(); //process creation (1st child)

    if(child1<0)
    {
        perror("fork");
    }
    
    if(child1==0)
    {
    	printf(BLUE "[Child1] Started. PID=%d PPID=%d\n",getpid(), getppid());
    	
        for (int i = 0; i < num+1; i++)
        {
            if(i%2==0) //even
            {
            	time(&seconds);
                printf(BLUE "[CHILD1]: Heartbeat PID= %d Time= %ld, x= %d\n", getpid(), seconds,i);
                sleep(1);
                snprintf(buffer,32,"message from %d\n", getpid()); 
				write(fd_in,buffer,strlen(buffer));
            }
        }
        printf(BLUE "[CHILD1]:Terminating!\n");
        exit(0);
    }
    else 
    {
        child2 = fork();//process creation (2nd child)
        if(child2<0)
        {
            perror("fork");
        }
        if (child2 == 0) 
        {
        	printf(GREEN "[Child2] Started. PID=%d PPID=%d\n",getpid(), getppid());

            for (int i = 0; i < num+1; i++)
            {
                if(i%2!=0) //odd
                {
                	time(&seconds);
                    printf(GREEN "[CHILD2]: Heartbeat PID= %d Time= %ld, x= %d\n", getpid(), seconds,i);
                    sleep(1);
                    snprintf(buffer,32,"message from %d\n", getpid());
					write(fd_in,buffer,strlen(buffer));
                }
            }
            printf(GREEN "[CHILD2]:Terminating!\n");
            exit(0); 
        }
        else 
        {
        	for (int i = 0; i < ((num/2)+1); i++)
            {
            	time(&seconds);
                printf(RED "[PARENT]: Heartbeat PID= %d Time= %ld\n", getpid(), seconds);
                sleep(1);
                snprintf(buffer,32,"message from %d\n", getpid());
				write(fd_in,buffer,strlen(buffer));
            }
            printf(RED "[PARENT]: Waiting for child\n");
            child1 = wait (&status);
 			printf (RED "[PARENT]: Child with PID=%d terminated\n", child1);
            
            printf(RED "[PARENT]: Waiting for child\n");
			child2 = wait (&status);
 			printf (RED "[PARENT]: Child with PID=%d terminated\n", child2);
			
 			lseek(fd_in,0,SEEK_SET);
			readFile(fd_in);
			close (fd_in);
			remove(argv[1]);
            exit(0);
        }
    }
    return 0;
}
