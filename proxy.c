#include <stdio.h>
#include "csapp.h"
#include "sbuf.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAXLINE 8192
#define NTHREADS 4
#define SBUFSIZE 16
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void read_requesthdrs(rio_t *rp);
int request_hdrs(int connfd, char *hostname, char *method,char *filename, char *portnum);
int echo(int connfd);
int parse_uri(char *uri, char *hostname, char *portnum, char *filename);
sbuf_t sbuf;

int echo(int connfd){
  int host_fd;
  rio_t rio_client;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char hostname[MAXLINE], filename[MAXLINE], portnum[MAXLINE];
  
  Rio_readinitb(&rio_client, connfd);
  Rio_readlineb(&rio_client, buf, MAXLINE);
  sscanf(buf, "%s %s %s\n", method, uri, version);
  if(strcasecmp(method, "GET")){
    printf("doesn't implement %s method.", method);
    return ;
  }
  parse_uri(uri, hostname, portnum, filename); // parsing uri -> hostname, portname, filename.
  read_requesthdrs(&rio_client); // read last of cliet header.

  request_hdrs(connfd, hostname, method, filename, portnum);
  return 1;
}

void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  while (strcmp(buf, "\r\n"))
  {
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

int parse_uri(char *uri, char *hostname, char *portnum, char *filename)
{
  char *p1, *p2, *p3;
  char temp[MAXLINE];
  int is_port = 0;

  p1 = strchr(uri, '/');
  if (p2 = strchr(p1 + 2, ':'))
  {
    *p2 = '\0';
    strcpy(hostname, p1 + 2);
    strcpy(temp, p2 + 1);
    is_port = 1;
  }
  else {
    strcpy(temp, p1 + 2);
  }
  p3 = strchr(temp, '/');
  *p3 = '\0';
  if (is_port)
    strcpy(portnum, temp);
  else {
    strcpy(hostname, temp);
    strcpy(portnum, "80");
  }
  strcpy(filename, p3 + 1);
  return is_port;
}

int request_hdrs(int connfd, char *hostname, char* method, char *filename, char *portnum){
  int n;
  char buf[MAXLINE] = "";
  char acceptbuf[MAXLINE] = "";
  rio_t rio;
  int host_fd;
  // open client socket
  host_fd = Open_clientfd(hostname, portnum);
  //   printf("connection is failed\n");
  //   exit(1);
  // } else printf("Connection success (%s, %s)\n", hostname, portnum);
  Rio_readinitb(&rio, host_fd);

  sprintf(buf, "%s /%s HTTP/1.0\r\n", method, filename);
  sprintf(buf, "%sHOST: %s:%s\r\n", buf, hostname,portnum);
  sprintf(buf, "%s%s", buf, user_agent_hdr);
  sprintf(buf, "%sConnection: close\r\n", buf);
  sprintf(buf, "%sProxy-Connection: close\r\n\r\n", buf);
  // printf("%s", buf);
  // srcfd = getaddrinfo(hostname, NULL, &hints, &listp);
  // srcfd = Open_clientfd(hostname, portnum);
  Rio_writen(host_fd, buf, strlen(buf));
  printf("Request Headers:\n");
  
  while ( n = Rio_readlineb(&rio, acceptbuf, MAXLINE) > 0){
    Rio_writen(connfd, acceptbuf, strlen(acceptbuf));
  }

  Close(host_fd);
  return host_fd;
}

int main(int ac, char **av)
{
  int listenfd_cli, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  pthread_t ptid;

  if (ac != 2)
  {
    fprintf(stderr, "put only one args. usage : %s <port>", av[0]);
    exit(1);
  }
  listenfd_cli = Open_listenfd(av[1]); // open listen socket for client.
  sbuf_init(&sbuf, SBUFSIZE);

  for (int i = 0; i < NTHREADS; i++){
    Pthread_creadte(&ptid, NULL, thread, NULL);
  }

  while (1)
  {
    clientlen = sizeof(struct sockaddr_storage);
    connfd = Accept(listenfd_cli, (SA *)&clientaddr, &clientlen);
    sbuf_insert(&sbuf, connfd);
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
              0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
  }
  return 0;
}

void *thread(void *connfd){
  Pthread_detach(pthread_self());
  while(1) {
    int fd = *(int *)connfd;
    echo(fd);
    close(fd);
  }
  return NULL;
}