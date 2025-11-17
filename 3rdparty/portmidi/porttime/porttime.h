/** @file porttime.h portable interface to millisecond timer. */

/* CHANGE LOG FOR PORTTIME
  10-Jun-03 Mark Nelson & RBD
    boost priority of timer thread in ptlinux.c implementation
 */

/* Should there be a way to choose the source of time here? */

#ifdef WIN32
#ifndef INT32_DEFINED
// rather than having users install a special .h file for windows, 
// just put the required definitions inline here. portmidi.h uses
// these too, so the definitions are (unfortunately) duplicated there
typedef int int32_t;
typedef unsigned int uint32_t;
#define INT32_DEFINED
#endif
#else
#include <stdint.h> // needed for int32_t
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PMEXPORT
#ifdef _WINDLL
#define PMEXPORT __declspec(dllexport)
#else
#define PMEXPORT 
#endif
#endif

/** @defgroup grp_porttime PortTime: Millisecond Timer
    @{
*/

typedef enum {
    ptNoError = 0,         /* success */
    ptHostError = -10000,  /* a system-specific error occurred */
    ptAlreadyStarted,      /* cannot start timer because it is already started */
    ptAlreadyStopped,      /* cannot stop timer because it is already stopped */
    ptInsufficientMemory   /* memory could not be allocated */
} PtError; /**< @brief @enum  PtError PortTime error code; a common return type.
            * No error is indicated by zero; errors are indicated by < 0.
            */

/** real time or time offset in milliseconds. */
typedef int32_t PtTimestamp;

/** a function that gets a current time */
typedef void (PtCallback)(PtTimestamp timestamp, void *userData);

/** start a real-time clock service.

    @param resolution the timer resolution in ms. The time will advance every
    \p resolution ms.

    @param callback a function pointer to be called every resolution ms.

    @param userData is passed to \p callback as a parameter.

    @return #ptNoError on success. See #PtError for other values.
*/
PMEXPORT PtError Pt_Start(int resolution, PtCallback *callback, void *userData);

/** stop the timer.

    @return #ptNoError on success. See #PtError for other values.
*/
PMEXPORT PtError Pt_Stop(void);

/** test if the timer is running.
 
    @return TRUE or FALSE
*/
PMEXPORT int Pt_Started(void);

/** get the current time in ms.

    @return the current time
*/
PMEXPORT PtTimestamp Pt_Time(void);

/** pauses the current thread, allowing other threads to run.

    @param duration the length of the pause in ms. The true duration 
    of the pause may be rounded to the nearest or next clock tick
    as determined by resolution in #Pt_Start().
*/
PMEXPORT void Pt_Sleep(int32_t duration);

/** @} */

#ifdef __cplusplus
}
#endif
