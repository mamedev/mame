/* multivirtual.c -- test for creating two input and two output virtual ports */
/*
 * Roger B. Dannenberg
 * Oct 2021
 */
#include "portmidi.h"
#include "porttime.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"

#define OUTPUT_BUFFER_SIZE 0
#define DEVICE_INFO NULL
#define DRIVER_INFO NULL
#define TIME_PROC ((PmTimeProcPtr) Pt_Time)
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

int latency = 0;

static void prompt_and_exit(void)
{
    printf("type ENTER...");
    while (getchar() != '\n') ;
    /* this will clean up open ports: */
    exit(-1);
}


static PmError checkerror(PmError err)
{
    if (err == pmHostError) {
        /* it seems pointless to allocate memory and copy the string,
         * so I will do the work of Pm_GetHostErrorText directly
         */
        char errmsg[80];
        Pm_GetHostErrorText(errmsg, 80);
        printf("PortMidi found host error...\n  %s\n", errmsg);
        prompt_and_exit();
    } else if (err < 0) {
        printf("PortMidi call failed...\n  %s\n", Pm_GetErrorText(err));
        prompt_and_exit();
    }
    return err;
}

static int msg_count[2] = {0, 0};

void poll_input(PmStream *in, int which)
{
    PmEvent buffer[1];
    int pitch, expected, length;
    PmError status = Pm_Poll(in);
    if (status == TRUE) {
        length = Pm_Read(in, buffer, 1);
        if (length > 0) {
            printf("Got message %d from portmidi%d: "
                   "time %ld, %2x %2x %2x\n",
                   msg_count[which], which + 1, (long) buffer[0].timestamp,
                   (status = Pm_MessageStatus(buffer[0].message)),
                   (pitch = Pm_MessageData1(buffer[0].message)),
                   Pm_MessageData2(buffer[0].message));
            if (status == 0x90) {  /* 1 & 2 are on/off 60, 3 & 4 are 61, etc. */
                expected = (((msg_count[which] - 1) / 2) % 12) + 60 +
                           which * 12;
                if (pitch != expected) {
                    printf("WARNING: expected pitch %d, got pitch %d\n",
                           expected, pitch);
                }
            }
            msg_count[which]++;
        } else {
            assert(0);
        }
    }
}


void wait_until(PmTimestamp when, PmStream *in1, PmStream *in2)
{
    while (when > Pt_Time()) {
        poll_input(in1, 0);
        poll_input(in2, 1);
        Pt_Sleep(10);
    }
}


/* create one virtual output device and one input device */
void init(const char *name, PmStream **midi_out, PmStream **midi_in,
          int *id_out, int *id_in)
{
    PmEvent buffer[1];

    *id_out = checkerror(Pm_CreateVirtualOutput(name, NULL, DEVICE_INFO));
    checkerror(Pm_OpenOutput(midi_out, *id_out, DRIVER_INFO, OUTPUT_BUFFER_SIZE,
                             TIME_PROC, TIME_INFO, latency));
    printf("Virtual Output \"%s\" id %d created and opened.\n", name, *id_out);
    
    *id_in = checkerror(Pm_CreateVirtualInput(name, NULL, DRIVER_INFO));
    checkerror(Pm_OpenInput(midi_in, *id_in, NULL, 0, NULL, NULL));
    printf("Virtual Input \"%s\" id %d created and opened.\n", name, *id_in);
    Pm_SetFilter(*midi_in, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX);
    /* empty the buffer after setting filter, just in case anything
       got through */
    while (Pm_Read(*midi_in, buffer, 1)) ;
}


void main_test(int num)
{
    PmStream *midi1_out;
    PmStream *midi2_out;
    PmStream *midi1_in;
    PmStream *midi2_in;
    int id1_out;
    int id2_out;
    int id1_in;
    int id2_in;
    int32_t next_time;
    PmEvent buffer[1];
    PmTimestamp timestamp;
    int pitch = 60;
    int expected_count = num + 1;  /* add 1 for MIDI Program message */

    /* It is recommended to start timer before Midi; otherwise, PortMidi may
       start the timer with its (default) parameters
     */
    TIME_START;

    init("portmidi1", &midi1_out, &midi1_in, &id1_out, &id1_in);
    init("portmidi2", &midi2_out, &midi2_in, &id2_out, &id2_in);
    
    printf("Type ENTER to send messages: ");
    while (getchar() != '\n') ;

    buffer[0].timestamp = Pt_Time();
#define PROGRAM 0
    buffer[0].message = Pm_Message(0xC0, PROGRAM, 0);
    Pm_Write(midi1_out, buffer, 1);
    Pm_Write(midi2_out, buffer, 1);
    next_time = Pt_Time() + 1000;  /* wait 1s */
    while (num > 0) {
        wait_until(next_time, midi1_in, midi2_in);
        Pm_WriteShort(midi1_out, next_time, Pm_Message(0x90, pitch, 100));
        Pm_WriteShort(midi2_out, next_time, Pm_Message(0x90, pitch + 12, 100));
        printf("Note On pitch %d\n", pitch);
        num--;
        next_time += 500;

        wait_until(next_time, midi1_in, midi2_in);
        Pm_WriteShort(midi1_out, next_time, Pm_Message(0x90, pitch, 0));
        Pm_WriteShort(midi2_out, next_time, Pm_Message(0x90, pitch + 12, 0));
        printf("Note Off pitch %d\n", pitch);
        num--;
        pitch = (pitch + 1) % 12 + 60;
        next_time += 500;
    }
    wait_until(next_time, midi1_in, midi2_in);  /* get final note-offs */

    printf("Got %d messages from portmidi1 and %d from portmidi2; "
           "expected %d.\n", msg_count[0], msg_count[1], expected_count);
    
    /* close devices (this not explicitly needed in most implementations) */
    printf("ready to close...");
    checkerror(Pm_Close(midi1_out));
    checkerror(Pm_Close(midi2_out));
    checkerror(Pm_Close(midi1_in));
    checkerror(Pm_Close(midi2_in));
    printf("done closing.\nNow delete the virtual devices...");
    checkerror(Pm_DeleteVirtualDevice(id1_out));
    checkerror(Pm_DeleteVirtualDevice(id1_in));
    checkerror(Pm_DeleteVirtualDevice(id2_out));
    checkerror(Pm_DeleteVirtualDevice(id2_in));
    printf("done deleting.\n");
}


void show_usage()
{
    printf("Usage: multivirtual [-h] [-l latency-in-ms] [n]\n"
           "    -h for this message,\n"
           "    -l ms designates latency for precise timing (default 0),\n"
           "    n is number of message to send each output, not counting\n"
           "        initial program change.\n"
           "sends change program to 1, then one note per second with 0.5s on,\n"
           "0.5s off, for n/2 seconds to both output ports portmidi1 and\n"
           "portmidi2. portmidi1 gets pitches from C4 (60). portmidi2 gets\n"
           "pitches an octave higher. Latency >0 uses the device driver for \n"
           "precise timing (see PortMidi documentation). Inputs print what\n"
           "they get and print WARNING if they get something unexpected.\n"
           "The expected test is use two instances of testio to loop\n"
           "portmidi1 back to portmidi1 and portmidi2 back to portmidi2.\n"); 
    exit(0);
}


int main(int argc, char *argv[])
{
    int num = 10;
    int i;
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            show_usage();
        } else if (strcmp(argv[i], "-l") == 0 && (i + 1 < argc)) {
            i = i + 1;
            latency = atoi(argv[i]);
            printf("Latency will be %d\n", latency);
        } else {
            num = atoi(argv[1]);
            if (num <= 0) {
                show_usage();
            }
            printf("Sending %d messages.\n", num);
        }
    }

    main_test(num);
    
    printf("finished sendvirtual test...type ENTER to quit...");
    while (getchar() != '\n') ;
    return 0;
}
