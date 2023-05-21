#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <poll.h>

typedef void (*handler_t)(void *reactor, int fd);


// Node
typedef struct FD_action
{
    int fd;
    handler_t handler;
    int pfd_index;          // index to fd inside Reactor->pfds..
    struct FD_action *next;

} FD_action;

// LinkedList
typedef struct Reactor
{
    FD_action *head;
    struct pollfd *pfds; // array saves all 'pfd' in every FD_action
    int count;
    int size;
    bool is_on;
    pthread_t ptid;

} Reactor;


void* react(void *args);
FD_action* create_FD_action(int fd, handler_t handler);
void *createReactor();
void stopReactor(void *this);
void startReactor(void *this);
void def_from_pfds(FD_action *fd_actions, struct pollfd pfds[], int fd, int *fd_count);
void removeFD(void *this, int fd);
void add_to_pfds(struct pollfd *pfds[], int newfd, int *fd_count, int *fd_size, Reactor *r);
void addFD(void *this, int fd, handler_t handler);
void waitFor(void *this);