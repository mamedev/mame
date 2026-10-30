/** @file pmutil.h lock-free queue for building MIDI 
                   applications with PortMidi. 

    PortMidi is not reentrant, and locks can suffer from priority
    inversion.  To support coordination between system callbacks, a
    high-priority thread created with PortTime, and the main
    application thread, PortMidi uses a lock-free, non-blocking
    queue. The queue implementation is not particular to MIDI and is
    available for other uses.
 */

#ifndef PORTMIDI_PMUTIL_H
#define PORTMIDI_PMUTIL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** @defgroup grp_pmutil Lock-free Queue
    @{
*/

/** The queue representation is opaque. Declare a queue as PmQueue * */
typedef void PmQueue;

/** create a single-reader, single-writer queue. 

    @param num_msgs the number of messages the queue can hold

    @param bytes_per_msg the fixed message size

    @return the allocated and initialized queue, or NULL if memory
    cannot be allocated. Allocation uses pm_alloc().

    The queue only accepts fixed sized messages. 

    This queue implementation uses the "light pipe" algorithm which
    operates correctly even with multi-processors and out-of-order
    memory writes. (see Alexander Dokumentov, "Lock-free Interprocess
    Communication," Dr. Dobbs Portal, http://www.ddj.com/,
    articleID=189401457, June 15, 2006. This algorithm requires that
    messages be translated to a form where no words contain
    zeros. Each word becomes its own "data valid" tag. Because of this
    translation, we cannot return a pointer to data still in the queue
    when the "peek" method is called. Instead, a buffer is
    preallocated so that data can be copied there. Pm_QueuePeek()
    dequeues a message into this buffer and returns a pointer to it. A
    subsequent Pm_Dequeue() will copy from this buffer.

    This implementation does not try to keep reader/writer data in
    separate cache lines or prevent thrashing on cache lines. 
    However, this algorithm differs by doing inserts/removals in
    units of messages rather than units of machine words. Some
    performance improvement might be obtained by not clearing data
    immediately after a read, but instead by waiting for the end
    of the cache line, especially if messages are smaller than
    cache lines. See the Dokumentov article for explanation.

    The algorithm is extended to handle "overflow" reporting. To
    report an overflow, the sender writes the current tail position to
    a field.  The receiver must acknowlege receipt by zeroing the
    field. The sender will not send more until the field is zeroed.
 */
PMEXPORT PmQueue *Pm_QueueCreate(long num_msgs, int32_t bytes_per_msg);
    
/** destroy a queue and free its storage. 

    @param queue a queue created by #Pm_QueueCreate().

    @return pmNoError or an error code.

    Uses pm_free(). 

 */
PMEXPORT PmError Pm_QueueDestroy(PmQueue *queue);

/** remove one message from the queue, copying it into \p msg.

    @param queue a queue created by #Pm_QueueCreate().

    @param msg address to which the message, if any, is copied.

    @return 1 if successful, and 0 if the queue is empty.  Returns
    #pmBufferOverflow if what would have been the next thing in the
    queue was dropped due to overflow. (So when overflow occurs, the
    receiver can receive a queue full of messages before getting the
    overflow report. This protocol ensures that the reader will be
    notified when data is lost due to overflow.
 */
PMEXPORT PmError Pm_Dequeue(PmQueue *queue, void *msg);

/** insert one message into the queue, copying it from \p msg.

    @param queue a queue created by #Pm_QueueCreate().

    @param msg address of the message to be enqueued.

    @return #pmNoError if successful and #pmBufferOverflow if the
    queue was already full. If #pmBufferOverflow is returned, the
    overflow flag is set.
 */
PMEXPORT PmError Pm_Enqueue(PmQueue *queue, void *msg);

/** test if the queue is full.

    @param queue a queue created by #Pm_QueueCreate().

    @return non-zero iff the queue is empty, and #pmBadPtr if \p queue
    is NULL.

    The full condition may change immediately because a parallel
    dequeue operation could be in progress.  The result is
    pessimistic: if it returns false (zero) to the single writer, then
    #Pm_Enqueue() is guaranteed to succeed.
 */
PMEXPORT int Pm_QueueFull(PmQueue *queue);

/** test if the queue is empty.

    @param queue a queue created by #Pm_QueueCreate().

    @return zero iff the queue is either empty or NULL.

    The empty condition may change immediately because a parallel
    enqueue operation could be in progress. Furthermore, the 
    result is optimistic: it may say false, when due to 
    out-of-order writes, the full message has not arrived. Therefore,
    #Pm_Dequeue() could still return 0 after #Pm_QueueEmpty() returns
    false.
*/
PMEXPORT int Pm_QueueEmpty(PmQueue *queue);

/** get a pointer to the item at the head of the queue.

    @param queue a queue created by #Pm_QueueCreate().

    @result a pointer to the head message or NULL if the queue is empty.

    The message is not removed from the queue.  #Pm_QueuePeek() will
    not indicate when an overflow occurs. If you want to get and check
    #pmBufferOverflow messages, use the return value of
    #Pm_QueuePeek() *only* as an indication that you should call
    #Pm_Dequeue(). At the point where a direct call to #Pm_Dequeue()
    would return #pmBufferOverflow, #Pm_QueuePeek() will return NULL,
    but internally clear the #pmBufferOverflow flag, enabling
    #Pm_Enqueue() to resume enqueuing messages. A subsequent call to
    #Pm_QueuePeek() will return a pointer to the first message *after*
    the overflow.  Using this as an indication to call #Pm_Dequeue(),
    the first call to #Pm_Dequeue() will return #pmBufferOverflow. The
    second call will return success, copying the same message pointed
    to by the previous #Pm_QueuePeek().

    When to use #Pm_QueuePeek(): (1) when you need to look at the message
    data to decide who should be called to receive it. (2) when you need
    to know a message is ready but cannot accept the message.

    Note that #Pm_QueuePeek() is not a fast check, so if possible, you 
    might as well just call #Pm_Dequeue() and accept the data if it is there.
 */
PMEXPORT void *Pm_QueuePeek(PmQueue *queue);

/** allows the writer (enqueuer) to signal an overflow
    condition to the reader (dequeuer). 

    @param queue a queue created by #Pm_QueueCreate().

    @return #pmNoError if overflow is set, or #pmBadPtr if queue is
    NULL, or #pmBufferOverflow if buffer is already in an overflow
    state.

    E.g., when transfering data from the OS to an application, if the
    OS indicates a buffer overrun, #Pm_SetOverflow() can be used to
    insure that the reader receives a #pmBufferOverflow result from
    #Pm_Dequeue().
 */
PMEXPORT PmError Pm_SetOverflow(PmQueue *queue);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PORTMIDI_PMUTIL_H */
