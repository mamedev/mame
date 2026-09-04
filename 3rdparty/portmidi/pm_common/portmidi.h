#ifndef PORTMIDI_PORTMIDI_H
#define PORTMIDI_PORTMIDI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
 * PortMidi Portable Real-Time MIDI Library
 * PortMidi API Header File
 * Latest version available at: http://sourceforge.net/projects/portmedia
 *
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 * Copyright (c) 2001-2006 Roger B. Dannenberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortMidi license; however, 
 * the PortMusic community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also
 * requested that these non-binding requests be included along with the 
 * license above.
 */

/* CHANGELOG FOR PORTMIDI
 *     (see ../CHANGELOG.txt)
 *
 * NOTES ON HOST ERROR REPORTING: 
 *
 *    PortMidi errors (of type PmError) are generic,
 *    system-independent errors.  When an error does not map to one of
 *    the more specific PmErrors, the catch-all code pmHostError is
 *    returned. This means that PortMidi has retained a more specific
 *    system-dependent error code. The caller can get more information
 *    by calling Pm_GetHostErrorText() to get a text string describing
 *    the error. Host errors can arise asynchronously from callbacks,
 *    * so there is no specific return code. Asynchronous errors are
 *    checked and reported by Pm_Poll. You can also check by calling
 *    Pm_HasHostError().  If this returns TRUE, Pm_GetHostErrorText()
 *    will return a text description of the error.
 *
 * NOTES ON COMPILE-TIME SWITCHES
 *
 *    DEBUG assumes stdio and a console. Use this if you want
 *        automatic, simple error reporting, e.g. for prototyping. If
 *        you are using MFC or some other graphical interface with no
 *        console, DEBUG probably should be undefined.
 *    PM_CHECK_ERRORS more-or-less takes over error checking for
 *        return values, stopping your program and printing error
 *        messages when an error occurs. This also uses stdio for
 *        console text I/O. You can selectively disable this error
 *        checking by declaring extern int pm_check_errors; and 
 *        setting pm_check_errors = FALSE; You can also reenable.
 */
/** 
    \defgroup grp_basics Basic Definitions
    @{
*/

#include <stdint.h>

#ifdef _WINDLL
#define PMEXPORT __declspec(dllexport)
#else
#define PMEXPORT 
#endif

#ifndef FALSE
    #define FALSE 0
#endif
#ifndef TRUE
    #define TRUE 1
#endif

/** default size of buffers for sysex transmission: */
#define PM_DEFAULT_SYSEX_BUFFER_SIZE 1024


typedef enum {
    pmNoError = 0, /**< Normal return value indicating no error. */
    pmNoData = 0, /**< @brief No error, also indicates no data available.
                   * Use this constant where a value greater than zero would
                   * indicate data is available.
                   */
    pmGotData = 1, /**< A "no error" return also indicating data available. */
    pmHostError = -10000,  /**< error was returned from the system level below
                                PortMidi. See ##Pm_GetHostErrorText() */
    pmInvalidDeviceId, /**< Out of range or 
                        * output device when input is requested or 
                        * input device when output is requested or
                        * device is already opened. 
                        */
    pmInsufficientMemory,
    pmBufferTooSmall,
    pmBufferOverflow, /**< buffer overflow (see #Pm_Read) */
    pmBadPtr, /**< #PortMidiStream parameter is NULL or
               * stream is not opened or
               * stream is output when input is required or
               * stream is input when output is required. */
    pmBadData, /**< Illegal midi data, e.g., missing EOX. */
    pmInternalError,
    pmBufferMaxSize, /**< Buffer is already as large as it can be. */
    pmNotImplemented, /**< The function is not implemented, nothing was done. */
    pmInterfaceNotSupported, /**< The requested interface is not supported. */
    pmNameConflict, /**< Cannot create virtual device because name is taken. */
    pmDeviceRemoved  /**< Output attempted after (USB) device was removed. */
    /* NOTE: If you add a new error type, you must update Pm_GetErrorText(). */
} PmError; /**< @brief @enum PmError PortMidi error code; a common return type. 
            * No error is indicated by zero; errors are indicated by < 0.
            */

/**
    Pm_Initialize() is the library initialization function - call this before
    using the library.

    *NOTE:* PortMidi scans for available devices when #Pm_Initialize
    is called.  To observe subsequent changes in the available
    devices, you must shut down PortMidi by calling #Pm_Terminate and
    then restart by calling #Pm_Initialize again. *IMPORTANT*: On
    MacOS, #Pm_Initialize *must* always be called on the same
    thread. Otherwise, changes in the available MIDI devices will
    *not* be seen by PortMidi. As an example, if you start PortMidi in
    a thread for processing MIDI, do not try to rescan devices by
    calling #Pm_Initialize in a GUI thread. Instead, start PortMidi
    the first time and every time in the GUI thread.  Alternatively,
    let the GUI request a restart in the MIDI thread.  (These
    restrictions only apply to macOS.) Speaking of threads, on all
    platforms, you are allowed to call #Pm_Initialize in one thread,
    yet send MIDI or poll for incoming MIDI in another
    thread. However, PortMidi is not "thread safe," which means you
    cannot allow threads to call PortMidi functions concurrently.

    @return pmNoError.

    PortMidi is designed to support multiple interfaces (such as ALSA,
    CoreMIDI and WinMM). It is possible to return pmNoError because there
    are no supported interfaces. In that case, zero devices will be 
    available.
*/
PMEXPORT PmError Pm_Initialize(void);

/**
    Pm_Terminate() is the library termination function - call this after
    using the library.
*/
PMEXPORT PmError Pm_Terminate(void);

/** Represents an open MIDI device. */
typedef void PortMidiStream;

/** A shorter form of #PortMidiStream. */
#define PmStream PortMidiStream

/** Test whether stream has a pending host error. Normally, the client
    finds out about errors through returned error codes, but some
    errors can occur asynchronously where the client does not
    explicitly call a function, and therefore cannot receive an error
    code.  The client can test for a pending error using
    #Pm_HasHostError(). If true, the error can be accessed by calling
    #Pm_GetHostErrorText().  Pm_Poll() is similar to Pm_HasHostError(),
    but if there is no error, it will return TRUE (1) if there is a
    pending input message.
*/
PMEXPORT int Pm_HasHostError(PortMidiStream * stream);


/** Translate portmidi error number into human readable message.
    These strings are constants (set at compile time) so client has 
    no need to allocate storage.
*/
PMEXPORT const char *Pm_GetErrorText(PmError errnum);

/** Translate portmidi host error into human readable message.
    These strings are computed at run time, so client has to allocate storage.
    After this routine executes, the host error is cleared. 
*/
PMEXPORT void Pm_GetHostErrorText(char * msg, unsigned int len);

/** Any host error msg has at most this many characters, including EOS. */
#define PM_HOST_ERROR_MSG_LEN 256u 

/** Devices are represented as small integers. Device ids range from 0
    to Pm_CountDevices()-1. Pm_GetDeviceInfo() is used to get information
    about the device, and Pm_OpenInput() and PmOpenOutput() are used to
    open the device.
*/
typedef int PmDeviceID;

/** This PmDeviceID (constant) value represents no device and may be
    returned by  Pm_GetDefaultInputDeviceID() or
    Pm_GetDefaultOutputDeviceID() if no default exists.
*/
#define pmNoDevice -1

/** MIDI device information is returned in this structure, which is
    owned by PortMidi and read-only to applications. See Pm_GetDeviceInfo().
*/
typedef struct {
    int structVersion; /**< @brief this internal structure version */ 
    const char *interf; /**< @brief underlying MIDI API, e.g. 
                                    "MMSystem" or "DirectX" */
    char *name;   /**< @brief device name, e.g. "USB MidiSport 1x1" */
    int input; /**< @brief true iff input is available */
    int output; /**< @brief true iff output is available */
    int opened; /**< @brief used by generic PortMidi for error checking */
    int is_virtual; /**< @brief true iff this is/was a virtual device */
} PmDeviceInfo;
#define PM_DEVICEINFO_VERS 200

/** Version number of PmDeviceInfo, stored in #PmDeviceInfo::structVersion 
    field */
#define PM_DEVICEINFO_VERS 200

/** MIDI system-dependent device or driver info is passed in this
    structure, which is owned by the caller.
*/
enum PmSysDepPropertyKey {
    pmKeyNone = 0,  /**< a "noop" key value */
    /** CoreMIDI Manufacturer name, value is string */
    pmKeyCoreMidiManufacturer = 1,
    /** Linux ALSA snd_seq_port_info_set_name, value is a string. Can be 
        passed in PmSysDepInfo to Pm_OpenInput or Pm_OpenOutput when opening
        a device. The created port will be named accordingly and will be 
        visible for externally made connections (subscriptions). (Linux ALSA
        ports are always enabled for this, but only get application-specific
        names if you give it one.) This key/value is ignored when opening
        virtual ports, which are named when they are created.) */
    pmKeyAlsaPortName = 2,
    /** Linux ALSA snd_seq_set_client_name, value is a string.
        Can be passed in PmSysDepInfo to Pm_OpenInput or Pm_OpenOutput.
        Pm_CreateVirtualInput or Pm_CreateVirtualOutput. Will override
        any previously set client name and applies to all ports. */
    pmKeyAlsaClientName = 3
    /* if system-dependent code introduces more options, register
       the key here to avoid conflicts. */
};
#define PM_SYSDEPINFO_VERS 210

/** System-dependent information can be passed when creating and opening
    ports using this data structure, which stores alternating keys and
    values (addresses). See `pm_test/sendvirtual.c`, `pm_test/recvvirtual.c`,
    and `pm_test/testio.c` for examples.
 */
typedef struct {
    int structVersion;  /**< this structure version */
    int length;  /**< number of properties in this structure */
    struct {
        enum PmSysDepPropertyKey key;
        const void *value;
    } properties[];  /**< array of key/value pairs */
} PmSysDepInfo;

/** Version number of PmSysDepInfo, stored in #PmSysDepInfo::structVersion
    field */
#define PM_SYSDEPINFO_VERS 210



/** Get devices count, ids range from 0 to Pm_CountDevices()-1. */
PMEXPORT int Pm_CountDevices(void);

/**
    Return the default device ID or pmNoDevice if there are no devices.
    The result (but not pmNoDevice) can be passed to Pm_OpenMidi().
    
    The use of these functions is not recommended. There is no natural
    "default device" on any system, so defaults must be set by users.
    (Currently, PortMidi just returns the first device it finds as
    "default", so if there *is* a default, implementors should use
    pm_add_device to add system default input and output devices
    first.)

    The recommended solution is pass the burden to applications. It is
    easy to scan devices with PortMidi and build a device menu, and to
    save menu selections in application preferences for next
    time. This is my recommendation for any GUI program. For simple
    command-line applications and utilities, see pm_test where all the
    test programs now accept device numbers on the command line and/or
    prompt for their entry.

    On linux, you can create virtual ports and use an external program
    to set up inter-application and device connections.

    Some advice for preferences: MIDI devices used to be built-in or
    plug-in cards, so the numbers rarely changed. Now MIDI devices are
    often plug-in USB devices, so device numbers change, and you
    probably need to design to reinitialize PortMidi to rescan
    devices. MIDI is pretty stateless, so this isn't a big problem,
    although it means you cannot find a new device while playing or
    recording MIDI.

    Since device numbering can change whenever a USB device is plugged
    in, preferences should record *names* of devices rather than
    device numbers. It is simple enough to use string matching to find
    a prefered device, so PortMidi does not provide any built-in
    lookup function.
*/
PMEXPORT PmDeviceID Pm_GetDefaultInputDeviceID(void);

/** @brief see PmDeviceID Pm_GetDefaultInputDeviceID() */
PMEXPORT PmDeviceID Pm_GetDefaultOutputDeviceID(void);

/** Find a device that matches a pattern. 

    @param pattern a substring of the device name, or if the pattern
    contains the two-character separator ", ", then the first part of
    the pattern represents a device interface substring and the second
    part after the separator represents a device name substring. 

    @param is_input restricts the search to an input when true, or an
    output when false.

    @return the number of the first device whose device interface
    contains the interface pattern (if any), whose device name
    contains the name pattern, and whose direction (input or output)
    matches the \p is_input parameter. If no match is found, #pmNoDevice
    (-1) is returned.
*/
PMEXPORT PmDeviceID Pm_FindDevice(char *pattern, int is_input);


/** Represents a millisecond clock with arbitrary start time. 
    This type is used for all MIDI timestamps and clocks.
*/
typedef int32_t PmTimestamp;

/** @brief function pointer to retrieve the time in milliseconds.
    This is the time used in all PortMidi timestamps. The use of
    a function pointer allows the user to derive time from an audio
    sample count to synchronize MIDI to audio, or from a remote
    machine through clock synchronization protocols to synchronize
    MIDI across multiple machine. The time is independent of the
    internal system timestamps (e.g., MacOS CoreMIDI uses its own
    clock and timestamp representation, and PortMidi translates
    between different clocks, which need not be synchronized.) */
typedef PmTimestamp (*PmTimeProcPtr)(void *time_info);

/** TRUE if t1 before t2 */
#define PmBefore(t1,t2) (((t1)-(t2)) < 0)
/** @} */
/** 
    \defgroup grp_device Input/Output Devices Handling
    @{
*/
/** Get a PmDeviceInfo structure describing a MIDI device.
    
    @param id the device to be queried.

    If \p id is out of range or if the device designates a deleted
    virtual device, the function returns NULL.

    The returned structure is owned by the PortMidi implementation and
    must not be manipulated or freed. The pointer is guaranteed to be
    valid between calls to Pm_Initialize() and Pm_Terminate().
*/
PMEXPORT const PmDeviceInfo *Pm_GetDeviceInfo(PmDeviceID id);

/** Open a MIDI device for input.

    @param stream the address of a #PortMidiStream pointer which will
    receive a pointer to the newly opened stream.

    @param inputDevice the ID of the device to be opened (see #PmDeviceID).

    @param inputSysDepInfo a pointer to an optional system-dependent
    data structure (a #PmSysDepInfo struct) containing additional
    information for device setup or handle processing. This parameter
    is never required for correct operation. If not used, specify
    NULL.  Declared `void *` here for backward compatibility. Note that
    with Linux ALSA, you can use this parameter to specify a client name
    and port name.

    @param bufferSize the number of input events to be buffered
    waiting to be read using #Pm_Read(). Messages will be lost if the
    number of unread messages exceeds this value.

    @param time_proc (address of) a procedure that returns time in
    milliseconds. It may be NULL, in which case a default millisecond
    timebase (PortTime) is used. If the application wants to use
    PortTime, it should start the timer (call Pt_Start) before calling
    Pm_OpenInput or Pm_OpenOutput. If the application tries to start
    the timer *after* Pm_OpenInput or Pm_OpenOutput, it may get a
    ptAlreadyStarted error from Pt_Start, and the application's
    preferred time resolution and callback function will be ignored.
    \p time_proc result values are appended to incoming MIDI data,
    normally by mapping system-provided timestamps to the \p time_proc
    timestamps to maintain the precision of system-provided
    timestamps.

    @param time_info is a pointer passed to time_proc.

    @return #pmNoError and places a pointer to a valid
    #PortMidiStream in the stream argument.  If the open operation
    fails, a nonzero error code is returned (see #PmError) and
    the value of stream is invalid.

    Any stream that is successfully opened should eventually be closed
    by calling Pm_Close().
*/
PMEXPORT PmError Pm_OpenInput(PortMidiStream** stream,
                PmDeviceID inputDevice,
                void *inputSysDepInfo,
                int32_t bufferSize,
                PmTimeProcPtr time_proc,
                void *time_info);

/** Open a MIDI device for output.

    @param stream the address of a #PortMidiStream pointer which will
    receive a pointer to the newly opened stream.

    @param outputDevice the ID of the device to be opened (see #PmDeviceID).

    @param outputSysDepInfo a pointer to an optional system-specific
    data structure (a #PmSysDepInfo struct) containing additional
    information for device setup or handle processing. This parameter
    is never required for correct operation. If not used, specify
    NULL. Declared `void *` here for backward compatibility. Note that
    with Linux ALSA, you can use this parameter to specify a client name
    and port name.

    @param bufferSize the number of output events to be buffered
    waiting for output. In some cases -- see below -- PortMidi does
    not buffer output at all and merely passes data to a lower-level
    API, in which case \p bufferSize is ignored. Since MIDI speeds now
    vary from 1 to 50 or more messages per ms (over USB), put some
    thought into this number. E.g. if latency is 20ms and you want to
    burst 100 messages in that time (5000 messages per second), you
    should set \p bufferSize to at least 100. The default on Windows
    assumes an average rate of 500 messages per second and in this
    example, output would be slowed waiting for free buffers.
    
    @param latency the delay in milliseconds applied to timestamps
    to determine when the output should actually occur. (If latency is
    < 0, 0 is assumed.)  If latency is zero, timestamps are ignored
    and all output is delivered immediately. If latency is greater
    than zero, output is delayed until the message timestamp plus the
    latency. (NOTE: the time is measured relative to the time source
    indicated by time_proc. Timestamps are absolute, not relative
    delays or offsets.) In some cases, PortMidi can obtain better
    timing than your application by passing timestamps along to the
    device driver or hardware, so the best strategy to minimize jitter
    is: wait until the real time to send the message, compute the
    message, attach the *ideal* output time (not the current real
    time, because some time may have elapsed), and send the
    message. The \p latency will be added to the timestamp, and
    provided the elapsed computation time has not exceeded \p latency,
    the message will be delivered according to the timestamp. If the
    real time is already past the timestamp, the message will be
    delivered as soon as possible. Latency may also help you to
    synchronize MIDI data to audio data by matching \p latency to the
    audio buffer latency.

    @param time_proc (address of) a pointer to a procedure that
    returns time in milliseconds. It may be NULL, in which case a
    default millisecond timebase (PortTime) is used. If the
    application wants to use PortTime, it should start the timer (call
    Pt_Start) before calling #Pm_OpenInput or #Pm_OpenOutput. If the
    application tries to start the timer *after* #Pm_OpenInput or
    #Pm_OpenOutput, it may get a #ptAlreadyStarted error from #Pt_Start,
    and the application's preferred time resolution and callback
    function will be ignored.  \p time_proc times are used to schedule
    outgoing MIDI data (when latency is non-zero), usually by mapping
    from time_proc timestamps to internal system timestamps to
    maintain the precision of system-supported timing.

    @param time_info a pointer passed to time_proc.

    @return #pmNoError and places a pointer to a valid #PortMidiStream
    in the stream argument.  If the operation fails, a nonzero error
    code is returned (see PMError) and the value of \p stream is
    invalid.

    Note: ALSA appears to have a fixed-size priority queue for timed
    output messages. Testing indicates the queue can hold a little
    over 400 3-byte MIDI messages. Thus, you can send 10,000
    messages/second if the latency is 30ms (30ms * 10000 msgs/sec *
    0.001 sec/ms = 300 msgs), but not if the latency is 50ms
    (resulting in about 500 pending messages, which is greater than
    the 400 message limit). Since timestamps in ALSA are relative,
    they are of less value than absolute timestamps in macOS and
    Windows. This is a limitation of ALSA and apparently a design
    flaw.

    Example 1: If I provide a timestamp of 5000, latency is 1, and
    time_proc returns 4990, then the desired output time will be when
    time_proc returns timestamp+latency = 5001. This will be 5001-4990
    = 11ms from now.

    Example 2: If I want to send at exactly 5010, and latency is 10, I
    should wait until 5000, compute the messages and provide a
    timestamp of 5000. As long as computation takes less than 10ms,
    the message will be delivered at time 5010.

    Example 3 (recommended): It is often convenient to ignore latency.
    E.g. if a sequence says to output at time 5010, just wait until
    5010, compute the message and use 5010 for the timestamp. Delivery
    will then be at 5010+latency, but unless you are synchronizing to
    something else, the absolute delay by latency will not matter.

    Any stream that is successfully opened should eventually be closed
    by calling Pm_Close().
*/
PMEXPORT PmError Pm_OpenOutput(PortMidiStream** stream,
                PmDeviceID outputDevice,
                void *outputSysDepInfo,
                int32_t bufferSize,
                PmTimeProcPtr time_proc,
                void *time_info,
                int32_t latency);

/** Create  a virtual input device.

    @param name gives the virtual device name, which is visible to
    other applications.

    @param interf is the interface (System API) used to create the
    device Default interfaces are "MMSystem", "CoreMIDI" and
    "ALSA". Currently, these are the only ones implemented, but future
    implementations could support DirectMusic, Jack, sndio, or others.

    @param sysDepInfo contains interface-dependent additional
    information (a #PmSysDepInfo struct), e.g., hints or options. This
    parameter is never required for correct operation. If not used,
    specify NULL. Declared `void *` here for backward compatibility.

    @return a device ID or #pmNameConflict (\p name is invalid or
    already exists) or #pmInterfaceNotSupported (\p interf is does not
    match a supported interface).

    The created virtual device appears to other applications as if it
    is an output device. The device must be opened to obtain a stream
    and read from it.

    Virtual devices are not supported by Windows (Multimedia API). Calls
    on Windows do nothing except return #pmNotImplemented.
*/
PMEXPORT PmError Pm_CreateVirtualInput(const char *name,
                                       const char *interf,
                                       void *sysDepInfo);

/** Create a virtual output device.

    @param name gives the virtual device name, which is visible to
    other applications.

    @param interf is the interface (System API) used to create the
    device Default interfaces are "MMSystem", "CoreMIDI" and
    "ALSA". Currently, these are the only ones implemented, but future
    implementations could support DirectMusic, Jack, sndio, or others.

    @param sysDepInfo contains interface-dependent additional
    information (a #PmSysDepInfo struct), e.g., hints or options. This
    parameter is never required for correct operation. If not used,
    specify NULL. Declared `void *` here for backward compatibility.

    @return a device ID or #pmInvalidDeviceId (\p name is invalid or
    already exists) or #pmInterfaceNotSupported (\p interf is does not
    match a supported interface).

    The created virtual device appears to other applications as if it
    is an input device. The device must be opened to obtain a stream
    and write to it.

    Virtual devices are not supported by Windows (Multimedia API). Calls
    on Windows do nothing except return #pmNotImplemented.
*/
PMEXPORT PmError Pm_CreateVirtualOutput(const char *name,
                                        const char *interf,
                                        void *sysDepInfo);

/** Remove a virtual device.

   @param device a device ID (small integer) designating the device.

   The device is removed; other applications can no longer see or open
   this virtual device, which may be either for input or output. The
   device must not be open. The device ID may be reused, but existing
   devices are not renumbered. This means that the device ID could be
   in the range from 0 to #Pm_CountDevices(), yet the device ID does
   not designate a device. In that case, passing the ID to
   #Pm_GetDeviceInfo() will return NULL.

   @return #pmNoError if the device was deleted or #pmInvalidDeviceId
   if the device is open, already deleted, or \p device is out of
   range.
*/
PMEXPORT PmError Pm_DeleteVirtualDevice(PmDeviceID device);
  /** @} */

/**
   @defgroup grp_events_filters Events and Filters Handling
   @{
*/

/* Filter bit-mask definitions */
/** filter active sensing messages (0xFE): */
#define PM_FILT_ACTIVE (1 << 0x0E)
/** filter system exclusive messages (0xF0): */
#define PM_FILT_SYSEX (1 << 0x00)
/** filter MIDI clock message (0xF8) */
#define PM_FILT_CLOCK (1 << 0x08)
/** filter play messages (start 0xFA, stop 0xFC, continue 0xFB) */
#define PM_FILT_PLAY ((1 << 0x0A) | (1 << 0x0C) | (1 << 0x0B))
/** filter tick messages (0xF9) */
#define PM_FILT_TICK (1 << 0x09)
/** filter undefined FD messages */
#define PM_FILT_FD (1 << 0x0D)
/** filter undefined real-time messages */
#define PM_FILT_UNDEFINED PM_FILT_FD
/** filter reset messages (0xFF) */
#define PM_FILT_RESET (1 << 0x0F)
/** filter all real-time messages */
#define PM_FILT_REALTIME (PM_FILT_ACTIVE | PM_FILT_SYSEX | PM_FILT_CLOCK | \
    PM_FILT_PLAY | PM_FILT_UNDEFINED | PM_FILT_RESET | PM_FILT_TICK)
/** filter note-on and note-off (0x90-0x9F and 0x80-0x8F */
#define PM_FILT_NOTE ((1 << 0x19) | (1 << 0x18))
/** filter channel aftertouch (most midi controllers use this) (0xD0-0xDF)*/
#define PM_FILT_CHANNEL_AFTERTOUCH (1 << 0x1D)
/** per-note aftertouch (0xA0-0xAF) */
#define PM_FILT_POLY_AFTERTOUCH (1 << 0x1A)
/** filter both channel and poly aftertouch */
#define PM_FILT_AFTERTOUCH (PM_FILT_CHANNEL_AFTERTOUCH | \
                            PM_FILT_POLY_AFTERTOUCH)
/** Program changes (0xC0-0xCF) */
#define PM_FILT_PROGRAM (1 << 0x1C)
/** Control Changes (CC's) (0xB0-0xBF)*/
#define PM_FILT_CONTROL (1 << 0x1B)
/** Pitch Bender (0xE0-0xEF*/
#define PM_FILT_PITCHBEND (1 << 0x1E)
/** MIDI Time Code (0xF1)*/
#define PM_FILT_MTC (1 << 0x01)
/** Song Position (0xF2) */
#define PM_FILT_SONG_POSITION (1 << 0x02)
/** Song Select (0xF3)*/
#define PM_FILT_SONG_SELECT (1 << 0x03)
/** Tuning request (0xF6) */
#define PM_FILT_TUNE (1 << 0x06)
/** All System Common messages (mtc, song position, song select, tune request) */
#define PM_FILT_SYSTEMCOMMON (PM_FILT_MTC | PM_FILT_SONG_POSITION | \
                              PM_FILT_SONG_SELECT | PM_FILT_TUNE)


/** Set filters on an open input stream to drop selected input types.
    
    @param stream an open MIDI input stream.

    @param filters indicate message types to filter (block).

    @return #pmNoError or an error code.

    By default, only active sensing messages are filtered.
    To prohibit, say, active sensing and sysex messages, call
    Pm_SetFilter(stream, PM_FILT_ACTIVE | PM_FILT_SYSEX);

    Filtering is useful when midi routing or midi thru functionality
    is being provided by the user application.
    For example, you may want to exclude timing messages (clock, MTC,
    start/stop/continue), while allowing note-related messages to pass.
    Or you may be using a sequencer or drum-machine for MIDI clock
    information but want to exclude any notes it may play.
 */
PMEXPORT PmError Pm_SetFilter(PortMidiStream* stream, int32_t filters);

/** Create a mask that filters one channel. */
#define Pm_Channel(channel) (1<<(channel))

/** Filter incoming messages based on channel.

    @param stream an open MIDI input stream.

    @param mask indicates channels to be received.

    @return #pmNoError or an error code.

    The \p mask is a 16-bit bitfield corresponding to appropriate channels.
    The #Pm_Channel macro can assist in calling this function.
    I.e. to receive only input on channel 1, call with
    Pm_SetChannelMask(Pm_Channel(1));
    Multiple channels should be OR'd together, like
    Pm_SetChannelMask(Pm_Channel(10) | Pm_Channel(11))

    Note that channels are numbered 0 to 15 (not 1 to 16). Most 
    synthesizer and interfaces number channels starting at 1, but
    PortMidi numbers channels starting at 0.

    All channels are allowed by default
*/
PMEXPORT PmError Pm_SetChannelMask(PortMidiStream *stream, int mask);

/** Terminate outgoing messages immediately.

    @param stream an open MIDI output stream.

    @result #pmNoError or an error code.

    The caller should immediately close the output port; this call may
    result in transmission of a partial MIDI message.  There is no
    abort for Midi input because the user can simply ignore messages
    in the buffer and close an input device at any time. If the
    specified behavior cannot be achieved through the system-level
    interface (ALSA, CoreMIDI, etc.), the behavior may be that of 
    Pm_Close().
 */
PMEXPORT PmError Pm_Abort(PortMidiStream* stream);
     
/** Close a midi stream, flush any pending buffers if possible.

    @param stream an open MIDI input or output stream.

    @result #pmNoError or an error code.

    If the system-level interface (ALSA, CoreMIDI, etc.) does not
    support flushing remaining messages, the behavior may be one of
    the following (most preferred first): block until all pending
    timestamped messages are delivered; deliver messages to a server
    or kernel process for later delivery but return immediately; drop
    messages (as in Pm_Abort()).  Therefore, to be safe, applications
    should wait until the output queue is empty before calling
    Pm_Close(). E.g. calling Pt_Sleep(100 + latency); will give a
    100ms "cushion" beyond latency (if any) before closing.
*/
PMEXPORT PmError Pm_Close(PortMidiStream* stream);

/** (re)synchronize to the time_proc passed when the stream was opened.

    @param stream an open MIDI input or output stream.

    @result #pmNoError or an error code.

    Typically, this is used when the stream must be opened before the
    time_proc reference is actually advancing. In this case, message
    timing may be erratic, but since timestamps of zero mean "send
    immediately," initialization messages with zero timestamps can be
    written without a functioning time reference and without
    problems. Before the first MIDI message with a non-zero timestamp
    is written to the stream, the time reference must begin to advance
    (for example, if the time_proc computes time based on audio
    samples, time might begin to advance when an audio stream becomes
    active). After time_proc return values become valid, and BEFORE
    writing the first non-zero timestamped MIDI message, call
    Pm_Synchronize() so that PortMidi can observe the difference
    between the current time_proc value and its MIDI stream time.
    
    In the more normal case where time_proc values advance
    continuously, there is no need to call #Pm_Synchronize. PortMidi
    will always synchronize at the first output message and
    periodically thereafter.
*/
PMEXPORT PmError Pm_Synchronize(PortMidiStream* stream);


/** Encode a short Midi message into a 32-bit word. If data1
    and/or data2 are not present, use zero.
*/
#define Pm_Message(status, data1, data2) \
         ((((data2) << 16) & 0xFF0000) | \
          (((data1) << 8) & 0xFF00) | \
          ((status) & 0xFF))
/** Extract the status field from a 32-bit midi message. */
#define Pm_MessageStatus(msg) ((msg) & 0xFF)
/** Extract the 1st data field (e.g., pitch) from a 32-bit midi message. */
#define Pm_MessageData1(msg) (((msg) >> 8) & 0xFF)
/** Extract the 2nd data field (e.g., velocity) from a 32-bit midi message. */
#define Pm_MessageData2(msg) (((msg) >> 16) & 0xFF)

typedef uint32_t PmMessage; /**< @brief see #PmEvent */
/**
   All MIDI data comes in the form of PmEvent structures. A sysex
   message is encoded as a sequence of PmEvent structures, with each
   structure carrying 4 bytes of the message, i.e. only the first
   PmEvent carries the status byte.

   All other MIDI messages take 1 to 3 bytes and are encoded in a whole
   PmMessage with status in the low-order byte and remaining bytes 
   unused, i.e., a 3-byte note-on message will occupy 3 low-order bytes
   of PmMessage, leaving the high-order byte unused.

   Note that MIDI allows nested messages: the so-called "real-time" MIDI 
   messages can be inserted into the MIDI byte stream at any location, 
   including within a sysex message. MIDI real-time messages are one-byte
   messages used mainly for timing (see the MIDI spec). PortMidi retains 
   the order of non-real-time MIDI messages on both input and output, but 
   it does not specify exactly how real-time messages are processed. This
   is particulary problematic for MIDI input, because the input parser 
   must either prepare to buffer an unlimited number of sysex message 
   bytes or to buffer an unlimited number of real-time messages that 
   arrive embedded in a long sysex message. To simplify things, the input
   parser is allowed to pass real-time MIDI messages embedded within a 
   sysex message, and it is up to the client to detect, process, and 
   remove these messages as they arrive.

   When receiving sysex messages, the sysex message is terminated
   by either an EOX status byte (anywhere in the 4 byte messages) or
   by a non-real-time status byte in the low order byte of the message.
   If you get a non-real-time status byte but there was no EOX byte, it 
   means the sysex message was somehow truncated. This is not
   considered an error; e.g., a missing EOX can result from the user
   disconnecting a MIDI cable during sysex transmission.

   A real-time message can occur within a sysex message. A real-time 
   message will always occupy a full PmEvent with the status byte in 
   the low-order byte of the PmEvent message field. (This implies that
   the byte-order of sysex bytes and real-time message bytes may not
   be preserved -- for example, if a real-time message arrives after
   3 bytes of a sysex message, the real-time message will be delivered
   first. The first word of the sysex message will be delivered only
   after the 4th byte arrives, filling the 4-byte PmEvent message field.
   
   The timestamp field is observed when the output port is opened with
   a non-zero latency. A timestamp of zero means "use the current time",
   which in turn means to deliver the message with a delay of
   latency (the latency parameter used when opening the output port.)
   Do not expect PortMidi to sort data according to timestamps -- 
   messages should be sent in the correct order, and timestamps MUST 
   be non-decreasing. See also "Example" for Pm_OpenOutput() above.

   A sysex message will generally fill many #PmEvent structures. On 
   output to a #PortMidiStream with non-zero latency, the first timestamp
   on sysex message data will determine the time to begin sending the 
   message. PortMidi implementations may ignore timestamps for the 
   remainder of the sysex message. 
   
   On input, the timestamp ideally denotes the arrival time of the 
   status byte of the message. The first timestamp on sysex message 
   data will be valid. Subsequent timestamps may denote 
   when message bytes were actually received, or they may be simply 
   copies of the first timestamp.

   Timestamps for nested messages: If a real-time message arrives in 
   the middle of some other message, it is enqueued immediately with 
   the timestamp corresponding to its arrival time. The interrupted 
   non-real-time message or 4-byte packet of sysex data will be enqueued 
   later. The timestamp of interrupted data will be equal to that of
   the interrupting real-time message to insure that timestamps are
   non-decreasing.
 */
typedef struct {
    PmMessage      message;    /**< up to 4 bytes of MIDI data */
    PmTimestamp    timestamp;  /**< PortMidi time of message */
} PmEvent;

/** @} */

/** \defgroup grp_io Reading and Writing Midi Messages
    @{
*/
/** Retrieve midi data into a buffer. 

    @param stream the open input stream.

    @param buffer input data is stored here

    @param length the length of buffer (number of #PmEvent, not bytes)

    @return the number of events read, or, if the result is negative,
    a #PmError value will be returned.

    The Buffer Overflow Problem

    The problem: if an input overflow occurs, data will be lost,
    ultimately because there is no flow control all the way back to
    the data source.  When data is lost, the receiver should be
    notified and some sort of graceful recovery should take place,
    e.g. you shouldn't resume receiving in the middle of a long sysex
    message.

    With a lock-free fifo, which is pretty much what we're stuck with
    to enable portability to the Mac, it's tricky for the producer and
    consumer to synchronously reset the buffer and resume normal
    operation.

    Solution: the entire buffer managed by PortMidi will be flushed
    when an overflow occurs. The consumer (#Pm_Read()) gets an error
    message (#pmBufferOverflow) and ordinary processing resumes as
    soon as a new message arrives. The remainder of a partial sysex
    message is not considered to be a "new message" and will be
    flushed as well.
*/
PMEXPORT int Pm_Read(PortMidiStream *stream, PmEvent *buffer, int32_t length);

/** Test whether input is available.

    @param stream an open input stream.

    @return TRUE, FALSE, or an error value. 

    If there was an asynchronous error, pmHostError is returned and you must
    call again to determine if input is (also) available.

    You should probably *not* use this function. Call Pm_Read()
    instead. If it returns 0, then there is no data available. It is
    possible for Pm_Poll() to return TRUE before the complete message
    is available, so Pm_Read() could return 0 even after Pm_Poll()
    returns TRUE. Only call Pm_Poll() if you want to know that data is
    probably available even though you are not ready to receive data.
*/
PMEXPORT PmError Pm_Poll(PortMidiStream *stream);

/** Write MIDI data from a buffer. 

    @param stream an open output stream.

    @param buffer (address of) an array of MIDI event data.

    @param length the length of the \p buffer.

    @return #pmNoError, #pmBadPtr (if \p stream is not valid and opened),
        #pmDeviceRemoved (if the MIDI device no longer exists),
        #pmBadData (if \p buffer data does not represent valid MIDI, e.g.,
        nested SYSEX messages, or #pmHostError (error returned from API's
        MIDI write operation, see #Pm_GetHostErrorText). 

    \b buffer may contain:
        - short messages 
        - sysex messages that are converted into a sequence of PmEvent
          structures, e.g. sending data from a file or forwarding them
          from midi input, with 4 SysEx bytes per PmEvent message,
          low-order byte first, until the last message, which may
          contain from 1 to 4 bytes ending in MIDI EOX (0xF7).
        - PortMidi allows 1-byte real-time messages to be embedded
          within SysEx messages, but only on 4-byte boundaries so
          that SysEx data always uses a full 4 bytes (except possibly
          at the end). Each real-time message always occupies a full
          PmEvent (3 of the 4 bytes in the PmEvent's message are
          ignored) even when embedded in a SysEx message.

    Use Pm_WriteSysEx() to write a sysex message stored as a contiguous 
    array of bytes.

    Sysex data may contain embedded real-time messages.

    \p buffer is managed by the caller. The buffer may be destroyed
    as soon as this call returns.
*/
PMEXPORT PmError Pm_Write(PortMidiStream *stream, PmEvent *buffer,
                          int32_t length);

/** Write a timestamped non-system-exclusive midi message.

    @param stream an open output stream.

    @param when timestamp for the event.

    @param msg the data for the event.

    @return #pmNoError, #pmBadPtr (if \p stream is not valid and opened),
        #pmDeviceRemoved (if the MIDI device no longer exists),
        #pmBadData (if \p buffer data does not represent valid MIDI, e.g.,
        nested SYSEX messages, or #pmHostError (error returned from API's
        MIDI write operation, see #Pm_GetHostErrorText). 

    Messages are delivered in order, and timestamps must be
    non-decreasing. (But timestamps are ignored if the stream was
    opened with latency = 0, and otherwise, non-decreasing timestamps
    are "corrected" to the lowest valid value.)
*/
PMEXPORT PmError Pm_WriteShort(PortMidiStream *stream, PmTimestamp when,
                               PmMessage msg);

/** Write a timestamped system-exclusive midi message.

    @param stream an open output stream.

    @param when timestamp for the event.

    @param msg the sysex message, terminated with an EOX status byte.

    @return #pmNoError, #pmBadPtr (if \p stream is not valid and opened),
        #pmDeviceRemoved (if the MIDI device no longer exists),
        #pmBadData (if \p buffer data does not represent valid MIDI, e.g.,
        nested SYSEX messages, or #pmHostError (error returned from API's
        MIDI write operation, see #Pm_GetHostErrorText). 

    \p msg is managed by the caller and may be destroyed when this
    call returns.
*/
PMEXPORT PmError Pm_WriteSysEx(PortMidiStream *stream, PmTimestamp when, 
                               unsigned char *msg);

/** @} */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PORTMIDI_PORTMIDI_H */
