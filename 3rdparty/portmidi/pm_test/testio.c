#include "portmidi.h"
#include "porttime.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"

#define INPUT_BUFFER_SIZE 100
#define OUTPUT_BUFFER_SIZE 0
#define TIME_PROC ((int32_t (*)(void *)) Pt_Time)
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

#define WAIT_FOR_ENTER while (getchar() != '\n') ;

int32_t latency = 0;
int verbose = FALSE;
PmSysDepInfo *sysdepinfo = NULL;

/* crash the program to test whether midi ports are closed */
/**/
void doSomethingReallyStupid(void) {
    int * tmp = NULL;
    *tmp = 5;
}


/* exit the program without any explicit cleanup */
/**/
void doSomethingStupid(void) {
    assert(0);
}


/* read a number from console */
/**/
int get_number(const char *prompt)
{
    int n = 0, i;
    fputs(prompt, stdout);
    while (n != 1) {
        n = scanf("%d", &i);
        WAIT_FOR_ENTER
    }
    return i;
}


static void set_sysdepinfo(char m_or_p, const char *name)
{
    if (!sysdepinfo) {
        // allocate some space we will alias with open-ended PmDriverInfo:
        // there is space for 4 parameters:
        static char dimem[sizeof(PmSysDepInfo) + sizeof(void *) * 8];
        sysdepinfo = (PmSysDepInfo *) dimem;
        // build the driver info structure:
        sysdepinfo->structVersion = PM_SYSDEPINFO_VERS;
        sysdepinfo->length = 0;
    }
    if (sysdepinfo->length > 1) {
        printf("Error: sysdepinfo was allocated to hold 2 parameters\n");
        exit(1);
    }
    int i = sysdepinfo->length++;
    enum PmSysDepPropertyKey k = pmKeyNone;
    if (m_or_p == 'm') k = pmKeyCoreMidiManufacturer;
    else if (m_or_p == 'p') k = pmKeyAlsaPortName;
    else if (m_or_p == 'c') k = pmKeyAlsaClientName;
    sysdepinfo->properties[i].key = k;
    sysdepinfo->properties[i].value = name;
}


/*
 * the somethingStupid parameter can be set to simulate a program crash.
 * We want PortMidi to close Midi ports automatically in the event of a
 * crash because Windows does not (and this may cause an OS crash)
 */
void main_test_input(unsigned int somethingStupid) {
    PmStream * midi;
    PmError status, length;
    PmEvent buffer[1];
    int num = 10;
    int i = get_number("Type input number: ");
    /* It is recommended to start timer before Midi; otherwise, PortMidi may
       start the timer with its (default) parameters
     */
    TIME_START;

    /* open input device */
    Pm_OpenInput(&midi, 
                 i,
                 sysdepinfo,
                 INPUT_BUFFER_SIZE, 
                 TIME_PROC, 
                 TIME_INFO);

    printf("Midi Input opened. Reading %d Midi messages...\n", num);
    Pm_SetFilter(midi, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX);
    /* empty the buffer after setting filter, just in case anything
       got through */
    while (Pm_Poll(midi)) {
        Pm_Read(midi, buffer, 1);
    }
    /* now start paying attention to messages */
    i = 0; /* count messages as they arrive */
    while (i < num) {
        status = Pm_Poll(midi);
        if (status == TRUE) {
            length = Pm_Read(midi, buffer, 1);
            if (length > 0) {
                printf("Got message %d @ time %ld: timestamp %ld, "
                       "%2lx %2lx %2lx\n", i, (long) Pt_Time(),
                       (long) buffer[0].timestamp,
                       (long) Pm_MessageStatus(buffer[0].message),
                       (long) Pm_MessageData1(buffer[0].message),
                       (long) Pm_MessageData2(buffer[0].message));
                i++;
            } else {
                assert(0);
            }
        }
        /* simulate crash if somethingStupid is 1 or 2 */
        if ((i > (num/2)) && (somethingStupid == 1)) {
            doSomethingStupid();
        } else if ((i > (num/2)) && (somethingStupid == 2)) {
            doSomethingReallyStupid();
        }
    }

    /* close device (this not explicitly needed in most implementations) */
    printf("ready to close...");

    Pm_Close(midi);
    printf("done closing...");
}



void main_test_output(int isochronous_test)
{
    PmStream * midi;
    int32_t off_time;
    int chord[] = { 60, 67, 76, 83, 90 };
    #define chord_size 5 
    PmEvent buffer[chord_size];
    PmTimestamp timestamp;

    /* determine which output device to use */
    int i = get_number("Type output number: ");

    /* It is recommended to start timer before PortMidi */
    TIME_START;

    /* open output device -- since PortMidi avoids opening a timer
       when latency is zero, we will pass in a NULL timer pointer
       for that case. If PortMidi tries to access the time_proc,
       we will crash, so this test will tell us something. */
    Pm_OpenOutput(&midi, 
                  i, 
                  sysdepinfo,
                  OUTPUT_BUFFER_SIZE, 
                  (latency == 0 ? NULL : TIME_PROC),
                  (latency == 0 ? NULL : TIME_INFO), 
                  latency);
    printf("Midi Output opened with %ld ms latency.\n", (long) latency);

    /* output note on/off w/latency offset; hold until user prompts */
    printf("ready to send program 1 change... (type ENTER):");
    WAIT_FOR_ENTER
    /* if we were writing midi for immediate output, we could always use
       timestamps of zero, but since we may be writing with latency, we
       will explicitly set the timestamp to "now" by getting the time.
       The source of timestamps should always correspond to the TIME_PROC
       and TIME_INFO parameters used in Pm_OpenOutput(). */
    buffer[0].timestamp = Pt_Time();
    /* Send a program change to increase the chances we will hear notes */
    /* Program 0 is usually a piano, but you can change it here: */
#define PROGRAM 0
    buffer[0].message = Pm_Message(0xC0, PROGRAM, 0);
    Pm_Write(midi, buffer, 1);

    if (isochronous_test) { // play 4 notes per sec for 20s 
        int count;
        PmTimestamp start;
        if (latency < 100) {
            printf("Warning: latency < 100, but this test sends messages"
                   " at times that are jittered by up to 100ms, so you"
                   " may hear uneven timing\n");
        }
        printf("Starting in 1s..."); fflush(stdout);
        Pt_Sleep(1000);
        start = Pt_Time();
        for (count = 0; count < 80; count++) {
            PmTimestamp next_time;
            buffer[0].timestamp = start + count * 250;
            buffer[0].message = Pm_Message(0x90, 69, 100);
            buffer[1].timestamp = start + count * 250 + 200;
            buffer[1].message = Pm_Message(0x90, 69, 0);
            Pm_Write(midi, buffer, 2);
            next_time = start + (count + 1) * 250;
            // sleep for a random time up to 100ms to add jitter to
            // the times at which we send messages. PortMidi timing
            // should remove the jitter if latency > 100
            while (Pt_Time() < next_time) {
                Pt_Sleep(rand() % 100);
            }
        }
        printf("Done sending 80 notes at 4 notes per second.\n");
    } else {
        PmError err = 0;
        printf("ready to note-on... (type ENTER):");
        WAIT_FOR_ENTER
        buffer[0].timestamp = Pt_Time();
        buffer[0].message = Pm_Message(0x90, 60, 100);
        if ((err = Pm_Write(midi, buffer, 1))) {
            printf("Pm_Write returns error: %d (%s)\n", 
                   err, Pm_GetErrorText(err));
            if (err == pmHostError) {
                char errmsg[128];
                Pm_GetHostErrorText(errmsg, 127);
                printf("    Host error: %s\n", errmsg);
            }
        }
        printf("ready to note-off... (type ENTER):");
        WAIT_FOR_ENTER
        buffer[0].timestamp = Pt_Time();
        buffer[0].message = Pm_Message(0x90, 60, 0);
        if ((err = Pm_Write(midi, buffer, 1))) {
            printf("Pm_Write returns error: %d (%s)\n",
                err, Pm_GetErrorText(err));
            if (err == pmHostError) {
                char errmsg[128];
                Pm_GetHostErrorText(errmsg, 127);
                printf("    Host error: %s\n", errmsg);
            }
        }

        /* output short note on/off w/latency offset; hold until user prompts */
        printf("ready to note-on (short form)... (type ENTER):");
        WAIT_FOR_ENTER
        Pm_WriteShort(midi, Pt_Time(),
                      Pm_Message(0x90, 60, 100));
        printf("ready to note-off (short form)... (type ENTER):");
        WAIT_FOR_ENTER
        Pm_WriteShort(midi, Pt_Time(),
                      Pm_Message(0x90, 60, 0));

        /* output several note on/offs to test timing. 
           Should be 1s between notes */
        if (latency == 0) {
            printf("chord should not arpeggiate, latency == 0\n");
        } else {
            printf("chord should arpeggiate (latency = %ld > 0\n",
                   (long) latency);
        }
        printf("ready to chord-on/chord-off... (type ENTER):");
        WAIT_FOR_ENTER
        timestamp = Pt_Time();
        printf("starting timestamp %ld\n", (long) timestamp);
        for (i = 0; i < chord_size; i++) {
            buffer[i].timestamp = timestamp + 1000 * i;
            buffer[i].message = Pm_Message(0x90, chord[i], 100);
        }
        Pm_Write(midi, buffer, chord_size);

        off_time = timestamp + 1000 + chord_size * 1000; 
        while (Pt_Time() < off_time)
            /* There was a report that Pm_Write with zero length sent last
             * message again, so call Pm_Write here to see if note repeats
             */
            Pm_Write(midi, buffer, 0);
            Pt_Sleep(20);  /* wait */
        
        for (i = 0; i < chord_size; i++) {
            buffer[i].timestamp = timestamp + 1000 * i;
            buffer[i].message = Pm_Message(0x90, chord[i], 0);
        }
        Pm_Write(midi, buffer, chord_size);    
    }

    /* close device (this not explicitly needed in most implementations) */
    printf("ready to close and terminate... (type ENTER):");
    WAIT_FOR_ENTER
	
    Pm_Close(midi);
    Pm_Terminate();
    printf("done closing and terminating...\n");
}


void main_test_both(void)
{
    int i = 0;
    int in, out;
    PmStream * midi, * midiOut;
    PmEvent buffer[1];
    PmError status, length;
    int num = 11;
    
    in = get_number("Type input number: ");
    out = get_number("Type output number: ");

    /* In is recommended to start timer before PortMidi */
    TIME_START;

    Pm_OpenOutput(&midiOut, 
                  out, 
                  sysdepinfo,
                  OUTPUT_BUFFER_SIZE, 
                  TIME_PROC,
                  TIME_INFO, 
                  latency);
    printf("Midi Output opened with %ld ms latency.\n", (long) latency);
    /* open input device */
    Pm_OpenInput(&midi, 
                 in,
                 sysdepinfo,
                 INPUT_BUFFER_SIZE, 
                 TIME_PROC, 
                 TIME_INFO);
    printf("Midi Input opened. Reading %d Midi messages...\n", num);
    Pm_SetFilter(midi, PM_FILT_ACTIVE | PM_FILT_CLOCK);
    /* empty the buffer after setting filter, just in case anything
       got through */
    while (Pm_Poll(midi)) {
        Pm_Read(midi, buffer, 1);
    }
    i = 0;
    while (i < num) {
        status = Pm_Poll(midi);
        if (status == TRUE) {
            length = Pm_Read(midi,buffer,1);
            if (length > 0) {
                Pm_Write(midiOut, buffer, 1);
                printf("Got message %d @ time %ld: timestamp %ld, "
                       "%2lx %2lx %2lx\n", i, (long) Pt_Time(),
                       (long) buffer[0].timestamp,
                       (long) Pm_MessageStatus(buffer[0].message),
                       (long) Pm_MessageData1(buffer[0].message),
                       (long) Pm_MessageData2(buffer[0].message));
                i++;
            } else {
                assert(0);
            }
        }
    }
    /* allow time for last message to go out */
    Pt_Sleep(100 + latency);

    /* close midi devices */
    Pm_Close(midi);
    Pm_Close(midiOut);
    Pm_Terminate(); 
}


/* main_test_stream exercises windows winmm API's stream mode */
/*    The winmm stream mode is used for latency>0, and sends
   timestamped messages. The timestamps are relative (delta) 
   times, whereas PortMidi times are absolute. Since peculiar
   things happen when messages are not always sent in advance,
   this function allows us to exercise the system and test it.
 */
void main_test_stream(void) {
    PmStream * midi;
    PmEvent buffer[16];

	/* determine which output device to use */
    int i = get_number("Type output number: ");

    latency = 500; /* ignore LATENCY for this test and
				      fix the latency at 500ms */

    /* It is recommended to start timer before PortMidi */
    TIME_START;

    /* open output device */
    Pm_OpenOutput(&midi, 
                  i, 
                  sysdepinfo,
                  OUTPUT_BUFFER_SIZE, 
                  TIME_PROC,
                  TIME_INFO, 
                  latency);
    printf("Midi Output opened with %ld ms latency.\n", (long) latency);

    /* output note on/off w/latency offset; hold until user prompts */
    printf("ready to send output... (type ENTER):");
    WAIT_FOR_ENTER

    /* if we were writing midi for immediate output, we could always use
       timestamps of zero, but since we may be writing with latency, we
       will explicitly set the timestamp to "now" by getting the time.
       The source of timestamps should always correspond to the TIME_PROC
       and TIME_INFO parameters used in Pm_OpenOutput(). */
    buffer[0].timestamp = Pt_Time();
    buffer[0].message = Pm_Message(0xC0, 0, 0);
    buffer[1].timestamp = buffer[0].timestamp;
    buffer[1].message = Pm_Message(0x90, 60, 100);
    buffer[2].timestamp = buffer[0].timestamp + 1000;
    buffer[2].message = Pm_Message(0x90, 62, 100);
    buffer[3].timestamp = buffer[0].timestamp + 2000;
    buffer[3].message = Pm_Message(0x90, 64, 100);
    buffer[4].timestamp = buffer[0].timestamp + 3000;
    buffer[4].message = Pm_Message(0x90, 66, 100);
    buffer[5].timestamp = buffer[0].timestamp + 4000;
    buffer[5].message = Pm_Message(0x90, 60, 0);
    buffer[6].timestamp = buffer[0].timestamp + 4000;
    buffer[6].message = Pm_Message(0x90, 62, 0);
    buffer[7].timestamp = buffer[0].timestamp + 4000;
    buffer[7].message = Pm_Message(0x90, 64, 0);
    buffer[8].timestamp = buffer[0].timestamp + 4000;
    buffer[8].message = Pm_Message(0x90, 66, 0);

    Pm_Write(midi, buffer, 9);
#ifdef SEND8
    /* Now, we're ready for the real test.
       Play 4 notes at now, now+500, now+1000, and now+1500
       Then wait until now+2000.
       Play 4 more notes as before.
       We should hear 8 evenly spaced notes. */
    now = Pt_Time();
    for (i = 0; i < 4; i++) {
        buffer[i * 2].timestamp = now + (i * 500);
        buffer[i * 2].message = Pm_Message(0x90, 60, 100);
        buffer[i * 2 + 1].timestamp = now + 250 + (i * 500);
        buffer[i * 2 + 1].message = Pm_Message(0x90, 60, 0);
    }
    Pm_Write(midi, buffer, 8);

    while (Pt_Time() < now + 2500) 
        Pt_Sleep(10);
    /* now we are 500 ms behind schedule, but since the latency
       is 500, the delay should not be audible */
    now += 2000;
    for (i = 0; i < 4; i++) {
        buffer[i * 2].timestamp = now + (i * 500);
        buffer[i * 2].message = Pm_Message(0x90, 60, 100);
        buffer[i * 2 + 1].timestamp = now + 250 + (i * 500);
        buffer[i * 2 + 1].message = Pm_Message(0x90, 60, 0);
    }
    Pm_Write(midi, buffer, 8);
#endif
    /* close device (this not explicitly needed in most implementations) */
    printf("ready to close and terminate... (type ENTER):");
    WAIT_FOR_ENTER

    Pm_Close(midi);
    Pm_Terminate();
    printf("done closing and terminating...\n");
}


void show_usage(void)
{
    printf("Usage: test [-h] [-l latency-in-ms] [-c clientname] "
           "[-p portname] [-v]\n"
           "    -h for this help message (only)\n"
           "    -l for latency\n"
           "    -c name designates a client name (linux only),\n"
           "    -p name designates a port name (linux only),\n"
           "    -v for verbose (enables more output)\n");
}

int main(int argc, char *argv[])
{
    int default_in;
    int default_out;
    int i = 0, n = 0;
    int test_input = 0, test_output = 0, test_both = 0, somethingStupid = 0;
    int isochronous_test = 0;
    int stream_test = 0;
    int latency_valid = FALSE;
    
    show_usage();
    if (sizeof(void *) == 8) 
        printf("Apparently this is a 64-bit machine.\n");
    else if (sizeof(void *) == 4) 
        printf ("Apparently this is a 32-bit machine.\n");
    
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            exit(0);
        } else if (strcmp(argv[i], "-p") == 0 && (i + 1 < argc)) {
            i = i + 1;
            const char *port_name = argv[i];
            set_sysdepinfo('p', port_name);
            printf("Port name will be %s\n", port_name);
        } else if (strcmp(argv[i], "-c") == 0 && (i + 1 < argc)) {
            i = i + 1;
            set_sysdepinfo('c', argv[i]);
            printf("Client name will be %s\n", argv[i]);
        } else if (strcmp(argv[i], "-l") == 0 && (i + 1 < argc)) {
            i = i + 1;
            latency = atoi(argv[i]);
            printf("Latency will be %ld\n", (long) latency);
            latency_valid = TRUE;
        } else if (strcmp(argv[i], "-v") == 0) {
            printf("Verbose is now TRUE\n");
            verbose = TRUE;  /* not currently used for anything */
        } else {
            show_usage();
            exit(0);
        }
    }

    while (!latency_valid) {
        int lat; // declared int to match "%d"
        printf("Latency in ms: ");
        if (scanf("%d", &lat) == 1) {
            latency = (int32_t) lat; // coerce from "%d" to known size
        latency_valid = TRUE;
        }
    }

    /* determine what type of test to run */
    printf("begin portMidi test...\n");
    printf("enter your choice...\n    1: test input\n"
           "    2: test input (fail w/assert)\n"
           "    3: test input (fail w/NULL assign)\n"
           "    4: test output\n    5: test both\n"
           "    6: stream test (for WinMM)\n"
           "    7. isochronous out\n");
    while (n != 1) {
        n = scanf("%d", &i);
        WAIT_FOR_ENTER
        switch(i) {
        case 1: 
            test_input = 1;
            break;
        case 2: 
            test_input = 1;
            somethingStupid = 1;
            break;
        case 3: 
            test_input = 1;
            somethingStupid = 2;
            break;
        case 4: 
            test_output = 1;
            break;
        case 5:
            test_both = 1;
            break;
        case 6:
            stream_test = 1;
            break;
        case 7:
            test_output = 1;
            isochronous_test = 1;
            break;
        default:
            printf("got %d (invalid input)\n", n);
            break;
        }
    }
    
    /* list device information */
    default_in = Pm_GetDefaultInputDeviceID();
    default_out = Pm_GetDefaultOutputDeviceID();
    for (i = 0; i < Pm_CountDevices(); i++) {
        char *deflt;
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if (((test_input  | test_both) & info->input) |
            ((test_output | test_both | stream_test) & info->output)) {
            printf("%d: %s, %s", i, info->interf, info->name);
            if (info->input) {
                deflt = (i == default_in ? "default " : "");
                printf(" (%sinput)", deflt);
            }
            if (info->output) {
                deflt = (i == default_out ? "default " : "");
                printf(" (%soutput)", deflt);
            }
            printf("\n");
        }
    }
    
    /* run test */
    if (stream_test) {
        main_test_stream();
    } else if (test_input) {
        main_test_input(somethingStupid);
    } else if (test_output) {
        main_test_output(isochronous_test);
    } else if (test_both) {
        main_test_both();
    }
    
    printf("finished portMidi test...type ENTER to quit...");
    WAIT_FOR_ENTER
    return 0;
}
