#include "../csapp.h"


int main(int ac, char **av){
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (ac != 3){
        fprintf(stderr, "usage: %s <host> <port>", av[0]);
        exit(0);
    }
    host = av[1];
    port = av[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while (Fgets(buf, MAXLINE, stdin) != NULL){
        Rio_writen(clientfd, buf, strlen(buf));
        Rio_readlineb(&rio, buf, MAXLINE);
        Fputs(buf, stdout);
    }
    Close(clientfd);
    exit(0);
}