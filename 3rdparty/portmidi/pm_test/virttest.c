/* virttest.c -- test for creating/deleting virtual ports */
/*
 * Roger B. Dannenberg
 * Oct 2021

This test is performed by running 2 instances of the program.  The
first instance makes input and output ports named portmidi and waits
for a message. The second tries to do the same, but will fail because
portmidi already exists. It then opens portmidi (both input and
output). In greater detail:

FIRST INSTANCE           SECOND INSTANCE
--------------           ---------------

initialize PortMidi      initialize PortMidi
create portmidi in
create portmidi out
wait for input
                         create portmidi in -> fails
                         open portmidi in/out
                         send to portmidi
recv from portmidi
send to portmidi
wait 1s                  recv from portmidi
                         close portmidi in and out
                         terminate PortMidi
list all devices:
 - check for correct number
 - check for good description of portmidi in port (open)
 - check for good description of portmidi out port (open)
close portmidi in
list all devices:
 - check for correct number
 - check for good description of portmidi in port (closed)
 - check for good description of portmidi out port (open)
close portmidi out
list all devices:
 - check for correct number
 - check for good description of portmidi in port (closed)
 - check for good description of portmidi out port (closed)
delete portmidi in
 - check for correct number
 - check for NULL description of portmidi in port
 - check for good description of portmidi out port (closed)
delete portmidi out
 - check for correct number
 - check for NULL description of portmidi in port
 - check for NULL description of portmidi out port
terminate portmidi
REPEAT 3 TIMES           wait 2 seconds to give head start to other instance
                         REPEAT 3 TIMES
 */

#include "portmidi.h"
#include "porttime.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"

#define OUTPUT_BUFFER_SIZE 0
#define INPUT_BUFFER_SIZE 10
#define DEVICE_INFO NULL
#define DRIVER_INFO NULL
#define TIME_PROC ((PmTimeProcPtr) Pt_Time)
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */


static void prompt_and_exit(void)
{
    printf("type ENTER...");
    while (getchar() != '\n') ;
    /* this will clean up open ports: */
    exit(-1);
}


static PmError printerror(PmError err, const char *msg)
{
    if (err == pmHostError) {
        /* it seems pointless to allocate memory and copy the string,
         * so I will do the work of Pm_GetHostErrorText directly
         */
        char errmsg[80];
        Pm_GetHostErrorText(errmsg, 80);
        printf("%s\n    %s\n", msg, errmsg);
    } else if (err < 0) {
        printf("%s\n    %s\n", msg, Pm_GetErrorText(err));
    }
    return err;
}


static PmError checkerror(PmError err)
{
    if (err < 0) {
        printerror(err, "PortMidi call failed...");
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


void show_usage()
{
    printf("Usage: virttest\n"
           "    run two instances to test virtual port create/delete\n");
}


void check_info(int id, char stat, int input, int virtual)
{
    const PmDeviceInfo *info = Pm_GetDeviceInfo(id);
    if (stat == 'd') {
        if (info) {
            printf("Expected device %d to be deleted.\n", id);
            prompt_and_exit();
        }
        return;
    }
    if (!info) {
        printf("Expected device %d to not be deleted.\n", id);
        prompt_and_exit();
    }
    if (strcmp("portmidi", info->name) != 0) {
        printf("Device %d name is %s, not \"portmidi\".\n", id, info->name);
        prompt_and_exit();
    }
    if (info->input != input || (!info->output) != input) {
        printf("Device %d input/output fields are wrong.\n", id);
        prompt_and_exit();
    }
    if ((!info->opened && stat == 'o') || (info->opened && stat == 'c')) {
        printf("Device %d opened==%d, status should be %c.\n", id,
               info->opened, stat);
        prompt_and_exit();
    }
    if (info->is_virtual != virtual) {
        printf("Expected device %d to be virtual.\n", id);
        prompt_and_exit();
    }
}


/* stat is 'o' for open, 'c' for closed, 'd' for deleted device */
void check_ports(int cnt, int in_id, char in_stat,
                 int out_id, char out_stat, int virtual)
{
    if (cnt != Pm_CountDevices()) {
        printf("Device count changed from %d to %d.\n", cnt, Pm_CountDevices());
        prompt_and_exit();
    }
    check_info(in_id, in_stat, TRUE, virtual);
    check_info(out_id, out_stat, FALSE, virtual);
}


void devices_list()
{
    int i;
    for (i = 0; i < Pm_CountDevices(); i++) {
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if (info) {
            printf("%d: %s %s %s %s\n", i, info->name,
                   (info->input ? "input" : "output"),
                   (info->is_virtual ? "virtual" : "real_device"),
                   (info->opened ? "opened" : "closed"));
        }
    }
}


void test2()
{
    PmStream *out = NULL;
    PmStream *in = NULL;
    int out_id;
    int in_id;
    PmEvent buffer[1];
    PmTimestamp timestamp;
    int pitch = 60;
    int device_count = 0;
    int i;

    printf("This must be virttest instance #2\n");

    /* find and open portmidi in and out */
    device_count = Pm_CountDevices();
    for (i = 0; i < device_count; i++) {
        const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
        if (info && strcmp(info->name, "portmidi") == 0) {
            if (info->input) {
                checkerror(Pm_OpenInput(&in, i, DRIVER_INFO,
                                   INPUT_BUFFER_SIZE, TIME_PROC, TIME_INFO));
                in_id = i;
            } else {
                checkerror(Pm_OpenOutput(&out, i, DRIVER_INFO,
                                         OUTPUT_BUFFER_SIZE, NULL, NULL, 0));
                out_id = i;
            }
        }
    }
    if (!in) {
        printf("Did not open portmidi as input (virtual output).\n");
        prompt_and_exit();
    }        
    if (!out) {
        printf("Did not open portmidi as output (virtual input).\n");
        prompt_and_exit();
    }        
    printf("Input device %d and output device %d are open.\n", in_id, out_id);

    /* send a message */
    buffer[0].timestamp = 0;
    buffer[0].message = Pm_Message(0x90, pitch, 100);
    checkerror(Pm_Write(out, buffer, 1));

    /* wait for reply */
    printf("Sent message, waiting for reply...\n");
    while (Pm_Read(in, buffer, 1) < 1) Pt_Sleep(10);
    
    printf("********** GOT THE MESSAGE, SHUTTING DOWN ************\n");

    /* close in */
    checkerror(Pm_Close(in));
    check_ports(device_count, in_id, 'c', out_id, 'o', FALSE);
    printf("Closed input %d\n", in_id);

    /* close out */
    checkerror(Pm_Close(out));
    check_ports(device_count, in_id, 'c', out_id, 'c', FALSE);
    printf("Closed output %d\n", out_id);

    Pt_Sleep(1000);
    /* wrap it up */
    Pm_Terminate();
    printf("Got reply and terminated...\n");
    Pt_Sleep(2000);  /* 2 seconds because other is waiting 1s. */
    /* 1 more second to make sure other shuts down before test repeats. */
}

extern int pm_check_errors;

void test()
{
    PmStream *out;
    PmStream *in;
    int out_id;
    int in_id;
    PmEvent buffer[1];
    PmTimestamp timestamp;
    int device_count = 0;

    TIME_START;

    printf("******** INITIALIZING PORTMIDI ***********\n");
    timestamp = Pt_Time();
    Pm_Initialize();
    printf("Pm_Initialize took %dms\n", Pt_Time() - timestamp);
    devices_list();

    pm_check_errors = FALSE;  /* otherwise, PM_CHECK_ERRORS, if defined,     */
    /* can cause this program to report an error and exit on pmNameConflict. */
    in_id = Pm_CreateVirtualInput("portmidi", NULL, DEVICE_INFO);
    pm_check_errors = TRUE;  /* there should be no other errors */
    if (in_id < 0) {
        printerror(in_id, "Pm_CreateVirtualInput failed...");
        test2();
        return;
    }
    printf("Created portmidi virtual input; this is virttest instance #1\n");
    out_id = checkerror(Pm_CreateVirtualOutput("portmidi", NULL, DRIVER_INFO));
    device_count = Pm_CountDevices();
    
    checkerror(Pm_OpenInput(&in, in_id, NULL, 0, NULL, NULL));
    checkerror(Pm_OpenOutput(&out, out_id, DRIVER_INFO, OUTPUT_BUFFER_SIZE,
                             TIME_PROC, TIME_INFO, 0));
    printf("Created/Opened input %d and output %d\n", in_id, out_id);
    Pm_SetFilter(in, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX);
    /* empty the buffer after setting filter, just in case anything
       got through */
    while (Pm_Read(in, buffer, 1)) ;

    /* wait for input */
    printf("Waiting for input...\n");
    while (Pm_Read(in, buffer, 1) < 1) Pt_Sleep(10);

    /* send two replies (only one would be fine) */
    checkerror(Pm_Write(out, buffer, 1));
    printf("Received input, writing output...\n");

    /* wait 1s so receiver can get the message before we shut down */
    Pt_Sleep(1000);
    printf("****** Closing everything and shutting down...\n");

    /* expect 2 open ports */
    check_ports(device_count, in_id, 'o', out_id, 'o', TRUE);
    /* close in */
    checkerror(Pm_Close(in));
    check_ports(device_count, in_id, 'c', out_id, 'o', TRUE);

    /* close out */
    checkerror(Pm_Close(out));
    check_ports(device_count, in_id, 'c', out_id, 'c', TRUE);

    /* delete in */
    checkerror(Pm_DeleteVirtualDevice(in_id));
    check_ports(device_count, in_id, 'd', out_id, 'c', TRUE);

    /* delete out */
    checkerror(Pm_DeleteVirtualDevice(out_id));
    check_ports(device_count, in_id, 'd', out_id, 'd', TRUE);
    
    /* we are done */
    Pm_Terminate();
}


int main(int argc, char *argv[])
{
    int i;
    show_usage();
    for (i = 0; i < 3; i++) {
        test();
    }    
    printf("finished virttest (SUCCESS). Type ENTER to quit...");
    while (getchar() != '\n') ;
    return 0;
}
