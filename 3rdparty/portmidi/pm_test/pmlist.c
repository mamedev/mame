/* pmlist.c -- list portmidi devices and numbers
 *
 * This program lists devices. When you type return, it
 * restarts portmidi and lists devices again. It is mainly
 * a test for shutting down and restarting.
 *
 * Roger B. Dannenberg, Feb 2022
 */

#include "portmidi.h"
#include "porttime.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "assert.h"

#define DEVICE_INFO NULL
#define DRIVER_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

#define STRING_MAX 80 /* used for console input */

void show_usage(void)
{
    printf("Usage: pmlist [-h]\n        -h means help.\n"
           "    Type return to rescan and list devices, q<ret> to quit\n");
}


int main(int argc, char *argv[])
{
    if (argc > 1) {
        show_usage();
        exit(0);
    }

    while (1) {
        char input[STRING_MAX];
        const char *deflt;
        const char *in_or_out;
        int default_in, default_out, i;

        // Pm_Initialize();
        /* list device information */
        default_in = Pm_GetDefaultInputDeviceID();
        default_out = Pm_GetDefaultOutputDeviceID();
        for (i = 0; i < Pm_CountDevices(); i++) {
            const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
            printf("%d: %s, %s", i, info->interf, info->name);
            deflt = "";
            if (i == default_out || i == default_in) {
                deflt = "default ";
            }
            in_or_out = (info->input ? "input" : "output");
            printf(" (%s%s)\n", deflt, in_or_out);
        }
        if (fgets(input, STRING_MAX, stdin) && input[0] == 'q') {
            return 0;
        }
        Pm_Terminate();
    }
    return 0;
}
