#include "portmidi.h"
#include "porttime.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"

#define INPUT_BUFFER_SIZE 100
#define TIME_PROC ((PmTimeProcPtr) Pt_Time)
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

#define STRING_MAX 80 /* used for console input */

char *portname = "portmidi";

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


void main_test_input(int num)
{
    PmStream *midi;
    PmError status, length;
    PmEvent buffer[1];
    int id;
    int i = 0; /* count messages as they arrive */
    /* It is recommended to start timer before Midi; otherwise, PortMidi may
       start the timer with its (default) parameters
     */
    TIME_START;

    /* create a virtual input device */
    id = checkerror(Pm_CreateVirtualInput(port_name, NULL, sysdepinfo));
    checkerror(Pm_OpenInput(&midi, id, sysdepinfo, 0, NULL, NULL));

    printf("Midi Input opened. Reading %d Midi messages...\n", num);
    Pm_SetFilter(midi, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX);
    /* empty the buffer after setting filter, just in case anything
       got through */
    while (Pm_Poll(midi)) {
        Pm_Read(midi, buffer, 1);
    }
    /* now start paying attention to messages */
    while (i < num) {
        status = Pm_Poll(midi);
        if (status == TRUE) {
            length = Pm_Read(midi, buffer, 1);
            if (length > 0) {
                printf("Got message %d: time %ld, %2lx %2lx %2lx\n",
                       i,
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

    /* close device (this not explicitly needed in most implementations) */
    printf("ready to close...");
    Pm_Close(midi);
    printf("done closing.\nNow delete the virtual device...");
    checkerror(Pm_DeleteVirtualDevice(id));
    printf("done deleting.\n");
}


void show_usage(void)
{
    printf("Usage: recvvirtual [-h] [-m manufacturer] [-c clientname] "
           "[-p portname] [n]\n"
           "    -h for this message,\n"
           "    -m name designates a manufacturer name (macOS only),\n"
           "    -c name designates a client name (linux only),\n"
           "    -p name designates a port name (linux only),\n"
           "    n is number of message to wait for.\n");
    exit(0);
}


int main(int argc, char *argv[])
{
    char line[STRING_MAX];
    int num = 10;
    int i;
    if (argc <= 1) {
        show_usage();
    }
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0) {
            show_usage();
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
            printf("Waiting for %d messages.\n", num);
        }
    }

    main_test_input(num);
    
    printf("finished portMidi test...type ENTER to quit...");
    while (getchar() != '\n') ;
    return 0;
}
