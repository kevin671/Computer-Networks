#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <sys/wait.h>
#include <netdb.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include <cstdint>
#include <cstdlib>
#include <signal.h>
#define MAX_LINE 256 

void sig_child(int sig)
{
    int pid, status;
    pid = wait(&status);
    printf("PID: %d, terminated¥n", pid);
}

int main()
{
    char inbuf[MAX_LINE]; 
    char obuf[MAX_LINE]; 
    pthread_t tid;
    intptr_t connfd;
    int sfd;
    socklen_t len;
    int uid = 10045;
    struct sockaddr_in server; 

    signal(SIGCHLD, sig_child);

    server.sin_family = AF_INET;
    server.sin_port = htons(uid+12600);
    server.sin_addr.s_addr = INADDR_ANY;
    inet_pton(AF_INET, "18.179.23.157", &server.sin_addr);

    sfd = socket(AF_INET, SOCK_STREAM, 0);
    bind(sfd, (struct sockaddr *)&server, sizeof(server));
    listen(sfd, 5);

    for(;;) {
        connfd = accept(sfd, (struct sockaddr *)&server, &len);

        pid_t pid = fork();
        if (pid == -1){
            perror("Fail fork()");
        }else if(pid == 0){
            close(sfd);

            memset(&inbuf, 0, sizeof(inbuf));
            recv((int)(intptr_t) connfd, inbuf, sizeof(inbuf), 0);
            printf("%s", inbuf);

            memset(&obuf, 0, sizeof(obuf));
            snprintf(obuf, sizeof(obuf), 
            "HTTP/1.0 200 OK¥r¥n"
            "Content-Type: text/html¥r¥n"
            "¥r¥n"
            "<font color=red><h1>HELLO</h1></font>¥r¥n");
            send((int)(intptr_t) connfd, obuf, (int)strlen(obuf), 0);

            exit(0);
        }
        close(connfd);
    }
}
