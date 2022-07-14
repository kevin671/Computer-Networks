#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/epoll.h>
#include <time.h>
#define PORT 8080
#define BUF_LEN 1024
#define MAX_EPOLL 80

size_t serial_msg_num = 0;

struct FdInfo
{
   int fd;
   char *name;
   struct FdInfo *next;
};

struct FdInfo *add_fd_to_epoll_instance(int epfd, struct FdInfo *fd_current_ptr, int target_fd)
{
   struct epoll_event event;
   event.events = EPOLLIN;
   event.data.ptr = malloc(sizeof(struct FdInfo));
   if (event.data.ptr == NULL)
   {
       exit(EXIT_FAILURE);
   }
   ((struct FdInfo *)event.data.ptr)->fd = target_fd;
   ((struct FdInfo *)event.data.ptr)->next = NULL;
   ((struct FdInfo *)event.data.ptr)->name = NULL;
   if (epoll_ctl(epfd, EPOLL_CTL_ADD, target_fd, &event) == -1)
   {
       exit(EXIT_FAILURE);
   }
   if (fd_current_ptr != NULL)
   {
       fd_current_ptr->next = (struct FdInfo *)event.data.ptr;
   }
   return (struct FdInfo *)event.data.ptr;
}

void add_name_to_fd_info(struct FdInfo *conn, char *name)
{
   conn->name = (char *)calloc(1, sizeof(char) * strlen(name));
   if (conn->name == NULL)
   {
       exit(EXIT_FAILURE);
   }
   strcpy(conn->name, name);
}

void edit_msg(char *msg, char *buf, char *name)
{
   char str_time[32] = {'\0'};
   time_t t = time(NULL);
   struct tm *date = localtime(&t);
   strftime(str_time, sizeof(str_time), "%Y/%m/%d %H:%M:%S", date);
   sprintf(msg, "[%ld %s %s]\n   %s\n", ++serial_msg_num, name, str_time, buf);
}

void write(struct FdInfo *fd_info_head, char *msg)
{
   if (fd_info_head == NULL)
   {
       return;
   }
   write(fd_info_head->fd, msg, strlen(msg));
   write(fd_info_head->next, msg);
}

struct FdInfo *delete_fd_from_epoll_instance(int epfd, struct FdInfo *fd_info_head, struct FdInfo *conn)
{
   close(conn->fd);
   struct FdInfo *h = fd_info_head;
   while (h->next != NULL && h->next->fd != conn->fd)
   {
       h = h->next;
   }
   h->next = h->next->next;
   epoll_ctl(epfd, EPOLL_CTL_DEL, conn->fd, NULL);
   free(conn->name);
   free(conn);
   while (h->next != NULL)
   {
       h = h->next;
   }
   return h;
}

void trim_nl(char *str)
{
   char *p;
   p = strchr(str, '\n');
   if (p != NULL)
   {
       *p = '\0';
   }
   p = strchr(str, '\r');
   if (p != NULL)
   {
       *p = '\0';
   }
}

int build_server(struct sockaddr_in *address)
{
   int server_fd, opt = 1;
   if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
   {
       exit(EXIT_FAILURE);
   }

   if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                  &opt, sizeof(opt)))
   {
       exit(EXIT_FAILURE);
   }
   address->sin_family = AF_INET;
   address->sin_addr.s_addr = INADDR_ANY;
   address->sin_port = htons(PORT);

   if (bind(server_fd, (struct sockaddr *)address, sizeof(*address)) < 0)
   {
       exit(EXIT_FAILURE);
   }

   if (listen(server_fd, 5) < 0)
   {
       exit(EXIT_FAILURE);
   }
   return server_fd;
}

struct FdInfo *init_epoll_event(int *epfd, int server_fd)
{
   char server_name[] = "server";
   *epfd = epoll_create(MAX_EPOLL);
   struct FdInfo *fd_info_current = add_fd_to_epoll_instance(*epfd, NULL, server_fd);
   add_name_to_fd_info(fd_info_current, server_name);
   return fd_info_current;
}

int main(int argc, char *argv[])
{
   int new_socket;
   struct sockaddr_in address;
   int addrlen = sizeof(address);
   int server_fd = build_server(&address);
   printf("build server\n");

   int epfd, event_readable;
   struct epoll_event events[MAX_EPOLL];
   struct FdInfo *fd_info_current = init_epoll_event(&epfd, server_fd);
   struct FdInfo *fd_info_head = fd_info_current;

   char buf[BUF_LEN] = {0};
   char msg[BUF_LEN] = {0};

   while (1)
   {
       event_readable = epoll_wait(epfd, events, MAX_EPOLL, -1);
       memset(buf, 0, sizeof(buf));
       memset(msg, 0, sizeof(msg));
       for (int i = 0; i < event_readable; i++)
       {
           struct FdInfo *conn = (struct FdInfo *)events[i].data.ptr;
           if (conn->fd == server_fd)
           {
               if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) == -1)
               {
                   continue;
               }
               add_fd_to_epoll_instance(epfd, fd_info_current, new_socket);
               fd_info_current = fd_info_current->next;
           }
           else if (read(conn->fd, buf, sizeof(buf)) <= 0)
           {
               if (strlen(conn->name) > 0)
               {
                   sprintf(buf, "%sがログアウトしました。\n", conn->name);
                   edit_msg(msg, buf, fd_info_head->name);
                   printf("%s\n", msg);
                   write(fd_info_head->next, msg);
               }
               fd_info_current = delete_fd_from_epoll_instance(epfd, fd_info_head, conn);
           }
           trim_nl(buf);
           if (conn->name == NULL)
           {
               add_name_to_fd_info(conn, buf);
               sprintf(buf, "%sがログインしました。\n", conn->name);
               edit_msg(msg, buf, fd_info_head->name);
           }
           else
           {
               edit_msg(msg, buf, conn->name);
           }
           printf("%s\n", msg);
           write(fd_info_head->next, msg);
       }
   }
}
