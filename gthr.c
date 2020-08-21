#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "uthash.h"
#include "queue.c"
#include <sys/signal.h>
#include <sys/time.h>

#ifdef __APPLE__
    const int INITIAL_TICKS_REMAINING = 2;     //default wait time for IO-bound threads
    const int QUANTUM = 3;                      //duration a RR thread can run for
    const int TICK_PERIOD_S = 1;
    const int TICK_PERIOD_U = 0;
#endif

#ifdef __linux__
    #define INITIAL_TICKS_REMAINING  2     //default wait time for IO-bound threads
    #define QUANTUM 3                      //duration a RR thread can run for
    #define TICK_PERIOD_S 1
    #define TICK_PERIOD_U 0
#endif
int time_left_RR = 0;       //amount of time RR has left to run
int thread_it = 0;

struct task_tick_record 
{
    struct gt * gt_id;          /* key */
    int prio;
    int ticks_remaining;
    UT_hash_handle hh;      /* makes this structure hashable */
};

//declare the hash (table)
struct task_tick_record *threads = NULL;    /* important! initialize to NULL */

enum {
    MaxGThreads = 16,
    StackSize = 0x400000,
};

struct gt 
{
    struct gtctx {
        uint64_t rsp;
        uint64_t r15;
        uint64_t r14;
        uint64_t r13;
        uint64_t r12;
        uint64_t rbx;
        uint64_t rbp;
    } ctx;
    enum {
        Unused,
        Running,
        Ready,
    } st;

    enum {
        RR,
        FIFO,
    } policy;
    int priority;//1-99
};

struct gt gttbl[MaxGThreads];
struct gt *gtcur;

void gtinit(int prio, int policy);
void gtret(int ret);
void gtswtch(struct gtctx *old, struct gtctx *new);
bool gtyield(int wait);
bool gtyield_x(int x);
static void gtstop(void);
int gtgo(void (*f)(void), int prio, int policy);

void
gtinit( int prio, int policy)
{
    gtcur = &gttbl[0];
    gtcur->st = Running;        //won't this get reset to Ready by gtgo?
    gtcur->priority = prio;
    gtcur->policy = policy;
}

void __attribute__((noreturn))
gtret(int ret)
{
    if (gtcur != &gttbl[0]) 
    {
        gtcur->st = Unused;
        gtyield(0);
        assert(!"reachable");
    }
    //Assumption: gttbl[0] is always a CPU bound thread

    bool isEmpty = false;           //both priority queues and hash table are empty
    while (!isEmpty) 
    {
        int x = HASH_COUNT(threads);
        if (!gtyield(0)) 
        {
            if (x == 0) 
            {
                isEmpty = true;
            }
        }
    }
    destroy();          //deallocates the 99 priority queues
    exit(ret);
}

static void
gtstop(void) { gtret(0); }

int gtgo(void (*f)  (void), int prio, int policy  )
{
    char *stack;
    struct gt *p;

    for (p = &gttbl[0];; p++)
        if (p == &gttbl[MaxGThreads])
            return -1;
        else if (p->st == Unused)
            break;

    stack = malloc(StackSize);
    if (!stack)
        return -1;

    *(uint64_t *)&stack[StackSize -  8] = (uint64_t)gtstop;
    *(uint64_t *)&stack[StackSize - 16] = (uint64_t)f;              //is f invoked here?
    p->ctx.rsp = (uint64_t)&stack[StackSize - 16];
    p->st = Ready;
    if (prio<1)
        prio = 1;
    p->priority = prio;
    p->policy = policy;
    //place each READY gthread on its respective ready queue
    enqueue(p, prio);
    return 0;
}


/* Now, let's run some simple threaded code. */

void
f_io(void)
{
    int i, id;

    id = ++thread_it;
    for (i = 0; i < 10; i++) 
    {
        if (gtcur-> policy == RR)
        printf("Policy: Round Robin ");
        else
        printf("Policy: FIFO ");
        printf("thread id: %d current iteration: %d\n", id, i);
        sleep(1);                         //all RR threads will yield here if time_left_RR is 0
        if ((i%6)==0)
        gtyield(INITIAL_TICKS_REMAINING);
        else
        gtyield(0);
    }
}

void
f(void)
{
    int i, id;

    id = ++thread_it;
    for (i = 0; i < 10; i++) 
    {
    if (gtcur-> policy == RR)
        printf("Policy: Round Robin ");
    else
        printf("Policy: FIFO ");
        printf("thread id: %d current iteration: %d\n", id, i);
        sleep(1);                         //all RR threads will yield here if time_left_RR is 0
    gtyield(0);
    }
}
//finished thread calls gtyield to stay on its ready queue as Unused

//add the gthread referred to by gt_id into the waitlist (hash) with ticks_remaining
void add_gthread(struct gt *gt_id, int prio, int ticks_remaining) 
{
    struct task_tick_record* s;

    s = (struct task_tick_record *) malloc(sizeof(struct task_tick_record));
    s->gt_id = gt_id;
    s->prio  = prio;
    s->ticks_remaining = ticks_remaining;
    HASH_ADD(hh, threads, gt_id, sizeof(gt_id), s);
}

//I/O gthread yields to anything!
//Right now: if gtcur (FIFO IO version) finds another gthread of same priority, then gtcur goes onto waitlist (i.e. behaves like RR)
//I/O bound gthread goes to the waitlist
//pre: gtcur is NOT in any of the priority queues 0-99
bool gtyield_x (int x) 
{
    struct gt *p;
    struct gtctx *old, *new;

    p = gtcur;
    //top down scan of ready queues from 99 --> 0
    for (int i = NUMBER_OF_QUEUES - 1; i >= 0; i--) 
    {
        if (!isEmpty(i))  
    {
            //address of first gthread in non-empty queue with priority i
            p = peek(i);

            //can't just dequeue it and run it, what if its UNUSED or gtcur itself?
            int count = 1;      //ensure you only examine as many gthreads are in the queue
            while (p->st != Ready && count < priority_queues[i]->itemCount) 
        {
                dequeue(i);
                enqueue(p, i);
                p = peek(i);        //peek at next gthread on the queue
                count++;
            }
            if (p->st != Ready) {       //thread is UNUSED or RUNNING (gtcur)
                //examined all gthreads in this queue, move on to queue with lower priority
                continue;
            }

            //-------identified a gthread to replace the currently running thread--------------

            //remove the gthread to-be-run from the queue
            dequeue(i);

            //set the state of the currently running thread back to ready
            if (gtcur->st != Unused)
        {
                gtcur->st = Ready;
        printf("\n\n **** I/O thread sent to wait****\n\n");
                add_gthread(gtcur, gtcur->priority, INITIAL_TICKS_REMAINING);

        }
            //DIFFERENT: add gtcur to waitlist

            time_left_RR = QUANTUM;

            //perform context switch from gtcur to p
            p->st = Running;
            old = &gtcur->ctx;
            new = &p->ctx;
            gtcur = p;
            gtswtch(old, new);    //context switch out of the program now, just start f
            return true;
        }
    }
    return false;   //nothing to yield
}

//CPU-bound thread goes to the back of its respective queue
//pre: gtcur is NOT in any of the priority queues 0-99
//TODO: combine gtyield and gtyield_x into one method: gtyield(x)
bool gtyield(int time) 
{
    if (time == 0)
    {
    struct gt *p;
    struct gtctx *old, *new;

    //Checks RR schedule to be sure its quantum has finished

    p = gtcur;
    //top down scan of ready queues from 99 --> 0
    for (int i = NUMBER_OF_QUEUES - 1; i >= 0; i--) 
    {
        //if gtcur is FIFO and still running, only consider gthreads of > priority to yield to
        if (gtcur->policy == FIFO) 
    {
            if (i <= gtcur->priority && gtcur->st != Unused) 
        {
                return false;
            }
        }
        //if gtcur is RR and still running, only consider gthreads of >= priority to yield to
        else if (gtcur->policy == RR) 
    {
            if (i < gtcur->priority && gtcur->st != Unused) 
        {
                return false;
            }
        if (i == gtcur->priority && gtcur->st != Unused)
    {
            //Here, we are checking possible gthreads of equal priority, we check for our quantum being done first
            if (gtcur->policy == RR && time_left_RR != 0 && gtcur->st != Unused)
            {
            return false;
            }
        }
        }

        //at this point, either gtcur is done (i.e. Unused) OR or RR considering >= priority gthreads or FIFO considering > priority gthreads
        if (!isEmpty(i))  
    {
            //address of first gthread in non-empty queue with priority i
            p = peek(i);

            int count = 1;      //ensure you only examine as many gthreads are in the queue
            while (p->st != Ready && count < priority_queues[i]->itemCount) 
        {
                dequeue(i);
                enqueue(p, i);
                p = peek(i);        //peek at next gthread on the queue
                count++;
            }
            if (p->st != Ready) {       //thread is UNUSED or RUNNING (gtcur)
                //examined all gthreads in this queue, move on to queue with lower priority
                continue;
            }

            //-------identified a gthread to replace the currently running thread--------------

            //remove the addr of the gthread to-be-run from the queue
            dequeue(i);

            //if gtcur is done running f, its state is Unused
            if (gtcur->st != Unused)
                gtcur->st = Ready;
            //DIFFERENT: put gtcur on the back of its respective queue (not onto waitlist)
            enqueue(gtcur, gtcur->priority);

            time_left_RR = QUANTUM;

            //perform context switch from gtcur to p
            p->st = Running;
            old = &gtcur->ctx;
            new = &p->ctx;
            gtcur = p;
            gtswtch(old, new);    //context switch out of the program now, just start f
            return true;
        }
    }
    return false;   //nothing to yield
    }
    else
    {
        return gtyield_x(time);
    }
}



//represents a tick
//invoked every TICK_PERIOD_S
void sigFunc(int sig) 
{
    struct task_tick_record * s, *tmp;

    //decrement the ticks remaining on all threads in the hash
    HASH_ITER(hh, threads, s, tmp) 
    {
        s->ticks_remaining--;
        if (s->ticks_remaining == 0) 
    {
            //move I/O task back to its respective priority queue
            enqueue(s->gt_id, s->prio);

            //remove this entry from the hash
            HASH_DEL(threads, s);
            free(s);
        }
    }
    time_left_RR--;
}
