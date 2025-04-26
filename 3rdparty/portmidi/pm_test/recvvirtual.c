#include "portmidi.h"
#include "porttime.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"

#define INPUT_BUFFER_SIZE 100
#define DRIVER_INFO NULL
#define TIME_PROC ((PmTimeProcPtr) Pt_Time)
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

#define STRING_MAX 80 /* used for console input */

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
    id = checkerror(Pm_CreateVirtualInput("portmidi", NULL, DRIVER_INFO));
    checkerror(Pm_OpenInput(&midi, id, NULL, 0, NULL, NULL));

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


void show_usage()
{
    printf("Usage: recvvirtual [-h] [n]\n    use -h for this message,\n"
           "    n is number of message to wait for.\n");
    exit(0);
}


int main(int argc, char *argv[])
{
    char line[STRING_MAX];
    int num = 10;
    
    if (argc > 2) {
        show_usage();
    } else if (argc == 2) {
        if (strcmp(argv[1], "-h") == 0) {
            show_usage();
        } else {
            num = atoi(argv[1]);
            if (num <= 0) {
                show_usage();
            }
        }
    }

    main_test_input(num);
    
    printf("finished portMidi test...type ENTER to quit...");
    while (getchar() != '\n') ;
    return 0;
}
