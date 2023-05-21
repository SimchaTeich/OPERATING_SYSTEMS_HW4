#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include "st_reactor.h"

#define PORT "9034"
Reactor *reactor;

void clientHandler(void *r, int fd)
{
    Reactor *reactor = (Reactor *)r;

    char buf[256]; // Buffer for client data
    FD_action *p;
    int nbytes = recv(fd, buf, sizeof buf, 0);              
    
    if(nbytes <= 0)
    {
        // Got error or connection closed by cient
        if(nbytes == 0)
        {
            // Connection closed
            printf("pollserver: socket %d hung up\n", fd);
        }
        else
        {
            perror("recv");
        }

        close(fd); // Bye!
        removeFD(reactor, fd);
        // def_from_pfds(pfds, i, &fd_count);
    }
    else
    {
        // We got some good data from a client
        for(p=reactor->head; p != NULL; p = p->next)
        {
            // Send to everyone!
            int dest_fd = p->fd;

            // Except the listener and ourselves...
            if(dest_fd != fd)
            {
                if(send(dest_fd, buf, nbytes, 0) == -1)
                {
                    perror("send");
                }
            }
        }
    }
}


// Get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    // IPv4
    if(sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    
    // IPv6
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}


// Return a listening socket
int get_listener_socket()
{
    int listener; // Listening socket descriptor
    int yes = 1;  // For setsockopt() SO_REUSEADDR, below
    int rv;

    struct addrinfo hints, *ai, *p;

    // Get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0)
    {
        fprintf(stderr, "pollserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for(p = ai; p != NULL; p = p->ai_next)
    {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(listener < 0) { continue; }

        // Lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if(bind(listener, p->ai_addr, p->ai_addrlen) < 0)
        {
            close(listener);
            continue;
        }

        break;
    }

    freeaddrinfo(ai); // All done with this

    // If we got here, it means we didnt get bound
    if(p == NULL)
    {
        return -1;
    }

    // Listen
    if(listen(listener, 10) == -1)
    {
        return -1;
    }

    return listener;
}


int main()
{
    int listener; // Listening socket descirptor

    int newfd;    // Newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // Client address
    socklen_t addrlen;

    char remoteIP[INET6_ADDRSTRLEN];

    reactor = (Reactor *)createReactor();
    startReactor(reactor);

    // Set up and get the listener socket
    listener = get_listener_socket();
    if(listener == -1)
    {
        fprintf(stderr, "error getting listening socket\n");
        exit(1);
    }

    // Main loop
    for(;;)
    {
        
        addrlen = sizeof remoteaddr;
        newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);

        if(newfd == -1)
        {
            perror("accept");
            exit(1);
        }
        else
        {
            addFD(reactor, newfd, clientHandler);

            printf("pollserver: new connection from %s on socket %d\n", inet_ntop(remoteaddr.ss_family,
                                                                                    get_in_addr((struct sockaddr *)&remoteaddr),
                                                                                    remoteIP, INET6_ADDRSTRLEN), newfd);
        }
    }

    return 0;
}