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


FD_action* create_FD_action(int fd, handler_t handler)
{
    FD_action *new_FD_action = (FD_action*)malloc(sizeof(FD_action));
    new_FD_action->fd = fd;
    new_FD_action->handler = handler;
    new_FD_action->next = NULL;

    return new_FD_action;
}


void *createReactor()
{
    Reactor *reactor = (Reactor*)malloc(sizeof(Reactor));
    reactor->head = NULL;
    reactor->pfds = NULL;
    reactor->size = 0;
    reactor->is_on = false;

    return reaction;
}


void stopReactor(void *this)
{
    Reactor *reactor = (Reactor *)this;
    
    if(reactor->is_on)
    {
        pthread_cancel(reactor->ptid);
        reactor->is_on = false;
    }
}


void startReactor(void *this)
{
    Reactor *reactor = (Reactor *)this;

    if(!reactor->is_on)
    {
        pthread_create(&(reactor->ptid), NULL, &react, reactor);
        reactor->is_on = true;
    }
}


void removeFD(void *this, int fd)
{
    // TODO
}


void addFD(void *this, int fd, handler_t handler)
{
    Reactor *reactor = (Reactor *)this;

    FD_action *new_FD_action = create_FD_action(fd, handler);


}


void waitFor(void *this)
{
    Reactor *reactor = (Reactor *)this;
    
    if(reactor->is_on)
    {
        pthread_join(reactor->ptid);
        reactor->is_on = false;
    }
}