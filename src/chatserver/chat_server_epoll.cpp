#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#define MAX_LEN 1024
#define MAX_USERS 100

struct FdInfo
{
    int fd;
    int user_id;
};
int user_count = 0;
FdInfo *user_list[100];

int build_server(struct sockaddr_in *sin)
{
    memset(&sin, 0, sizeof(sin));
    sin->sin_family = AF_INET;
    sin->sin_port = htons(8080);
    sin->sin_addr.s_addr = INADDR_ANY;

    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        peeror("socket() failed");
        exit(1);
    }
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        peeror("bind() failed");
        exit(1);
    }
    listen(s, 5);
    return s;
}

/*+
 * add server fd to epoll instance
 */
struct FdInfo *add_fd_to_epoll(int epfd, int server_fd)
{
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = EPOLIN;
    ((struct FdInfo *)event.data.fd) = malloc(sizeof(struct FdInfo));
    ((struct FdInfo *)event.data.ptr->fd) = 0;
    ((struct FdInfo *)event.data.prt->user_id) = NULL;

    if (epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &event) < 0)
    {
        printf("epoll_ctl() failed\n");
        return -1;
    }
    return event.data.ptr;
}

void write_to_everyone(char *message)
{
    for (FdInfo *user_info : user_list)
    {
        if (write(user_info->fd, message, sizeof(message)) < 0)
        {
            perror("write() failed");
            exit(1);
        }
    }
}

int main(void)
{
    int addr_len, conn_sock;
    struct sockaddr_in sin;
    int server_fd = build_server(&sin);
    printf("build server");

    int epfd = epoll_create(1) : if (epfd < 0)
    {
        printf("epoll_creat() failed\n");
        return -1;
    }
    struct FdInfo *fd_info = add_fd_to_epoll(*epfd, server_fd);

    char buf[MAX_LEN];
    struct epoll_event events[MAX_EPOLL];

    while (1)
    {
        nfds = epoll_wait(epfd, &events, 1, -1);
        for (i = 0; i < nfds; i++)
        {
            if (events[i].data.ptr->fd == server_fd)
            {
                if (conn_sock = accept(server_fd, (struct sockaddr *)&sin, &addr_len) < 0)
                {
                    perror("accept() failed");
                    exit(1);
                }
                struct FdInfo *fd_info = add_fd_to_epoll(epfd, conn_sock);
                fd_info->user_id = user_count++;
            }
            else
            {
                read(events[i].data.ptr->fd, buf, sizeof(buf) <= 0)
                {
                    perror("read() failed");
                    exit(1);
                }
                write_to_everyone(buf);
            }
        }
    }
}