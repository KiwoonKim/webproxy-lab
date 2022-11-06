#include <stdio.h>
#include "csapp.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAXLINE 8192
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void echo(int connfd){
  size_t n;
  char buf[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio, connect);
  while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){
    printf("server received %d bytes\n", (int)n);
    Rio_writen(connfd, buf, n);
  }
}
/*
  squantial web proxy
*/
int main(int ac, char **av)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  if (ac != 2)
  {
    fprintf(stderr, "put only one args. usage : %s <port>", av[0]);
    exit(1);
  }
  listenfd = Open_listenfd(av[1]); // open listen socket.
  printf("%s", user_agent_hdr);
  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);                       // accept client.
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0); // get client name
    printf("Accept from (%s, %s)", hostname, port);
    echo(connfd);
    Close(connfd);
  }
  return 0;
}
