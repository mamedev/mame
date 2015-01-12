#include "osdepend.h"

const options_entry osd_options::s_option_entries[] =
{
    // debugging options
    { NULL,                                   NULL,       OPTION_HEADER,     "OSD DEBUGGING OPTIONS" },
    { OSDOPTION_LOG,                          "0",        OPTION_BOOLEAN,    "generate an error.log file" },
    { OSDOPTION_VERBOSE ";v",                 "0",        OPTION_BOOLEAN,    "display additional diagnostic information" },
    { OSDOPTION_DEBUG ";d",                   "0",        OPTION_BOOLEAN,    "enable/disable debugger" },
    { OSDOPTION_DEBUGGER,                     OSDOPTVAL_AUTO,      OPTION_STRING,    "debugger used : " },
    { OSDOPTION_OSLOG,                        "0",        OPTION_BOOLEAN,    "output error.log data to the system debugger" },
    { OSDOPTION_WATCHDOG ";wdog",             "0",        OPTION_INTEGER,    "force the program to terminate if no updates within specified number of seconds" },

    // performance options
    { NULL,                                   NULL,       OPTION_HEADER,     "OSD PERFORMANCE OPTIONS" },
    { OSDOPTION_MULTITHREADING ";mt",         "0",        OPTION_BOOLEAN,    "enable multithreading; this enables rendering and blitting on a separate thread" },
    { OSDOPTION_NUMPROCESSORS ";np",          OSDOPTVAL_AUTO,      OPTION_STRING,     "number of processors; this overrides the number the system reports" },
    { OSDOPTION_BENCH,                        "0",        OPTION_INTEGER,    "benchmark for the given number of emulated seconds; implies -video none -sound none -nothrottle" },
    // video options
    { NULL,                                   NULL,       OPTION_HEADER,     "OSD VIDEO OPTIONS" },
// OS X can be trusted to have working hardware OpenGL, so default to it on for the best user experience
    { OSDOPTION_VIDEO,                        OSDOPTVAL_AUTO,     OPTION_STRING,     "video output method: " },
    { OSDOPTION_NUMSCREENS "(1-4)",           "1",        OPTION_INTEGER,    "number of screens to create; usually, you want just one" },
    { OSDOPTION_WINDOW ";w",                  "0",        OPTION_BOOLEAN,    "enable window mode; otherwise, full screen mode is assumed" },
    { OSDOPTION_MAXIMIZE ";max",              "1",        OPTION_BOOLEAN,    "default to maximized windows; otherwise, windows will be minimized" },
    { OSDOPTION_KEEPASPECT ";ka",             "1",        OPTION_BOOLEAN,    "constrain to the proper aspect ratio" },
    { OSDOPTION_UNEVENSTRETCH ";ues",         "1",        OPTION_BOOLEAN,    "allow non-integer stretch factors" },
    { OSDOPTION_WAITVSYNC ";vs",              "0",        OPTION_BOOLEAN,    "enable waiting for the start of VBLANK before flipping screens; reduces tearing effects" },
    { OSDOPTION_SYNCREFRESH ";srf",           "0",        OPTION_BOOLEAN,    "enable using the start of VBLANK for throttling instead of the game time" },

    // per-window options
    { NULL,                                   NULL,             OPTION_HEADER,    "OSD PER-WINDOW VIDEO OPTIONS" },
    { OSDOPTION_SCREEN,                   OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_ASPECT ";screen_aspect",  OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio for all screens; 'auto' here will try to make a best guess" },
    { OSDOPTION_RESOLUTION ";r",          OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution for all screens; format is <width>x<height>[@<refreshrate>] or 'auto'" },
    { OSDOPTION_VIEW,                     OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for all screens" },

    { OSDOPTION_SCREEN "0",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the first screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_ASPECT "0",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the first screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_RESOLUTION "0;r0",        OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the first screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
    { OSDOPTION_VIEW "0",                    OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the first screen" },

    { OSDOPTION_SCREEN "1",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the second screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_ASPECT "1",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the second screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_RESOLUTION "1;r1",        OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the second screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
    { OSDOPTION_VIEW "1",                    OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the second screen" },

    { OSDOPTION_SCREEN "2",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the third screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_ASPECT "2",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the third screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_RESOLUTION "2;r2",        OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the third screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
    { OSDOPTION_VIEW "2",                    OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the third screen" },

    { OSDOPTION_SCREEN "3",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "explicit name of the fourth screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_ASPECT "3",                  OSDOPTVAL_AUTO,   OPTION_STRING,    "aspect ratio of the fourth screen; 'auto' here will try to make a best guess" },
    { OSDOPTION_RESOLUTION "3;r3",        OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred resolution of the fourth screen; format is <width>x<height>[@<refreshrate>] or 'auto'" },
    { OSDOPTION_VIEW "3",                    OSDOPTVAL_AUTO,   OPTION_STRING,    "preferred view for the fourth screen" },

    // full screen options
    { NULL,                                   NULL,  OPTION_HEADER,     "OSD FULL SCREEN OPTIONS" },
    { OSDOPTION_SWITCHRES,                    "0",   OPTION_BOOLEAN,    "enable resolution switching" },

    // sound options
    { NULL,                                   NULL,  OPTION_HEADER,     "OSD SOUND OPTIONS" },
    { OSDOPTION_SOUND,                        OSDOPTVAL_AUTO, OPTION_STRING,     "sound output method: " },
    { OSDOPTION_AUDIO_LATENCY "(1-5)",        "2",   OPTION_INTEGER,    "set audio latency (increase to reduce glitches, decrease for responsiveness)" },

    // End of list
    { NULL }
};
