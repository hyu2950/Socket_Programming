/*
** serverEE
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
#define MYPORT "23617" // the port users will be connecting to
#define MAXBUFLEN 120
#define FILEDIR "./ee.txt"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

char* read_file(char *code, char* categeray,char *buf)
{
    // read file
    FILE *fp = fopen(FILEDIR, "r");
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char tmp[MAXBUFLEN];
    char *l,*token,*tmp1;
    char* info[MAXBUFLEN]; 
    if (fp == NULL)
    {
        printf("File %s not found", FILEDIR);
        return NULL;
    }

    while ((read = getline(&line, &len, fp)) != -1)
    {
        char *pos;
        if ((pos=strchr(line, '\n')) != NULL)
            *pos = '\0';
        strcpy(tmp, line);
        tmp1 = strtok(tmp, ",");
        token = tmp1;

        if (strcmp(code, token) == 0)
        {
            int i = 0;
            while (tmp1 != NULL)
            {
                info[i++] = tmp1;
                tmp1 = strtok(NULL, ",");
            }
            if (categeray[2] == 'e'){
                strcpy(buf,info[1]);
                return info[1];
            } 
            if (categeray[2] == 'o'){
                strcpy(buf,info[2]);
                return info[2];
            } 
            if (categeray[2] == 'y'){
                strcpy(buf,info[3]);
                return info[3];
            } 
            if (categeray[2] == 'u'){
                strcpy(buf,info[4]);
                return info[4];
            } 

        }
    }

    strcpy(buf,(char *)"\n");
    return (char *)"\n";
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
    char *username, *code, *category, *tmp1;

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
    printf("The ServerEE is up and running using UDP on port 23617.\n");
    while (1)
    {

        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
                                 (struct sockaddr *)&their_addr, &addr_len)) == -1)
        {
            perror("recvfrom");
            exit(1);
        }

      
        //  printf("listener: packet is %d bytes long\n", numbytes);
        buf[numbytes] = '\0';
        printf("listener: packet contains \"%s\"\n", buf);
        strcpy(tmp, buf);
        tmp1 = strtok(tmp, ":");
        tmp1 = strtok(NULL, ":");
        code = tmp1;
        tmp1 = strtok(NULL, ":");
        category = tmp1;
        printf("The ServerEE received a request from the Main Server about the %s of %s.\n",category,code);
        // printf("%s    %s\n",token,buf);
        char* data = read_file(code,category, buf);
       // printf("%s\n", data);
       
        if (data[0] == '\n'){
            printf("Didn't find the course %s.\n", code);
        } else{
             printf("The course information has been found: The  %s of %s is %s.\n", category,code,buf);
        }
      
        numbytes = sendto(sockfd, buf, strlen(buf), 0,
                          (struct sockaddr *)&their_addr, addr_len);
        if (numbytes == -1)
        {
            perror("listener: sendto");
            exit(1);
        }
        else
        {
            printf("The ServerEE finished sending the response to the Main Server.\n");
        }
    }

    close(sockfd);

    return 0;
}
