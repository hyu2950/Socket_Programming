/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT "25617" // the port client will be connecting to

#define MAXDATASIZE 100 // max number of bytes we can get at once

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    char message[120];
    char username[50], password[50];
    struct sockaddr_in my_addr;
    socklen_t addrlen;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;



    if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    char myIP[16];
    unsigned int myPort;
    bzero(&my_addr, sizeof(my_addr));
    socklen_t len = sizeof(my_addr);
    getsockname(sockfd, (struct sockaddr *) &my_addr, &len);
    inet_ntop(AF_INET, &my_addr.sin_addr, myIP, sizeof(myIP));
    myPort = ntohs(my_addr.sin_port);

    // printf("Local ip address: %s\n", myIP);
    // printf("Local port : %u\n", myPort);
    //inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
    //          s, sizeof s);
    // printf("client: connecting to %s\n", s);
    printf("The client is up and running.\n");

    freeaddrinfo(servinfo); // all done with this structure



    for (int i = 2; i >= 0; i--)
    {
        printf("Please enter the username:");
        scanf("%s", username);
        printf("Please enter the password:");
        scanf("%s", password);
        sprintf(message, "%s,%s", username, password);

        printf("%s sent an authentication request to the main server.\n", username);

        if ((numbytes = send(sockfd, message, strlen(message), 0)) == -1)
        {
            perror("send");
            exit(1);
        }
        fflush(stdout);

        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("recv");
            exit(1);
        }

        buf[numbytes] = '\0';

        printf("%s received the result of authentication using TCP over port %u.\n", username,myPort);

       // printf("client: received '%s'\n", buf);
        if (buf[0] == '2')
        {
            printf("Authentication is successful\n");
            break;
        }
        else 
        {
            if (buf[0] == '1')
            {
                printf("Authentication failed:Password does not match\n");
            }
            else if (buf[0] == '0')
            {
                printf("Authentication failed:Username Does not exist\n");
            }
            if (i != 0){
            printf("Attempts remaining:%d\n", i);
            } else{
            printf("Authentication Failed for 3 attempts.Client will shut down.\n");
            close(sockfd);
            exit(1);
            }
        }
    }

    while (1)
    {
        char code[10], category[50];
        printf("Please enter the course code to query:");
        scanf("%s", code);
        printf("Please enter the category(Credit/Professor/Days/CourseName):");
        scanf("%s", category);
        if (code[0] == 'C' || code[0] == 'c')
        {
            sprintf(message, "0:%s:%s:%s:", code, category,username);
        }
        else
        {
            sprintf(message, "1:%s:%s:%s:", code, category,username);
        }

        if ((numbytes = send(sockfd, message, strlen(message), 0)) == -1)
        {
            perror("send");
            exit(1);
        }
        fflush(stdout);
        printf("%s sent a request to the main server.\n",username);

        if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1)
        {
            perror("recv");
            exit(1);
        }
        printf("The client received the response from the Main server using TCP over port %u.\n",myPort);


        buf[numbytes] = '\0';

        if (buf[0] == '\n')
        {
            printf("Didn't find the course %s.\n", code);
        }
        else
        {
            printf("The %s of %s is %s.\n", category, code, buf);
        }
        printf("\n-----Start a new request-----\n");
    }
    close(sockfd);
    return 0;
}
