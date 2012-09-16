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
    TYPE DEFINITIONS
***************************************************************************/

typedef int (*command_func_type)(int argc, char *argv[]);

struct command_entry
{
    const char *command;
    command_func_type command_func;
};



/* Pin fuse row configuration */
#define CNoOutputEnableFuseRow (UINT16)~0



/* Pin fuse row configuration */
struct pin_fuse_rows
{
    UINT16 pin;                  /* Pin number */
    UINT16 fuserowoutputenable; /* Fuse row for the output enable */
    UINT16 fuserowtermstart;    /* Fuse row for the first term */
    UINT16 fuserowtermend;      /* Fuse row for the last term */
};



/* Pin fuse column configuration */
struct pin_fuse_columns
{
    UINT16 pin;             /* Pin number */
    UINT16 lowfusecolumn;  /* Column number for low output */
    UINT16 highfusecolumn; /* Column number for high output */
};


struct pal_data;

typedef void (*print_product_terms_func)(const pal_data* pal, const jed_data* jed);

struct pal_data
{
    const char *name;
    const pin_fuse_rows *pinfuserows;
    UINT16 pinfuserowscount;
    const pin_fuse_columns *pinfusecolumns;
    UINT16 pinfusecolumnscount;
    print_product_terms_func print_product_terms;
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
static void print_gal18v10_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20l8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20l10_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20r4_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20r6_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20r8_product_terms(const pal_data* pal, const jed_data* jed);



/***************************************************************************
    GLOBAL VARIABLES
***************************************************************************/

static UINT8 *srcbuf;
static size_t srcbuflen;

static UINT8 *dstbuf;
static size_t dstbuflen;

static UINT16 inputpins[26];
static UINT16 inputpinscount;

static UINT16 outputpins[26];
static UINT16 outputpinscount;

static pin_fuse_rows pal10l8pinfuserows[] = {
    {12, CNoOutputEnableFuseRow, 280, 300},
    {13, CNoOutputEnableFuseRow, 240, 260},
    {14, CNoOutputEnableFuseRow, 200, 220},
    {15, CNoOutputEnableFuseRow, 160, 180},
    {16, CNoOutputEnableFuseRow, 120, 140},
    {17, CNoOutputEnableFuseRow, 80, 100},
    {18, CNoOutputEnableFuseRow, 40, 60},
    {19, CNoOutputEnableFuseRow, 0, 20}};

static pin_fuse_rows pal10h8pinfuserows[] = {
    {12, CNoOutputEnableFuseRow, 280, 300},
    {13, CNoOutputEnableFuseRow, 240, 260},
    {14, CNoOutputEnableFuseRow, 200, 220},
    {15, CNoOutputEnableFuseRow, 160, 180},
    {16, CNoOutputEnableFuseRow, 120, 140},
    {17, CNoOutputEnableFuseRow, 80, 100},
    {18, CNoOutputEnableFuseRow, 40, 60},
    {19, CNoOutputEnableFuseRow, 0, 20}};

static pin_fuse_rows pal12l6pinfuserows[] = {
    {13, CNoOutputEnableFuseRow, 288, 360},
    {14, CNoOutputEnableFuseRow, 240, 264},
    {15, CNoOutputEnableFuseRow, 192, 216},
    {16, CNoOutputEnableFuseRow, 144, 168},
    {17, CNoOutputEnableFuseRow, 96, 120}, 
    {18, CNoOutputEnableFuseRow, 0, 72}};

static pin_fuse_rows pal12h6pinfuserows[] = {
    {13, CNoOutputEnableFuseRow, 288, 360},
    {14, CNoOutputEnableFuseRow, 240, 264},
    {15, CNoOutputEnableFuseRow, 192, 216},
    {16, CNoOutputEnableFuseRow, 144, 168},
    {17, CNoOutputEnableFuseRow, 96, 120}, 
    {18, CNoOutputEnableFuseRow, 0, 72}};

static pin_fuse_rows pal14l4pinfuserows[] = {
    {14, CNoOutputEnableFuseRow, 336, 420},
    {15, CNoOutputEnableFuseRow, 224, 308},
    {16, CNoOutputEnableFuseRow, 112, 196},
    {17, CNoOutputEnableFuseRow, 0, 84}};

static pin_fuse_rows pal14h4pinfuserows[] = {
    {14, CNoOutputEnableFuseRow, 336, 420},
    {15, CNoOutputEnableFuseRow, 224, 308},
    {16, CNoOutputEnableFuseRow, 112, 196},
    {17, CNoOutputEnableFuseRow, 0, 84}};

static pin_fuse_rows pal16l2pinfuserows[] = {
    {15, CNoOutputEnableFuseRow, 256, 480},
    {16, CNoOutputEnableFuseRow, 0, 224}};

static pin_fuse_rows pal16h2pinfuserows[] = {
    {15, CNoOutputEnableFuseRow, 256, 480},
    {16, CNoOutputEnableFuseRow, 0, 224}};

static pin_fuse_rows pal16c1pinfuserows[] = {
    {15, CNoOutputEnableFuseRow, 256, 480},
    {16, CNoOutputEnableFuseRow, 0, 224}};

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
    {14, CNoOutputEnableFuseRow, 1280, 1504}, /* Registered Output */
    {15, CNoOutputEnableFuseRow, 1024, 1248}, /* Registered Output */
    {16, CNoOutputEnableFuseRow, 768, 992},   /* Registered Output */
    {17, CNoOutputEnableFuseRow, 512, 736},   /* Registered Output */
    {18, 256, 288, 480},
    {19, 0, 32, 224}};

static pin_fuse_rows pal16r6pinfuserows[] = {
    {12, 1792, 1824, 2016},
    {13, CNoOutputEnableFuseRow, 1536, 1760}, /* Registered Output */
    {14, CNoOutputEnableFuseRow, 1280, 1504}, /* Registered Output */
    {15, CNoOutputEnableFuseRow, 1024, 1248}, /* Registered Output */
    {16, CNoOutputEnableFuseRow, 768, 992},   /* Registered Output */
    {17, CNoOutputEnableFuseRow, 512, 736},   /* Registered Output */
    {18, CNoOutputEnableFuseRow, 256, 480},   /* Registered Output */
    {19, 0, 32, 224}};

static pin_fuse_rows pal16r8pinfuserows[] = {
    {12, CNoOutputEnableFuseRow, 1792, 2016}, /* Registered Output */
    {13, CNoOutputEnableFuseRow, 1536, 1760}, /* Registered Output */
    {14, CNoOutputEnableFuseRow, 1280, 1504}, /* Registered Output */
    {15, CNoOutputEnableFuseRow, 1024, 1248}, /* Registered Output */
    {16, CNoOutputEnableFuseRow, 768, 992},   /* Registered Output */
    {17, CNoOutputEnableFuseRow, 512, 736},   /* Registered Output */
    {18, CNoOutputEnableFuseRow, 256, 480},   /* Registered Output */
    {19, CNoOutputEnableFuseRow, 0, 224}};    /* Registered Output */

static pin_fuse_rows gal18v10pinfuserows[] = {
    {9,  3096, 3132, 3384},
    {11, 2772, 2808, 3060},
    {12, 2448, 2484, 2736},
    {13, 2124, 2160, 2412},
    {14, 1728, 1764, 2088},
    {15, 1332, 1368, 1692},
    {16, 1008, 1044, 1296},
    {17, 684,  720,  972},
    {18, 360,  396,  648},
    {19, 36,   72,   324}};

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
    {17, 1600, 1640, 1840},
    {18, 1280, 1320, 1560},
    {19, 960, 1000, 1240},
    {20, 640, 680, 920},
    {21, 320, 360, 600},
    {22, 0, 40, 280}};

static pin_fuse_rows pal20r6pinfuserows[] = {
    {15, 2240, 2280, 2520},
    {16, 1920, 1960, 2200},
    {17, 1600, 1640, 1840},
    {18, 1280, 1320, 1560},
    {19, 960, 1000, 1240},
    {20, 640, 680, 920},
    {21, 320, 360, 600},
    {22, 0, 40, 280}};

static pin_fuse_rows pal20r8pinfuserows[] = {
    {15, 2240, 2280, 2520},
    {16, 1920, 1960, 2200},
    {17, 1600, 1640, 1840},
    {18, 1280, 1320, 1560},
    {19, 960, 1000, 1240},
    {20, 640, 680, 920},
    {21, 320, 360, 600},
    {22, 0, 40, 280}};

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

static pin_fuse_columns gal18v10pinfusecolumns[] = {
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
    {19, 3,  2}};

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
        pal10l8pinfuserows,
        sizeof(pal10l8pinfuserows) / sizeof(pal10l8pinfuserows[0]),
        pal10l8pinfusecolumns,
        sizeof(pal10l8pinfusecolumns) / sizeof(pal10l8pinfusecolumns[0]),
        print_pal10l8_product_terms},
    {"PAL10H8",
        pal10h8pinfuserows,
        sizeof(pal10h8pinfuserows) / sizeof(pal10h8pinfuserows[0]),
        pal10h8pinfusecolumns,
        sizeof(pal10h8pinfusecolumns) / sizeof(pal10h8pinfusecolumns[0]),
        print_pal10h8_product_terms},
    {"PAL12H6",
        pal12h6pinfuserows,
        sizeof(pal12h6pinfuserows) / sizeof(pal12h6pinfuserows[0]),
        pal12h6pinfusecolumns,
        sizeof(pal12h6pinfusecolumns) / sizeof(pal12h6pinfusecolumns[0]),
        print_pal12h6_product_terms},
    {"PAL14H4",
        pal14h4pinfuserows,
        sizeof(pal14h4pinfuserows) / sizeof(pal14h4pinfuserows[0]),
        pal14h4pinfusecolumns,
        sizeof(pal14h4pinfusecolumns) / sizeof(pal14h4pinfusecolumns[0]),
        print_pal14h4_product_terms},
    {"PAL16H2",
        pal16h2pinfuserows,
        sizeof(pal16h2pinfuserows) / sizeof(pal16h2pinfuserows[0]),
        pal16h2pinfusecolumns,
        sizeof(pal16h2pinfusecolumns) / sizeof(pal16h2pinfusecolumns[0]),
        print_pal16h2_product_terms},
    {"PAL16C1",
        pal16c1pinfuserows,
        sizeof(pal16c1pinfuserows) / sizeof(pal16c1pinfuserows[0]),
        pal16c1pinfusecolumns,
        sizeof(pal16c1pinfusecolumns) / sizeof(pal16c1pinfusecolumns[0]),
        print_pal16c1_product_terms},
    {"PAL12L6",
        pal12l6pinfuserows,
        sizeof(pal12l6pinfuserows) / sizeof(pal12l6pinfuserows[0]),
        pal12l6pinfusecolumns,
        sizeof(pal12l6pinfusecolumns) / sizeof(pal12l6pinfusecolumns[0]),
        print_pal12l6_product_terms},
    {"PAL14L4",
        pal14l4pinfuserows,
        sizeof(pal14l4pinfuserows) / sizeof(pal14l4pinfuserows[0]),
        pal14l4pinfusecolumns,
        sizeof(pal14l4pinfusecolumns) / sizeof(pal14l4pinfusecolumns[0]),
        print_pal14l4_product_terms},
    {"PAL16L2",
        pal16l2pinfuserows,
        sizeof(pal16l2pinfuserows) / sizeof(pal16l2pinfuserows[0]),
        pal16l2pinfusecolumns,
        sizeof(pal16l2pinfusecolumns) / sizeof(pal16l2pinfusecolumns[0]),
        print_pal16l2_product_terms},
    {"15S8", NULL, 0, NULL, 0, NULL},
    {"PLS153", NULL, 0, NULL, 0, NULL},
    {"PAL16L8",
        pal16l8pinfuserows,
        sizeof(pal16l8pinfuserows) / sizeof(pal16l8pinfuserows[0]),
        pal16l8pinfusecolumns,
        sizeof(pal16l8pinfusecolumns) / sizeof(pal16l8pinfusecolumns[0]),
        print_pal16l8_product_terms},
    {"PAL16R4",
        pal16r4pinfuserows,
        sizeof(pal16r4pinfuserows) / sizeof(pal16r4pinfuserows[0]),
        pal16r4pinfusecolumns,
        sizeof(pal16r4pinfusecolumns) / sizeof(pal16r4pinfusecolumns),
        print_pal16r4_product_terms},
    {"PAL16R6",
        pal16r6pinfuserows,
        sizeof(pal16r6pinfuserows) / sizeof(pal16r6pinfuserows[0]),
        pal16r6pinfusecolumns,
        sizeof(pal16r6pinfusecolumns) / sizeof(pal16r6pinfusecolumns),
        print_pal16r6_product_terms},
    {"PAL16R8",
        pal16r8pinfuserows,
        sizeof(pal16r8pinfuserows) / sizeof(pal16r8pinfuserows[0]),
        pal16r8pinfusecolumns,
        sizeof(pal16r8pinfusecolumns) / sizeof(pal16r8pinfusecolumns),
        print_pal16r8_product_terms},
    {"PAL16RA8", NULL, 0, NULL, 0, NULL},
    {"PAL16V8R", NULL, 0, NULL, 0, NULL},
    {"PALCE16V8", NULL, 0, NULL, 0, NULL},
    {"GAL16V8A", NULL, 0, NULL, 0, NULL},
    {"18CV8", NULL, 0, NULL, 0, NULL},
    {"GAL18V10",
        gal18v10pinfuserows,
        sizeof(gal18v10pinfuserows) / sizeof(gal18v10pinfuserows[0]),
        gal18v10pinfusecolumns,
        sizeof(gal18v10pinfusecolumns) / sizeof(gal18v10pinfusecolumns),
        print_gal18v10_product_terms},
    {"PAL20L8",
        pal20l8pinfuserows,
        sizeof(pal20l8pinfuserows) / sizeof(pal20l8pinfuserows[0]),
        pal20l8pinfusecolumns,
        sizeof(pal20l8pinfusecolumns) / sizeof(pal20l8pinfusecolumns[0]),
        print_pal20l8_product_terms},
    {"PAL20L10",
        pal20l10pinfuserows,
        sizeof(pal20l10pinfuserows) / sizeof(pal20l10pinfuserows[0]),
        pal20l10pinfusecolumns,
        sizeof(pal20l10pinfusecolumns) / sizeof(pal20l10pinfusecolumns[0]),
        print_pal20l10_product_terms},
    {"PAL20R4",
        pal20r4pinfuserows,
        sizeof(pal20r4pinfuserows) / sizeof(pal20r4pinfuserows[0]),
        pal20r4pinfusecolumns,
        sizeof(pal20r4pinfusecolumns) / sizeof(pal20r4pinfusecolumns[0]),
        print_pal20r4_product_terms},
    {"PAL20R6",
        pal20r6pinfuserows,
        sizeof(pal20r6pinfuserows) / sizeof(pal20r6pinfuserows[0]),
        pal20r6pinfusecolumns,
        sizeof(pal20r6pinfusecolumns) / sizeof(pal20r6pinfusecolumns[0]),
        print_pal20r6_product_terms},
    {"PAL20R8",
        pal20r8pinfuserows,
        sizeof(pal20r8pinfuserows) / sizeof(pal20r8pinfuserows[0]),
        pal20r8pinfusecolumns,
        sizeof(pal20r8pinfusecolumns) / sizeof(pal20r8pinfusecolumns[0]),
        print_pal20r8_product_terms},
    {"PAL20X4", NULL, 0, NULL, 0, NULL},
    {"PAL20X8", NULL, 0, NULL, 0, NULL},
    {"PAL20X10", NULL, 0, NULL, 0, NULL},
    {"PAL22V10", NULL, 0, NULL, 0, NULL},
    {"GAL20V8A", NULL, 0, NULL, 0, NULL},
    {"GAL22V10", NULL, 0, NULL, 0, NULL},
    {"PLS100", NULL, 0, NULL, 0, NULL}};



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

static pal_data* find_pal_data(const char *name)
{
    int index;

    for (index = 0; index < sizeof(paldata) / sizeof(paldata[0]);
         ++index)
    {
        if (!core_stricmp(name, paldata[index].name))
        {
            return &paldata[index];
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
    is_fuse_row_blown - checks if a fuse row is
    all blown
-------------------------------------------------*/

static int is_fuse_row_blown(const pal_data* pal, const jed_data* jed, UINT16 fuserow)
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
    is_output_pin - determines if the pin is an
    output pin
-------------------------------------------------*/

static int is_output_pin(UINT16 pin)
{
    UINT16 index;

    for (index = 0; index < outputpinscount; ++index)
    {
        if (outputpins[index] == pin)
        {
            return 1;
        }
    }

    return 0;
}



/*-------------------------------------------------
    find_input_pins - finds the input pins of a
    pal
-------------------------------------------------*/

static void find_input_pins(const pal_data* pal, const jed_data* jed)
{
    UINT16 column;

    inputpinscount = 0;

    for (column = 0; column < pal->pinfusecolumnscount; ++column)
    {
        if (!is_output_pin(pal->pinfusecolumns[column].pin))
        {
            inputpins[inputpinscount] = pal->pinfusecolumns[column].pin;

            ++inputpinscount;
        }
    }
}



/*-------------------------------------------------
    find_output_pins - finds the output pins of a
    pal
-------------------------------------------------*/

static void find_output_pins(const pal_data* pal, const jed_data* jed)
{
    UINT16 column, columncount, index;
    int fuseblown;

    outputpinscount = 0;
    columncount = calc_fuse_column_count(pal);

    for (index = 0; index < pal->pinfuserowscount; ++index)
    {
        fuseblown = 0;

        if (pal->pinfuserows[index].fuserowoutputenable == CNoOutputEnableFuseRow)
        {
            fuseblown = 1;
        }
        else
        {
            for (column = 0; column < columncount; ++column)
            {
                if (jed_get_fuse(jed, pal->pinfuserows[index].fuserowoutputenable + column) == 1)
                {
                    fuseblown = 1;
                }
            }
        }

        if (fuseblown)
        {
            outputpins[outputpinscount] = pal->pinfuserows[index].pin;

            ++outputpinscount;
        }
    }
}



static int is_output_pin_used(const pal_data* pal, const jed_data* jed, const pin_fuse_rows* fuse_rows)
{
    UINT16 row, column, columncount;

    columncount = calc_fuse_column_count(pal);

    for (row = fuse_rows->fuserowtermstart;
         row <= fuse_rows->fuserowtermend;
         row += columncount)
    {
        for (column = 0; column < columncount; ++column)
        {
            if (jed_get_fuse(jed, row + column))
            {
                return 1;
            }
        }
    }

    return 0;
}



/*-------------------------------------------------
    generate_product_terms - prints the product terms
    for a fuse row
-------------------------------------------------*/

static void generate_product_terms(const pal_data* pal, const jed_data* jed, UINT16 fuserow, char* buffer)
{
    UINT16 index;
    int lowfusestate, highfusestate, haveterm;
    char tmpbuffer[20];

    *buffer = 0;
    haveterm = 0;

    for (index = 0; index < pal->pinfusecolumnscount; ++index)
    {
        lowfusestate = jed_get_fuse(jed, fuserow + pal->pinfusecolumns[index].lowfusecolumn);
        highfusestate = jed_get_fuse(jed, fuserow + pal->pinfusecolumns[index].highfusecolumn);

        if (!lowfusestate && highfusestate)
        {
            if (haveterm)
            {
                strcat(buffer,  " & ");
            }

            if (!is_output_pin(pal->pinfusecolumns[index].pin))
            {
                sprintf(tmpbuffer, "/i%d", pal->pinfusecolumns[index].pin);
                strcat(buffer, tmpbuffer);
            }
            else
            {
                sprintf(tmpbuffer, "/o%d", pal->pinfusecolumns[index].pin);
                strcat(buffer, tmpbuffer);
            }

            haveterm = 1;
        }
        else if (lowfusestate && !highfusestate)
        {
            if (haveterm)
            {
                strcat(buffer, " & ");
            }

            if (!is_output_pin(pal->pinfusecolumns[index].pin))
            {
                sprintf(tmpbuffer, "i%d", pal->pinfusecolumns[index].pin);
                strcat(buffer, tmpbuffer);
            }
            else
            {
                sprintf(tmpbuffer, "o%d", pal->pinfusecolumns[index].pin);
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

static void print_product_terms(const pal_data* pal, const jed_data* jed, int activestate)
{
    UINT16 index, row, columncount;
    char buffer[200];
    int haveterms, indent, indentindex;

    columncount = calc_fuse_column_count(pal);

    for (index = 0; index < pal->pinfuserowscount; ++index)
    {
        if (is_output_pin(pal->pinfuserows[index].pin) &&
            is_output_pin_used(pal, jed, &pal->pinfuserows[index]))
        {
            indent = 0;

            if (!activestate)
            {
                printf("/");

                ++indent;
            }

            sprintf(buffer, "o%d = ", pal->pinfuserows[index].pin);

            printf("%s", buffer);

            haveterms = 0;
            indent += strlen(buffer);

            for (row = pal->pinfuserows[index].fuserowtermstart;
                 row <= pal->pinfuserows[index].fuserowtermend;
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

            printf("o%d.oe = ", pal->pinfuserows[index].pin);

            if (pal->pinfuserows[index].fuserowoutputenable == CNoOutputEnableFuseRow ||
                is_fuse_row_blown(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
            {
                printf("vcc\n");
            }
            else
            {
                generate_product_terms(pal, jed, pal->pinfuserows[index].fuserowoutputenable, buffer);

                printf("%s\n", buffer);
            }

            printf("\n");
        }
    }

    printf("\n");
}



/*-------------------------------------------------
    print_pal10l8_product_terms - prints the product
    terms for a PAL10L8
-------------------------------------------------*/

static void print_pal10l8_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed, 0);
}



/*-------------------------------------------------
    print_pal10h8_product_terms - prints the product
    terms for a PAL10H8
-------------------------------------------------*/

static void print_pal10h8_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed, 1);
}



/*-------------------------------------------------
    print_pal12l6_product_terms - prints the product
    terms for a PAL12L6
-------------------------------------------------*/

static void print_pal12l6_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed, 0);
}



/*-------------------------------------------------
    print_pal12h6_product_terms - prints the product
    terms for a PAL12H6
-------------------------------------------------*/

static void print_pal12h6_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed, 1);
}



/*-------------------------------------------------
    print_pal14l4_product_terms - prints the product
    terms for a PAL14L4
-------------------------------------------------*/

static void print_pal14l4_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed, 0);
}



/*-------------------------------------------------
    print_pal14h4_product_terms - prints the product
    terms for a PAL14H4
-------------------------------------------------*/

static void print_pal14h4_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed, 1);
}



/*-------------------------------------------------
    print_pal16l2_product_terms - prints the product
    terms for a PAL16L2
-------------------------------------------------*/

static void print_pal16l2_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed, 0);
}



/*-------------------------------------------------
    print_pal16h2_product_terms - prints the product
    terms for a PAL16H2
-------------------------------------------------*/

static void print_pal16h2_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed, 1);
}



/*-------------------------------------------------
    print_pal16h2_product_terms - prints the product
    terms for a PAL16C1
-------------------------------------------------*/

static void print_pal16c1_product_terms(const pal_data* pal, const jed_data* jed)
{
    printf("Viewing product terms are not supported.\n");
}



/*-------------------------------------------------
    print_pal16l8_product_terms - prints the product
    terms for a PAL16L8
-------------------------------------------------*/

static void print_pal16l8_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed, 0);
}



/*-------------------------------------------------
    print_pal16r4_product_terms - prints the product
    terms for a PAL16R4
-------------------------------------------------*/

static void print_pal16r4_product_terms(const pal_data* pal, const jed_data* jed)
{
    printf("Viewing product terms are not supported.\n");
}



/*-------------------------------------------------
    print_pal16r6_product_terms - prints the product
    terms for a PAL16R6
-------------------------------------------------*/

static void print_pal16r6_product_terms(const pal_data* pal, const jed_data* jed)
{
    printf("Viewing product terms are not supported.\n");
}



/*-------------------------------------------------
    print_pal16r8_product_terms - prints the product
    terms for a PAL16R8
-------------------------------------------------*/

static void print_pal16r8_product_terms(const pal_data* pal, const jed_data* jed)
{
    printf("Viewing product terms are not supported.\n");
}



/*-------------------------------------------------
    print_gal18v10_product_terms - prints the product
    terms for a GAL18V10
-------------------------------------------------*/

static void print_gal18v10_product_terms(const pal_data* pal, const jed_data* jed)
{
    printf("Viewing product terms are not supported.\n");
}



/*-------------------------------------------------
    print_pal20l8_product_terms - prints the product
    terms for a PAL20L8
-------------------------------------------------*/

static void print_pal20l8_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed, 0);
}



/*-------------------------------------------------
    print_pal20l10_product_terms - prints the product
    terms for a PAL20L10
-------------------------------------------------*/

static void print_pal20l10_product_terms(const pal_data* pal, const jed_data* jed)
{
    print_product_terms(pal, jed, 0);
}



/*-------------------------------------------------
    print_pal20r4_product_terms - prints the product
    terms for a PAL20R4
-------------------------------------------------*/

static void print_pal20r4_product_terms(const pal_data* pal, const jed_data* jed)
{
    printf("Viewing product terms are not supported.\n");
}



/*-------------------------------------------------
    print_pal20r6_product_terms - prints the product
    terms for a PAL20R6
-------------------------------------------------*/

static void print_pal20r6_product_terms(const pal_data* pal, const jed_data* jed)
{
    printf("Viewing product terms are not supported.\n");
}



/*-------------------------------------------------
    print_pal20r8_product_terms - prints the product
    terms for a PAL20R8
-------------------------------------------------*/

static void print_pal20r8_product_terms(const pal_data* pal, const jed_data* jed)
{
    printf("Viewing product terms are not supported.\n");
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
    pal_data* pal;
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

    find_output_pins(pal, &jed);
    find_input_pins(pal, &jed);

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
    main - primary entry point
-------------------------------------------------*/

int main(int argc, char *argv[])
{
    command_entry command_entries[] = {
        {"-convert", &command_convert},
        {"-view",    &command_view}};
    int index;

	/* needs at least two arguments */
	if (argc < 4)
	{
		return print_usage();
	}

    for (index = 0; index < sizeof(command_entries) / sizeof(command_entries[0]); ++index)
    {
        if (!strcmp(argv[1], command_entries[index].command))
            return command_entries[index].command_func(argc - 2, &argv[2]);
    }

    return print_usage();
}
