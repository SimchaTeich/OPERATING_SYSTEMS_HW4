#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <poll.h>

typedef void (*handler_t)(int f
                              d);

// Node
typedef struct FD_action
{
    int fd;
    handler_t handler;
    struct pollfd *pfd; // just ONE pointer to pollfd.
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

void *react(void *args)
{
    Reactor *reacrtor = (Reactor *)args;
    FD_action *p;

    for (;;)
    {
        if (poll(reacrtor->pfds, reacrtor->size, 2500) == -1)
        {
            perror("poll");
            exit(1);
        }

        for (p = reacrtor->head; p != NULL; p = p->next)
        {
            if (p->pfd[0].revents != POLLIN)
                continue;

            p->handler(p->fd);
        }
    }

    return NULL;
}

FD_action *create_FD_action(int fd, handler_t handler)
{
    FD_action *new_FD_action = (FD_action *)malloc(sizeof(FD_action));
    new_FD_action->fd = fd;
    new_FD_action->handler = handler;
    new_FD_action->pfd = NULL; // update outside!
    new_FD_action->next = NULL;

    return new_FD_action;
}

void *createReactor()
{
    Reactor *reactor = (Reactor *)malloc(sizeof(Reactor));
    reactor->head = NULL;
    reactor->pfds = NULL;
    reactor->size = 0;
    reactor->is_on = false;

    return reaction;
}

void stopReactor(void *this)
{
    Reactor *reactor = (Reactor *)this;

    if (reactor->is_on)
    {
        pthread_cancel(reactor->ptid);
        reactor->is_on = false;
    }
}

void startReactor(void *this)
{
    Reactor *reactor = (Reactor *)this;

    if (!reactor->is_on)
    {
        pthread_create(&(reactor->ptid), NULL, &react, reactor);
        reactor->is_on = true;
    }
}

void removeFD(void *this, int fd)
{
    // TODO: DONE - i thought.
    Reactor *reactor = (Reactor *)this;
    // 1. keep the top of list(head)
    FD_action *current = reactor->head;
    // 2. create none(NULL) FD to point on him
    // if exsist what we need to remove(target)
    FD_action *prevois = NULL;
    // this idea is basic that we was handle linked list delete function :)
    while (current != NULL)
    {
        if (current->fd = fd)
        {
            if (prevois == NULL)
            {
                reactor->head = current->next;
            }
            else
            {
                prevois->next = current->next;
            }
            // delete
            free(current);
            // i think that after free we done with that and nothing to do ?
            //-wrote just for verification and simulate to track write logic :)
            return;
        }
        prevois = current;
        // get next
        current = current->next;
    }
}
// TODO
void addFD(void *this, int fd, handler_t handler)
{
    Reactor *reactor = (Reactor *)this;

    // insert new node to the head of the list.
    FD_action *new_FD_action = create_FD_action(fd, handler);
    new_FD_action->next = reactor->head;
    reactor->head = new_FD_action;

    // TODO: create and insert new pollfd inpt array: reactor->pfds
    // TODO: take a poiner to the new pollfd and assignment into reactor->head->pfd

    reactor->size++;
}

void waitFor(void *this)
{
    Reactor *reactor = (Reactor *)this;

    if (reactor->is_on)
    {
        pthread_join(reactor->ptid);
        reactor->is_on = false;
    }
}