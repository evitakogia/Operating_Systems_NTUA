#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <sys/epoll.h>

#define Cyan "\033[0;96m"
#define WHITE "\033[37m"
#define RED "\033[31;1m"
#define YELLOW "\033[33m"

void check_neg(int ret, const char *msg) 
{
    if (ret < 0) 
    {
        perror(msg);
        exit(EXIT_FAILURE);
    }
}

int make_socket_non_blocking (int sockfd)
{
  int flags, s;

  flags = fcntl (sockfd, F_GETFL, 0);
  check_neg(flags, "non blocking flags");

  flags |= O_NONBLOCK;
  s = fcntl (sockfd, F_SETFL, flags);
  check_neg(s, "non blocking s");

  return 0;
}

void server_get(char *buf)
{
    int code, light;
    float temperature;
    time_t timestamp;
    struct tm *info;

    sscanf(buf, "%d %d %e %ld ", &code, &light, &temperature, &timestamp);

    printf(Cyan"Latest event:\n");

    if (code==0)
        printf(WHITE"boot (0)\n");
    if (code==1)
        printf(WHITE"setup(1)\n");
    if (code==2)
        printf(WHITE"interval (2)\n");
    if (code==3)
        printf(WHITE"button (3)\n");
    if (code==4)
        printf(WHITE"motion (4)\n");
    
    printf("Temperature is : %.2f\n",temperature/100);
    printf("Light level is: %d\n", light);
    
    info = localtime( &timestamp );
    printf("Timestamp is: %s \n", asctime(info));
}

void server_help()
{
    printf(Cyan"Available commands:\n");
    printf(Cyan"help                     ");
    printf(WHITE"Print this help message\n");
    printf(Cyan"exit                     ");
    printf(WHITE"Exit\n");
    printf(Cyan"get                      ");
    printf(WHITE"Retrieve sensor data from server\n");
    printf(Cyan"N name surname reason    ");
    printf(WHITE"Permission to go out during quarantine\n");
}

void debug(char *c, char *buf, int deb)
{
    if (deb==1)
        printf(YELLOW"[DEBUG] %s: %s\n"WHITE, c, buf);
}

int main(int argc, char **argv)
{   
    int sock_fd, epoll_fd, connect_status, close_status, w, r, status_non_blocking, num_ready, port, i, deb;
    char host[36], buf[50], *p;
    struct epoll_event events[4], event;
    struct sockaddr_in server;
    struct hostent *server_host;

    strcpy(host,"lab4-server.dslab.os.grnetcloud.net\0");
    port = 18080;
    deb = 0;
    for (i = 0; i < argc; ++i)
    {
        if(strncmp(argv[i], "--host", 6) == 0)
            strcpy(host, argv[i+1]);
        else if(strncmp(argv[i], "--port", 6) == 0)
            port = atoi(argv[i+1]);
        else if(strncmp(argv[i], "--debug", 6) == 0)
            deb = 1;
    }
    //creat a socket
    sock_fd = socket(AF_INET,SOCK_STREAM,0);
    check_neg(sock_fd, "socket");

    //specify an address for the socket
    server_host=gethostbyname(host);
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    bcopy(server_host->h_addr, &server.sin_addr, server_host->h_length);
   

    //Add socket to epoll
    epoll_fd = epoll_create1(0);
    check_neg(epoll_fd, "epoll");

    
    while(1)
    {
        connect_status = connect(sock_fd,(struct sockaddr*) &server, sizeof(server));
        check_neg(connect_status, "connect");
        status_non_blocking = make_socket_non_blocking(sock_fd);
        check_neg(status_non_blocking, "status non blocking");
        event.events = EPOLLIN|EPOLLET;
        event.data.fd = sock_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock_fd, &event);
        
        status_non_blocking = make_socket_non_blocking(0);
        check_neg(status_non_blocking, "status non blocking");
        event.events = EPOLLIN|EPOLLET;
        event.data.fd = 0;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &event);
        num_ready = epoll_wait(epoll_fd, events, 5, 3);
        printf("%d\n", events[i].data.fd);
        for(i = 0; i < num_ready; i++) 
        {
            r = read(events[i].data.fd, buf, sizeof buf);
            check_neg(r, "read");
            if (events[i].data.fd == 0)
            {
                if(strncmp(buf, "help", 4) == 0)
                {
                    server_help();
                    memset(buf, 0, sizeof(buf[50]));  
                }
                else if(strncmp(buf, "exit", 4) == 0)
                {
                    close_status = close (events[i].data.fd);
                    check_neg(close_status, "close epoll events");
                    printf(Cyan"socket exit\n");
                    return 0;
                }
                else
                { 
                    w = write(sock_fd, buf, strlen(buf));
                    check_neg(w, "write");
                    debug("send", buf, deb);
                    memset(buf, 0, sizeof(buf));
                }
            }
            if (events[i].data.fd != 0)
            {
                debug("read", buf, deb);
                if(buf[1] == ' ')
                {   
                    server_get(buf);
                    memset(buf, 0, sizeof(buf));
                }
                else if(strncmp(buf, "try again", 9) == 0)
                {
                    printf(RED"check 'help'\n"WHITE);
                    memset(buf, 0, sizeof(buf));
                }
                else if(strncmp(buf, "ACK", 3) == 0)
                {
                    printf(Cyan"Response: ");
                    printf(WHITE"%s\n", buf);
                    memset(buf, 0, sizeof(buf));
                }
                else
                {
                    printf(Cyan"Send verification code: ");
                    printf(WHITE"%s\n", buf);
                    memset(buf, 0, sizeof(buf));
                }
            }      
        }
    }
}
