#include "st_reactor.h"



void printLinkedList(FD_action *head)
{
    while(head != NULL)
    {
        printf("[fd %d, indx: %d]-->", head->fd, head->pfd_index);
        head = head->next;
    }
    printf("NULL\n");
}



void* react(void *args)
{
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
            if(reactor->pfds[p->pfd_index].revents != POLLIN) continue;
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
    new_FD_action->pfd_index = -1; // update later!
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

    // updates correct FD_action about new place of pfd (becaue last place was befor...)
    FD_action *curr = fd_actions;
    while(curr != NULL)
    {
        if(curr->pfd_index == (*fd_count-1))
        {
            curr->pfd_index = i;
            break;
        }
        curr = curr->next;
    }
    
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
        reactor->head = curr->next;
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

    printLinkedList(reactor->head);
}


void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size, Reactor *r)
{
    // If we don't have room, add more space in the pfds array
    if (*fd_count == *fd_size)
    {
        *fd_size *= 2; // Double it

        *pfds = realloc(*pfds, sizeof(**pfds) * (*fd_size));
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

    // update index of new pollfd into FD_action node.
    reactor->head->pfd_index = reactor->count - 1;

    printLinkedList(reactor->head);
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