#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <poll.h>


typedef void (*handler_t)(int fd);

// Node
typedef struct FD_action
{
    int fd;
    handler_t handler;
    struct pollfd pfd[1];   // just ONE pointer to pollfd.
    struct FD_action *next;

} FD_action;


// LinkedList
typedef struct Reactor
{
    FD_action *head;
    struct pollfd *pfds; // array saves all 'pfd' in every FD_action
    int size;
    bool is_on;
    pthread_t ptid;

} Reactor;


void* react(void *args)
{
    Reactor *reacrtor = (Reactor *)args;
    FD_action *p;

    for(;;)
    {
        if(poll(reacrtor->pfds, reacrtor->size, 2500) == -1)
        {
            perror("poll");
            exit(1);
        }
        
        for(p = reacrtor->head; p != NULL; p=p->next)
        {
            if(p->pfd[0].revents != POLLIN) continue;

            p->handler(p->fd);
        }
    }

    return NULL;
}


void *createReactor()
{
    Reactor *reactor = (Reactor*)malloc(sizeof(Reactor));
    reactor->head = NULL;
    reactor->size = 0;

    return reaction;
}


void stopReactor(Reactor *this)
{
    if(this->is_on)
    {
        pthread_cancel(this->ptid);
        this->is_on = false;
    }
}


void startReactor(Reactor *this)
{
    if(!this->is_on)
    {
        pthread_create(&(this->ptid), NULL, &react, this);
        this->is_on = true;
    }
}


void removeFD(void *this, int fd)
{
    // TODO
}


void addFD(void *this, int fd, handler_t handler)
{
    // TODO
}


void waitFor(void *this)
{
    if(this->is_on)
    {
        pthread_join(this->ptid);
        this->is_on = false;
    }
}