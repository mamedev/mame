/* sendvirtual.c -- test for creating a virtual device and sending to it */
/*
 * Roger B. Dannenberg
 * Sep 2021
 */
#include "portmidi.h"
#include "porttime.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"

#define OUTPUT_BUFFER_SIZE 0
#define TIME_PROC ((PmTimeProcPtr) Pt_Time)
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

int latency = 0;
PmSysDepInfo *sysdepinfo = NULL;
char *port_name = "portmidi";

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


void wait_until(PmTimestamp when)
{
    PtTimestamp now = Pt_Time();
    if (when > now) {
        Pt_Sleep(when - now);
    }
}


void main_test_output(int num)
{
    PmStream *midi;
    int32_t next_time;
    PmEvent buffer[1];
    PmTimestamp timestamp;
    int pitch = 60;
    int id;

    /* It is recommended to start timer before Midi; otherwise, PortMidi may
       start the timer with its (default) parameters
     */
    TIME_START;

    /* create a virtual output device */
    id = checkerror(Pm_CreateVirtualOutput(port_name, NULL, sysdepinfo));
    checkerror(Pm_OpenOutput(&midi, id, sysdepinfo, OUTPUT_BUFFER_SIZE,
                             TIME_PROC, TIME_INFO, latency));

    printf("Midi Output Virtual Device \"%s\" created.\n", port_name);
    printf("Type ENTER to send messages: ");
    while (getchar() != '\n') ;

    buffer[0].timestamp = Pt_Time();
#define PROGRAM 0
    buffer[0].message = Pm_Message(0xC0, PROGRAM, 0);
    Pm_Write(midi, buffer, 1);
    next_time = Pt_Time() + 1000;  /* wait 1s */
    while (num > 0) {
        wait_until(next_time);
        Pm_WriteShort(midi, next_time, Pm_Message(0x90, pitch, 100));
        printf("Note On pitch %d\n", pitch);
        num--;
        next_time += 500;

        wait_until(next_time);
        Pm_WriteShort(midi, next_time, Pm_Message(0x90, pitch, 0));
        printf("Note Off pitch %d\n", pitch);
        num--;
        pitch = (pitch + 1) % 12 + 60;
        next_time += 500;
    }

    /* close device (this not explicitly needed in most implementations) */
    printf("ready to close...");
    Pm_Close(midi);
    printf("done closing.\nNow delete the virtual device...");
    checkerror(Pm_DeleteVirtualDevice(id));
    printf("done deleting.\n");
}


void show_usage(void)
{
    printf("Usage: sendvirtual [-h] [-l latency-in-ms] [-m manufacturer] "
           "[-c clientname] [-p portname] [n]\n"
           "    -h for this message,\n"
           "    -l ms designates latency for precise timing (default 0),\n"
           "    -m name designates a manufacturer name (macOS only),\n"
           "    -c name designates a client name (linux only),\n"
           "    -p name designates a port name (linux only),\n"
           "    n is number of message to send.\n"
           "sends change program to 1, then one note per second with 0.5s on,\n"
           "0.5s off, for n/2 seconds. Latency >0 uses the device driver for \n"
           "precise timing (see PortMidi documentation).\n"); 
    exit(0);
}


int main(int argc, char *argv[])
{
    int num = 10;
    int i;
    if (argc <= 1) {
        show_usage();
    }
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            show_usage();
        } else if (strcmp(argv[i], "-l") == 0 && (i + 1 < argc)) {
            i = i + 1;
            latency = atoi(argv[i]);
            printf("Latency will be %d\n", latency);
        } else if (strcmp(argv[i], "-m") == 0 && (i + 1 < argc)) {
            i = i + 1;
            set_sysdepinfo('m', argv[i]);
            printf("Manufacturer name will be %s\n", argv[i]);
        } else if (strcmp(argv[i], "-p") == 0 && (i + 1 < argc)) {
            i = i + 1;
            port_name = argv[i];
            set_sysdepinfo('p', port_name);
            printf("Port name will be %s\n", port_name);
        } else if (strcmp(argv[i], "-c") == 0 && (i + 1 < argc)) {
            i = i + 1;
            set_sysdepinfo('c', argv[i]);
            printf("Client name will be %s\n", argv[i]);
        } else {
            num = atoi(argv[i]);
            if (num <= 0) {
                printf("Zero value or non-number for n\n");
                show_usage();
            }
            printf("Sending %d messages.\n", num);
        }
    }

    main_test_output(num);
    
    printf("finished sendvirtual test...type ENTER to quit...");
    while (getchar() != '\n') ;
    return 0;
}
