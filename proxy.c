#include <stdio.h>
#include "csapp.h"
#include "cache.h"
#include "sbuf.h"
/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
// #define MAXLINE 8192
#define NTHREADS 4
#define SBUFSIZE 16
/*
  For concurrency: pre-thread declare.
  For caching: using semi-doubly linked list. MAX_CACHE_COUNT = max lenght of the list. here is 10.
  not yet: protecting cache_list by P&V.
*/
/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void read_requesthdrs(rio_t *rp);
int request_hdrs(int connfd, char* uri, char *hostname, char *method,char *filename, char *portnum);
int echo(int connfd);
int parse_uri(char *uri, char *hostname, char *portnum, char *filename);
void *thread(void* fd);

sbuf_t sbuf;
static cache_list* cachelist;

int echo(int connfd){
  rio_t rio_client;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE], tmp[MAXLINE];
  char hostname[MAXLINE], filename[MAXLINE], portnum[MAXLINE];
  
  Rio_readinitb(&rio_client, connfd);
  Rio_readlineb(&rio_client, buf, MAXLINE);
  printf("Request headers:\n");
  printf("%s\n", buf);
  sscanf(buf, "%s %s %s\n", method, uri, version);
  if(strcasecmp(method, "GET")){
    sprintf(buf, "doesn't implement %s method.", method);
    Rio_writen(connfd, buf, strlen(buf));
    return -1;
  }
  read_requesthdrs(&rio_client); // read last of cliet header.
  strcpy(tmp, uri);
  parse_uri(tmp, hostname, portnum, filename); // parsing uri -> hostname, portname, filename.

  request_hdrs(connfd, uri, hostname, method, filename, portnum);
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

int request_hdrs(int connfd, char* uri, char *hostname, char* method, char *filename, char *portnum){
  int host_fd;
  char buf[MAXLINE], acceptbuf[MAX_OBJECT_SIZE];
  rio_t rio;
  // open client socket
  char *ret = find_cache(cachelist, uri);
  if (ret != NULL){
    Rio_writen(connfd, ret, MAX_OBJECT_SIZE);
    return 0;
  }
  host_fd = Open_clientfd(hostname, portnum);
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
  printf("%s", buf);

  Rio_readnb(&rio, acceptbuf, MAX_OBJECT_SIZE);
  insert_cache(cachelist, uri, acceptbuf);
  Rio_writen(connfd, acceptbuf, MAX_OBJECT_SIZE);
  Close(host_fd);
  return 1;
}

void *thread(void *connfd){
  Pthread_detach(pthread_self());
  while(1) {
    int fd = sbuf_remove(&sbuf);
    echo(fd);
    close(fd);
  }
  return NULL;
}

int main(int ac, char **av)
{
  int listenfd_cli, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  pthread_t ptid;

  cachelist = cache_init();
  if (ac != 2)
  {
    fprintf(stderr, "put only one args. usage : %s <port>", av[0]);
    exit(1);
  }
  listenfd_cli = Open_listenfd(av[1]); // open listen socket for client.
  sbuf_init(&sbuf, SBUFSIZE);

  for (int i = 0; i < NTHREADS; i++){
    Pthread_create(&ptid, NULL, thread, NULL);
  }

  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd_cli, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    sbuf_insert(&sbuf, connfd);
  }
  del_cache(cachelist);
  return 0;
}
