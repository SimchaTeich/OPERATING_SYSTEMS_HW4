#include "st_reactor.h"


void* react(void *args)
{
    printf("inside react()\n");
    Reactor *reactor = (Reactor *)args;
    FD_action *p;

    for(;;)
    {
        if(poll(reactor->pfds, reactor->count, 3000) == -1)
        {
            perror("poll");
            exit(1);
        }
        
        for(p = reactor->head; p != NULL; p = p->next)
        {
            if(p->pfd->revents != POLLIN) continue;
            p->handler(reactor, p->fd);
        }
    }

    return NULL;
}


FD_action* create_FD_action(int fd, handler_t handler)
{
    FD_action *new_FD_action = (FD_action*)malloc(sizeof(FD_action));
    new_FD_action->fd = fd;
    new_FD_action->handler = handler;
    new_FD_action->pfd = NULL; // update outside!
    new_FD_action->next = NULL;

    return new_FD_action;
}


void *createReactor()
{
    Reactor *reactor = (Reactor*)malloc(sizeof(Reactor));
    reactor->head = NULL;
    reactor->size = 5;
    reactor->count = 0;
    reactor->pfds = (struct pollfd *)malloc(sizeof(struct pollfd) * reactor->size);;
    reactor->is_on = false;

    return reactor;
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

// Remove an index fron the set
void def_from_pfds(FD_action *fd_actions, struct pollfd pfds[], int fd, int *fd_count)
{
    // find index for the fd will remove:
    int i;
    for(i = 0; i < *fd_count; i++)
    {
        if(pfds[i].fd == fd) break;
    }

    // Copy the one from the end over this one
    pfds[i] = pfds[*fd_count-1];

    // updates first FD_action about new place of pfd (becaue last place was befor...)
    fd_actions->pfd =  &pfds[i];
    

    (*fd_count)--;
}


void removeFD(void *this, int fd)
{
    Reactor *reactor = (Reactor *)this;

    // remove from pollfd from pfds array
    def_from_pfds(reactor->head, reactor->pfds, fd, &(reactor->count));

    // TODO: remove FD_action from linkedList
    FD_action *curr = reactor->head;

    if(curr->fd == fd)
    {
        reactor->head =  curr->next;
        free(curr);
    }
    else
    {
        while(curr->next && curr->next->fd != fd)
        {
            curr = curr->next;
        }

        FD_action *remove = curr->next;
        curr->next = curr->next->next;
        if(remove)
        {
            free(remove);
        }
    }
}


void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size, Reactor *r)
{
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size)
    {
        *fd_size *= 2; // Double it

        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));

        // updates pfd-s of all FD_actions
        FD_action *curr = r->head;
        int i = 0;
        while(curr != NULL)
        {
            curr->pfd = &((*pfds)[r->count - i]);
            curr = curr->next;
            i++;
        }
    }

    (*pfds)[*fd_count].fd = newfd;
    (*pfds)[*fd_count].events = POLLIN; // Check ready-to-read


    (*fd_count)++;
}


void addFD(void *this, int fd, handler_t handler)
{
    Reactor *reactor = (Reactor *)this;

    // insert new node to the head of the list.
    FD_action *new_FD_action = create_FD_action(fd, handler);
    new_FD_action->next = reactor->head;
    reactor->head = new_FD_action;

    // create and insert new pollfd into array: reactor->pfds
    add_to_pfds(&(reactor->pfds), fd, &(reactor->count), &(reactor->size), reactor);

    // take the poiner to the new pollfd and assignment into reactor->head->pfd
    reactor->head->pfd = &(reactor->pfds[reactor->count - 1]);
}


void waitFor(void *this)
{
    Reactor *reactor = (Reactor *)this;
    
    if(reactor->is_on)
    {
        pthread_join(reactor->ptid, NULL);
        reactor->is_on = false;
    }
}