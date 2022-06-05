#include <stdio.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#define MAX_LINE 256 
int main(){ 
  char buf[MAX_LINE]; 
  int s; 

  struct sockaddr_in server; 
  server.sin_family = AF_INET; 
  server.sin_port = htons(13); 
  int a = inet_pton(AF_INET, "18.179.23.157", &server.sin_addr); 
  //int a = inet_pton(AF_INET6, "2406:da14:941:3c03::20", &server.sin_addr); 
  
  if ((s = socket(PF_INET, SOCK_STREAM, 0))<0) { 
    perror("socket"); return 1; 
  } 
  if (connect(s, (struct sockaddr *)&server, sizeof(server))<0){ 
    perror("connect"); 
    close(s); 
    return 1; 
  } 
  memset(buf, 0, sizeof(buf)); 
  int n = read(s, buf, sizeof(buf)); 
  printf("%s¥n", buf); 
}
