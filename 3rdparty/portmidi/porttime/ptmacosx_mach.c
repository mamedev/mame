/* ptmacosx.c -- portable timer implementation for mac os x */

#include <stdlib.h>
#include <stdio.h>
#include <CoreAudio/HostTime.h>

#import <mach/mach.h>
#import <mach/mach_error.h>
#import <mach/mach_time.h>
#import <mach/clock.h>
#include <unistd.h>
#include <AvailabilityMacros.h>

#include "porttime.h"
#include "sys/time.h"
#include "pthread.h"

#ifndef NSEC_PER_MSEC
#define NSEC_PER_MSEC 1000000
#endif
#define THREAD_IMPORTANCE 63

/* QOS headers are available as of macOS 10.10 */
#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101000
#include "sys/qos.h"
#define HAVE_APPLE_QOS 1
#else
#undef HAVE_APPLE_QOS
#endif

static int time_started_flag = FALSE;
static UInt64 start_time;
static pthread_t pt_thread_pid;

/* note that this is static data -- we only need one copy */
typedef struct {
    int id;
    int resolution;
    PtCallback *callback;
    void *userData;
} pt_callback_parameters;

static int pt_callback_proc_id = 0;

static void *Pt_CallbackProc(void *p)
{
    pt_callback_parameters *parameters = (pt_callback_parameters *) p;
    int mytime = 1;

    kern_return_t error;
    thread_extended_policy_data_t extendedPolicy;
    thread_precedence_policy_data_t precedencePolicy;

    extendedPolicy.timeshare = 0;
    error = thread_policy_set(mach_thread_self(), THREAD_EXTENDED_POLICY,
                              (thread_policy_t)&extendedPolicy,
                              THREAD_EXTENDED_POLICY_COUNT);
    if (error != KERN_SUCCESS) {
        mach_error("Couldn't set thread timeshare policy", error);
    }

    precedencePolicy.importance = THREAD_IMPORTANCE;
    error = thread_policy_set(mach_thread_self(), THREAD_PRECEDENCE_POLICY,
                              (thread_policy_t)&precedencePolicy,
                              THREAD_PRECEDENCE_POLICY_COUNT);
    if (error != KERN_SUCCESS) {
        mach_error("Couldn't set thread precedence policy", error);
    }
    
    /* Most important, set real-time constraints.
       Define the guaranteed and max fraction of time for the audio thread.
       These "duty cycle" values can range from 0 to 1.  A value of 0.5
       means the scheduler would give half the time to the thread.
       These values have empirically been found to yield good behavior.
       Good means that audio performance is high and other threads won't starve.
    */
    const double kGuaranteedAudioDutyCycle = 0.75;
    const double kMaxAudioDutyCycle = 0.85;
    
    /* Define constants determining how much time the audio thread can
       use in a given time quantum.  All times are in milliseconds.
    */
    /* About 128 frames @44.1KHz */
    const double kTimeQuantum = 2.9;
    
    /* Time guaranteed each quantum. */
    const double kAudioTimeNeeded = kGuaranteedAudioDutyCycle * kTimeQuantum;
    
    /* Maximum time each quantum. */
    const double kMaxTimeAllowed = kMaxAudioDutyCycle * kTimeQuantum;
    
    /* Get the conversion factor from milliseconds to absolute time
       which is what the time-constraints call needs.
    */
    mach_timebase_info_data_t tb_info;
    mach_timebase_info(&tb_info);
    double ms_to_abs_time =
    ((double)tb_info.denom / (double)tb_info.numer) * 1000000;
    
    thread_time_constraint_policy_data_t time_constraints;
    time_constraints.period = (uint32_t)(kTimeQuantum * ms_to_abs_time);
    time_constraints.computation = (uint32_t)(kAudioTimeNeeded * ms_to_abs_time);
    time_constraints.constraint = (uint32_t)(kMaxTimeAllowed * ms_to_abs_time);
    time_constraints.preemptible = 0;
    
    error = thread_policy_set(mach_thread_self(),
                               THREAD_TIME_CONSTRAINT_POLICY,
                               (thread_policy_t)&time_constraints,
                               THREAD_TIME_CONSTRAINT_POLICY_COUNT);
    if (error != KERN_SUCCESS) {
        mach_error("Couldn't set thread precedence policy", error);
    }

    /* to kill a process, just increment the pt_callback_proc_id */
    /* printf("pt_callback_proc_id %d, id %d\n", pt_callback_proc_id,
              parameters->id); */
    while (pt_callback_proc_id == parameters->id) {
        /* wait for a multiple of resolution ms */
        UInt64 wait_time;
        int delay = mytime++ * parameters->resolution - Pt_Time();
        PtTimestamp timestamp;
        if (delay < 0) delay = 0;
        wait_time = AudioConvertNanosToHostTime((UInt64)delay * NSEC_PER_MSEC);
        wait_time += AudioGetCurrentHostTime();
        mach_wait_until(wait_time);
        timestamp = Pt_Time();
        (*(parameters->callback))(timestamp, parameters->userData);
    }
    free(parameters);
    return NULL;
}


PtError Pt_Start(int resolution, PtCallback *callback, void *userData)
{
    if (time_started_flag) return ptAlreadyStarted;
    start_time = AudioGetCurrentHostTime();
    
    if (callback) {
        int res;
        pt_callback_parameters *parms;

        parms = (pt_callback_parameters *) malloc(sizeof(pt_callback_parameters));
        if (!parms) return ptInsufficientMemory;
        parms->id = pt_callback_proc_id;
        parms->resolution = resolution;
        parms->callback = callback;
        parms->userData = userData;
        
#ifdef HAVE_APPLE_QOS
        pthread_attr_t qosAttribute;
        pthread_attr_init(&qosAttribute);
        pthread_attr_set_qos_class_np(&qosAttribute, 
                                      QOS_CLASS_USER_INTERACTIVE, 0);

        res = pthread_create(&pt_thread_pid, &qosAttribute, Pt_CallbackProc, 
                             parms);
#else
        res = pthread_create(&pt_thread_pid, NULL, Pt_CallbackProc, parms);
#endif
        
        struct sched_param sp;
        memset(&sp, 0, sizeof(struct sched_param));
        sp.sched_priority = sched_get_priority_max(SCHED_RR);
        if (pthread_setschedparam(pthread_self(), SCHED_RR, &sp)  == -1) {
            return ptHostError;
        }
        
        if (res != 0) return ptHostError;
    }
    
    time_started_flag = TRUE;
    return ptNoError;
}


PtError Pt_Stop(void)
{
    /* printf("Pt_Stop called\n"); */
    pt_callback_proc_id++;
    pthread_join(pt_thread_pid, NULL);
    time_started_flag = FALSE;
    return ptNoError;
}


int Pt_Started(void)
{
    return time_started_flag;
}


PtTimestamp Pt_Time(void)
{
    UInt64 clock_time, nsec_time;
    clock_time = AudioGetCurrentHostTime() - start_time;
    nsec_time = AudioConvertHostTimeToNanos(clock_time);
    return (PtTimestamp)(nsec_time / NSEC_PER_MSEC);
}


void Pt_Sleep(int32_t duration)
{
    usleep(duration * 1000);
}
