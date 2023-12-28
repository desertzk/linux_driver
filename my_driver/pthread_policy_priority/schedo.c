#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <assert.h>

static int get_thread_policy(pthread_attr_t *attr)
{
    int plicy;
    int rs=pthread_attr_getschedpolicy(attr,&plicy);

    assert(rs==0); 

    switch(plicy)
    {
        case SCHED_FIFO:
        printf("policy=SCHED_FIFO.\n");
        break;

        case SCHED_RR:
        printf("policy=SCHED_RR.\n");
        break;

        case SCHED_OTHER:
        printf("policy=SCHED_OTHER.\n");
        break;

        default:
        printf("policy=UNKNOWN.\n");
        break;
    }

    return plicy;
}

static void show_thread_priority(pthread_attr_t *attr,int policy)
{
    int priority=sched_get_priority_max(policy); 
    assert(priority!=-1);
    printf("max_priority=%d\n",priority);

    priority=sched_get_priority_min(policy); 
    assert(priority!=-1);
    printf("min_priority=%d\n",priority);
}


static int get_thread_priority(pthread_attr_t *attr)
{
    struct sched_param param;
    int rs=pthread_attr_getschedparam(attr,&param);
    assert(rs==0);

    printf("priority=%d",param.__sched_priority);


    return param.__sched_priority;
}


static void set_thread_policy(pthread_attr_t *attr,int policy)
{
    int rs=pthread_attr_setschedpolicy(attr,policy);

    assert(0==rs);

    get_thread_policy(attr);

}

int main()
{
    pthread_attr_t attr;
    struct sched_param sched;

    int rs=pthread_attr_init(&attr);
    assert(0==rs);

    int plicy=get_thread_policy(&attr);
    printf("output current configuration of priority.\n");
    show_thread_priority(&attr,plicy);

    printf("output SCHED_FIFO of priority.\n");
    show_thread_priority(&attr,SCHED_FIFO);

    printf("output SCHED_RR of priority.\n");
    show_thread_priority(&attr,SCHED_RR);
    printf("output priority of current thread.\n");

    int priority=get_thread_priority(&attr);
    printf("set thrad policy.\n");
    printf("set SCHED_FIFO polity.\n");
    set_thread_policy(&attr,SCHED_FIFO); 

    printf("set SCHED_RR policy.\n");
    set_thread_policy(&attr,SCHED_RR); 
    printf("restore current policy.\n");
    set_thread_policy(&attr,plicy);

    rs=pthread_attr_destroy(&attr);
    assert(0==rs);

    return 0;
}























