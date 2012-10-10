/***************************************************************************

    jedutil.c

    JEDEC file utilities.

****************************************************************************

    Copyright Aaron Giles
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in
          the documentation and/or other materials provided with the
          distribution.
        * Neither the name 'MAME' nor the names of its contributors may be
          used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY AARON GILES ''AS IS'' AND ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL AARON GILES BE LIABLE FOR ANY DIRECT,
    INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

****************************************************************************

    Binary file format:

    Offset
        0 = Total number of fuses (32 bits)
        4 = Raw fuse data, packed 8 bits at a time, LSB to MSB

****************************************************************************

    Known types:

    20-pin devices:
        PAL10H8     = QP20 QF0320
        PAL12H6     = QP20 QF0320
        PAL14H4     = QP20
        PAL16H2     = QP20
        PAL16C1     = QP20
        PAL10L8     = QP20 QF0320
        PAL12L6     = QP20
        PAL14L4     = QP20
        PAL16L2     = QP20

        15S8        = QP20 QF0448

        PLS153      = QP20 QF1842

        PAL16L8     = QP20 QF2048

        PAL16R4     = QP20
        PAL16R6     = QP20
        PAL16R8     = QP20
        PAL16RA8    = QP20 QF2056

        PAL16V8R    = QP20 QF2194
        PALCE16V8   = QP20 QF2194
        GAL16V8A    = QP20 QF2194

        18CV8       = QP20 QF2696

    24-pin devices:
        PAL20L8     = QP24
        PAL20L10    = QP24
        PAL20R4     = QP24
        PAL20R6     = QP24
        PAL20R8     = QP24

        PAL20X4     = QP24
        PAL20X8     = QP24
        PAL20X10    = QP24

        PAL22V10    = QP24

        GAL20V8A    = QP24 QF2706
        GAL22V10    = QP24 QF5892

    28-pin devices:
        PLS100      = QP28 QF1928

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "corestr.h"
#include "jedparse.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ARRAY_LEN(_array) (sizeof(_array) / sizeof(_array[0]))

#define NO_OUTPUT_ENABLE_FUSE_ROW 0xFFFF

/* Output pin flags */
#define OUTPUT_ACTIVELOW     0x00000001
#define OUTPUT_ACTIVEHIGH    0x00000002
#define OUTPUT_COMBINATORIAL 0x00000004
#define OUTPUT_REGISTERED    0x00000008

/* Fuse state flag */
#define LOW_FUSE_BLOWN     0x00000001
#define HIGH_FUSE_BLOWN    0x00000002
#define LOWHIGH_FUSE_BLOWN 0x00000004
#define NO_FUSE_BLOWN      0x00000008

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef int (*command_func_type)(int argc, char *argv[]);

typedef struct _command_entry command_entry;
struct _command_entry
{
    const char *command;
    command_func_type command_func;
};



/* Pin fuse row configuration */
typedef struct _pin_fuse_rows pin_fuse_rows;
struct _pin_fuse_rows
{
    UINT16 pin;                 /* Pin number */
    UINT16 fuserowoutputenable; /* Fuse row for the output enable */
    UINT16 fuserowtermstart;    /* Fuse row for the first term */
    UINT16 fuserowtermend;      /* Fuse row for the last term */
};



/* Pin fuse column configuration */
typedef struct _pin_fuse_columns pin_fuse_columns;
struct _pin_fuse_columns
{
    UINT16 pin;            /* Pin number */
    UINT16 lowfusecolumn;  /* Column number for low output */
    UINT16 highfusecolumn; /* Column number for high output */
};


typedef struct _pal_data pal_data;

typedef void (*print_product_terms_func)(const pal_data* pal, const jed_data* jed);
typedef void (*config_pins_func)(const pal_data* pal, const jed_data* jed);
typedef int (*is_product_term_enabled_func)(const pal_data* pal, const jed_data* jed, UINT16 fuserow);

struct _pal_data
{
    const char *name;
    const pin_fuse_rows *pinfuserows;
    UINT16 pinfuserowscount;
    const pin_fuse_columns *pinfusecolumns;
    UINT16 pinfusecolumnscount;
    print_product_terms_func print_product_terms;
    config_pins_func config_pins;
    is_product_term_enabled_func is_product_term_enabled;
};



/* Pin output configuration */
typedef struct _pin_output_config pin_output_config;
struct _pin_output_config
{
    UINT16 pin;
    UINT16 flags;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

static void print_pal10l8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal10h8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal12l6_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal12h6_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal14l4_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal14h4_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16l2_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16h2_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16c1_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16l8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16r4_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16r6_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16r8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_gal16v8_product_terms(const pal_data* pal, const jed_data* jed);
/*static void print_gal18v10_product_terms(const pal_data* pal, const jed_data* jed);*/
static void print_pal20l8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20l10_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20r4_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20r6_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20r8_product_terms(const pal_data* pal, const jed_data* jed);



static void config_pal10l8_pins(const pal_data* pal, const jed_data* jed);
static void config_pal10h8_pins(const pal_data* pal, const jed_data* jed);
static void config_pal12l6_pins(const pal_data* pal, const jed_data* jed);
static void config_pal12h6_pins(const pal_data* pal, const jed_data* jed);
static void config_pal14l4_pins(const pal_data* pal, const jed_data* jed);
static void config_pal14h4_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16l2_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16h2_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16c1_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16l8_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16r4_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16r6_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16r8_pins(const pal_data* pal, const jed_data* jed);
static void config_gal16v8_pins(const pal_data* pal, const jed_data* jed);
/*static void config_gal18v10_pins(const pal_data* pal, const jed_data* jed);*/
static void config_pal20l8_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20l10_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20r4_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20r6_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20r8_pins(const pal_data* pal, const jed_data* jed);



static int is_gal16v8_product_term_enabled(const pal_data* pal, const jed_data* jed, UINT16 fuserow);



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static UINT8 *srcbuf;
static size_t srcbuflen;

static UINT8 *dstbuf;
static size_t dstbuflen;

static UINT16 inputpins[26];
static UINT16 inputpinscount;

static pin_output_config outputpins[26];
static UINT16 outputpinscount;

static pin_fuse_rows pal10l8pinfuserows[] = {
    {12, NO_OUTPUT_ENABLE_FUSE_ROW, 280, 300},
    {13, NO_OUTPUT_ENABLE_FUSE_ROW, 240, 260},
    {14, NO_OUTPUT_ENABLE_FUSE_ROW, 200, 220},
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 160, 180},
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 120, 140},
    {17, NO_OUTPUT_ENABLE_FUSE_ROW, 80, 100},
    {18, NO_OUTPUT_ENABLE_FUSE_ROW, 40, 60},
    {19, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 20}};

static pin_fuse_rows pal10h8pinfuserows[] = {
    {12, NO_OUTPUT_ENABLE_FUSE_ROW, 280, 300},
    {13, NO_OUTPUT_ENABLE_FUSE_ROW, 240, 260},
    {14, NO_OUTPUT_ENABLE_FUSE_ROW, 200, 220},
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 160, 180},
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 120, 140},
    {17, NO_OUTPUT_ENABLE_FUSE_ROW, 80, 100},
    {18, NO_OUTPUT_ENABLE_FUSE_ROW, 40, 60},
    {19, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 20}};

static pin_fuse_rows pal12l6pinfuserows[] = {
    {13, NO_OUTPUT_ENABLE_FUSE_ROW, 288, 360},
    {14, NO_OUTPUT_ENABLE_FUSE_ROW, 240, 264},
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 192, 216},
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 144, 168},
    {17, NO_OUTPUT_ENABLE_FUSE_ROW, 96, 120},
    {18, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 72}};

static pin_fuse_rows pal12h6pinfuserows[] = {
    {13, NO_OUTPUT_ENABLE_FUSE_ROW, 288, 360},
    {14, NO_OUTPUT_ENABLE_FUSE_ROW, 240, 264},
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 192, 216},
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 144, 168},
    {17, NO_OUTPUT_ENABLE_FUSE_ROW, 96, 120},
    {18, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 72}};

static pin_fuse_rows pal14l4pinfuserows[] = {
    {14, NO_OUTPUT_ENABLE_FUSE_ROW, 336, 420},
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 224, 308},
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 112, 196},
    {17, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 84}};

static pin_fuse_rows pal14h4pinfuserows[] = {
    {14, NO_OUTPUT_ENABLE_FUSE_ROW, 336, 420},
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 224, 308},
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 112, 196},
    {17, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 84}};

static pin_fuse_rows pal16l2pinfuserows[] = {
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 480},
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 224}};

static pin_fuse_rows pal16h2pinfuserows[] = {
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 480},
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 224}};

static pin_fuse_rows pal16c1pinfuserows[] = {
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 480},
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 480}};

static pin_fuse_rows pal16l8pinfuserows[] = {
    {12, 1792, 1824, 2016},
    {13, 1536, 1568, 1760},
    {14, 1280, 1312, 1504},
    {15, 1024, 1056, 1248},
    {16, 768, 800, 992},
    {17, 512, 544, 736},
    {18, 256, 288, 480},
    {19, 0, 32, 224}};

static pin_fuse_rows pal16r4pinfuserows[] = {
    {12, 1792, 1824, 2016},
    {13, 1536, 1568, 1760},
    {14, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1504}, /* Registered Output */
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 1024, 1248}, /* Registered Output */
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 768, 992},   /* Registered Output */
    {17, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 736},   /* Registered Output */
    {18, 256, 288, 480},
    {19, 0, 32, 224}};

static pin_fuse_rows pal16r6pinfuserows[] = {
    {12, 1792, 1824, 2016},
    {13, NO_OUTPUT_ENABLE_FUSE_ROW, 1536, 1760}, /* Registered Output */
    {14, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1504}, /* Registered Output */
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 1024, 1248}, /* Registered Output */
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 768, 992},   /* Registered Output */
    {17, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 736},   /* Registered Output */
    {18, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 480},   /* Registered Output */
    {19, 0, 32, 224}};

static pin_fuse_rows pal16r8pinfuserows[] = {
    {12, NO_OUTPUT_ENABLE_FUSE_ROW, 1792, 2016}, /* Registered Output */
    {13, NO_OUTPUT_ENABLE_FUSE_ROW, 1536, 1760}, /* Registered Output */
    {14, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1504}, /* Registered Output */
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 1024, 1248}, /* Registered Output */
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 768, 992},   /* Registered Output */
    {17, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 736},   /* Registered Output */
    {18, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 480},   /* Registered Output */
    {19, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 224}};    /* Registered Output */

static pin_fuse_rows gal16v8pinfuserows[] = {
    {12, 0, 0, 0},
    {13, 0, 0, 0},
    {14, 0, 0, 0},
    {15, 0, 0, 0},
    {16, 0, 0, 0},
    {17, 0, 0, 0},
    {18, 0, 0, 0},
    {19, 0, 0, 0}};

/*static pin_fuse_rows gal18v10pinfuserows[] = {
    {9,  3096, 3132, 3384},
    {11, 2772, 2808, 3060},
    {12, 2448, 2484, 2736},
    {13, 2124, 2160, 2412},
    {14, 1728, 1764, 2088},
    {15, 1332, 1368, 1692},
    {16, 1008, 1044, 1296},
    {17, 684,  720,  972},
    {18, 360,  396,  648},
    {19, 36,   72,   324}};*/

static pin_fuse_rows pal20l8pinfuserows[] = {
    {15, 2240, 2280, 2520},
    {16, 1920, 1960, 2200},
    {17, 1600, 1640, 1880},
    {18, 1280, 1320, 1560},
    {19, 960, 1000, 1240},
    {20, 640, 680, 920},
    {21, 320, 360, 600},
    {22, 0, 40, 280}};

static pin_fuse_rows pal20l10pinfuserows[] = {
    {14, 1440, 1480, 1560},
    {15, 1280, 1320, 1400},
    {16, 1120, 1160, 1240},
    {17, 960,  1000, 1080},
    {18, 800,  840,  920},
    {19, 640,  680,  760},
    {20, 480,  520,  600},
    {21, 320,  360,  440},
    {22, 160,  200,  280},
    {23, 0,    40,   120}};

static pin_fuse_rows pal20r4pinfuserows[] = {
    {15, 2240, 2280, 2520},
    {16, 1920, 1960, 2200},
    {17, NO_OUTPUT_ENABLE_FUSE_ROW, 1600, 1880}, /* Registered Output */
    {18, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1560}, /* Registered Output */
    {19, NO_OUTPUT_ENABLE_FUSE_ROW, 960, 1240},  /* Registered Output */
    {20, NO_OUTPUT_ENABLE_FUSE_ROW, 640, 920},   /* Registered Output */
    {21, 320, 360, 600},
    {22, 0, 40, 280}};

static pin_fuse_rows pal20r6pinfuserows[] = {
    {15, 2240, 2280, 2520},
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 1920, 2200}, /* Registered Output */
    {17, NO_OUTPUT_ENABLE_FUSE_ROW, 1600, 1880}, /* Registered Output */
    {18, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1560}, /* Registered Output */
    {19, NO_OUTPUT_ENABLE_FUSE_ROW, 960, 1240},  /* Registered Output */
    {20, NO_OUTPUT_ENABLE_FUSE_ROW, 640, 920},   /* Registered Output */
    {21, NO_OUTPUT_ENABLE_FUSE_ROW, 320, 600},   /* Registered Output */
    {22, 0, 40, 280}};

static pin_fuse_rows pal20r8pinfuserows[] = {
    {15, NO_OUTPUT_ENABLE_FUSE_ROW, 2240, 2520}, /* Registered Output */
    {16, NO_OUTPUT_ENABLE_FUSE_ROW, 1920, 2200}, /* Registered Output */
    {17, NO_OUTPUT_ENABLE_FUSE_ROW, 1600, 1880}, /* Registered Output */
    {18, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1560}, /* Registered Output */
    {19, NO_OUTPUT_ENABLE_FUSE_ROW, 960, 1240},  /* Registered Output */
    {20, NO_OUTPUT_ENABLE_FUSE_ROW, 640, 920},   /* Registered Output */
    {21, NO_OUTPUT_ENABLE_FUSE_ROW, 320, 600},   /* Registered Output */
    {22, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 280}};    /* Registered Output */

static pin_fuse_columns pal10l8pinfusecolumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 7, 6},
    {5, 9, 8},
    {6, 11, 10},
    {7, 13, 12},
    {8, 15, 14},
    {9, 17, 16},
    {11, 19, 18}};

static pin_fuse_columns pal10h8pinfusecolumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 7, 6},
    {5, 9, 8},
    {6, 11, 10},
    {7, 13, 12},
    {8, 15, 14},
    {9, 17, 16},
    {11, 19, 18}};

static pin_fuse_columns pal12l6pinfusecolumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 11, 10},
    {6, 13, 12},
    {7, 15, 14},
    {8, 17, 16},
    {9, 21, 20},
    {11, 23, 22},
    {12, 19, 18},
    {19, 7, 6}};

static pin_fuse_columns pal12h6pinfusecolumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 11, 10},
    {6, 13, 12},
    {7, 15, 14},
    {8, 17, 16},
    {9, 21, 20},
    {11, 23, 22},
    {12, 19, 18},
    {19, 7, 6}};

static pin_fuse_columns pal14l4pinfusecolumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 15, 14},
    {7, 17, 16},
    {8, 21, 20},
    {9, 25, 24},
    {11, 27, 26},
    {12, 23, 22},
    {13, 19, 18},
    {18, 11, 10},
    {19, 7, 6}};

static pin_fuse_columns pal14h4pinfusecolumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 15, 14},
    {7, 17, 16},
    {8, 21, 20},
    {9, 25, 24},
    {11, 27, 26},
    {12, 23, 22},
    {13, 19, 18},
    {18, 11, 10},
    {19, 7, 6}};

static pin_fuse_columns pal16l2pinfusecolumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {11, 31, 30},
    {12, 27, 26},
    {13, 23, 22},
    {14, 19, 18},
    {17, 15, 14},
    {18, 11, 10},
    {19, 7, 6}};

static pin_fuse_columns pal16h2pinfusecolumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {11, 31, 30},
    {12, 27, 26},
    {13, 23, 22},
    {14, 19, 18},
    {17, 15, 14},
    {18, 11, 10},
    {19, 7, 6}};

static pin_fuse_columns pal16c1pinfusecolumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {11, 31, 30},
    {12, 27, 26},
    {13, 23, 22},
    {14, 19, 18},
    {17, 15, 14},
    {18, 11, 10},
    {19, 7, 6}};

static pin_fuse_columns pal16l8pinfusecolumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {11, 31, 30},
    {13, 27, 26},
    {14, 23, 22},
    {15, 19, 18},
    {16, 15, 14},
    {17, 11, 10},
    {18, 7, 6}};

static pin_fuse_columns pal16r4pinfusecolumns[] = {
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {12, 31, 30},
    {13, 27, 26},
    {14, 23, 22}, /* Registered Output */
    {15, 19, 18}, /* Registered Output */
    {16, 15, 14}, /* Registered Output */
    {17, 11, 10}, /* Registered Output */
    {18, 7, 6},
    {19, 3, 2}};

static pin_fuse_columns pal16r6pinfusecolumns[] = {
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {12, 31, 30},
    {13, 27, 26}, /* Registered Output */
    {14, 23, 22}, /* Registered Output */
    {15, 19, 18}, /* Registered Output */
    {16, 15, 14}, /* Registered Output */
    {17, 11, 10}, /* Registered Output */
    {18, 7, 6},   /* Registered Output */
    {19, 3, 2}};

static pin_fuse_columns pal16r8pinfusecolumns[] = {
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {12, 31, 30}, /* Registered Output */
    {13, 27, 26}, /* Registered Output */
    {14, 23, 22}, /* Registered Output */
    {15, 19, 18}, /* Registered Output */
    {16, 15, 14}, /* Registered Output */
    {17, 11, 10}, /* Registered Output */
    {18, 7, 6},   /* Registered Output */
    {19, 3, 2}};  /* Registered Output */

static pin_fuse_columns gal16v8pinfusecolumns[] = {
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0}};

/*static pin_fuse_columns gal18v10pinfusecolumns[] = {
    {1,  1,  0},
    {2,  5,  4},
    {3,  9,  8},
    {4,  13, 12},
    {5,  17, 16},
    {6,  21, 20},
    {7,  25, 24},
    {8,  29, 28},
    {9,  35, 34},
    {11, 33, 32},
    {12, 31, 30},
    {13, 27, 26},
    {14, 23, 22},
    {15, 19, 18},
    {16, 15, 14},
    {17, 11, 10},
    {18, 7,  6},
    {19, 3,  2}};*/

static pin_fuse_columns pal20l8pinfusecolumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {13, 39, 38},
    {14, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {23, 7, 6}};

static pin_fuse_columns pal20l10pinfusecolumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {13, 39, 38},
    {15, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {22, 7, 6}};

static pin_fuse_columns pal20r4pinfusecolumns[] = {
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {14, 39, 38},
    {15, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {22, 7, 6},
    {23, 3, 2}};

static pin_fuse_columns pal20r6pinfusecolumns[] = {
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {14, 39, 38},
    {15, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {22, 7, 6},
    {23, 3, 2}};

static pin_fuse_columns pal20r8pinfusecolumns[] = {
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {14, 39, 38},
    {15, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {22, 7, 6},
    {23, 3, 2}};

static pal_data paldata[] = {
    {"PAL10L8",
        pal10l8pinfuserows, ARRAY_LEN(pal10l8pinfuserows),
        pal10l8pinfusecolumns, ARRAY_LEN(pal10l8pinfusecolumns),
        print_pal10l8_product_terms,
        config_pal10l8_pins,
        NULL},
    {"PAL10H8",
        pal10h8pinfuserows, ARRAY_LEN(pal10h8pinfuserows),
        pal10h8pinfusecolumns, ARRAY_LEN(pal10h8pinfusecolumns),
        print_pal10h8_product_terms,
        config_pal10h8_pins,
        NULL},
    {"PAL12H6",
        pal12h6pinfuserows, ARRAY_LEN(pal12h6pinfuserows),
        pal12h6pinfusecolumns, ARRAY_LEN(pal12h6pinfusecolumns),
        print_pal12h6_product_terms,
        config_pal12h6_pins,
        NULL},
    {"PAL14H4",
        pal14h4pinfuserows, ARRAY_LEN(pal14h4pinfuserows),
        pal14h4pinfusecolumns, ARRAY_LEN(pal14h4pinfusecolumns),
        print_pal14h4_product_terms,
        config_pal14h4_pins,
        NULL},
    {"PAL16H2",
        pal16h2pinfuserows, ARRAY_LEN(pal16h2pinfuserows),
        pal16h2pinfusecolumns, ARRAY_LEN(pal16h2pinfusecolumns),
        print_pal16h2_product_terms,
        config_pal16h2_pins,
        NULL},
    {"PAL16C1",
        pal16c1pinfuserows, ARRAY_LEN(pal16c1pinfuserows),
        pal16c1pinfusecolumns, ARRAY_LEN(pal16c1pinfusecolumns),
        print_pal16c1_product_terms,
        config_pal16c1_pins,
        NULL},
    {"PAL12L6",
        pal12l6pinfuserows, ARRAY_LEN(pal12l6pinfuserows),
        pal12l6pinfusecolumns, ARRAY_LEN(pal12l6pinfusecolumns),
        print_pal12l6_product_terms,
        config_pal12l6_pins,
        NULL},
    {"PAL14L4",
        pal14l4pinfuserows, ARRAY_LEN(pal14l4pinfuserows),
        pal14l4pinfusecolumns, ARRAY_LEN(pal14l4pinfusecolumns),
        print_pal14l4_product_terms,
        config_pal14l4_pins,
        NULL},
    {"PAL16L2",
        pal16l2pinfuserows, ARRAY_LEN(pal16l2pinfuserows),
        pal16l2pinfusecolumns, ARRAY_LEN(pal16l2pinfusecolumns),
        print_pal16l2_product_terms,
        config_pal16l2_pins,
        NULL},
    /*{"15S8", NULL, 0, NULL, 0, NULL, NULL, NULL},
    {"PLS153", NULL, 0, NULL, 0, NULL, NULL, NULL},*/
    {"PAL16L8",
        pal16l8pinfuserows, ARRAY_LEN(pal16l8pinfuserows),
        pal16l8pinfusecolumns, ARRAY_LEN(pal16l8pinfusecolumns),
        print_pal16l8_product_terms,
        config_pal16l8_pins,
        NULL},
    {"PAL16R4",
        pal16r4pinfuserows, ARRAY_LEN(pal16r4pinfuserows),
        pal16r4pinfusecolumns, ARRAY_LEN(pal16r4pinfusecolumns),
        print_pal16r4_product_terms,
        config_pal16r4_pins,
        NULL},
    {"PAL16R6",
        pal16r6pinfuserows, ARRAY_LEN(pal16r6pinfuserows),
        pal16r6pinfusecolumns, ARRAY_LEN(pal16r6pinfusecolumns),
        print_pal16r6_product_terms,
        config_pal16r6_pins,
        NULL},
    {"PAL16R8",
        pal16r8pinfuserows, ARRAY_LEN(pal16r8pinfuserows),
        pal16r8pinfusecolumns, ARRAY_LEN(pal16r8pinfusecolumns),
        print_pal16r8_product_terms,
        config_pal16r8_pins,
        NULL},
    /*{"PAL16RA8", NULL, 0, NULL, 0, NULL, NULL, NULL},
    {"PAL16V8R", NULL, 0, NULL, 0, NULL, NULL, NULL},
    {"PALCE16V8", NULL, 0, NULL, 0, NULL, NULL, NULL},*/
    {"GAL16V8",
        gal16v8pinfuserows, ARRAY_LEN(gal16v8pinfuserows),
        gal16v8pinfusecolumns, ARRAY_LEN(gal16v8pinfusecolumns),
        print_gal16v8_product_terms,
        config_gal16v8_pins,
        is_gal16v8_product_term_enabled},
    /*{"18CV8", NULL, 0, NULL, 0, NULL},
    {"GAL18V10",
        gal18v10pinfuserows, ARRAY_LEN(gal18v10pinfuserows),
        gal18v10pinfusecolumns, ARRAY_LEN(gal18v10pinfusecolumns),
        print_gal18v10_product_terms,
        config_gal18v10_pins,
        NULL},*/
    {"PAL20L8",
        pal20l8pinfuserows, ARRAY_LEN(pal20l8pinfuserows),
        pal20l8pinfusecolumns, ARRAY_LEN(pal20l8pinfusecolumns),
        print_pal20l8_product_terms,
        config_pal20l8_pins,
        NULL},
    {"PAL20L10",
        pal20l10pinfuserows, ARRAY_LEN(pal20l10pinfuserows),
        pal20l10pinfusecolumns, ARRAY_LEN(pal20l10pinfusecolumns),
        print_pal20l10_product_terms,
        config_pal20l10_pins,
        NULL},
    {"PAL20R4",
        pal20r4pinfuserows, ARRAY_LEN(pal20r4pinfuserows),
        pal20r4pinfusecolumns, ARRAY_LEN(pal20r4pinfusecolumns),
        print_pal20r4_product_terms,
        config_pal20r4_pins,
        NULL},
    {"PAL20R6",
        pal20r6pinfuserows, ARRAY_LEN(pal20r6pinfuserows),
        pal20r6pinfusecolumns, ARRAY_LEN(pal20r6pinfusecolumns),
        print_pal20r6_product_terms,
        config_pal20r6_pins,
        NULL},
    {"PAL20R8",
        pal20r8pinfuserows, ARRAY_LEN(pal20r8pinfuserows),
        pal20r8pinfusecolumns, ARRAY_LEN(pal20r8pinfusecolumns),
        print_pal20r8_product_terms,
        config_pal20r8_pins, NULL}/*,
    {"PAL20X4", NULL, 0, NULL, 0, NULL, NULL, NULL},
    {"PAL20X8", NULL, 0, NULL, 0, NULL, NULL, NULL},
    {"PAL20X10", NULL, 0, NULL, 0, NULL, NULL, NULL},
    {"PAL22V10", NULL, 0, NULL, 0, NULL, NULL, NULL},
    {"GAL20V8A", NULL, 0, NULL, 0, NULL, NULL, NULL},
    {"GAL22V10", NULL, 0, NULL, 0, NULL, NULL, NULL},
    {"PLS100", NULL, 0, NULL, 0, NULL, NULL, NULL}*/};



/***************************************************************************
    CORE IMPLEMENTATION
***************************************************************************/

/*-------------------------------------------------
    is_jed_file - test if the file extension is
    that of a JED file
-------------------------------------------------*/

static int is_jed_file(const char *file)
{
	int len;

	/* does the source end in '.jed'? */
	len = strlen(file);

	return (file[len - 4] == '.' &&
           tolower((UINT8)file[len - 3]) == 'j' &&
           tolower((UINT8)file[len - 2]) == 'e' &&
           tolower((UINT8)file[len - 1]) == 'd');
}



/*-------------------------------------------------
    find_pal_data - finds the data associated
    with a pal name
-------------------------------------------------*/

static const pal_data* find_pal_data(const char *name)
{
    int index;

    for (index = 0; index < ARRAY_LEN(paldata); ++index)
    {
        if (!core_stricmp(name, paldata[index].name))
        {
            return &paldata[index];
        }
    }

    return NULL;
}



/*-------------------------------------------------
    find_fuse_rows - finds the fuse row data for
    an output pin.
-------------------------------------------------*/

static const pin_fuse_rows* find_fuse_rows(const pal_data* pal, UINT16 pin)
{
    UINT16 index;

    for (index = 0; index < pal->pinfuserowscount; ++index)
    {
        if (pal->pinfuserows[index].pin == pin)
        {
            return &pal->pinfuserows[index];
        }
    }

    return NULL;
}



/*-------------------------------------------------
    find_fuse_columns - finds the fuse column
    data for an input pin.
-------------------------------------------------*/

static const pin_fuse_columns* find_fuse_columns(const pal_data* pal, UINT16 pin)
{
    UINT16 index;

    for (index = 0; index < pal->pinfusecolumnscount; ++index)
    {
        if (pal->pinfusecolumns[index].pin == pin)
        {
            return &pal->pinfusecolumns[index];
        }
    }

    return NULL;
}



/*-------------------------------------------------
    calc_fuse_column_count - calculates the total
    columns of a pal
-------------------------------------------------*/

static UINT16 calc_fuse_column_count(const pal_data* pal)
{
    return pal->pinfusecolumnscount * 2;
}



/*-------------------------------------------------
    all_fuses_in_row_blown - checks if a fuse row
    is all blown
-------------------------------------------------*/

static int all_fuses_in_row_blown(const pal_data* pal, const jed_data* jed, UINT16 fuserow)
{
    UINT16 columncount = calc_fuse_column_count(pal);
    UINT16 column;

    for (column = 0; column < columncount; ++column)
    {
        if (!jed_get_fuse(jed, fuserow + column))
        {
            return 0;
        }
    }

    return 1;
}



/*-------------------------------------------------
    any_fuses_in_row_blown - checks if any fuses in
    a row have been blown.
-------------------------------------------------*/

static int any_fuses_in_row_blown(const pal_data* pal, const jed_data* jed, UINT16 fuserow)
{
    UINT16 columncount = calc_fuse_column_count(pal);
    UINT16 column;

    for (column = 0; column < columncount; ++column)
    {
        if (jed_get_fuse(jed, fuserow + column))
        {
            return 1;
        }
    }

    return 0;
}



/*-------------------------------------------------
    set_input_pins - saves the pins that can be
    used by a product term
-------------------------------------------------*/

static void set_input_pins(const UINT16* pins, UINT16 pin_count)
{
    UINT16 index;

    for (index = 0; index < pin_count; ++index)
    {
        inputpins[index] = pins[index];

        ++inputpinscount;
    }
}



/*-------------------------------------------------
    set_output_pins - saves the output pins
-------------------------------------------------*/

static void set_output_pins(const pin_output_config* pin_output_configs, UINT16 pin_count)
{
    UINT16 index;

    for (index = 0; index < pin_count; ++index)
    {
        outputpins[index].pin = pin_output_configs[index].pin;
        outputpins[index].flags = pin_output_configs[index].flags;

        ++outputpinscount;
    }
}



/*-------------------------------------------------
    is_output_pin - determines if the pin is an
    output pin
-------------------------------------------------*/

static int is_output_pin(UINT16 pin)
{
    UINT16 index;

    for (index = 0; index < outputpinscount; ++index)
    {
        if (outputpins[index].pin == pin)
        {
            return 1;
        }
    }

    return 0;
}



/*-------------------------------------------------
    get_pin_output_flags - gets the flags
    of an output pin
-------------------------------------------------*/

static UINT16 get_pin_output_flags(UINT16 pin)
{
    UINT16 index;

    for (index = 0; index < outputpinscount; ++index)
    {
        if (outputpins[index].pin == pin)
        {
            return outputpins[index].flags;
        }
    }

    return 0;
}



/*-------------------------------------------------
    get_pin_fuse_state - gets the fuse state of
    an input pin
-------------------------------------------------*/

static UINT16 get_pin_fuse_state(const pal_data* pal, const jed_data* jed, UINT16 pin, UINT16 fuserow)
{
    const pin_fuse_columns* fuse_columns = find_fuse_columns(pal, pin);
    int lowfusestate, highfusestate;

    if (!fuse_columns)
    {
        fprintf(stderr, "Fuse column data missing for pin %d!\n", pin);

        return NO_FUSE_BLOWN;
    }

    lowfusestate = jed_get_fuse(jed, fuserow + fuse_columns->lowfusecolumn);
    highfusestate = jed_get_fuse(jed, fuserow + fuse_columns->highfusecolumn);

    if (!lowfusestate && highfusestate)
    {
        return LOW_FUSE_BLOWN;
    }
    else if (lowfusestate && !highfusestate)
    {
        return HIGH_FUSE_BLOWN;
    }
    else if (!lowfusestate && !highfusestate)
    {
        return NO_FUSE_BLOWN;
    }

    return LOWHIGH_FUSE_BLOWN;
}



/*-------------------------------------------------
    generate_product_terms - prints the product
    terms for a fuse row
-------------------------------------------------*/

static void generate_product_terms(const pal_data* pal, const jed_data* jed, UINT16 fuserow, char* buffer)
{
    UINT16 index, pin, fuse_state, haveterm, flags;
    char tmpbuffer[20];

    *buffer = 0;
    haveterm = 0;

    if (pal->is_product_term_enabled && !pal->is_product_term_enabled(pal, jed, fuserow))
    {
        return;        
    }

    for (index = 0; index < inputpinscount; ++index)
    {
        pin = inputpins[index];

        fuse_state = get_pin_fuse_state(pal, jed, pin, fuserow);

        if (fuse_state == LOW_FUSE_BLOWN)
        {
            if (haveterm)
            {
                strcat(buffer,  " & ");
            }

            if (!is_output_pin(pin))
            {
                sprintf(tmpbuffer, "/i%d", pin);
                strcat(buffer, tmpbuffer);
            }
            else
            {
                flags = get_pin_output_flags(pin);

                if (flags & OUTPUT_COMBINATORIAL)
                {
                    sprintf(tmpbuffer, "/o%d", pin);
                }
                else if (flags & OUTPUT_REGISTERED)
                {
                    sprintf(tmpbuffer, "/rf%d", pin);
                }

                strcat(buffer, tmpbuffer);
            }

            haveterm = 1;
        }

        if (fuse_state == HIGH_FUSE_BLOWN)
        {
            if (haveterm)
            {
                strcat(buffer, " & ");
            }

            if (!is_output_pin(pin))
            {
                sprintf(tmpbuffer, "i%d", pin);
                strcat(buffer, tmpbuffer);
            }
            else
            {
                flags = get_pin_output_flags(pin);

                if (flags & OUTPUT_COMBINATORIAL)
                {
                    sprintf(tmpbuffer, "o%d", pin);
                }
                else if (flags & OUTPUT_REGISTERED)
                {
                    sprintf(tmpbuffer, "rf%d", pin);
                }

                strcat(buffer, tmpbuffer);
            }

            haveterm = 1;
        }
    }
}



/*-------------------------------------------------
    print_product_terms - prints the product terms
    for a pal
-------------------------------------------------*/

static void print_product_terms(const pal_data* pal, const jed_data* jed)
{
    UINT16 index, columncount, flags, row, haveterms;
    char buffer[200];
    int indent, indentindex;
    const pin_fuse_rows* fuse_rows;

    columncount = calc_fuse_column_count(pal);

    for (index = 0; index < outputpinscount; ++index)
    {
        flags = outputpins[index].flags;

        indent = 0;

        if (flags & OUTPUT_ACTIVELOW)
        {
            printf("/");

            ++indent;
        }

        if (flags & OUTPUT_COMBINATORIAL)
        {
            sprintf(buffer, "o%d = ", outputpins[index].pin);
        }
        else if (flags & OUTPUT_REGISTERED)
        {
            sprintf(buffer, "rf%d := ", outputpins[index].pin);
        }
        else
        {
        	fprintf(stderr, "Unknown output type for pin %d!\n", outputpins[index].pin);

            continue;
        }

        printf("%s", buffer);

        haveterms = 0;
        indent += strlen(buffer);

        fuse_rows = find_fuse_rows(pal, outputpins[index].pin);

        if (!fuse_rows)
        {
        	fprintf(stderr, "Fuse row data missing!\n");

            continue;
        }

        for (row = fuse_rows->fuserowtermstart; row <= fuse_rows->fuserowtermend;
             row += columncount)
        {
            generate_product_terms(pal, jed, row, buffer);

            if (strlen(buffer) > 0)
            {
                if (haveterms)
                {
                    printf(" +\n");

                    for (indentindex = 0; indentindex < indent; ++indentindex)
                    {
                        printf(" ");
                    }
                }
                else
                {
                    haveterms = 1;
                }

                printf("%s", buffer);
            }
        }

        printf("\n");

        /* output enable equations */

        if (flags & OUTPUT_COMBINATORIAL)
        {
            printf("o%d.oe = ", outputpins[index].pin);

            if (fuse_rows->fuserowoutputenable == NO_OUTPUT_ENABLE_FUSE_ROW ||
                all_fuses_in_row_blown(pal, jed, fuse_rows->fuserowoutputenable))
            {
                printf("vcc\n");
            }
            else
            {
                generate_product_terms(pal, jed, fuse_rows->fuserowoutputenable, buffer);

                printf("%s\n", buffer);
            }
        }
        else if (flags & OUTPUT_REGISTERED)
        {
            printf("rf%d.oe = OE\n", outputpins[index].pin);
        }

        printf("\n");
    }
}



/*-------------------------------------------------
    print_pal10l8_product_terms - prints the product
    terms for a PAL10L8
-------------------------------------------------*/

static void print_pal10l8_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal10h8_product_terms - prints the product
    terms for a PAL10H8
-------------------------------------------------*/

static void print_pal10h8_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal12l6_product_terms - prints the product
    terms for a PAL12L6
-------------------------------------------------*/

static void print_pal12l6_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal12h6_product_terms - prints the product
    terms for a PAL12H6
-------------------------------------------------*/

static void print_pal12h6_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal14l4_product_terms - prints the product
    terms for a PAL14L4
-------------------------------------------------*/

static void print_pal14l4_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal14h4_product_terms - prints the product
    terms for a PAL14H4
-------------------------------------------------*/

static void print_pal14h4_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16l2_product_terms - prints the product
    terms for a PAL16L2
-------------------------------------------------*/

static void print_pal16l2_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16h2_product_terms - prints the product
    terms for a PAL16H2
-------------------------------------------------*/

static void print_pal16h2_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16h2_product_terms - prints the product
    terms for a PAL16C1
-------------------------------------------------*/

static void print_pal16c1_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16l8_product_terms - prints the product
    terms for a PAL16L8
-------------------------------------------------*/

static void print_pal16l8_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16r4_product_terms - prints the product
    terms for a PAL16R4
-------------------------------------------------*/

static void print_pal16r4_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16r6_product_terms - prints the product
    terms for a PAL16R6
-------------------------------------------------*/

static void print_pal16r6_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16r8_product_terms - prints the product
    terms for a PAL16R8
-------------------------------------------------*/

static void print_pal16r8_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_gal16v8_product_terms - prints the product
    terms for a GAL16V8
-------------------------------------------------*/

static void print_gal16v8_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_gal18v10_product_terms - prints the product
    terms for a GAL18V10
-------------------------------------------------*/

/*static void print_gal18v10_product_terms(const pal_data* pal, const jed_data* jed)
{
    printf("Viewing product terms are not supported.\n");
}*/



/*-------------------------------------------------
    print_pal20l8_product_terms - prints the product
    terms for a PAL20L8
-------------------------------------------------*/

static void print_pal20l8_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal20l10_product_terms - prints the product
    terms for a PAL20L10
-------------------------------------------------*/

static void print_pal20l10_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal20r4_product_terms - prints the product
    terms for a PAL20R4
-------------------------------------------------*/

static void print_pal20r4_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal20r6_product_terms - prints the product
    terms for a PAL20R6
-------------------------------------------------*/

static void print_pal20r6_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal20r8_product_terms - prints the product
    terms for a PAL20R8
-------------------------------------------------*/

static void print_pal20r8_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed);
}



/*-------------------------------------------------
    config_pal10l8_pins - configures the pins for
    a PAL10L8
-------------------------------------------------*/

static void config_pal10l8_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
    static pin_output_config output_pins[] = {
        {12, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {13, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {14, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {17, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {18, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {19, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL}};

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, ARRAY_LEN(output_pins));
}



/*-------------------------------------------------
    config_pal10h8_pins - configures the pins for
    a PAL10H8
-------------------------------------------------*/

static void config_pal10h8_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
    static pin_output_config output_pins[] = {
        {12, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {13, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {14, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {15, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {17, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {18, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {19, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL}};

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, ARRAY_LEN(output_pins));
}



/*-------------------------------------------------
    config_pal12l6_pins - configures the pins for
    a PAL12L6
-------------------------------------------------*/

static void config_pal12l6_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 19};
    static pin_output_config output_pins[] = {
        {13, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {14, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {17, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {18, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL}};

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, ARRAY_LEN(output_pins));
}



/*-------------------------------------------------
    config_pal12h6_pins - configures the pins for
    a PAL12H6
-------------------------------------------------*/

static void config_pal12h6_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 19};
    static pin_output_config output_pins[] = {
        {13, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {14, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {15, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {17, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {18, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL}};

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, ARRAY_LEN(output_pins));
}



/*-------------------------------------------------
    config_pal14l4_pins - configures the pins for
    a PAL14L4
-------------------------------------------------*/

static void config_pal14l4_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 18, 19};
    static pin_output_config output_pins[] = {
        {14, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {17, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL}};

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, ARRAY_LEN(output_pins));
}



/*-------------------------------------------------
    config_pal14h4_pins - configures the pins for
    a PAL14H4
-------------------------------------------------*/

static void config_pal14h4_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 18, 19};
    static pin_output_config output_pins[] = {
        {14, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {15, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {17, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL}};

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, ARRAY_LEN(output_pins));
}



/*-------------------------------------------------
    config_pal16l2_pins - configures the pins for
    a PAL16L2
-------------------------------------------------*/

static void config_pal16l2_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 17, 18, 19};
    static pin_output_config output_pins[] = {
        {15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL}};

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, ARRAY_LEN(output_pins));
}



/*-------------------------------------------------
    config_pal16h2_pins - configures the pins for
    a PAL16H2
-------------------------------------------------*/

static void config_pal16h2_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 17, 18, 19};
    static pin_output_config output_pins[] = {
        {15, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL},
        {16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL}};

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, ARRAY_LEN(output_pins));
}



/*-------------------------------------------------
    config_pal16c1_pins - configures the pins for
    a PAL16C1
-------------------------------------------------*/

static void config_pal16c1_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 17, 18, 19};
    static pin_output_config output_pins[] = {
        {15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL},
        {16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL}};

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, ARRAY_LEN(output_pins));
}



/*-------------------------------------------------
    config_pal16l8_pins - configures the pins for
    a PAL16L8
-------------------------------------------------*/

static void config_pal16l8_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 14, 15, 16, 17, 18};
    pin_output_config output_pins[8];
    UINT16 output_pin_count, index;

    output_pin_count = 0;

    for (index = 0; index < pal->pinfuserowscount; ++index)
    {
        if (any_fuses_in_row_blown(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
        {
            output_pins[output_pin_count].pin = pal->pinfuserows[index].pin;
            output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

            ++output_pin_count;
        }
    }

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_pal16r4_pins - configures the pins for
    a PAL16R4
-------------------------------------------------*/

static void config_pal16r4_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19};
    static UINT16 registered_pins[] = {14, 15, 16, 17};
    pin_output_config output_pins[8];
    UINT16 output_pin_count, index;

    output_pin_count = 0;

    if (any_fuses_in_row_blown(pal, jed, 1792))
    {
        output_pins[output_pin_count].pin = 12;
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

        ++output_pin_count;
    }

    if (any_fuses_in_row_blown(pal, jed, 1536))
    {
        output_pins[output_pin_count].pin = 13;
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

        ++output_pin_count;
    }

    for (index = 0; index < ARRAY_LEN(registered_pins); ++index)
    {
        output_pins[output_pin_count].pin = registered_pins[index];
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_REGISTERED;

        ++output_pin_count;
    }

    if (any_fuses_in_row_blown(pal, jed, 256))
    {
        output_pins[output_pin_count].pin = 18;
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

        ++output_pin_count;
    }

    if (any_fuses_in_row_blown(pal, jed, 0))
    {
        output_pins[output_pin_count].pin = 19;
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

        ++output_pin_count;
    }

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_pal16r6_pins - configures the pins
    for a PAL16R6
-------------------------------------------------*/

static void config_pal16r6_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19};
    static UINT16 registered_pins[] = {13, 14, 15, 16, 17, 18};
    pin_output_config output_pins[8];
    UINT16 output_pin_count, index;

    output_pin_count = 0;

    if (any_fuses_in_row_blown(pal, jed, 1792))
    {
        output_pins[output_pin_count].pin = 12;
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

        ++output_pin_count;
    }

    for (index = 0; index < ARRAY_LEN(registered_pins); ++index)
    {
        output_pins[output_pin_count].pin = registered_pins[index];
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_REGISTERED;

        ++output_pin_count;
    }

    if (any_fuses_in_row_blown(pal, jed, 0))
    {
        output_pins[output_pin_count].pin = 19;
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

        ++output_pin_count;
    }

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_pal16r8_pins - configures the pins for
    a PAL16R8
-------------------------------------------------*/

static void config_pal16r8_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19};
    static pin_output_config output_pins[] = {
        {12, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {13, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {14, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {15, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {16, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {17, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {18, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {19, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED}};

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, ARRAY_LEN(output_pins));
}



/*-------------------------------------------------
    config_gal16v8_pins - configures the pins for
    a GAL16V8
-------------------------------------------------*/

static void config_gal16v8_pins(const pal_data* pal, const jed_data* jed)
{
    typedef struct _output_logic_macrocell output_logic_macrocell;
    struct _output_logic_macrocell
    {
        UINT16 pin;
        UINT16 xor_fuse;
        UINT16 ac1_fuse;
    };

    static output_logic_macrocell macrocells[] = {
        {12, 2055, 2127},
        {13, 2054, 2126},
        {14, 2053, 2125},
        {15, 2052, 2124},
        {16, 2051, 2123},
        {17, 2050, 2122},
        {18, 2049, 2121},
        {19, 2048, 2120}};
    static pin_fuse_rows pinfuserows_registered[] = {
        {12, NO_OUTPUT_ENABLE_FUSE_ROW, 1792, 2016},
        {13, NO_OUTPUT_ENABLE_FUSE_ROW, 1536, 1760},
        {14, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1504},
        {15, NO_OUTPUT_ENABLE_FUSE_ROW, 1024, 1248},
        {16, NO_OUTPUT_ENABLE_FUSE_ROW, 768, 992},
        {17, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 736},
        {18, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 480},
        {19, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 224}};
    static pin_fuse_rows pinfuserows_combinatorial[] = {
        {12, 1792, 1824, 2016},
        {13, 1536, 1568, 1760},
        {14, 1280, 1312, 1504},
        {15, 1024, 1056, 1248},
        {16, 768, 800, 992},
        {17, 512, 544, 736},
        {18, 256, 288, 480},
        {19, 0, 32, 224}};
    static pin_fuse_columns pinfusecolumns_registered[] = {
        {2,  1,  0},
        {3,  5,  4},
        {4,  9,  8},
        {5,  13, 12},
        {6,  17, 16},
        {7,  21, 20},
        {8,  25, 24},
        {9,  29, 28},
        {12, 31, 30},
        {13, 27, 26},
        {14, 23, 22},
        {15, 19, 18},
        {16, 15, 14},
        {17, 11, 10},
        {18, 7, 6},
        {19, 3, 2}};
    static pin_fuse_columns pinfusecolumns_combinatorialcomplex[] = {
        {1,  3,  2},
        {2,  1,  0},
        {3,  5,  4},
        {4,  9,  8},
        {5,  13, 12},
        {6,  17, 16},
        {7,  21, 20},
        {8,  25, 24},
        {9,  29, 28},
        {11, 31, 30},
        {13, 27, 26},
        {14, 23, 22},
        {15, 19, 18},
        {16, 15, 14},
        {17, 11, 10},
        {18, 7, 6}};
    static pin_fuse_columns pinfusecolumns_combinatorialsimple[] = {
        {1,  3,  2},
        {2,  1,  0},
        {3,  5,  4},
        {4,  9,  8},
        {5,  13, 12},
        {6,  17, 16},
        {7,  21, 20},
        {8,  25, 24},
        {9,  29, 28},
        {11, 31, 30},
        {12, 27, 26},
        {13, 23, 22},
        {14, 19, 18},
        {17, 15, 14},
        {18, 11, 10},
        {19, 7,  6}};
    static UINT16 input_pins_registered[] = {2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19};
    static UINT16 input_pins_combinatorialcomplex[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 14, 15, 16, 17, 18};
    static UINT16 input_pins_combinatorialsimple[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 17, 18, 19};
    pin_output_config output_pins[ARRAY_LEN(macrocells)];
    UINT16 index, output_pin_count;

    output_pin_count = 0;

    /* SYN Fuse: 0 - registered, 1 - combinatorial */

    if (jed_get_fuse(jed, 2192))
    {
        /* Combinatorial */
        /* AC0 Fuse: 0 - simple mode, 1 - complex mode */

        if (jed_get_fuse(jed, 2193))
        {
            /* Complex Mode */

            set_input_pins(input_pins_combinatorialcomplex, ARRAY_LEN(input_pins_combinatorialcomplex));

            memcpy(gal16v8pinfuserows, pinfuserows_combinatorial, sizeof(pinfuserows_combinatorial));
            memcpy(gal16v8pinfusecolumns, pinfusecolumns_combinatorialcomplex, sizeof(pinfusecolumns_combinatorialcomplex));

            for (index = 0; index < ARRAY_LEN(macrocells); ++index)
            {
                if (is_gal16v8_product_term_enabled(pal, jed, pal->pinfuserows[index].fuserowoutputenable) &&
                    any_fuses_in_row_blown(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
                {
                    output_pins[output_pin_count].pin = macrocells[index].pin;
                    output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL;

                    if (jed_get_fuse(jed, macrocells[index].xor_fuse))
                    {
                        output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
                    }
                    else
                    {
                        output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
                    }

                    ++output_pin_count;
                }
            }
        }
        else
        {
            /* Simple Mode */

            set_input_pins(input_pins_combinatorialsimple, ARRAY_LEN(input_pins_combinatorialsimple));

            memcpy(gal16v8pinfuserows, pinfuserows_registered, sizeof(pinfuserows_registered));
            memcpy(gal16v8pinfusecolumns, pinfusecolumns_combinatorialsimple, sizeof(pinfusecolumns_combinatorialsimple));

            for (index = 0; index < ARRAY_LEN(macrocells); ++index)
            {
                if (jed_get_fuse(jed, macrocells[index].ac1_fuse))
                {
                    /* Pin is for input only */

                    if (macrocells[index].pin == 15 || macrocells[index].pin == 16)
                    {
                        fprintf(stderr, "Pin %d cannot be configured as an input pin.\n",
                                macrocells[index].pin);
                    }
                }
                else
                {
                    output_pins[output_pin_count].pin = macrocells[index].pin;
                    output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL;

                    if (jed_get_fuse(jed, macrocells[index].xor_fuse))
                    {
                        output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
                    }
                    else
                    {
                        output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
                    }

                    ++output_pin_count;
                }
            }
        }
    }
    else
    {
        /* Registered */

        set_input_pins(input_pins_registered, ARRAY_LEN(input_pins_registered));

        memcpy(gal16v8pinfusecolumns, pinfusecolumns_registered, sizeof(pinfusecolumns_registered));

        for (index = 0; index < ARRAY_LEN(macrocells); ++index)
        {
            if (jed_get_fuse(jed, macrocells[index].ac1_fuse))
            {
                /* combinatorial pin */

                gal16v8pinfuserows[index].fuserowoutputenable = pinfuserows_combinatorial[index].fuserowoutputenable;
                gal16v8pinfuserows[index].fuserowtermstart = pinfuserows_combinatorial[index].fuserowtermstart;
                gal16v8pinfuserows[index].fuserowtermend = pinfuserows_combinatorial[index].fuserowtermend;

                if (is_gal16v8_product_term_enabled(pal, jed, pal->pinfuserows[index].fuserowoutputenable) &&
                    any_fuses_in_row_blown(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
                {
                    output_pins[output_pin_count].pin = macrocells[index].pin;
                    output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL;

                    if (jed_get_fuse(jed, macrocells[index].xor_fuse))
                    {
                        output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
                    }
                    else
                    {
                        output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
                    }

                    ++output_pin_count;
                }
            }
            else
            {
                /* registered pin */

                gal16v8pinfuserows[index].fuserowoutputenable = pinfuserows_registered[index].fuserowoutputenable;
                gal16v8pinfuserows[index].fuserowtermstart = pinfuserows_registered[index].fuserowtermstart;
                gal16v8pinfuserows[index].fuserowtermend = pinfuserows_registered[index].fuserowtermend;

                output_pins[output_pin_count].pin = macrocells[index].pin;
                output_pins[output_pin_count].flags = OUTPUT_REGISTERED;

                if (jed_get_fuse(jed, macrocells[index].xor_fuse))
                {
                    output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
                }
                else
                {
                    output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
                }

                ++output_pin_count;
            }
        }
    }

    set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_gal18v10_pins - configures the pins
    for a GAL18V10
-------------------------------------------------*/

/*static void config_gal18v10_pins(const pal_data* pal, const jed_data* jed)
{
}*/



/*-------------------------------------------------
    config_pal20l8_pins - configures the pins for
    a PAL20L8
-------------------------------------------------*/

static void config_pal20l8_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 16, 17, 18, 19, 20, 21, 23};
    pin_output_config output_pins[8];
    UINT16 output_pin_count, index;

    output_pin_count = 0;

    for (index = 0; index < pal->pinfuserowscount; ++index)
    {
        if (any_fuses_in_row_blown(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
        {
            output_pins[output_pin_count].pin = pal->pinfuserows[index].pin;
            output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

            ++output_pin_count;
        }
    }

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_pal20l10_pins - configures the pins for
    a PAL20L10
-------------------------------------------------*/

static void config_pal20l10_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 16, 17, 18, 19, 20, 21, 22};
    pin_output_config output_pins[10];
    UINT16 output_pin_count, index;

    output_pin_count = 0;

    for (index = 0; index < pal->pinfuserowscount; ++index)
    {
        if (any_fuses_in_row_blown(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
        {
            output_pins[output_pin_count].pin = pal->pinfuserows[index].pin;
            output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

            ++output_pin_count;
        }
    }

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_pal20r4_pins - configures the pins for
    a PAL20R4
-------------------------------------------------*/

static void config_pal20r4_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    static UINT16 registered_pins[] = {17, 18, 19, 20};
    pin_output_config output_pins[8];
    UINT16 output_pin_count, index;

    output_pin_count = 0;

    if (any_fuses_in_row_blown(pal, jed, 2240))
    {
        output_pins[output_pin_count].pin = 15;
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

        ++output_pin_count;
    }

    if (any_fuses_in_row_blown(pal, jed, 1920))
    {
        output_pins[output_pin_count].pin = 16;
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

        ++output_pin_count;
    }

    for (index = 0; index < ARRAY_LEN(registered_pins); ++index)
    {
        output_pins[output_pin_count].pin = registered_pins[index];
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_REGISTERED;

        ++output_pin_count;
    }

    if (any_fuses_in_row_blown(pal, jed, 320))
    {
        output_pins[output_pin_count].pin = 21;
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

        ++output_pin_count;
    }

    if (any_fuses_in_row_blown(pal, jed, 0))
    {
        output_pins[output_pin_count].pin = 22;
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

        ++output_pin_count;
    }

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_pal20r6_pins - configures the pins for
    a PAL20R6
-------------------------------------------------*/

static void config_pal20r6_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    static UINT16 registered_pins[] = {16, 17, 18, 19, 20, 21};
    pin_output_config output_pins[8];
    UINT16 output_pin_count, index;

    output_pin_count = 0;

    if (any_fuses_in_row_blown(pal, jed, 2240))
    {
        output_pins[output_pin_count].pin = 15;
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

        ++output_pin_count;
    }

    for (index = 0; index < ARRAY_LEN(registered_pins); ++index)
    {
        output_pins[output_pin_count].pin = registered_pins[index];
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_REGISTERED;

        ++output_pin_count;
    }

    if (any_fuses_in_row_blown(pal, jed, 0))
    {
        output_pins[output_pin_count].pin = 22;
        output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

        ++output_pin_count;
    }


    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_pal20r8_pins - configures the pins for
    a PAL20R8
-------------------------------------------------*/

static void config_pal20r8_pins(const pal_data* pal, const jed_data* jed)
{
    static UINT16 input_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    static pin_output_config output_pins[] = {
        {15, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {16, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {17, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {18, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {19, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {20, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {21, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED},
        {22, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED}};

    set_input_pins(input_pins, ARRAY_LEN(input_pins));
    set_output_pins(output_pins, ARRAY_LEN(output_pins));
}



/*-------------------------------------------------
    is_gal16v8_product_term_enabled - determins if
    a fuse row in a GAL16V8 is enabled
-------------------------------------------------*/

static int is_gal16v8_product_term_enabled(const pal_data* pal, const jed_data* jed, UINT16 fuserow)
{
    UINT16 fuse_ptd;

    fuse_ptd = (fuserow / calc_fuse_column_count(pal)) + 2128;

    if (fuse_ptd > 2191)
    {
        fprintf(stderr, "Fuse row %d is illegal!\n", fuserow);

        return 0;
    }

    return jed_get_fuse(jed, fuse_ptd);
}



/*-------------------------------------------------
    read_source_file - read a raw source file
    into an allocated memory buffer
-------------------------------------------------*/

static int read_source_file(const char *srcfile)
{
	size_t bytes;
	FILE *file;

	/* open the source file */
	file = fopen(srcfile, "rb");
	if (!file)
	{
		fprintf(stderr, "Unable to open source file '%s'!\n", srcfile);
		return 1;
	}

	/* allocate memory for the data */
	fseek(file, 0, SEEK_END);
	srcbuflen = ftell(file);
	fseek(file, 0, SEEK_SET);
	srcbuf = (UINT8 *)malloc(srcbuflen);
	if (!srcbuf)
	{
		fprintf(stderr, "Unable to allocate %d bytes for the source!\n", (int)srcbuflen);
		fclose(file);
		return 1;
	}

	/* read the data */
	bytes = fread(srcbuf, 1, srcbuflen, file);
	if (bytes != srcbuflen)
	{
		fprintf(stderr, "Error reading %d bytes from the source!\n", (int)srcbuflen);
		free(srcbuf);
		fclose(file);
		return 1;
	}

	/* close up shop */
	fclose(file);
	return 0;
}



/*-------------------------------------------------
    write_dest_file - write a memory buffer raw
    into a desintation file
-------------------------------------------------*/

static int write_dest_file(const char *dstfile)
{
	size_t bytes;
	FILE *file;

	/* open the source file */
	file = fopen(dstfile, "wb");
	if (!file)
	{
		fprintf(stderr, "Unable to open target file '%s'!\n", dstfile);
		return 1;
	}

	/* write the data */
	bytes = fwrite(dstbuf, 1, dstbuflen, file);
	if (bytes != dstbuflen)
	{
		fprintf(stderr, "Error writing %d bytes to the target!\n", (int)dstbuflen);
		fclose(file);
		return 1;
	}

	/* close up shop */
	fclose(file);
	return 0;
}



/*-------------------------------------------------
    print_usage - prints out the supported command
    line arguments
-------------------------------------------------*/

static int print_usage()
{
	fprintf(stderr,
		"Usage:\n"
		"  jedutil -convert <source.jed> <target.bin> [fuses] -- convert JED to binary form\n"
		"  jedutil -convert <source.bin> <target.jed> -- convert binary to JED form\n"
        "  jedutil -view <source.jed> <pal name> -- dump JED logic equations\n"
        "  jedutil -view <source.bin> <pal name> -- dump binary logic equations\n"
        "  jedutil -viewlist -- view list of supported devices\n"
	);

	return 0;
}



/*-------------------------------------------------
    command_convert - convert files
-------------------------------------------------*/

static int command_convert(int argc, char *argv[])
{
    const char *srcfile, *dstfile;
	int src_is_jed, dst_is_jed;
	int numfuses = 0;
	jed_data jed;
	int err;

	if (argc < 2)
	{
		return print_usage();
	}

	/* extract arguments */
	srcfile = argv[0];
	dstfile = argv[1];
	if (argc >= 3)
		numfuses = atoi(argv[2]);

	/* does the source end in '.jed'? */
	src_is_jed = is_jed_file(srcfile);

	/* does the destination end in '.jed'? */
	dst_is_jed = is_jed_file(dstfile);

	/* error if neither or both are .jed */
	if (!src_is_jed && !dst_is_jed)
	{
		fprintf(stderr, "At least one of the filenames must end in .jed!\n");
		return 1;
	}
	if (src_is_jed && dst_is_jed)
	{
		fprintf(stderr, "Both filenames cannot end in .jed!\n");
		return 1;
	}

	/* read the source file */
	err = read_source_file(srcfile);
	if (err != 0)
		return 1;

	/* if the source is JED, convert to binary */
	if (src_is_jed)
	{
		printf("Converting '%s' to binary form '%s'\n", srcfile, dstfile);

		/* read the JEDEC data */
		err = jed_parse(srcbuf, srcbuflen, &jed);
		switch (err)
		{
			case JEDERR_INVALID_DATA:	fprintf(stderr, "Fatal error: Invalid .JED file\n"); return 1;
			case JEDERR_BAD_XMIT_SUM:	fprintf(stderr, "Fatal error: Bad transmission checksum\n"); return 1;
			case JEDERR_BAD_FUSE_SUM:	fprintf(stderr, "Fatal error: Bad fusemap checksum\n"); return 1;
		}

		/* override the number of fuses */
		if (numfuses != 0)
			jed.numfuses = numfuses;

		/* print out data */
		printf("Source file read successfully\n");
		printf("  Total fuses = %d\n", jed.numfuses);

		/* generate the output */
		dstbuflen = jedbin_output(&jed, NULL, 0);
		dstbuf = (UINT8 *)malloc(dstbuflen);
		if (!dstbuf)
		{
			fprintf(stderr, "Unable to allocate %d bytes for the target buffer!\n", (int)dstbuflen);
			return 1;
		}
		dstbuflen = jedbin_output(&jed, dstbuf, dstbuflen);
	}

	/* if the source is binary, convert to JED */
	else
	{
		printf("Converting '%s' to JED form '%s'\n", srcfile, dstfile);

		/* read the binary data */
		err = jedbin_parse(srcbuf, srcbuflen, &jed);
		switch (err)
		{
			case JEDERR_INVALID_DATA:	fprintf(stderr, "Fatal error: Invalid binary JEDEC file\n"); return 1;
		}

		/* print out data */
		printf("Source file read successfully\n");
		printf("  Total fuses = %d\n", jed.numfuses);

		/* generate the output */
		dstbuflen = jed_output(&jed, NULL, 0);
		dstbuf = (UINT8 *)malloc(dstbuflen);
		if (!dstbuf)
		{
			fprintf(stderr, "Unable to allocate %d bytes for the target buffer!\n", (int)dstbuflen);
			return 1;
		}
		dstbuflen = jed_output(&jed, dstbuf, dstbuflen);
	}

	/* write the destination file */
	err = write_dest_file(dstfile);
	if (err != 0)
		return 1;

	printf("Target file written successfully\n");

    return 0;
}



/*-------------------------------------------------
    command_view - views the contents of a file
-------------------------------------------------*/

static int command_view(int argc, char *argv[])
{
    const char *srcfile, *palname;
	int is_jed;
    const pal_data* pal;
	jed_data jed;
	int err;

	if (argc < 2)
	{
		return print_usage();
	}

    /* extract arguments */
	srcfile = argv[0];
	palname = argv[1];

	/* does the source end in '.jed'? */
	is_jed = is_jed_file(srcfile);

    /* find the pal entry */
    pal = find_pal_data(palname);
    if (!pal)
    {
		fprintf(stderr, "Unknown pal name.\n");
		return 1;
    }

	/* read the source file */
	err = read_source_file(srcfile);
	if (err != 0)
		return 1;

	/* if the source is JED, convert to binary */
	if (is_jed)
	{
		/* read the JEDEC data */
		err = jed_parse(srcbuf, srcbuflen, &jed);
		switch (err)
		{
			case JEDERR_INVALID_DATA:	fprintf(stderr, "Fatal error: Invalid .JED file\n"); return 1;
			case JEDERR_BAD_XMIT_SUM:	fprintf(stderr, "Fatal error: Bad transmission checksum\n"); return 1;
			case JEDERR_BAD_FUSE_SUM:	fprintf(stderr, "Fatal error: Bad fusemap checksum\n"); return 1;
		}
    }
    else
    {
		/* read the binary data */
		err = jedbin_parse(srcbuf, srcbuflen, &jed);
		switch (err)
		{
			case JEDERR_INVALID_DATA:	fprintf(stderr, "Fatal error: Invalid binary JEDEC file\n"); return 1;
		}
    }

    /* generate equations from fuse map */

    pal->config_pins(pal, &jed);

    if (pal->print_product_terms)
    {
        pal->print_product_terms(pal, &jed);
    }
    else
    {
        fprintf(stderr, "Viewing product terms not supported for this pal type.");

        return 1;
    }

    return 0;
}



/*-------------------------------------------------
    command_viewlist - views the list of supported
                       jeds
-------------------------------------------------*/

static int command_viewlist(int argc, char *argv[])
{
    int index;

	if (argc > 0)
	{
		return print_usage();
	}

    for (index = 0; index < ARRAY_LEN(paldata); ++index)
    {
        printf("%s\n", paldata[index].name);
    }

    return 0;
}


/*-------------------------------------------------
    main - primary entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
    command_entry command_entries[] = {
        {"-convert",  &command_convert},
        {"-view",     &command_view},
        {"-viewlist", &command_viewlist}};
    int index;

	if (argc < 2)
	{
		return print_usage();
	}

    for (index = 0; index < ARRAY_LEN(command_entries); ++index)
    {
        if (!strcmp(argv[1], command_entries[index].command))
            return command_entries[index].command_func(argc - 2, &argv[2]);
    }

    return print_usage();
}
