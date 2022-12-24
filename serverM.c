/*
** server.c -- a stream socket server demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define SERVERPORTC "21617"
#define SERVERPORTCS "22617"
#define SERVERPORTEE "23617"
#define SERVERPORT "24617"
#define PORT "25617" // the port users will be connecting to
#define MAXBUFLEN 120
#define BACKLOG 10 // how many pending connections queue will hold

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void get_encrypted(char *text)
{
    for (int i = 0; text[i] != '\0'; i++)
    {
        char ch = text[i];
        if ((ch <= '9') && (ch >= '0'))
        {
            ch = (char)((ch - '0' + 4) % 10 + '0');
        }
        else if ((ch >= 'A' ) && (ch <= 'Z'))
        {
            ch = (char)((ch - 'A' + 4) % 26 + 'A');
        }
        else if ((ch >= 'a') && (ch <= 'z'))
        {
            ch = (char)((ch - 'a' + 4) % 26 + 'a');
        }
        text[i] = ch;
    }
}

int setupTCP()
{
    int sockfd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
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
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1)
        {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }

    return sockfd;
}

void setupUDP(char *message_buf)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo("localhost", SERVERPORTC, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }

    // loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "talker: failed to create socket\n");
        return;
    }
   
    printf("The main server sent an authentication request to serverC.\n");
    //printf("send:%s", message_buf);
    if ((numbytes = sendto(sockfd, message_buf, strlen(message_buf), 0, p->ai_addr, p->ai_addrlen)) == -1)
    {
        perror("talker: sendto");
        exit(1);
    }
   
    freeaddrinfo(servinfo); // done with servinfo

    int recv_bytes;

    recv_bytes = recvfrom(sockfd, message_buf, MAXBUFLEN-1, 0, NULL, NULL);
    if (recv_bytes == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    message_buf[recv_bytes] = '\0';
    printf("The main server received the result of the authentication request from ServerC using UDP over port 24617.\n");
   // printf("listener: packet contains \"%s\"\n", message_buf);
    close(sockfd);
    return;
}

void setUDPQuery(char *message_buf,char* port,char* server)
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo("localhost", port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return;
    }

    // loop through all the results and make a socket
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("talker: socket");
            continue;
        }

        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "talker: failed to create socket\n");
        return;
    }

    printf("The main server sent a request to server%s.\n",server);
    printf("send:%s", message_buf);
    if ((numbytes = sendto(sockfd, message_buf, strlen(message_buf), 0, p->ai_addr, p->ai_addrlen)) == -1)
    {
        perror("talker: sendto");
        exit(1);
    }
    freeaddrinfo(servinfo); // done with servinfo

    int recv_bytes;

    recv_bytes = recvfrom(sockfd, message_buf, MAXBUFLEN-1, 0, NULL, NULL);
    if (recv_bytes == -1)
    {
        perror("recvfrom");
        exit(1);
    }

    message_buf[recv_bytes] = '\0';
    printf("The main server received the response from Server%s using UDP over port 24617.\n",server);
    printf("listener: packet contains \"%s\"\n", message_buf);
    close(sockfd);
    return;
}


int main(void)
{
    int sockfd, new_fd, numbytes, udpsockfd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    char message_buf[MAXBUFLEN],tmp[MAXBUFLEN];
    char * username,* code, *category,*tmp1;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

   

    printf("The main server is up and running.\n"); 
    sockfd = setupTCP();
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    close(sockfd); // child doesn't need the listener
    while (1)
    { // main accept() loop
        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);
       // printf("server: got connection from %s\n", s);

      
       
       
        if ((numbytes = recv(new_fd, message_buf, MAXBUFLEN-1, 0)) == -1)
        {
            perror("recv");
            exit(1);
        }
        if (numbytes == 0)
        {
            sockfd = setupTCP();
             sin_size = sizeof their_addr;
             new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
             close(sockfd); 
            continue;
        }
        message_buf[numbytes] = '\0';
      
        

    //    printf("client: received '%s'\n", message_buf);

        if (message_buf[0] == '0' || message_buf[0] == '1')
        {
            strcpy(tmp, message_buf);
            tmp1 = strtok(tmp,":");
            tmp1 = strtok(NULL,":");
            code = tmp1;
            tmp1 = strtok(NULL,":");
            category= tmp1;
            tmp1 = strtok(NULL,":");
            username= tmp1;
            printf("The main server received from %s to query course %s about %s using TCP over port 25617.\n",username,code,category);
            if (message_buf[0] == '0'){
                setUDPQuery(message_buf,(char *)SERVERPORTCS,(char *)"CS");
            } else{
            printf("go to ServerEE\n");
                setUDPQuery(message_buf,(char *)SERVERPORTEE,(char *)"EE");
            }

            printf("The main server sent the query information to the client.\n");


        }
        else
        {   
           
            strcpy(tmp, message_buf);
            username = strtok(tmp, ":");
            printf("The main server received the authentication for %s using TCP over port 25617.\n",username);
            get_encrypted(message_buf);
            setupUDP(message_buf);
            printf("The main server sent the authentication result to the client.\n");
          
        }

          if (send(new_fd, message_buf, strlen(message_buf), 0) == -1)
            {
                perror("send");
            }
    }
    printf("client sock closed");
    close(new_fd);
    return 0;
}
