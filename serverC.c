/*
** serverC
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define MYPORT "21617" // the port users will be connecting to
#define MAXBUFLEN 120
#define FILEDIR "./cred.txt"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

char* read_file(char *username, char *buf)
{
    // read file
    FILE *fp = fopen(FILEDIR, "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char tmp[MAXBUFLEN];
    char *l;
    if (fp == NULL)
    {
        printf("File %s not found", FILEDIR);
        return (char*)"-1";
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *pos;
        if ((pos=strchr(line, '\n')) != NULL)
            *pos = '\0';
       
        strcpy(tmp, line);
        char *token = strtok(tmp, ",");
       
        if (strcmp(username, token) == 0)
        {
           //printf("read_file --- read:%s:buf:%s",line,buf);
            if (strcmp(buf, line) == 0)
            {
                return (char *)"2";
            }
            else
            {
                return (char *)"1";
            }
        }
    }
    return (char *)"0";
}

int main(void)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    char buf[MAXBUFLEN], tmp[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("listener: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("listener: bind");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "listener: failed to bind socket\n");
        return 2;
    }

    freeaddrinfo(servinfo);
    printf("The ServerC is up and running using UDP on port 21617.\n");
    while (1)
    {

        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                                 (struct sockaddr *)&their_addr, &addr_len)) == -1)
        {
            perror("recvfrom");
            exit(1);
        }

        printf("The ServerC received an authentication request from the Main Server.\n");
        //  printf("listener: packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        //  printf("listener: packet contains \"%s\"\n", buf);

        char* username;
        strcpy(tmp, buf);
        username = strtok(tmp, ",");
      

        // printf("%s    %s\n",token,buf);
        char* auth_res = read_file(username,buf);
        //printf("%s\n", auth_res);
       

        numbytes = sendto(sockfd, auth_res, strlen(auth_res), 0,
                          (struct sockaddr *)&their_addr, addr_len);
        if (numbytes == -1)
        {
            perror("listener: sendto");
            exit(1);
        }
        else
        {
            printf("The ServerC finished sending the response to the Main Server.\n");
        }
    }

    close(sockfd);

    return 0;
}
