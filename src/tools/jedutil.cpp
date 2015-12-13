// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    jedutil.c

    JEDEC file utilities.

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
        PAL14H4     = QP20 QF0448
        PAL16H2     = QP20 QF0512
        PAL16C1     = QP20 QF0512
        PAL10L8     = QP20 QF0320
        PAL12L6     = QP20 QF0320
        PAL14L4     = QP20 QF0448
        PAL16L2     = QP20 QF0512

        PAL10P8     = QP20 QF0328
        PAL12P6     = QP20 QF0390
        PAL14P4     = QP20 QF0452
        PAL16P2     = QP20 QF0514
        PAL16P8     = QP20 QF2056
        PAL16RP4    = QP20 QF2056
        PAL16RP6    = QP20 QF2056
        PAL16RP8    = QP20 QF2056

        15S8        = QP20 QF0448

        CK2605      = QP20 QF1106

        PLS153      = QP20 QF1842

        PAL16L8     = QP20 QF2048

        PAL16R4     = QP20 QF2048
        PAL16R6     = QP20 QF2048
        PAL16R8     = QP20 QF2048
        PAL16RA8    = QP20 QF2056?

        PAL16V8R    = QP20 QF2194
        PALCE16V8   = QP20 QF2194
        GAL16V8A    = QP20 QF2194

        18CV8       = QP20 QF2696

        EPL10P8     = QP20
        EPL12P6     = QP20
        EPL14P4     = QP20
        EPL16P2     = QP20
        EPL16P8     = QP20
        EPL16RP8    = QP20
        EPL16RP6    = QP20
        EPL16RP4    = QP20

    24-pin devices:
        PAL6L16     = QP24 QF0192
        PAL8L14     = QP24 QF0224
        PAL12H10    = QP24 QF0480
        PAL12L10    = QP24 QF0480
        PAL14H8     = QP24 QF0560
        PAL14L8     = QP24 QF0560
        PAL16H6     = QP24 QF0640
        PAL16L6     = QP24 QF0640
        PAL18H4     = QP24 QF0720
        PAL18L4     = QP24 QF0720
        PAL20C1     = QP24 QF0640
        PAL20L2     = QP24 QF0640

        PAL20L8     = QP24 QF2560
        PAL20L10    = QP24 QF1600
        PAL20R4     = QP24 QF2560
        PAL20R6     = QP24 QF2560
        PAL20R8     = QP24 QF2560

        PAL20X4     = QP24 QF1600
        PAL20X8     = QP24 QF1600
        PAL20X10    = QP24 QF1600

        PAL22V10    = QP24 QF5828?

        GAL20V8A    = QP24 QF2706
        GAL22V10    = QP24 QF5892

    28-pin devices:
        PLS100      = QP28 QF1928

****************************************************************************

    Thanks to Charles MacDonald (http://cgfm2.emuviews.com/) for providing
    information on how to decode the PLS153/82S153 and CK2605 fuse map.

***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "corestr.h"
#include "jedparse.h"
#include "plaparse.h"



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define NO_OUTPUT_ENABLE_FUSE_ROW 0xFFFF

/* Output pin flags */
#define OUTPUT_ACTIVELOW              0x00000001
#define OUTPUT_ACTIVEHIGH             0x00000002
#define OUTPUT_COMBINATORIAL          0x00000004
#define OUTPUT_REGISTERED             0x00000008
#define OUTPUT_FEEDBACK_OUTPUT        0x00000010 /* Feedback state depends on output enable */
#define OUTPUT_FEEDBACK_COMBINATORIAL 0x00000020 /* Feedback state independent of output enable */
#define OUTPUT_FEEDBACK_REGISTERED    0x00000040 /* Feedback state independent of output enable */
#define OUTPUT_FEEDBACK_NONE          0x00000080 /* Feedback not available */

/*
    Output Feedback Output

    OE -----------|
                  |
                 |-\
    IN ----------|  >----|----< OUT >
                 |-/     |
                         |
    FEEDBACK ------------|



    Output Feedback Combinatorial/Registered

    OE ----------------|
                       |
                      |-\
    IN ----------|----|  >----< OUT >
                 |    |-/
                 |
    FEEDBACK ----|
*/



/* Fuse state flag */
#define LOW_FUSE_BLOWN     0x00000001
#define HIGH_FUSE_BLOWN    0x00000002
#define LOWHIGH_FUSE_BLOWN 0x00000004
#define NO_FUSE_BLOWN      0x00000008



/* Symbols */
#define AND_SYMBOL "&"
#define OR_SYMBOL "+"
#define XOR_SYMBOL ":+:"

#define LOW_SYMBOL "/"

#define INPUT_SYMBOL "i"
#define OUTPUT_SYMBOL "o"
#define REGISTERED_FEEDBACK_OUTPUT_SYMBOL "rfo"
#define OUTPUT_FEEDBACK_SYMBOL "of"
#define REGISTERED_FEEDBACK_SYMBOL "rf"

#define COMBINATORIAL_ASSIGNMENT "="
#define REGISTERED_ASSIGNMENT ":="



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
typedef UINT16 (*get_pin_fuse_state_func)(const pal_data* pal, const jed_data* jed, UINT16 pin, UINT16 fuserow);

struct _pal_data
{
	const char *name;
	UINT32 numfuses;
	const pin_fuse_rows *pinfuserows;
	UINT16 pinfuserowscount;
	const pin_fuse_columns *pinfusecolumns;
	UINT16 pinfusecolumnscount;
	print_product_terms_func print_product_terms;
	config_pins_func config_pins;
	is_product_term_enabled_func is_product_term_enabled;
	get_pin_fuse_state_func get_pin_fuse_state;
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
static void print_palce16v8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_gal16v8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_peel18cv8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_gal18v10_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20l8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20l10_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20r4_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20r6_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20r8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20x4_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20x8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20x10_product_terms(const pal_data* pal, const jed_data* jed);
static void print_82s153_pls153_product_terms(const pal_data* pal, const jed_data* jed);
static void print_ck2605_product_terms(const pal_data* pal, const jed_data* jed);
#if defined(ricoh_pals)
static void print_epl10p8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_epl12p6_product_terms(const pal_data* pal, const jed_data* jed);
static void print_epl14p4_product_terms(const pal_data* pal, const jed_data* jed);
static void print_epl16p2_product_terms(const pal_data* pal, const jed_data* jed);
static void print_epl16p8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_epl16rp8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_epl16rp6_product_terms(const pal_data* pal, const jed_data* jed);
static void print_epl16rp4_product_terms(const pal_data* pal, const jed_data* jed);
#endif
static void print_pal10p8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal12p6_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal14p4_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16p2_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16p8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16rp4_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16rp6_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16rp8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal6l16_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal8l14_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal12h10_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal12l10_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal14h8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal14l8_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16h6_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal16l6_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal18h4_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal18l4_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20c1_product_terms(const pal_data* pal, const jed_data* jed);
static void print_pal20l2_product_terms(const pal_data* pal, const jed_data* jed);



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
static void config_palce16v8_pins(const pal_data* pal, const jed_data* jed);
static void config_gal16v8_pins(const pal_data* pal, const jed_data* jed);
static void config_peel18cv8_pins(const pal_data* pal, const jed_data* jed);
static void config_gal18v10_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20l8_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20l10_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20r4_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20r6_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20r8_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20x4_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20x8_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20x10_pins(const pal_data* pal, const jed_data* jed);
static void config_82s153_pls153_pins(const pal_data* pal, const jed_data* jed);
static void config_ck2605_pins(const pal_data* pal, const jed_data* jed);
#if defined(ricoh_pals)
static void config_epl10p8_pins(const pal_data* pal, const jed_data* jed);
static void config_epl12p6_pins(const pal_data* pal, const jed_data* jed);
static void config_epl14p4_pins(const pal_data* pal, const jed_data* jed);
static void config_epl16p2_pins(const pal_data* pal, const jed_data* jed);
static void config_epl16p8_pins(const pal_data* pal, const jed_data* jed);
static void config_epl16rp8_pins(const pal_data* pal, const jed_data* jed);
static void config_epl16rp6_pins(const pal_data* pal, const jed_data* jed);
static void config_epl16rp4_pins(const pal_data* pal, const jed_data* jed);
#endif
static void config_pal10p8_pins(const pal_data* pal, const jed_data* jed);
static void config_pal12p6_pins(const pal_data* pal, const jed_data* jed);
static void config_pal14p4_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16p2_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16p8_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16rp4_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16rp6_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16rp8_pins(const pal_data* pal, const jed_data* jed);
static void config_pal6l16_pins(const pal_data* pal, const jed_data* jed);
static void config_pal8l14_pins(const pal_data* pal, const jed_data* jed);
static void config_pal12h10_pins(const pal_data* pal, const jed_data* jed);
static void config_pal12l10_pins(const pal_data* pal, const jed_data* jed);
static void config_pal14h8_pins(const pal_data* pal, const jed_data* jed);
static void config_pal14l8_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16h6_pins(const pal_data* pal, const jed_data* jed);
static void config_pal16l6_pins(const pal_data* pal, const jed_data* jed);
static void config_pal18h4_pins(const pal_data* pal, const jed_data* jed);
static void config_pal18l4_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20c1_pins(const pal_data* pal, const jed_data* jed);
static void config_pal20l2_pins(const pal_data* pal, const jed_data* jed);



static int is_gal16v8_product_term_enabled(const pal_data* pal, const jed_data* jed, UINT16 fuserow);



static UINT16 get_peel18cv8_pin_fuse_state(const pal_data* pal, const jed_data* jed, UINT16 pin, UINT16 fuserow);



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

static pin_fuse_rows palce16v8pinfuserows[] = {
	{12, 0, 0, 0},
	{13, 0, 0, 0},
	{14, 0, 0, 0},
	{15, 0, 0, 0},
	{16, 0, 0, 0},
	{17, 0, 0, 0},
	{18, 0, 0, 0},
	{19, 0, 0, 0}};

static pin_fuse_rows gal16v8pinfuserows[] = {
	{12, 0, 0, 0},
	{13, 0, 0, 0},
	{14, 0, 0, 0},
	{15, 0, 0, 0},
	{16, 0, 0, 0},
	{17, 0, 0, 0},
	{18, 0, 0, 0},
	{19, 0, 0, 0}};

static pin_fuse_rows peel18cv8pinfuserows[] = {
	{12, 2556, 2016, 2268},
	{13, 2520, 1728, 1980},
	{14, 2484, 1440, 1692},
	{15, 2448, 1152, 1404},
	{16, 2412, 864, 1116},
	{17, 2376, 576, 828},
	{18, 2340, 288, 540},
	{19, 2304, 0, 252}};

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

static pin_fuse_rows pal20x4pinfuserows[] = {
	{14, 1440, 1480, 1560},
	{15, 1280, 1320, 1400},
	{16, 1120, 1160, 1240},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 960, 1080},  /* Registered Output */
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 800, 920},   /* Registered Output */
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 640, 760},   /* Registered Output */
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 480, 600},   /* Registered Output */
	{21, 320, 360, 440},
	{22, 160, 200, 280},
	{23, 0, 40, 120}};

static pin_fuse_rows pal20x8pinfuserows[] = {
	{14, 1440, 1480, 1560},
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1400}, /* Registered Output */
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 1120, 1240}, /* Registered Output */
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 960, 1080},  /* Registered Output */
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 800, 920},   /* Registered Output */
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 640, 760},   /* Registered Output */
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 480, 600},   /* Registered Output */
	{21, NO_OUTPUT_ENABLE_FUSE_ROW, 320, 440},   /* Registered Output */
	{22, NO_OUTPUT_ENABLE_FUSE_ROW, 160, 280},   /* Registered Output */
	{23, 0, 40, 120}};

static pin_fuse_rows pal20x10pinfuserows[] = {
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 1440, 1560}, /* Registered Output */
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1400}, /* Registered Output */
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 1120, 1240}, /* Registered Output */
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 960, 1080},  /* Registered Output */
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 800, 920},   /* Registered Output */
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 640, 760},   /* Registered Output */
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 480, 600},   /* Registered Output */
	{21, NO_OUTPUT_ENABLE_FUSE_ROW, 320, 440},   /* Registered Output */
	{22, NO_OUTPUT_ENABLE_FUSE_ROW, 160, 280},   /* Registered Output */
	{23, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 120}};    /* Registered Output */

static pin_fuse_rows _82s153_pls153pinfuserows[] = {
	{9,  1472, 0, 0},
	{11, 1508, 0, 0},
	{12, 1544, 0, 0},
	{13, 1580, 0, 0},
	{14, 1616, 0, 0},
	{15, 1652, 0, 0},
	{16, 1688, 0, 0},
	{17, 1724, 0, 0},
	{18, 1760, 0, 0},
	{19, 1796, 0, 0}};

static pin_fuse_rows ck2605pinfuserows[] = {
	{9,  736,  0, 0},
	{11, 772,  0, 0},
	{12, 808,  0, 0},
	{13, 844,  0, 0},
	{14, 880,  0, 0},
	{15, 916,  0, 0},
	{16, 952,  0, 0},
	{17, 988,  0, 0},
	{18, 1024, 0, 0},
	{19, 1060, 0, 0}};

#if defined(ricoh_pals)
static pin_fuse_rows epl10p8pinfuserows[] = {
	{12, NO_OUTPUT_ENABLE_FUSE_ROW, 560, 620},
	{13, NO_OUTPUT_ENABLE_FUSE_ROW, 480, 540},
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 400, 460},
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 320, 380},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 240, 300},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 160, 220},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 80, 140},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 60}};

static pin_fuse_rows epl12p6pinfuserows[] = {
	{13, NO_OUTPUT_ENABLE_FUSE_ROW, 576, 744},
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 480, 552},
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 384, 456},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 288, 360},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 192, 264},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 168}};

static pin_fuse_rows epl14p4pinfuserows[] = {
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 672, 868},
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 448, 644},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 224, 420},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 196}};

static pin_fuse_rows epl16p2pinfuserows[] = {
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 992},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 480}};

static pin_fuse_rows epl16p8pinfuserows[] = {
	{12, 1792, 1824, 2016},
	{13, 1536, 1568, 1760},
	{14, 1280, 1312, 1504},
	{15, 1024, 1056, 1248},
	{16, 768, 800, 992},
	{17, 512, 544, 736},
	{18, 256, 288, 480},
	{19, 0, 32, 224}};

static pin_fuse_rows epl16rp8pinfuserows[] = {
	{12, NO_OUTPUT_ENABLE_FUSE_ROW, 1792, 2016}, /* Registered Output */
	{13, NO_OUTPUT_ENABLE_FUSE_ROW, 1536, 1760}, /* Registered Output */
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1504}, /* Registered Output */
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 1024, 1248}, /* Registered Output */
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 768, 992},   /* Registered Output */
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 736},   /* Registered Output */
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 480},   /* Registered Output */
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 224}};    /* Registered Output */

static pin_fuse_rows epl16rp6pinfuserows[] = {
	{12, 1792, 1824, 2016},
	{13, NO_OUTPUT_ENABLE_FUSE_ROW, 1536, 1760}, /* Registered Output */
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1504}, /* Registered Output */
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 1024, 1248}, /* Registered Output */
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 768, 992},   /* Registered Output */
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 736},   /* Registered Output */
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 480},   /* Registered Output */
	{19, 0, 32, 224}};

static pin_fuse_rows epl16rp4pinfuserows[] = {
	{12, 1792, 1824, 2016},
	{13, 1536, 1568, 1760},
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1504}, /* Registered Output */
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 1024, 1248}, /* Registered Output */
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 768, 992},   /* Registered Output */
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 736},   /* Registered Output */
	{18, 256, 288, 480},
	{19, 0, 32, 224}};
#endif

static pin_fuse_rows pal10p8pinfuserows[] = {
	{12, NO_OUTPUT_ENABLE_FUSE_ROW, 280, 300},
	{13, NO_OUTPUT_ENABLE_FUSE_ROW, 240, 260},
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 200, 220},
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 160, 180},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 120, 140},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 80, 100},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 40, 60},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 20}};

static pin_fuse_rows pal12p6pinfuserows[] = {
	{13, NO_OUTPUT_ENABLE_FUSE_ROW, 288, 360},
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 240, 264},
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 192, 216},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 144, 168},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 96, 120},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 72}};

static pin_fuse_rows pal14p4pinfuserows[] = {
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 336, 420},
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 224, 308},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 112, 196},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 84}};

static pin_fuse_rows pal16p2pinfuserows[] = {
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 480},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 224}};

static pin_fuse_rows pal16p8pinfuserows[] = {
	{12, 1792, 1824, 2016},
	{13, 1536, 1568, 1760},
	{14, 1280, 1312, 1504},
	{15, 1024, 1056, 1248},
	{16, 768, 800, 992},
	{17, 512, 544, 736},
	{18, 256, 288, 480},
	{19, 0, 32, 224}};

static pin_fuse_rows pal16rp4pinfuserows[] = {
	{12, 1792, 1824, 2016},
	{13, 1536, 1568, 1760},
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1504}, /* Registered Output */
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 1024, 1248}, /* Registered Output */
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 768, 992},   /* Registered Output */
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 736},   /* Registered Output */
	{18, 256, 288, 480},
	{19, 0, 32, 224}};

static pin_fuse_rows pal16rp6pinfuserows[] = {
	{12, 1792, 1824, 2016},
	{13, NO_OUTPUT_ENABLE_FUSE_ROW, 1536, 1760}, /* Registered Output */
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1504}, /* Registered Output */
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 1024, 1248}, /* Registered Output */
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 768, 992},   /* Registered Output */
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 736},   /* Registered Output */
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 480},   /* Registered Output */
	{19, 0, 32, 224}};

static pin_fuse_rows pal16rp8pinfuserows[] = {
	{12, NO_OUTPUT_ENABLE_FUSE_ROW, 1792, 2016}, /* Registered Output */
	{13, NO_OUTPUT_ENABLE_FUSE_ROW, 1536, 1760}, /* Registered Output */
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1504}, /* Registered Output */
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 1024, 1248}, /* Registered Output */
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 768, 992},   /* Registered Output */
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 736},   /* Registered Output */
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 480},   /* Registered Output */
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 224}};    /* Registered Output */

static pin_fuse_rows pal6l16pinfuserows[] = {
	{1, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 0},
	{2, NO_OUTPUT_ENABLE_FUSE_ROW, 24, 24},
	{3, NO_OUTPUT_ENABLE_FUSE_ROW, 36, 36},
	{10, NO_OUTPUT_ENABLE_FUSE_ROW, 132, 132},
	{11, NO_OUTPUT_ENABLE_FUSE_ROW, 168, 168},
	{13, NO_OUTPUT_ENABLE_FUSE_ROW, 180, 180},
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 156, 156},
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 144, 144},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 120, 120},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 108, 108},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 96, 96},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 84, 84},
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 72, 72},
	{21, NO_OUTPUT_ENABLE_FUSE_ROW, 60, 60},
	{22, NO_OUTPUT_ENABLE_FUSE_ROW, 48, 48},
	{23, NO_OUTPUT_ENABLE_FUSE_ROW, 12, 12}};

static pin_fuse_rows pal8l14pinfuserows[] = {
	{1, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 0},
	{2, NO_OUTPUT_ENABLE_FUSE_ROW, 32, 32},
	{11, NO_OUTPUT_ENABLE_FUSE_ROW, 192, 192},
	{13, NO_OUTPUT_ENABLE_FUSE_ROW, 208, 208},
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 176, 176},
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 160, 160},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 144, 144},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 128, 128},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 112, 112},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 96, 96},
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 80, 80},
	{21, NO_OUTPUT_ENABLE_FUSE_ROW, 64, 64},
	{22, NO_OUTPUT_ENABLE_FUSE_ROW, 48, 48},
	{23, NO_OUTPUT_ENABLE_FUSE_ROW, 16, 16}};

static pin_fuse_rows pal12h10pinfuserows[] = {
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 432, 456},
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 384, 408},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 336, 360},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 288, 312},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 240, 264},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 192, 216},
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 144, 168},
	{21, NO_OUTPUT_ENABLE_FUSE_ROW, 96, 120},
	{22, NO_OUTPUT_ENABLE_FUSE_ROW, 48, 72},
	{23, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 24}};

static pin_fuse_rows pal12l10pinfuserows[] = {
	{14, NO_OUTPUT_ENABLE_FUSE_ROW, 432, 456},
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 384, 408},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 336, 360},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 288, 312},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 240, 264},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 192, 216},
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 144, 168},
	{21, NO_OUTPUT_ENABLE_FUSE_ROW, 96, 120},
	{22, NO_OUTPUT_ENABLE_FUSE_ROW, 48, 72},
	{23, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 24}};

static pin_fuse_rows pal14h8pinfuserows[] = {
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 448, 532},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 392, 420},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 336, 364},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 280, 308},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 224, 252},
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 168, 196},
	{21, NO_OUTPUT_ENABLE_FUSE_ROW, 112, 140},
	{22, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 84}};

static pin_fuse_rows pal14l8pinfuserows[] = {
	{15, NO_OUTPUT_ENABLE_FUSE_ROW, 448, 532},
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 392, 420},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 336, 364},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 280, 308},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 224, 252},
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 168, 196},
	{21, NO_OUTPUT_ENABLE_FUSE_ROW, 112, 140},
	{22, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 84}};

static pin_fuse_rows pal16h6pinfuserows[] = {
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 608},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 384, 480},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 320, 352},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 288},
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 128, 224},
	{21, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 96}};

static pin_fuse_rows pal16l6pinfuserows[] = {
	{16, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 608},
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 384, 480},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 320, 352},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 288},
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 128, 224},
	{21, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 96}};

static pin_fuse_rows pal18h4pinfuserows[] = {
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 504, 684},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 360, 468},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 216, 324},
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 180}};

static pin_fuse_rows pal18l4pinfuserows[] = {
	{17, NO_OUTPUT_ENABLE_FUSE_ROW, 504, 684},
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 360, 468},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 216, 324},
	{20, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 180}};

static pin_fuse_rows pal20c1pinfuserows[] = {
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 280},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 320, 600}};

static pin_fuse_rows pal20l2pinfuserows[] = {
	{18, NO_OUTPUT_ENABLE_FUSE_ROW, 320, 600},
	{19, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 280}};

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

static pin_fuse_columns palce16v8pinfusecolumns[] = {
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
	{14, 23, 22},
	{15, 19, 18},
	{16, 15, 14},
	{17, 11, 10},
	{18, 7, 6},
	{19, 3, 2}};

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

static pin_fuse_columns peel18cv8pinfusecolumns[] = {
	{1, 1, 0},
	{2, 5, 4},
	{3, 9, 8},
	{4, 13, 12},
	{5, 17, 16},
	{6, 21, 20},
	{7, 25, 24},
	{8, 29, 28},
	{9, 33, 32},
	{11, 3, 2},
	{12, 35, 34},
	{13, 31, 30},
	{14, 27, 26},
	{15, 23, 22},
	{16, 19, 18},
	{17, 15, 14},
	{18, 11, 10},
	{19, 7, 6}};

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

static pin_fuse_columns pal20x4pinfusecolumns[] = {
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

static pin_fuse_columns pal20x8pinfusecolumns[] = {
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

static pin_fuse_columns pal20x10pinfusecolumns[] = {
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
	{23, 3, 2}
};

static pin_fuse_columns _82s153_pls153pinfusecolumns[] = {
	{1,  1,  0},
	{2,  3,  2},
	{3,  5,  4},
	{4,  7,  6},
	{5,  9,  8},
	{6,  11, 10},
	{7,  13, 12},
	{8,  15, 14},
	{9,  17, 16},
	{11, 19, 18},
	{12, 21, 20},
	{13, 23, 22},
	{14, 25, 24},
	{15, 27, 26},
	{16, 29, 28},
	{17, 31, 30},
	{18, 33, 32},
	{19, 35, 34}};

static pin_fuse_columns ck2605pinfusecolumns[] = {
	{1,  1,  0},
	{2,  3,  2},
	{3,  5,  4},
	{4,  7,  6},
	{5,  9,  8},
	{6,  11, 10},
	{7,  13, 12},
	{8,  15, 14},
	{9,  17, 16},
	{11, 19, 18},
	{12, 21, 20},
	{13, 23, 22},
	{14, 25, 24},
	{15, 27, 26},
	{16, 29, 28},
	{17, 31, 30},
	{18, 33, 32},
	{19, 35, 34}};

#if defined(ricoh_pals)
static pin_fuse_columns epl10p8pinfusecolumns[] = {
	{1,  3,  2},
	{2,  1,  0},
	{3,  5,  4},
	{4,  7,  6},
	{5,  9,  8},
	{6,  11, 10},
	{7,  13, 12},
	{8,  15, 14},
	{9,  17, 16},
	{11, 19, 18}};

static pin_fuse_columns epl12p6pinfusecolumns[] = {
	{1,  3,  2},
	{2,  1,  0},
	{3,  5,  4},
	{4,  9,  8},
	{5,  11, 10},
	{6,  13, 12},
	{7,  15, 14},
	{8,  17, 16},
	{9,  19, 18},
	{11, 21, 20},
	{12, 19, 18},
	{19, 7,  6}};

static pin_fuse_columns epl14p4pinfusecolumns[] = {
	{1,  3,  2},
	{2,  1,  0},
	{3,  5,  4},
	{4,  9,  8},
	{5,  13, 12},
	{6,  15, 14},
	{7,  17, 16},
	{8,  21, 20},
	{9,  25, 24},
	{11, 27, 26},
	{12, 23, 22},
	{13, 19, 18},
	{18, 11, 10},
	{19, 7,  6}};

static pin_fuse_columns epl16p2pinfusecolumns[] = {
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

static pin_fuse_columns epl16p8pinfusecolumns[] = {
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
	{18, 7,  6}};

static pin_fuse_columns epl16rp8pinfusecolumns[] = {
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
	{18, 7,  6},
	{19, 3,  2}};

static pin_fuse_columns epl16rp6pinfusecolumns[] = {
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
	{18, 7,  6},
	{19, 3,  2}};

static pin_fuse_columns epl16rp4pinfusecolumns[] = {
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
	{18, 7,  6},
	{19, 3,  2}};
#endif

static pin_fuse_columns pal10p8pinfusecolumns[] = {
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

static pin_fuse_columns pal12p6pinfusecolumns[] = {
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

static pin_fuse_columns pal14p4pinfusecolumns[] = {
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

static pin_fuse_columns pal16p2pinfusecolumns[] = {
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

static pin_fuse_columns pal16p8pinfusecolumns[] = {
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

static pin_fuse_columns pal16rp4pinfusecolumns[] = {
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

static pin_fuse_columns pal16rp6pinfusecolumns[] = {
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

static pin_fuse_columns pal16rp8pinfusecolumns[] = {
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

static pin_fuse_columns pal6l16pinfusecolumns[] = {
	{4, 1, 0},
	{5, 3, 2},
	{6, 5, 4},
	{7, 7, 6},
	{8, 9, 8},
	{9, 11, 10}};

static pin_fuse_columns pal8l14pinfusecolumns[] = {
	{3, 1, 0},
	{4, 3, 2},
	{5, 5, 4},
	{6, 7, 6},
	{7, 9, 8},
	{8, 11, 10},
	{9, 13, 12},
	{10, 15, 14}};

static pin_fuse_columns pal12h10pinfusecolumns[] = {
	{1, 3, 2},
	{2, 1, 0},
	{3, 5, 4},
	{4, 7, 6},
	{5, 9, 8},
	{6, 11, 10},
	{7, 13, 12},
	{8, 15, 14},
	{9, 17, 16},
	{10, 19, 18},
	{11, 21, 20},
	{13, 23, 22}};

static pin_fuse_columns pal12l10pinfusecolumns[] = {
	{1, 3, 2},
	{2, 1, 0},
	{3, 5, 4},
	{4, 7, 6},
	{5, 9, 8},
	{6, 11, 10},
	{7, 13, 12},
	{8, 15, 14},
	{9, 17, 16},
	{10, 19, 18},
	{11, 21, 20},
	{13, 23, 22}};

static pin_fuse_columns pal14h8pinfusecolumns[] = {
	{1, 3, 2},
	{2, 1, 0},
	{3, 5, 4},
	{4, 9, 8},
	{5, 11, 10},
	{6, 13, 12},
	{7, 15, 14},
	{8, 17, 16},
	{9, 19, 18},
	{10, 21, 20},
	{11, 25, 24},
	{13, 27, 26},
	{14, 23, 22},
	{23, 7, 6}};

static pin_fuse_columns pal14l8pinfusecolumns[] = {
	{1, 3, 2},
	{2, 1, 0},
	{3, 5, 4},
	{4, 9, 8},
	{5, 11, 10},
	{6, 13, 12},
	{7, 15, 14},
	{8, 17, 16},
	{9, 19, 18},
	{10, 21, 20},
	{11, 25, 24},
	{13, 27, 26},
	{14, 23, 22},
	{23, 7, 6}};

static pin_fuse_columns pal16h6pinfusecolumns[] = {
	{1, 3, 2},
	{2, 1, 0},
	{3, 5, 4},
	{4, 9, 8},
	{5, 13, 12},
	{6, 15, 14},
	{7, 17, 16},
	{8, 19, 18},
	{9, 21, 20},
	{10, 25, 24},
	{11, 29, 28},
	{13, 31, 30},
	{14, 27, 26},
	{15, 23, 22},
	{22, 11, 10},
	{23, 7, 6}};

static pin_fuse_columns pal16l6pinfusecolumns[] = {
	{1, 3, 2},
	{2, 1, 0},
	{3, 5, 4},
	{4, 9, 8},
	{5, 13, 12},
	{6, 15, 14},
	{7, 17, 16},
	{8, 19, 18},
	{9, 21, 20},
	{10, 25, 24},
	{11, 29, 28},
	{13, 31, 30},
	{14, 27, 26},
	{15, 23, 22},
	{22, 11, 10},
	{23, 7, 6}};

static pin_fuse_columns pal18h4pinfusecolumns[] = {
	{1, 3, 2},
	{2, 1, 0},
	{3, 5, 4},
	{4, 9, 8},
	{5, 13, 12},
	{6, 17, 16},
	{7, 19, 18},
	{8, 21, 20},
	{9, 25, 24},
	{10, 29, 28},
	{11, 33, 32},
	{13, 35, 34},
	{14, 31, 30},
	{15, 27, 26},
	{16, 23, 22},
	{21, 15, 14},
	{22, 11, 10},
	{23, 7, 6}};

static pin_fuse_columns pal18l4pinfusecolumns[] = {
	{1, 3, 2},
	{2, 1, 0},
	{3, 5, 4},
	{4, 9, 8},
	{5, 13, 12},
	{6, 17, 16},
	{7, 19, 18},
	{8, 21, 20},
	{9, 25, 24},
	{10, 29, 28},
	{11, 33, 32},
	{13, 35, 34},
	{14, 31, 30},
	{15, 27, 26},
	{16, 23, 22},
	{21, 15, 14},
	{22, 11, 10},
	{23, 7, 6}};

static pin_fuse_columns pal20c1pinfusecolumns[] = {
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
	{15, 31, 30},
	{16, 27, 26},
	{17, 23, 22},
	{20, 19, 18},
	{21, 15, 14},
	{22, 11, 10},
	{23, 7, 6}};

static pin_fuse_columns pal20l2pinfusecolumns[] = {
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
	{15, 31, 30},
	{16, 27, 26},
	{17, 23, 22},
	{20, 19, 18},
	{21, 15, 14},
	{22, 11, 10},
	{23, 7, 6}};

static pal_data paldata[] = {
	{"PAL10L8", 320,
		pal10l8pinfuserows, ARRAY_LENGTH(pal10l8pinfuserows),
		pal10l8pinfusecolumns, ARRAY_LENGTH(pal10l8pinfusecolumns),
		print_pal10l8_product_terms,
		config_pal10l8_pins,
		nullptr,
		nullptr},
	{"PAL10H8", 320,
		pal10h8pinfuserows, ARRAY_LENGTH(pal10h8pinfuserows),
		pal10h8pinfusecolumns, ARRAY_LENGTH(pal10h8pinfusecolumns),
		print_pal10h8_product_terms,
		config_pal10h8_pins,
		nullptr,
		nullptr},
	{"PAL12H6", 384,
		pal12h6pinfuserows, ARRAY_LENGTH(pal12h6pinfuserows),
		pal12h6pinfusecolumns, ARRAY_LENGTH(pal12h6pinfusecolumns),
		print_pal12h6_product_terms,
		config_pal12h6_pins,
		nullptr,
		nullptr},
	{"PAL14H4", 448,
		pal14h4pinfuserows, ARRAY_LENGTH(pal14h4pinfuserows),
		pal14h4pinfusecolumns, ARRAY_LENGTH(pal14h4pinfusecolumns),
		print_pal14h4_product_terms,
		config_pal14h4_pins,
		nullptr,
		nullptr},
	{"PAL16H2", 512,
		pal16h2pinfuserows, ARRAY_LENGTH(pal16h2pinfuserows),
		pal16h2pinfusecolumns, ARRAY_LENGTH(pal16h2pinfusecolumns),
		print_pal16h2_product_terms,
		config_pal16h2_pins,
		nullptr,
		nullptr},
	{"PAL16C1", 512,
		pal16c1pinfuserows, ARRAY_LENGTH(pal16c1pinfuserows),
		pal16c1pinfusecolumns, ARRAY_LENGTH(pal16c1pinfusecolumns),
		print_pal16c1_product_terms,
		config_pal16c1_pins,
		nullptr,
		nullptr},
	{"PAL12L6", 384,
		pal12l6pinfuserows, ARRAY_LENGTH(pal12l6pinfuserows),
		pal12l6pinfusecolumns, ARRAY_LENGTH(pal12l6pinfusecolumns),
		print_pal12l6_product_terms,
		config_pal12l6_pins,
		nullptr,
		nullptr},
	{"PAL14L4", 448,
		pal14l4pinfuserows, ARRAY_LENGTH(pal14l4pinfuserows),
		pal14l4pinfusecolumns, ARRAY_LENGTH(pal14l4pinfusecolumns),
		print_pal14l4_product_terms,
		config_pal14l4_pins,
		nullptr,
		nullptr},
	{"PAL16L2", 512,
		pal16l2pinfuserows, ARRAY_LENGTH(pal16l2pinfuserows),
		pal16l2pinfusecolumns, ARRAY_LENGTH(pal16l2pinfusecolumns),
		print_pal16l2_product_terms,
		config_pal16l2_pins,
		nullptr,
		nullptr},
	/*{"15S8", 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL},*/
	{"PAL16L8", 2048,
		pal16l8pinfuserows, ARRAY_LENGTH(pal16l8pinfuserows),
		pal16l8pinfusecolumns, ARRAY_LENGTH(pal16l8pinfusecolumns),
		print_pal16l8_product_terms,
		config_pal16l8_pins,
		nullptr,
		nullptr},
	{"PAL16R4", 2048,
		pal16r4pinfuserows, ARRAY_LENGTH(pal16r4pinfuserows),
		pal16r4pinfusecolumns, ARRAY_LENGTH(pal16r4pinfusecolumns),
		print_pal16r4_product_terms,
		config_pal16r4_pins,
		nullptr,
		nullptr},
	{"PAL16R6", 2048,
		pal16r6pinfuserows, ARRAY_LENGTH(pal16r6pinfuserows),
		pal16r6pinfusecolumns, ARRAY_LENGTH(pal16r6pinfusecolumns),
		print_pal16r6_product_terms,
		config_pal16r6_pins,
		nullptr,
		nullptr},
	{"PAL16R8", 2048,
		pal16r8pinfuserows, ARRAY_LENGTH(pal16r8pinfuserows),
		pal16r8pinfusecolumns, ARRAY_LENGTH(pal16r8pinfusecolumns),
		print_pal16r8_product_terms,
		config_pal16r8_pins,
		nullptr,
		nullptr},
	/*{"PAL16RA8", 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL},
	{"PAL16V8R", 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL},*/
	{"PALCE16V8", 2194,
		palce16v8pinfuserows, ARRAY_LENGTH(palce16v8pinfuserows),
		palce16v8pinfusecolumns, ARRAY_LENGTH(palce16v8pinfusecolumns),
		print_palce16v8_product_terms,
		config_palce16v8_pins,
		nullptr,
		nullptr},
	{"GAL16V8", 2194,
		gal16v8pinfuserows, ARRAY_LENGTH(gal16v8pinfuserows),
		gal16v8pinfusecolumns, ARRAY_LENGTH(gal16v8pinfusecolumns),
		print_gal16v8_product_terms,
		config_gal16v8_pins,
		is_gal16v8_product_term_enabled,
		nullptr},
	{"18CV8", 2696,
		peel18cv8pinfuserows, ARRAY_LENGTH(peel18cv8pinfuserows),
		peel18cv8pinfusecolumns, ARRAY_LENGTH(peel18cv8pinfusecolumns),
		print_peel18cv8_product_terms,
		config_peel18cv8_pins,
		nullptr,
		get_peel18cv8_pin_fuse_state},
	{"GAL18V10", 3540,
		gal18v10pinfuserows, ARRAY_LENGTH(gal18v10pinfuserows),
		gal18v10pinfusecolumns, ARRAY_LENGTH(gal18v10pinfusecolumns),
		print_gal18v10_product_terms,
		config_gal18v10_pins,
		nullptr,
		nullptr},
	{"PAL20L8", 2560,
		pal20l8pinfuserows, ARRAY_LENGTH(pal20l8pinfuserows),
		pal20l8pinfusecolumns, ARRAY_LENGTH(pal20l8pinfusecolumns),
		print_pal20l8_product_terms,
		config_pal20l8_pins,
		nullptr,
		nullptr},
	{"PAL20L10", 1600,
		pal20l10pinfuserows, ARRAY_LENGTH(pal20l10pinfuserows),
		pal20l10pinfusecolumns, ARRAY_LENGTH(pal20l10pinfusecolumns),
		print_pal20l10_product_terms,
		config_pal20l10_pins,
		nullptr,
		nullptr},
	{"PAL20R4", 2560,
		pal20r4pinfuserows, ARRAY_LENGTH(pal20r4pinfuserows),
		pal20r4pinfusecolumns, ARRAY_LENGTH(pal20r4pinfusecolumns),
		print_pal20r4_product_terms,
		config_pal20r4_pins,
		nullptr,
		nullptr},
	{"PAL20R6", 2560,
		pal20r6pinfuserows, ARRAY_LENGTH(pal20r6pinfuserows),
		pal20r6pinfusecolumns, ARRAY_LENGTH(pal20r6pinfusecolumns),
		print_pal20r6_product_terms,
		config_pal20r6_pins,
		nullptr,
		nullptr},
	{"PAL20R8", 2560,
		pal20r8pinfuserows, ARRAY_LENGTH(pal20r8pinfuserows),
		pal20r8pinfusecolumns, ARRAY_LENGTH(pal20r8pinfusecolumns),
		print_pal20r8_product_terms,
		config_pal20r8_pins,
		nullptr,
		nullptr},
	{"PAL20X4", 1600,
		pal20x4pinfuserows, ARRAY_LENGTH(pal20x4pinfuserows),
		pal20x4pinfusecolumns, ARRAY_LENGTH(pal20x4pinfusecolumns),
		print_pal20x4_product_terms,
		config_pal20x4_pins,
		nullptr,
		nullptr},
	{"PAL20X8", 1600,
		pal20x8pinfuserows, ARRAY_LENGTH(pal20x8pinfuserows),
		pal20x8pinfusecolumns, ARRAY_LENGTH(pal20x8pinfusecolumns),
		print_pal20x8_product_terms,
		config_pal20x8_pins,
		nullptr,
		nullptr},
	{"PAL20X10", 1600,
		pal20x10pinfuserows, ARRAY_LENGTH(pal20x10pinfuserows),
		pal20x10pinfusecolumns, ARRAY_LENGTH(pal20x10pinfusecolumns),
		print_pal20x10_product_terms,
		config_pal20x10_pins,
		nullptr,
		nullptr},
	/*{"PAL22V10", 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL},
	{"GAL20V8A", 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL},
	{"GAL22V10", 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL},
	{"PLS100", 0, NULL, 0, NULL, 0, NULL, NULL, NULL, NULL},*/
	{"82S153", 1842,
		_82s153_pls153pinfuserows, ARRAY_LENGTH(_82s153_pls153pinfuserows),
		_82s153_pls153pinfusecolumns, ARRAY_LENGTH(_82s153_pls153pinfusecolumns),
		print_82s153_pls153_product_terms,
		config_82s153_pls153_pins,
		nullptr,
		nullptr},
	{"PLS153", 1842,
		_82s153_pls153pinfuserows, ARRAY_LENGTH(_82s153_pls153pinfuserows),
		_82s153_pls153pinfusecolumns, ARRAY_LENGTH(_82s153_pls153pinfusecolumns),
		print_82s153_pls153_product_terms,
		config_82s153_pls153_pins,
		nullptr,
		nullptr},
	{"CK2605", 1106,
		ck2605pinfuserows, ARRAY_LENGTH(ck2605pinfuserows),
		ck2605pinfusecolumns, ARRAY_LENGTH(ck2605pinfusecolumns),
		print_ck2605_product_terms,
		config_ck2605_pins,
		nullptr,
		nullptr},
#if defined(ricoh_pals)
	{"EPL10P8", 664,
		epl10p8pinfuserows, ARRAY_LENGTH(epl10p8pinfuserows),
		epl10p8pinfusecolumns, ARRAY_LENGTH(epl10p8pinfusecolumns),
		print_epl10p8_product_terms,
		config_epl10p8_pins,
		NULL,
		NULL},
	{"EPL12P6", 786,
		epl12p6pinfuserows, ARRAY_LENGTH(epl12p6pinfuserows),
		epl12p6pinfusecolumns, ARRAY_LENGTH(epl12p6pinfusecolumns),
		print_epl12p6_product_terms,
		config_epl12p6_pins,
		NULL,
		NULL},
	{"EPL14P4", 908,
		epl14p4pinfuserows, ARRAY_LENGTH(epl14p4pinfuserows),
		epl14p4pinfusecolumns, ARRAY_LENGTH(epl14p4pinfusecolumns),
		print_epl14p4_product_terms,
		config_epl14p4_pins,
		NULL,
		NULL},
	{"EPL16P2", 1030,
		epl16p2pinfuserows, ARRAY_LENGTH(epl16p2pinfuserows),
		epl16p2pinfusecolumns, ARRAY_LENGTH(epl16p2pinfusecolumns),
		print_epl16p2_product_terms,
		config_epl16p2_pins,
		NULL,
		NULL},
	{"EPL16P8", 2072,
		epl16p8pinfuserows, ARRAY_LENGTH(epl16p8pinfuserows),
		epl16p8pinfusecolumns, ARRAY_LENGTH(epl16p8pinfusecolumns),
		print_epl16p8_product_terms,
		config_epl16p8_pins,
		NULL,
		NULL},
	{"EPL16RP8", 2072,
		epl16rp8pinfuserows, ARRAY_LENGTH(epl16rp8pinfuserows),
		epl16rp8pinfusecolumns, ARRAY_LENGTH(epl16rp8pinfusecolumns),
		print_epl16rp8_product_terms,
		config_epl16rp8_pins,
		NULL,
		NULL},
	{"EPL16RP6", 2072,
		epl16rp6pinfuserows, ARRAY_LENGTH(epl16rp6pinfuserows),
		epl16rp6pinfusecolumns, ARRAY_LENGTH(epl16rp6pinfusecolumns),
		print_epl16rp6_product_terms,
		config_epl16rp6_pins,
		NULL,
		NULL},
	{"EPL16RP4", 2072,
		epl16rp4pinfuserows, ARRAY_LENGTH(epl16rp4pinfuserows),
		epl16rp4pinfusecolumns, ARRAY_LENGTH(epl16rp4pinfusecolumns),
		print_epl16rp4_product_terms,
		config_epl16rp4_pins,
		NULL,
		NULL},
#endif
	{"PAL10P8", 328,
		pal10p8pinfuserows, ARRAY_LENGTH(pal10p8pinfuserows),
		pal10p8pinfusecolumns, ARRAY_LENGTH(pal10p8pinfusecolumns),
		print_pal10p8_product_terms,
		config_pal10p8_pins,
		nullptr,
		nullptr},
	{"PAL12P6", 390,
		pal12p6pinfuserows, ARRAY_LENGTH(pal12p6pinfuserows),
		pal12p6pinfusecolumns, ARRAY_LENGTH(pal12p6pinfusecolumns),
		print_pal12p6_product_terms,
		config_pal12p6_pins,
		nullptr,
		nullptr},
	{"PAL14P4", 452,
		pal14p4pinfuserows, ARRAY_LENGTH(pal14p4pinfuserows),
		pal14p4pinfusecolumns, ARRAY_LENGTH(pal14p4pinfusecolumns),
		print_pal14p4_product_terms,
		config_pal14p4_pins,
		nullptr,
		nullptr},
	{"PAL16P2", 514,
		pal16p2pinfuserows, ARRAY_LENGTH(pal16p2pinfuserows),
		pal16p2pinfusecolumns, ARRAY_LENGTH(pal16p2pinfusecolumns),
		print_pal16p2_product_terms,
		config_pal16p2_pins,
		nullptr,
		nullptr},
	{"PAL16P8", 2056,
		pal16p8pinfuserows, ARRAY_LENGTH(pal16p8pinfuserows),
		pal16p8pinfusecolumns, ARRAY_LENGTH(pal16p8pinfusecolumns),
		print_pal16p8_product_terms,
		config_pal16p8_pins,
		nullptr,
		nullptr},
	{"PAL16RP4", 2056,
		pal16rp4pinfuserows, ARRAY_LENGTH(pal16rp4pinfuserows),
		pal16rp4pinfusecolumns, ARRAY_LENGTH(pal16rp4pinfusecolumns),
		print_pal16rp4_product_terms,
		config_pal16rp4_pins,
		nullptr,
		nullptr},
	{"PAL16RP6", 2056,
		pal16rp6pinfuserows, ARRAY_LENGTH(pal16rp6pinfuserows),
		pal16rp6pinfusecolumns, ARRAY_LENGTH(pal16rp6pinfusecolumns),
		print_pal16rp6_product_terms,
		config_pal16rp6_pins,
		nullptr,
		nullptr},
	{"PAL16RP8", 2056,
		pal16rp8pinfuserows, ARRAY_LENGTH(pal16rp8pinfuserows),
		pal16rp8pinfusecolumns, ARRAY_LENGTH(pal16rp8pinfusecolumns),
		print_pal16rp8_product_terms,
		config_pal16rp8_pins,
		nullptr,
		nullptr},
	{"PAL6L16", 192,
		pal6l16pinfuserows, ARRAY_LENGTH(pal6l16pinfuserows),
		pal6l16pinfusecolumns, ARRAY_LENGTH(pal6l16pinfusecolumns),
		print_pal6l16_product_terms,
		config_pal6l16_pins,
		nullptr,
		nullptr},
	{"PAL8L14", 224,
		pal8l14pinfuserows, ARRAY_LENGTH(pal8l14pinfuserows),
		pal8l14pinfusecolumns, ARRAY_LENGTH(pal8l14pinfusecolumns),
		print_pal8l14_product_terms,
		config_pal8l14_pins,
		nullptr,
		nullptr},
	{"PAL12H10", 480,
		pal12h10pinfuserows, ARRAY_LENGTH(pal12h10pinfuserows),
		pal12h10pinfusecolumns, ARRAY_LENGTH(pal12h10pinfusecolumns),
		print_pal12h10_product_terms,
		config_pal12h10_pins,
		nullptr,
		nullptr},
	{"PAL12L10", 480,
		pal12l10pinfuserows, ARRAY_LENGTH(pal12l10pinfuserows),
		pal12l10pinfusecolumns, ARRAY_LENGTH(pal12l10pinfusecolumns),
		print_pal12l10_product_terms,
		config_pal12l10_pins,
		nullptr,
		nullptr},
	{"PAL14H8", 560,
		pal14h8pinfuserows, ARRAY_LENGTH(pal14h8pinfuserows),
		pal14h8pinfusecolumns, ARRAY_LENGTH(pal14h8pinfusecolumns),
		print_pal14h8_product_terms,
		config_pal14h8_pins,
		nullptr,
		nullptr},
	{"PAL14L8", 560,
		pal14l8pinfuserows, ARRAY_LENGTH(pal14l8pinfuserows),
		pal14l8pinfusecolumns, ARRAY_LENGTH(pal14l8pinfusecolumns),
		print_pal14l8_product_terms,
		config_pal14l8_pins,
		nullptr,
		nullptr},
	{"PAL16H6", 640,
		pal16h6pinfuserows, ARRAY_LENGTH(pal16h6pinfuserows),
		pal16h6pinfusecolumns, ARRAY_LENGTH(pal16h6pinfusecolumns),
		print_pal16h6_product_terms,
		config_pal16h6_pins,
		nullptr,
		nullptr},
	{"PAL16L6", 640,
		pal16l6pinfuserows, ARRAY_LENGTH(pal16l6pinfuserows),
		pal16l6pinfusecolumns, ARRAY_LENGTH(pal16l6pinfusecolumns),
		print_pal16l6_product_terms,
		config_pal16l6_pins,
		nullptr,
		nullptr},
	{"PAL18H4", 720,
		pal18h4pinfuserows, ARRAY_LENGTH(pal18h4pinfuserows),
		pal18h4pinfusecolumns, ARRAY_LENGTH(pal18h4pinfusecolumns),
		print_pal18h4_product_terms,
		config_pal18h4_pins,
		nullptr,
		nullptr},
	{"PAL18L4", 720,
		pal18l4pinfuserows, ARRAY_LENGTH(pal18l4pinfuserows),
		pal18l4pinfusecolumns, ARRAY_LENGTH(pal18l4pinfusecolumns),
		print_pal18l4_product_terms,
		config_pal18l4_pins,
		nullptr,
		nullptr},
	{"PAL20C1", 640,
		pal20c1pinfuserows, ARRAY_LENGTH(pal20c1pinfuserows),
		pal20c1pinfusecolumns, ARRAY_LENGTH(pal20c1pinfusecolumns),
		print_pal20c1_product_terms,
		config_pal20c1_pins,
		nullptr,
		nullptr},
	{"PAL20L2", 640,
		pal20l2pinfuserows, ARRAY_LENGTH(pal20l2pinfuserows),
		pal20l2pinfusecolumns, ARRAY_LENGTH(pal20l2pinfusecolumns),
		print_pal20l2_product_terms,
		config_pal20l2_pins,
		nullptr,
		nullptr}};

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
    is_pla_file - test if the file extension is
    that of a Berkeley standard PLA file
-------------------------------------------------*/

static int is_pla_file(const char *file)
{
	int len;

	/* does the source end in '.pla'? */
	len = strlen(file);

	return (file[len - 4] == '.' &&
			tolower((UINT8)file[len - 3]) == 'p' &&
			tolower((UINT8)file[len - 2]) == 'l' &&
			tolower((UINT8)file[len - 1]) == 'a');
}



/*-------------------------------------------------
    find_pal_data - finds the data associated
    with a pal name
-------------------------------------------------*/

static const pal_data* find_pal_data(const char *name)
{
	int index;

	for (index = 0; index < ARRAY_LENGTH(paldata); ++index)
	{
		if (!core_stricmp(name, paldata[index].name))
		{
			return &paldata[index];
		}
	}

	return nullptr;
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

	return nullptr;
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

	return nullptr;
}



/*-------------------------------------------------
    find_pin_from_fuse_row - finds the pin
    associated with a fuse row
-------------------------------------------------*/

static UINT16 find_pin_from_fuse_row(const pal_data* pal, UINT16 fuserow)
{
	int index;

	for (index = 0; index < pal->pinfuserowscount; ++index)
	{
		if (pal->pinfuserows[index].fuserowoutputenable != NO_OUTPUT_ENABLE_FUSE_ROW)
		{
			if (pal->pinfuserows[index].fuserowoutputenable == fuserow)
			{
				return pal->pinfuserows[index].pin;
			}
		}

		if (fuserow >= pal->pinfuserows[index].fuserowtermstart &&
			fuserow <= pal->pinfuserows[index].fuserowtermend)
		{
			return pal->pinfuserows[index].pin;
		}
	}

	return 0;
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
    does_output_enable_fuse_row_allow_output - checks
    if an output enable fuse row contains a product
    term that enables the output.
-------------------------------------------------*/

static int does_output_enable_fuse_row_allow_output(const pal_data* pal, const jed_data* jed, UINT16 fuserow)
{
	int lowfusestate, highfusestate;
	UINT16 index;

	for (index = 0; index < pal->pinfusecolumnscount; ++index)
	{
		lowfusestate = jed_get_fuse(jed, fuserow + pal->pinfusecolumns[index].lowfusecolumn);
		highfusestate = jed_get_fuse(jed, fuserow + pal->pinfusecolumns[index].highfusecolumn);

		if (!lowfusestate && !highfusestate)
		{
			return 0;
		}
	}

	return 1;
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

		if (pal->get_pin_fuse_state)
		{
			fuse_state = pal->get_pin_fuse_state(pal, jed, pin, fuserow);
		}
		else
		{
			fuse_state = get_pin_fuse_state(pal, jed, pin, fuserow);
		}

		if (fuse_state == LOW_FUSE_BLOWN)
		{
			if (haveterm)
			{
				strcat(buffer, " " AND_SYMBOL " ");
			}

			if (!is_output_pin(pin))
			{
				sprintf(tmpbuffer, LOW_SYMBOL INPUT_SYMBOL "%d", pin);
				strcat(buffer, tmpbuffer);
			}
			else
			{
				flags = get_pin_output_flags(pin);

				if (flags & OUTPUT_FEEDBACK_OUTPUT)
				{
					if (flags & OUTPUT_COMBINATORIAL)
					{
						sprintf(tmpbuffer, LOW_SYMBOL OUTPUT_SYMBOL "%d", pin);
					}
					else if (flags & OUTPUT_REGISTERED)
					{
						sprintf(tmpbuffer, LOW_SYMBOL REGISTERED_FEEDBACK_OUTPUT_SYMBOL "%d", pin);
					}
					else
					{
						tmpbuffer[0] = 0;

						fprintf(stderr, "Unknown output feedback controlled by output enable type for pin %d!\n", pin);
					}
				}
				else if (flags & OUTPUT_FEEDBACK_COMBINATORIAL)
				{
					sprintf(tmpbuffer, LOW_SYMBOL OUTPUT_FEEDBACK_SYMBOL "%d", pin);
				}
				else if (flags & OUTPUT_FEEDBACK_REGISTERED)
				{
					sprintf(tmpbuffer, LOW_SYMBOL REGISTERED_FEEDBACK_SYMBOL "%d", pin);
				}
				else
				{
					tmpbuffer[0] = 0;

					fprintf(stderr, "Unknown output feedback type for pin %d!\n", pin);
				}

				strcat(buffer, tmpbuffer);
			}

			haveterm = 1;
		}

		if (fuse_state == HIGH_FUSE_BLOWN)
		{
			if (haveterm)
			{
				strcat(buffer, " " AND_SYMBOL " ");
			}

			if (!is_output_pin(pin))
			{
				sprintf(tmpbuffer, INPUT_SYMBOL "%d", pin);
				strcat(buffer, tmpbuffer);
			}
			else
			{
				flags = get_pin_output_flags(pin);

				if (flags & OUTPUT_FEEDBACK_OUTPUT)
				{
					if (flags & OUTPUT_COMBINATORIAL)
					{
						sprintf(tmpbuffer, OUTPUT_SYMBOL "%d", pin);
					}
					else if (flags & OUTPUT_REGISTERED)
					{
						sprintf(tmpbuffer, REGISTERED_FEEDBACK_OUTPUT_SYMBOL "%d", pin);
					}
					else
					{
						tmpbuffer[0] = 0;

						fprintf(stderr, "Unknown output feedback controlled by output enable type for pin %d!\n", pin);
					}
				}
				else if (flags & OUTPUT_FEEDBACK_COMBINATORIAL)
				{
					sprintf(tmpbuffer, OUTPUT_FEEDBACK_SYMBOL "%d", pin);
				}
				else if (flags & OUTPUT_FEEDBACK_REGISTERED)
				{
					sprintf(tmpbuffer, REGISTERED_FEEDBACK_SYMBOL "%d", pin);
				}
				else
				{
					tmpbuffer[0] = 0;

					fprintf(stderr, "Unknown output feedback type for pin %d!\n", pin);
				}

				strcat(buffer, tmpbuffer);
			}

			haveterm = 1;
		}
	}
}



/*-------------------------------------------------
    print_input_pins - prints out the input pins
-------------------------------------------------*/

static void print_input_pins()
{
	UINT16 index;

	printf("Inputs:\n\n");

	for (index = 0; index < inputpinscount; ++index)
	{
		printf("%d", inputpins[index]);

		if (index + 1 < inputpinscount)
		{
			printf(", ");
		}
	}

	printf("\n\n");
}



/*-------------------------------------------------
    print_output_pins - prints out the output pins
-------------------------------------------------*/

static void print_output_pins()
{
	UINT16 index, flags;

	printf("Outputs:\n\n");

	for (index = 0; index < outputpinscount; ++index)
	{
		flags = outputpins[index].flags;

		printf("%d (", outputpins[index].pin);

		if (flags & OUTPUT_COMBINATORIAL)
		{
			printf("Combinatorial, ");
		}
		else if (flags & OUTPUT_REGISTERED)
		{
			printf("Registered, ");
		}
		else
		{
			fprintf(stderr, "Unknown output type for pin %d!\n", outputpins[index].pin);
		}

		if (flags & OUTPUT_FEEDBACK_OUTPUT)
		{
			printf("Output feedback output, ");
		}
		else if (flags & OUTPUT_FEEDBACK_COMBINATORIAL)
		{
			printf("Output feedback combinatorial, ");
		}
		else if (flags & OUTPUT_FEEDBACK_REGISTERED)
		{
			printf("Output feedback registered, ");
		}
		else if (flags & OUTPUT_FEEDBACK_NONE)
		{
			printf("No output feedback, ");
		}
		else
		{
			fprintf(stderr, "Unknown output feedback type for pin %d!\n", outputpins[index].pin);
		}

		if (flags & OUTPUT_ACTIVELOW)
		{
			printf("Active low");
		}
		else if (flags & OUTPUT_ACTIVEHIGH)
		{
			printf("Active high");
		}
		else
		{
			fprintf(stderr, "Unknown output state type for pin %d!\n", outputpins[index].pin);
		}

		printf(")\n");
	}

	printf("\n");
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

	print_input_pins();
	print_output_pins();

	printf("Equations:\n\n");

	for (index = 0; index < outputpinscount; ++index)
	{
		flags = outputpins[index].flags;

		indent = 0;

		if (flags & OUTPUT_ACTIVELOW)
		{
			printf(LOW_SYMBOL);

			indent += strlen(LOW_SYMBOL);
		}

		if (flags & OUTPUT_COMBINATORIAL)
		{
			sprintf(buffer, OUTPUT_SYMBOL "%d " COMBINATORIAL_ASSIGNMENT " ", outputpins[index].pin);
		}
		else if (flags & OUTPUT_REGISTERED)
		{
			sprintf(buffer, REGISTERED_FEEDBACK_SYMBOL "%d " REGISTERED_ASSIGNMENT " ", outputpins[index].pin);
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
					printf(" " OR_SYMBOL "\n");

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
			printf(OUTPUT_SYMBOL "%d.oe " COMBINATORIAL_ASSIGNMENT " ", outputpins[index].pin);

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
			printf(REGISTERED_FEEDBACK_SYMBOL "%d.oe " COMBINATORIAL_ASSIGNMENT " ", outputpins[index].pin);

			if (fuse_rows->fuserowoutputenable == NO_OUTPUT_ENABLE_FUSE_ROW)
			{
				printf("OE\n");
			}
			else if (all_fuses_in_row_blown(pal, jed, fuse_rows->fuserowoutputenable))
			{
				printf("vcc\n");
			}
			else
			{
				generate_product_terms(pal, jed, fuse_rows->fuserowoutputenable, buffer);

				printf("%s\n", buffer);
			}
		}

		printf("\n");
	}
}



/*-------------------------------------------------
    config_palce16v8_pin_as_7_product_terms_and_oe_term - configures
    the fuse rows of a PALCE16V8 pin with seven
    product terms and one output enable product term.
-------------------------------------------------*/

static void config_palce16v8_pin_as_7_product_terms_and_oe_term(UINT16 pin)
{
	static pin_fuse_rows pinfuserows[] = {
		{12, 1792, 1824, 2016},
		{13, 1536, 1568, 1760},
		{14, 1280, 1312, 1504},
		{15, 1024, 1056, 1248},
		{16, 768, 800, 992},
		{17, 512, 544, 736},
		{18, 256, 288, 480},
		{19, 0, 32, 224}};
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(pinfuserows); ++index)
	{
		if (pinfuserows[index].pin == pin)
		{
			palce16v8pinfuserows[index].fuserowoutputenable = pinfuserows[index].fuserowoutputenable;
			palce16v8pinfuserows[index].fuserowtermstart = pinfuserows[index].fuserowtermstart;
			palce16v8pinfuserows[index].fuserowtermend = pinfuserows[index].fuserowtermend;

			break;
		}
	}
}



/*-------------------------------------------------
    config_palce16v8_pin_as_8_product_terms - configures
    the fuse rows of a PALCE16V8 pin with eight
    product terms and no output enable product term.
-------------------------------------------------*/

static void config_palce16v8_pin_as_8_product_terms(UINT16 pin)
{
	static pin_fuse_rows pinfuserows[] = {
		{12, NO_OUTPUT_ENABLE_FUSE_ROW, 1792, 2016},
		{13, NO_OUTPUT_ENABLE_FUSE_ROW, 1536, 1760},
		{14, NO_OUTPUT_ENABLE_FUSE_ROW, 1280, 1504},
		{15, NO_OUTPUT_ENABLE_FUSE_ROW, 1024, 1248},
		{16, NO_OUTPUT_ENABLE_FUSE_ROW, 768, 992},
		{17, NO_OUTPUT_ENABLE_FUSE_ROW, 512, 736},
		{18, NO_OUTPUT_ENABLE_FUSE_ROW, 256, 480},
		{19, NO_OUTPUT_ENABLE_FUSE_ROW, 0, 224}};
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(pinfuserows); ++index)
	{
		if (pinfuserows[index].pin == pin)
		{
			palce16v8pinfuserows[index].fuserowoutputenable = pinfuserows[index].fuserowoutputenable;
			palce16v8pinfuserows[index].fuserowtermstart = pinfuserows[index].fuserowtermstart;
			palce16v8pinfuserows[index].fuserowtermend = pinfuserows[index].fuserowtermend;

			break;
		}
	}
}



/*-------------------------------------------------
    print_pal20xxx_product_terms - prints the product
    terms for a PAL20X4, PAL20X8 and PAL20X10
-------------------------------------------------*/

static void print_pal20xxx_product_terms(const pal_data* pal, const jed_data* jed)
{
	UINT16 index, columncount, flags, row, haveterms, tmpindex;
	char buffer[200];
	int indent, indentindex, rowhasterms[4];
	const pin_fuse_rows* fuse_rows;

	columncount = calc_fuse_column_count(pal);

	print_input_pins();
	print_output_pins();

	printf("Equations:\n\n");

	for (index = 0; index < outputpinscount; ++index)
	{
		flags = outputpins[index].flags;

		indent = 0;

		if (flags & OUTPUT_COMBINATORIAL)
		{
			sprintf(buffer, LOW_SYMBOL OUTPUT_SYMBOL "%d " COMBINATORIAL_ASSIGNMENT " ", outputpins[index].pin);

			printf("%s", buffer);

			haveterms = 0;
			indent += strlen(buffer);

			fuse_rows = find_fuse_rows(pal, outputpins[index].pin);

			for (row = fuse_rows->fuserowtermstart; row <= fuse_rows->fuserowtermend;
					row += columncount)
			{
				generate_product_terms(pal, jed, row, buffer);

				if (strlen(buffer) > 0)
				{
					if (haveterms)
					{
						printf(" ");
						printf(OR_SYMBOL);
						printf("\n");

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

			/* output enable equation */

			printf(OUTPUT_SYMBOL "%d.oe " COMBINATORIAL_ASSIGNMENT " ", outputpins[index].pin);

			if (all_fuses_in_row_blown(pal, jed, fuse_rows->fuserowoutputenable))
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
			sprintf(buffer, LOW_SYMBOL REGISTERED_FEEDBACK_SYMBOL "%d " REGISTERED_ASSIGNMENT " ", outputpins[index].pin);

			printf("%s", buffer);

			haveterms = 0;
			indent += strlen(buffer);

			fuse_rows = find_fuse_rows(pal, outputpins[index].pin);
			tmpindex = 0;

			memset(rowhasterms, 0, sizeof(rowhasterms));

			for (row = fuse_rows->fuserowtermstart; row <= fuse_rows->fuserowtermend;
					row += columncount)
			{
				generate_product_terms(pal, jed, row, buffer);

				if (strlen(buffer) > 0)
				{
					rowhasterms[tmpindex] = 1;

					if (haveterms)
					{
						if (tmpindex == 1)
						{
							printf(" " OR_SYMBOL "\n");
						}
						else if (tmpindex == 2)
						{
							printf(" " XOR_SYMBOL "\n");
						}
						else if (tmpindex == 3)
						{
							if (rowhasterms[2])
							{
								printf(" " OR_SYMBOL "\n");
							}
							else
							{
								printf(" " XOR_SYMBOL "\n");
							}
						}

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

				++tmpindex;
			}

			printf("\n");

			/* output enable equation */

			printf(REGISTERED_FEEDBACK_SYMBOL "%d.oe " COMBINATORIAL_ASSIGNMENT " OE\n", outputpins[index].pin);
		}
		else
		{
			fprintf(stderr, "Unknown output type for pin %d!\n", outputpins[index].pin);
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
    print_palce16v8_product_terms - prints the product
    terms for a PALCE16V8
-------------------------------------------------*/

static void print_palce16v8_product_terms(const pal_data* pal, const jed_data* jed)
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
    print_peel18cv8_product_terms - prints the product
    terms for a PEEL18CV8
-------------------------------------------------*/

static void print_peel18cv8_product_terms(const pal_data* pal, const jed_data* jed)
{
	char buffer[200];

	print_product_terms(pal, jed);

	/* Synchronous Preset */

	generate_product_terms(pal, jed, 2592, buffer);

	if (strlen(buffer))
	{
		printf("Synchronous Preset:\n\n");
		printf("%s\n", buffer);
		printf("\n");
	}

	/* Asynchronous Clear */

	generate_product_terms(pal, jed, 2628, buffer);

	if (strlen(buffer))
	{
		printf("Asynchronous Clear:\n\n");
		printf("%s\n", buffer);
		printf("\n");
	}
}



/*-------------------------------------------------
    print_gal18v10_product_terms - prints the product
    terms for a GAL18V10
-------------------------------------------------*/

static void print_gal18v10_product_terms(const pal_data* pal, const jed_data* jed)
{
	char buffer[200];

	print_product_terms(pal, jed);

	/* Synchronous Reset */

	generate_product_terms(pal, jed, 3420, buffer);

	if (strlen(buffer))
	{
		printf("Synchronous Reset:\n\n");
		printf("%s\n", buffer);
		printf("\n");
	}

	/* Asynchronous Reset */

	generate_product_terms(pal, jed, 0, buffer);

	if (strlen(buffer))
	{
		printf("Asynchronous Reset:\n\n");
		printf("%s\n", buffer);
		printf("\n");
	}
}



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
    print_pal20x4_product_terms - prints the product
    terms for a PAL20X4
-------------------------------------------------*/

static void print_pal20x4_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_pal20xxx_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal20x8_product_terms - prints the product
    terms for a PAL20X8
-------------------------------------------------*/

static void print_pal20x8_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_pal20xxx_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal20x10_product_terms - prints the product
    terms for a PAL20X10
-------------------------------------------------*/

static void print_pal20x10_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_pal20xxx_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_82s153_pls153_product_terms - prints the product
    terms for a 82S153/PLS153
-------------------------------------------------*/

static void print_82s153_pls153_product_terms(const pal_data* pal, const jed_data* jed)
{
	UINT16 index, columncount, flags, row, haveterms, or_column, fuserow;
	char buffer[200];
	int indent, indentindex;
	const pin_fuse_rows* fuse_rows;

	columncount = calc_fuse_column_count(pal);

	print_input_pins();
	print_output_pins();

	printf("Equations:\n\n");

	for (index = 0; index < outputpinscount; ++index)
	{
		flags = outputpins[index].flags;

		indent = 0;

		if (flags & OUTPUT_ACTIVELOW)
		{
			printf(LOW_SYMBOL);

			indent += strlen(LOW_SYMBOL);
		}

		sprintf(buffer, OUTPUT_SYMBOL "%d " COMBINATORIAL_ASSIGNMENT " ", outputpins[index].pin);

		printf("%s", buffer);

		haveterms = 0;
		indent += strlen(buffer);

		fuse_rows = find_fuse_rows(pal, outputpins[index].pin);
		fuserow = 0;

		if (outputpins[index].pin != 9)
		{
			or_column = 19 - outputpins[index].pin;
		}
		else
		{
			or_column = 9;
		}

		for (row = 0; row < 32; ++row)
		{
			if (!jed_get_fuse(jed, fuserow + columncount + or_column))
			{
				generate_product_terms(pal, jed, fuserow, buffer);

				if (strlen(buffer) > 0)
				{
					if (haveterms)
					{
						printf(" " OR_SYMBOL "\n");

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

			fuserow += (columncount + 10);
		}

		printf("\n");

		/* output enable equations */

		printf(OUTPUT_SYMBOL "%d.oe " COMBINATORIAL_ASSIGNMENT " ", outputpins[index].pin);

		if (all_fuses_in_row_blown(pal, jed, fuse_rows->fuserowoutputenable))
		{
			printf("vcc\n");
		}
		else
		{
			generate_product_terms(pal, jed, fuse_rows->fuserowoutputenable, buffer);

			printf("%s\n", buffer);
		}

		printf("\n");
	}
}



/*-------------------------------------------------
    print_ck2605_product_terms - prints the product
    terms for a CK2605
-------------------------------------------------*/

static void print_ck2605_product_terms(const pal_data* pal, const jed_data* jed)
{
	UINT16 index, columncount, flags, row, haveterms, or_column, fuserow;
	char buffer[200];
	int indent, indentindex;
	const pin_fuse_rows* fuse_rows;

	columncount = calc_fuse_column_count(pal);

	print_input_pins();
	print_output_pins();

	printf("Equations:\n\n");

	for (index = 0; index < outputpinscount; ++index)
	{
		flags = outputpins[index].flags;

		indent = 0;

		if (flags & OUTPUT_ACTIVELOW)
		{
			printf(LOW_SYMBOL);

			indent += strlen(LOW_SYMBOL);
		}

		sprintf(buffer, OUTPUT_SYMBOL "%d " COMBINATORIAL_ASSIGNMENT " ", outputpins[index].pin);

		printf("%s", buffer);

		haveterms = 0;
		indent += strlen(buffer);

		fuse_rows = find_fuse_rows(pal, outputpins[index].pin);
		fuserow = 0;

		if (outputpins[index].pin != 9)
		{
			or_column = 19 - outputpins[index].pin;
		}
		else
		{
			or_column = 9;
		}

		for (row = 0; row < 16; ++row)
		{
			if (!jed_get_fuse(jed, fuserow + columncount + or_column))
			{
				generate_product_terms(pal, jed, fuserow, buffer);

				if (strlen(buffer) > 0)
				{
					if (haveterms)
					{
						printf(" " OR_SYMBOL "\n");

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

			fuserow += (columncount + 10);
		}

		printf("\n");

		/* output enable equations */

		printf(OUTPUT_SYMBOL "%d.oe " COMBINATORIAL_ASSIGNMENT " ", outputpins[index].pin);

		if (all_fuses_in_row_blown(pal, jed, fuse_rows->fuserowoutputenable))
		{
			printf("vcc\n");
		}
		else
		{
			generate_product_terms(pal, jed, fuse_rows->fuserowoutputenable, buffer);

			printf("%s\n", buffer);
		}

		printf("\n");
	}
}



#if defined(ricoh_pals)
/*-------------------------------------------------
    print_epl10p8_product_terms - prints the product
    terms for a EPL10P8
-------------------------------------------------*/

static void print_epl10p8_product_terms(const pal_data* pal, const jed_data* jed)
{
	typedef struct _memory_cell memory_cell;
	struct _memory_cell
	{
		UINT16 pin;
		UINT16 or_fuse; /* 0 - intact? */
		UINT16 xor_fuse; /* 0 - intact? */
	};

	static memory_cell memory_cells[] = {
		{12, 661, 662},
		{13, 658, 659},
		{14, 655, 656},
		{15, 652, 653},
		{16, 649, 650},
		{17, 646, 647},
		{18, 643, 644},
		{19, 640, 641}};
	UINT16 index, columncount, flags, haveterms, fuserow;
	char buffer[200];
	int indent, row, indentindex;
	const pin_fuse_rows* fuse_rows;

	printf("Warning: This is experimental support!\n");

	columncount = calc_fuse_column_count(pal);

	print_input_pins();
	print_output_pins();

	printf("Equations:\n\n");

	for (index = 0; index < ARRAY_LENGTH(memory_cells); ++index)
	{
		flags = outputpins[index].flags;

		indent = 0;

		if (flags & OUTPUT_ACTIVELOW)
		{
			printf(LOW_SYMBOL);

			indent += strlen(LOW_SYMBOL);
		}

		sprintf(buffer, OUTPUT_SYMBOL "%d " COMBINATORIAL_ASSIGNMENT " ", outputpins[index].pin);

		printf("%s", buffer);

		haveterms = 0;
		indent += strlen(buffer);

		fuse_rows = find_fuse_rows(pal, outputpins[index].pin);

		if (!jed_get_fuse(jed, memory_cells[index].or_fuse) ||
			!jed_get_fuse(jed, memory_cells[index].xor_fuse))
		{
			/* MMI PAL pin compatible configuration */

			fuserow = fuse_rows->fuserowtermstart;

			for (row = 0; row < 2; ++row)
			{
				generate_product_terms(pal, jed, fuserow, buffer);

				if (strlen(buffer) > 0)
				{
					if (haveterms)
					{
						printf(" " OR_SYMBOL "\n");

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

				fuserow += columncount;
			}

			printf("\n");

			printf(OUTPUT_SYMBOL "%d.oe " COMBINATORIAL_ASSIGNMENT " vcc\n", outputpins[index].pin);

			printf("\n");
		}
		else if (!jed_get_fuse(jed, memory_cells[index].or_fuse) ||
					jed_get_fuse(jed, memory_cells[index].xor_fuse))
		{
			/* or configuration */

			fuserow = fuse_rows->fuserowtermstart;

			for (row = 0; row < 4; ++row)
			{
				generate_product_terms(pal, jed, fuserow, buffer);

				if (strlen(buffer) > 0)
				{
					if (haveterms)
					{
						printf(" " OR_SYMBOL "\n");

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

				fuse_rows += columncount;
			}

			printf("\n");

			printf(OUTPUT_SYMBOL "%d.oe " COMBINATORIAL_ASSIGNMENT " vcc\n", outputpins[index].pin);

			printf("\n");
		}
		else if (jed_get_fuse(jed, memory_cells[index].or_fuse) ||
					!jed_get_fuse(jed, memory_cells[index].xor_fuse))
		{
			/* xor configuration */
		}
		else
		{
			fprintf(stderr, "Unknown fuse configuration for pin %d!", memory_cells[index].pin);
		}
	}

	printf("Warning: This is experimental support!\n");
}



/*-------------------------------------------------
    print_epl12p6_product_terms - prints the product
    terms for a EPL12P6
-------------------------------------------------*/

static void print_epl12p6_product_terms(const pal_data* pal, const jed_data* jed)
{
	fprintf(stderr, "Printing product terms not supported for this device!\n");
}



/*-------------------------------------------------
    print_epl14p4_product_terms - prints the product
    terms for a EPL14P4
-------------------------------------------------*/

static void print_epl14p4_product_terms(const pal_data* pal, const jed_data* jed)
{
	fprintf(stderr, "Printing product terms not supported for this device!\n");
}



/*-------------------------------------------------
    print_epl16p2_product_terms - prints the product
    terms for a EPL16P2
-------------------------------------------------*/

static void print_epl16p2_product_terms(const pal_data* pal, const jed_data* jed)
{
	fprintf(stderr, "Printing product terms not supported for this device!\n");
}



/*-------------------------------------------------
    print_epl16p8_product_terms - prints the product
    terms for a EPL16P8
-------------------------------------------------*/

static void print_epl16p8_product_terms(const pal_data* pal, const jed_data* jed)
{
	fprintf(stderr, "Printing product terms not supported for this device!\n");
}



/*-------------------------------------------------
    print_epl16rp8_product_terms - prints the product
    terms for a EPL16RP8
-------------------------------------------------*/

static void print_epl16rp8_product_terms(const pal_data* pal, const jed_data* jed)
{
	fprintf(stderr, "Printing product terms not supported for this device!\n");
}



/*-------------------------------------------------
    print_epl16rp6_product_terms - prints the product
    terms for a EPL16RP6
-------------------------------------------------*/

static void print_epl16rp6_product_terms(const pal_data* pal, const jed_data* jed)
{
	fprintf(stderr, "Printing product terms not supported for this device!\n");
}



/*-------------------------------------------------
    print_epl16rp4_product_terms - prints the product
    terms for a EPL16RP4
-------------------------------------------------*/

static void print_epl16rp4_product_terms(const pal_data* pal, const jed_data* jed)
{
	fprintf(stderr, "Printing product terms not supported for this device!\n");
}
#endif



/*-------------------------------------------------
    print_pal10p8_product_terms - prints the product
    terms for a PAL10P8
-------------------------------------------------*/

static void print_pal10p8_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_epl12p6_product_terms - prints the product
    terms for a PAL12P6
-------------------------------------------------*/

static void print_pal12p6_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_epl14p4_product_terms - prints the product
    terms for a PAL14P4
-------------------------------------------------*/

static void print_pal14p4_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_epl16p2_product_terms - prints the product
    terms for a PAL16P2
-------------------------------------------------*/

static void print_pal16p2_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16p8_product_terms - prints the product
    terms for a PAL16P8
-------------------------------------------------*/

static void print_pal16p8_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16rp4_product_terms - prints the product
    terms for a PAL16RP4
-------------------------------------------------*/

static void print_pal16rp4_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16rp6_product_terms - prints the product
    terms for a PAL16RP6
-------------------------------------------------*/

static void print_pal16rp6_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16rp8_product_terms - prints the product
    terms for a PAL16RP8
-------------------------------------------------*/

static void print_pal16rp8_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal6l16_product_terms - prints the product
    terms for a PAL6L16
-------------------------------------------------*/

static void print_pal6l16_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal8l14_product_terms - prints the product
    terms for a PAL8L14
-------------------------------------------------*/

static void print_pal8l14_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal12h10_product_terms - prints the product
    terms for a PAL12H10
-------------------------------------------------*/

static void print_pal12h10_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal12l10_product_terms - prints the product
    terms for a PAL12L10
-------------------------------------------------*/

static void print_pal12l10_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal14h8_product_terms - prints the product
    terms for a PAL14H8
-------------------------------------------------*/

static void print_pal14h8_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal14l8_product_terms - prints the product
    terms for a PAL14L8
-------------------------------------------------*/

static void print_pal14l8_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16h6_product_terms - prints the product
    terms for a PAL16H6
-------------------------------------------------*/

static void print_pal16h6_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal16l6_product_terms - prints the product
    terms for a PAL16L6
-------------------------------------------------*/

static void print_pal16l6_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal18h4_product_terms - prints the product
    terms for a PAL18H4
-------------------------------------------------*/

static void print_pal18h4_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal18l4_product_terms - prints the product
    terms for a PAL18L4
-------------------------------------------------*/

static void print_pal18l4_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal20lc1_product_terms - prints the product
    terms for a PAL20LC1
-------------------------------------------------*/

static void print_pal20c1_product_terms(const pal_data* pal, const jed_data* jed)
{
	print_product_terms(pal, jed);
}



/*-------------------------------------------------
    print_pal20l2_product_terms - prints the product
    terms for a PAL20L2
-------------------------------------------------*/

static void print_pal20l2_product_terms(const pal_data* pal, const jed_data* jed)
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
		{12, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{13, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{14, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal10h8_pins - configures the pins for
    a PAL10H8
-------------------------------------------------*/

static void config_pal10h8_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
	static pin_output_config output_pins[] = {
		{12, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{13, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{14, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{15, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal12l6_pins - configures the pins for
    a PAL12L6
-------------------------------------------------*/

static void config_pal12l6_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 19};
	static pin_output_config output_pins[] = {
		{13, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{14, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal12h6_pins - configures the pins for
    a PAL12H6
-------------------------------------------------*/

static void config_pal12h6_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 19};
	static pin_output_config output_pins[] = {
		{13, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{14, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{15, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal14l4_pins - configures the pins for
    a PAL14L4
-------------------------------------------------*/

static void config_pal14l4_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 18, 19};
	static pin_output_config output_pins[] = {
		{14, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal14h4_pins - configures the pins for
    a PAL14H4
-------------------------------------------------*/

static void config_pal14h4_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 18, 19};
	static pin_output_config output_pins[] = {
		{14, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{15, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal16l2_pins - configures the pins for
    a PAL16L2
-------------------------------------------------*/

static void config_pal16l2_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 17, 18, 19};
	static pin_output_config output_pins[] = {
		{15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal16h2_pins - configures the pins for
    a PAL16H2
-------------------------------------------------*/

static void config_pal16h2_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 17, 18, 19};
	static pin_output_config output_pins[] = {
		{15, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal16c1_pins - configures the pins for
    a PAL16C1
-------------------------------------------------*/

static void config_pal16c1_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 17, 18, 19};
	static pin_output_config output_pins[] = {
		{15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
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
		if (does_output_enable_fuse_row_allow_output(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
		{
			output_pins[output_pin_count].pin = pal->pinfuserows[index].pin;
			output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

			++output_pin_count;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
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

	if (does_output_enable_fuse_row_allow_output(pal, jed, 1792))
	{
		output_pins[output_pin_count].pin = 12;
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		++output_pin_count;
	}

	if (does_output_enable_fuse_row_allow_output(pal, jed, 1536))
	{
		output_pins[output_pin_count].pin = 13;
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		++output_pin_count;
	}

	for (index = 0; index < ARRAY_LENGTH(registered_pins); ++index)
	{
		output_pins[output_pin_count].pin = registered_pins[index];
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED;

		++output_pin_count;
	}

	if (does_output_enable_fuse_row_allow_output(pal, jed, 256))
	{
		output_pins[output_pin_count].pin = 18;
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		++output_pin_count;
	}

	if (does_output_enable_fuse_row_allow_output(pal, jed, 0))
	{
		output_pins[output_pin_count].pin = 19;
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		++output_pin_count;
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
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

	if (does_output_enable_fuse_row_allow_output(pal, jed, 1792))
	{
		output_pins[output_pin_count].pin = 12;
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		++output_pin_count;
	}

	for (index = 0; index < ARRAY_LENGTH(registered_pins); ++index)
	{
		output_pins[output_pin_count].pin = registered_pins[index];
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED;

		++output_pin_count;
	}

	if (does_output_enable_fuse_row_allow_output(pal, jed, 0))
	{
		output_pins[output_pin_count].pin = 19;
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		++output_pin_count;
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
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
		{12, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{13, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{14, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{15, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{16, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{17, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{18, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{19, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_palce16v8_pins - configures the pins for
    a PALCE16V8
-------------------------------------------------*/

static void config_palce16v8_pins(const pal_data* pal, const jed_data* jed)
{
	typedef struct _output_logic_macrocell output_logic_macrocell;
	struct _output_logic_macrocell
	{
		UINT16 pin;
		UINT16 sl0_fuse; /* registers allowed (0 - registered, 1 - not registered) */
		UINT16 sl1_fuse; /* output polarity (0 - low, 1 - high) */
		UINT16 fuserowoutputenable;
	};

	static output_logic_macrocell macrocells[] = {
		{12, 2127, 2055, 1792},
		{13, 2126, 2054, 1536},
		{14, 2125, 2053, 1280},
		{15, 2124, 2052, 1024},
		{16, 2123, 2051, 768},
		{17, 2122, 2050, 512},
		{18, 2121, 2049, 256},
		{19, 2120, 2048, 0}};
	static pin_fuse_columns pinfusecolumns_i_or_o[] = {
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
	static pin_fuse_columns pinfusecolumns_io[] = {
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
	static pin_fuse_columns pinfusecolumns_regs[] = {
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
		{14, 23, 22},
		{15, 19, 18},
		{16, 15, 14},
		{17, 11, 10},
		{18, 7, 6},
		{19, 3, 2}};
	static UINT16 input_pins_i_or_o[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
	static UINT16 input_pins_io[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
	static UINT16 input_pins_regs[] = {2, 3, 4, 5, 6, 7, 8, 9};
	static UINT16 sg0 = 2192;
	static UINT16 sg1 = 2193;
	UINT16 input_pins[18];
	pin_output_config output_pins[ARRAY_LENGTH(macrocells)];
	UINT16 index, input_pin_count, output_pin_count;

	input_pin_count = 0;
	output_pin_count = 0;

	if (!jed_get_fuse(jed, sg0))
	{
		/* Device uses registers */

		if (jed_get_fuse(jed, sg1))
		{
			memcpy(palce16v8pinfusecolumns, pinfusecolumns_regs, sizeof(pinfusecolumns_regs));

			memcpy(input_pins, input_pins_regs, sizeof(input_pins_regs));

			input_pin_count = ARRAY_LENGTH(input_pins_regs);

			for (index = 0; index < ARRAY_LENGTH(macrocells); ++index)
			{
				if (!jed_get_fuse(jed, macrocells[index].sl0_fuse))
				{
					/* Registered output */

					config_palce16v8_pin_as_8_product_terms(macrocells[index].pin);

					output_pins[output_pin_count].pin = macrocells[index].pin;
					output_pins[output_pin_count].flags = OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED;

					if (!jed_get_fuse(jed, macrocells[index].sl1_fuse))
					{
						output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
					}
					else
					{
						output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
					}

					++output_pin_count;

					input_pins[input_pin_count] = macrocells[index].pin;

					++input_pin_count;
				}
				else
				{
					/* Combinatorial I/O */

					if (does_output_enable_fuse_row_allow_output(pal, jed, macrocells[index].fuserowoutputenable))
					{
						config_palce16v8_pin_as_7_product_terms_and_oe_term(macrocells[index].pin);

						output_pins[output_pin_count].pin = macrocells[index].pin;
						output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

						if (!jed_get_fuse(jed, macrocells[index].sl1_fuse))
						{
							output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
						}
						else
						{
							output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
						}

						++output_pin_count;
					}

					input_pins[input_pin_count] = macrocells[index].pin;

					++input_pin_count;
				}
			}
		}
		else
		{
			fprintf(stderr, "Unknown configuration type!\n");
		}
	}
	else
	{
		/* Device uses no registers */

		if (jed_get_fuse(jed, sg1))
		{
			/* Combinatorial I/O (7 product terms and 1 output enable product term) */

			memcpy(palce16v8pinfusecolumns, pinfusecolumns_io, sizeof(pinfusecolumns_io));

			memcpy(input_pins, input_pins_io, sizeof(input_pins_io));

			input_pin_count = ARRAY_LENGTH(input_pins_io);

			for (index = 0; index < ARRAY_LENGTH(macrocells); ++index)
			{
				if (does_output_enable_fuse_row_allow_output(pal, jed, macrocells[index].fuserowoutputenable))
				{
					config_palce16v8_pin_as_7_product_terms_and_oe_term(macrocells[index].pin);

					output_pins[output_pin_count].pin = macrocells[index].pin;
					output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

					if (!jed_get_fuse(jed, macrocells[index].sl1_fuse))
					{
						output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
					}
					else
					{
						output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
					}

					++output_pin_count;
				}

				/* Pins 12 and 19 cannot be used as an input only an output. */

				if (macrocells[index].pin != 12 && macrocells[index].pin != 19)
				{
					input_pins[input_pin_count] = macrocells[index].pin;

					++input_pin_count;
				}
			}
		}
		else
		{
			/* Combinatorial Output or Input */

			memcpy(palce16v8pinfusecolumns, pinfusecolumns_i_or_o, sizeof(pinfusecolumns_i_or_o));

			memcpy(input_pins, input_pins_i_or_o, sizeof(input_pins_i_or_o));

			input_pin_count = ARRAY_LENGTH(input_pins_i_or_o);

			for (index = 0; index < ARRAY_LENGTH(macrocells); ++index)
			{
				if (!jed_get_fuse(jed, macrocells[index].sl0_fuse))
				{
					/* pin configured as an output only */

					config_palce16v8_pin_as_8_product_terms(macrocells[index].pin);

					output_pins[output_pin_count].pin = macrocells[index].pin;
					output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE;

					if (!jed_get_fuse(jed, macrocells[index].sl1_fuse))
					{
						output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
					}
					else
					{
						output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
					}

					++output_pin_count;
				}
				else
				{
					/* pin configured as an input only */

					input_pins[input_pin_count] = macrocells[index].pin;

					++input_pin_count;
				}
			}
		}
	}

	set_input_pins(input_pins, input_pin_count);
	set_output_pins(output_pins, output_pin_count);

	/* 2056 - 2119 are the 64 bit signature fuses */

	/* 2128 - 2135 product term 8? */
	/* 2136 - 2143 product term 7? */
	/* 2144 - 2151 product term 6? */
	/* 2152 - 2159 product term 5? */
	/* 2160 - 2167 product term 4? */
	/* 2168 - 2175 product term 3? */
	/* 2176 - 2183 product term 2? */
	/* 2184 - 2191 product term 1? */
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
	pin_output_config output_pins[ARRAY_LENGTH(macrocells)];
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

			set_input_pins(input_pins_combinatorialcomplex, ARRAY_LENGTH(input_pins_combinatorialcomplex));

			memcpy(gal16v8pinfuserows, pinfuserows_combinatorial, sizeof(pinfuserows_combinatorial));
			memcpy(gal16v8pinfusecolumns, pinfusecolumns_combinatorialcomplex, sizeof(pinfusecolumns_combinatorialcomplex));

			for (index = 0; index < ARRAY_LENGTH(macrocells); ++index)
			{
				if (is_gal16v8_product_term_enabled(pal, jed, pal->pinfuserows[index].fuserowoutputenable) &&
					does_output_enable_fuse_row_allow_output(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
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

					if (output_pins[output_pin_count].pin != 12 &&
						output_pins[output_pin_count].pin != 19)
					{
						output_pins[output_pin_count].flags |= OUTPUT_FEEDBACK_OUTPUT;
					}
					else
					{
						output_pins[output_pin_count].flags |= OUTPUT_FEEDBACK_NONE;
					}

					++output_pin_count;
				}
			}
		}
		else
		{
			/* Simple Mode */

			set_input_pins(input_pins_combinatorialsimple, ARRAY_LENGTH(input_pins_combinatorialsimple));

			memcpy(gal16v8pinfuserows, pinfuserows_registered, sizeof(pinfuserows_registered));
			memcpy(gal16v8pinfusecolumns, pinfusecolumns_combinatorialsimple, sizeof(pinfusecolumns_combinatorialsimple));

			for (index = 0; index < ARRAY_LENGTH(macrocells); ++index)
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

					if (output_pins[output_pin_count].pin != 15 &&
						output_pins[output_pin_count].pin != 16)
					{
						output_pins[output_pin_count].flags |= OUTPUT_FEEDBACK_OUTPUT;
					}
					else
					{
						output_pins[output_pin_count].flags |= OUTPUT_FEEDBACK_NONE;
					}

					++output_pin_count;
				}
			}
		}
	}
	else
	{
		/* Registered */

		set_input_pins(input_pins_registered, ARRAY_LENGTH(input_pins_registered));

		memcpy(gal16v8pinfusecolumns, pinfusecolumns_registered, sizeof(pinfusecolumns_registered));

		for (index = 0; index < ARRAY_LENGTH(macrocells); ++index)
		{
			if (jed_get_fuse(jed, macrocells[index].ac1_fuse))
			{
				/* combinatorial pin */

				gal16v8pinfuserows[index].fuserowoutputenable = pinfuserows_combinatorial[index].fuserowoutputenable;
				gal16v8pinfuserows[index].fuserowtermstart = pinfuserows_combinatorial[index].fuserowtermstart;
				gal16v8pinfuserows[index].fuserowtermend = pinfuserows_combinatorial[index].fuserowtermend;

				if (is_gal16v8_product_term_enabled(pal, jed, pal->pinfuserows[index].fuserowoutputenable) &&
					does_output_enable_fuse_row_allow_output(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
				{
					output_pins[output_pin_count].pin = macrocells[index].pin;
					output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

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
				output_pins[output_pin_count].flags = OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED;

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
    config_peel18cv8_pins - configures the pins
    for a PEEL18CV8
-------------------------------------------------*/

static void config_peel18cv8_pins(const pal_data* pal, const jed_data* jed)
{
	typedef struct _output_logic_macrocell output_logic_macrocell;
	struct _output_logic_macrocell
	{
		UINT16 pin;
		UINT16 polarity_fuse; /* 0 = active high or 1 = active low */
		UINT16 type_fuse; /* 1 = registered or 0 = combinatorial */
		UINT16 feedback1_fuse;
		UINT16 feedback2_fuse;
	};

	static output_logic_macrocell macrocells[] = {
		{12, 2692, 2693, 2694, 2695},
		{13, 2688, 2689, 2690, 2691},
		{14, 2684, 2685, 2686, 2687},
		{15, 2680, 2681, 2682, 2683},
		{16, 2676, 2677, 2678, 2679},
		{17, 2672, 2673, 2674, 2675},
		{18, 2668, 2669, 2670, 2671},
		{19, 2664, 2665, 2666, 2667}};
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19};
	pin_output_config output_pins[ARRAY_LENGTH(macrocells)];
	UINT16 index, output_pin_count;

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));

	output_pin_count = 0;

	for (index = 0; index < ARRAY_LENGTH(macrocells); ++index)
	{
		if (jed_get_fuse(jed, macrocells[index].feedback1_fuse) &&
			!jed_get_fuse(jed, macrocells[index].feedback2_fuse))
		{
			/* Combinatorial Feedback (pin is output only) */

			output_pins[output_pin_count].pin = macrocells[index].pin;
			output_pins[output_pin_count].flags = OUTPUT_FEEDBACK_COMBINATORIAL;

			if (jed_get_fuse(jed, macrocells[index].type_fuse))
			{
				output_pins[output_pin_count].flags |= OUTPUT_REGISTERED;
			}
			else
			{
				output_pins[output_pin_count].flags |= OUTPUT_COMBINATORIAL;
			}

			if (jed_get_fuse(jed, macrocells[index].polarity_fuse))
			{
				output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
			}
			else
			{
				output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
			}

			++output_pin_count;
		}
		else if (!jed_get_fuse(jed, macrocells[index].feedback1_fuse) &&
					!jed_get_fuse(jed, macrocells[index].feedback2_fuse))
		{
			/* Register Feedback (pin is output only) */

			output_pins[output_pin_count].pin = macrocells[index].pin;
			output_pins[output_pin_count].flags = OUTPUT_FEEDBACK_REGISTERED;

			if (jed_get_fuse(jed, macrocells[index].type_fuse))
			{
				output_pins[output_pin_count].flags |= OUTPUT_REGISTERED;
			}
			else
			{
				output_pins[output_pin_count].flags |= OUTPUT_COMBINATORIAL;
			}

			if (jed_get_fuse(jed, macrocells[index].polarity_fuse))
			{
				output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
			}
			else
			{
				output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
			}

			++output_pin_count;
		}
		else if (jed_get_fuse(jed, macrocells[index].feedback1_fuse) &&
					jed_get_fuse(jed, macrocells[index].feedback2_fuse))
		{
			/* Bi-directional I/O (pin can be input or output) */

			if (does_output_enable_fuse_row_allow_output(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
			{
				output_pins[output_pin_count].pin = macrocells[index].pin;
				output_pins[output_pin_count].flags = OUTPUT_FEEDBACK_OUTPUT;

				if (jed_get_fuse(jed, macrocells[index].type_fuse))
				{
					output_pins[output_pin_count].flags |= OUTPUT_REGISTERED;
				}
				else
				{
					output_pins[output_pin_count].flags |= OUTPUT_COMBINATORIAL;
				}

				if (jed_get_fuse(jed, macrocells[index].polarity_fuse))
				{
					output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
				}
				else
				{
					output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
				}

				++output_pin_count;
			}
		}
		else if (!jed_get_fuse(jed, macrocells[index].feedback1_fuse) &&
					jed_get_fuse(jed, macrocells[index].feedback2_fuse))
		{
			fprintf(stderr, "Unknown input/feedback select configuration.  (Pin %d)\n",
					macrocells[index].pin);

			continue;
		}
	}

	set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_gal18v10_pins - configures the pins
    for a GAL18V10
-------------------------------------------------*/

static void config_gal18v10_pins(const pal_data* pal, const jed_data* jed)
{
	typedef struct _output_logic_macrocell output_logic_macrocell;
	struct _output_logic_macrocell
	{
		UINT16 pin;
		UINT16 s0_fuse; /* 0 - active low, 1 - active high */
		UINT16 s1_fuse; /* 0 - registered, 1 - combinatorial */
	};

	static output_logic_macrocell macrocells[] = {
		{9, 3474, 3475},
		{11, 3472, 3473},
		{12, 3470, 3471},
		{13, 3468, 3469},
		{14, 3466, 3467},
		{15, 3464, 3465},
		{16, 3462, 3463},
		{17, 3460, 3461},
		{18, 3458, 3459},
		{19, 3456, 3457}};
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19};
	pin_output_config output_pins[ARRAY_LENGTH(macrocells)];
	UINT16 index, output_pin_count;

	output_pin_count = 0;

	for (index = 0; index < ARRAY_LENGTH(output_pins); ++index)
	{
		if (jed_get_fuse(jed, macrocells[index].s1_fuse))
		{
			/* Combinatorial output or dedicated input */

			if (does_output_enable_fuse_row_allow_output(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
			{
				output_pins[output_pin_count].pin = macrocells[index].pin;
				output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

				if (!jed_get_fuse(jed, macrocells[index].s0_fuse))
				{
					output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
				}
				else
				{
					output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
				}

				++output_pin_count;
			}
		}
		else
		{
			/* Registered output */

			output_pins[output_pin_count].pin = macrocells[index].pin;
			output_pins[output_pin_count].flags = OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED;

			if (!jed_get_fuse(jed, macrocells[index].s0_fuse))
			{
				output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
			}
			else
			{
				output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
			}

			++output_pin_count;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, output_pin_count);
}



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
		if (does_output_enable_fuse_row_allow_output(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
		{
			output_pins[output_pin_count].pin = pal->pinfuserows[index].pin;
			output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

			if (pal->pinfuserows[index].pin != 15 &&
				pal->pinfuserows[index].pin != 22)
			{
				output_pins[output_pin_count].flags |= OUTPUT_FEEDBACK_OUTPUT;
			}
			else
			{
				output_pins[output_pin_count].flags |= OUTPUT_FEEDBACK_NONE;
			}

			++output_pin_count;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
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
		if (does_output_enable_fuse_row_allow_output(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
		{
			output_pins[output_pin_count].pin = pal->pinfuserows[index].pin;
			output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL;

			if (pal->pinfuserows[index].pin != 23 &&
				pal->pinfuserows[index].pin != 14)
			{
				output_pins[output_pin_count].flags |= OUTPUT_FEEDBACK_OUTPUT;
			}
			else
			{
				output_pins[output_pin_count].flags |= OUTPUT_FEEDBACK_NONE;
			}

			++output_pin_count;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
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

	if (does_output_enable_fuse_row_allow_output(pal, jed, 2240))
	{
		output_pins[output_pin_count].pin = 15;
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		++output_pin_count;
	}

	if (does_output_enable_fuse_row_allow_output(pal, jed, 1920))
	{
		output_pins[output_pin_count].pin = 16;
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		++output_pin_count;
	}

	for (index = 0; index < ARRAY_LENGTH(registered_pins); ++index)
	{
		output_pins[output_pin_count].pin = registered_pins[index];
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED;

		++output_pin_count;
	}

	if (does_output_enable_fuse_row_allow_output(pal, jed, 320))
	{
		output_pins[output_pin_count].pin = 21;
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		++output_pin_count;
	}

	if (does_output_enable_fuse_row_allow_output(pal, jed, 0))
	{
		output_pins[output_pin_count].pin = 22;
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		++output_pin_count;
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
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

	if (does_output_enable_fuse_row_allow_output(pal, jed, 2240))
	{
		output_pins[output_pin_count].pin = 15;
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		++output_pin_count;
	}

	for (index = 0; index < ARRAY_LENGTH(registered_pins); ++index)
	{
		output_pins[output_pin_count].pin = registered_pins[index];
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED;

		++output_pin_count;
	}

	if (does_output_enable_fuse_row_allow_output(pal, jed, 0))
	{
		output_pins[output_pin_count].pin = 22;
		output_pins[output_pin_count].flags = OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		++output_pin_count;
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
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
		{15, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{16, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{17, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{18, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{19, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{20, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{21, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{22, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal20x4_pins - configures the pins for
    a PAL20X4
-------------------------------------------------*/

static void config_pal20x4_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
	static pin_output_config output_pins[] = {
		{14, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT},
		{15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT},
		{16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT},
		{17, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{18, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{19, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{20, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{21, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT},
		{22, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT},
		{23, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}

/*-------------------------------------------------
    config_pal20x10_pins - configures the pins for
    a PAL20X8
-------------------------------------------------*/

static void config_pal20x8_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
	static pin_output_config output_pins[] = {
		{14, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT},
		{15, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{16, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{17, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{18, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{19, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{20, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{21, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{22, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{23, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal20x10_pins - configures the pins for
    a PAL20X10
-------------------------------------------------*/

static void config_pal20x10_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
	static pin_output_config output_pins[] = {
		{14, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{15, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{16, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{17, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{18, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{19, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{20, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{21, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{22, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{23, OUTPUT_ACTIVELOW | OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_82s153_pls153_pins - configures the pins for
    a 82S153/PLS153
-------------------------------------------------*/

static void config_82s153_pls153_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19};
	pin_output_config output_pins[10];
	UINT16 output_pin_count, index;

	output_pin_count = 0;

	for (index = 0; index < pal->pinfuserowscount; ++index)
	{
		if (does_output_enable_fuse_row_allow_output(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
		{
			output_pins[output_pin_count].pin = pal->pinfuserows[index].pin;
			output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

			if (jed_get_fuse(jed, 1832 + (9 - index)))
			{
				output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
			}
			else
			{
				output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
			}

			++output_pin_count;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_ck2605_pins - configures the pins for
    a CK2605
-------------------------------------------------*/

static void config_ck2605_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19};
	pin_output_config output_pins[10];
	UINT16 output_pin_count, index;

	output_pin_count = 0;

	for (index = 0; index < pal->pinfuserowscount; ++index)
	{
		if (does_output_enable_fuse_row_allow_output(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
		{
			output_pins[output_pin_count].pin = pal->pinfuserows[index].pin;
			output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

			if (jed_get_fuse(jed, 1096 + (9 - index)))
			{
				output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
			}
			else
			{
				output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
			}

			++output_pin_count;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, output_pin_count);
}



#if defined(ricoh_pals)
/*-------------------------------------------------
    config_epl10p8_pins - configures the pins for
    a EPL10P8
-------------------------------------------------*/

static void config_epl10p8_pins(const pal_data* pal, const jed_data* jed)
{
	typedef struct _memory_cell memory_cell;
	struct _memory_cell
	{
		UINT16 pin;
		UINT16 polarity_fuse; /* 0 - active low?, 1 - active high? */
	};

	static memory_cell memory_cells[] = {
		{12, 663},
		{13, 660},
		{14, 657},
		{15, 654},
		{16, 651},
		{17, 648},
		{18, 645},
		{19, 642}};
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
	pin_output_config output_pins[8];
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(memory_cells); ++index)
	{
		output_pins[index].pin = memory_cells[index].pin;
		output_pins[index].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE;

		if (!jed_get_fuse(jed, memory_cells[index].polarity_fuse))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_epl12p6_pins - configures the pins for
    a EPL12P6
-------------------------------------------------*/

static void config_epl12p6_pins(const pal_data* pal, const jed_data* jed)
{
	typedef struct _memory_cell memory_cell;
	struct _memory_cell
	{
		UINT16 pin;
		UINT16 polarity_fuse; /* 0 - active low?, 1 - active high? */
		UINT16 or_fuse; /* 0 - intact? */
		UINT16 xor_fuse; /* 0 - intact? */
	};

	static memory_cell memory_cells[] = {
		{13, 785, 783, 784},
		{14, 782, 780, 781},
		{15, 779, 777, 778},
		{16, 776, 774, 775},
		{17, 773, 771, 772},
		{18, 770, 768, 769}};
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 19};
	pin_output_config output_pins[8];
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(memory_cells); ++index)
	{
		output_pins[index].pin = memory_cells[index].pin;
		output_pins[index].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE;

		if (!jed_get_fuse(jed, memory_cells[index].polarity_fuse))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_epl14p4_pins - configures the pins for
    a EPL14P4
-------------------------------------------------*/

static void config_epl14p4_pins(const pal_data* pal, const jed_data* jed)
{
	typedef struct _memory_cell memory_cell;
	struct _memory_cell
	{
		UINT16 pin;
		UINT16 polarity_fuse; /* 0 - active low?, 1 - active high? */
		UINT16 or_fuse; /* 0 - intact? */
		UINT16 xor_fuse; /* 0 - intact? */
	};

	static memory_cell memory_cells[] = {
		{14, 907, 905, 906},
		{15, 904, 902, 903},
		{16, 901, 899, 900},
		{17, 898, 896, 897}};
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 18, 19};
	pin_output_config output_pins[8];
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(memory_cells); ++index)
	{
		output_pins[index].pin = memory_cells[index].pin;
		output_pins[index].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE;

		if (!jed_get_fuse(jed, memory_cells[index].polarity_fuse))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_epl16p2_pins - configures the pins for
    a EPL16P2
-------------------------------------------------*/

static void config_epl16p2_pins(const pal_data* pal, const jed_data* jed)
{
	typedef struct _memory_cell memory_cell;
	struct _memory_cell
	{
		UINT16 pin;
		UINT16 polarity_fuse; /* 0 - active low?, 1 - active high? */
		UINT16 or_fuse; /* 0 - intact? */
		UINT16 xor_fuse; /* 0 - intact? */
	};

	static memory_cell memory_cells[] = {
		{15, 1029, 1027, 1028},
		{16, 1026, 1024, 1025}};
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 17, 18, 19};
	pin_output_config output_pins[8];
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(memory_cells); ++index)
	{
		output_pins[index].pin = memory_cells[index].pin;
		output_pins[index].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE;

		if (!jed_get_fuse(jed, memory_cells[index].polarity_fuse))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_epl16p8_pins - configures the pins for
    a EPL16P8
-------------------------------------------------*/

static void config_epl16p8_pins(const pal_data* pal, const jed_data* jed)
{
	typedef struct _memory_cell memory_cell;
	struct _memory_cell
	{
		UINT16 pin;
		UINT16 polarity_fuse; /* 0 - active low?, 1 - active high? */
		UINT16 or_fuse; /* 0 - intact? */
		UINT16 xor_fuse; /* 0 - intact? */
	};

	static memory_cell memory_cells[] = {
		{12, 2071, 2069, 2070},
		{13, 2068, 2066, 2067},
		{14, 2065, 2063, 2064},
		{15, 2062, 2060, 2061},
		{16, 2059, 2057, 2058},
		{17, 2056, 2054, 2055},
		{18, 2053, 2051, 2052},
		{19, 2050, 2048, 2049}};
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
	pin_output_config output_pins[8];
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(memory_cells); ++index)
	{
		output_pins[index].pin = memory_cells[index].pin;
		output_pins[index].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE;

		if (!jed_get_fuse(jed, memory_cells[index].polarity_fuse))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_epl16rp8_pins - configures the pins for
    a EPL16RP8
-------------------------------------------------*/

static void config_epl16rp8_pins(const pal_data* pal, const jed_data* jed)
{
	typedef struct _memory_cell memory_cell;
	struct _memory_cell
	{
		UINT16 pin;
		UINT16 polarity_fuse; /* 0 - active low?, 1 - active high? */
		UINT16 or_fuse; /* 0 - intact? */
		UINT16 xor_fuse; /* 0 - intact? */
	};

	static memory_cell memory_cells[] = {
		{12, 2071, 2069, 2070},
		{13, 2068, 2066, 2067},
		{14, 2065, 2063, 2064},
		{15, 2062, 2060, 2061},
		{16, 2059, 2057, 2058},
		{17, 2056, 2054, 2055},
		{18, 2053, 2051, 2052},
		{19, 2050, 2048, 2049}};
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
	pin_output_config output_pins[8];
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(memory_cells); ++index)
	{
		output_pins[index].pin = memory_cells[index].pin;
		output_pins[index].flags = OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED;

		if (!jed_get_fuse(jed, memory_cells[index].polarity_fuse))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_epl16rp6_pins - configures the pins for
    a EPL16RP6
-------------------------------------------------*/

static void config_epl16rp6_pins(const pal_data* pal, const jed_data* jed)
{
	typedef struct _memory_cell memory_cell;
	struct _memory_cell
	{
		UINT16 pin;
		UINT16 polarity_fuse; /* 0 - active low?, 1 - active high? */
		UINT16 or_fuse; /* 0 - intact? */
		UINT16 xor_fuse; /* 0 - intact? */
	};

	static memory_cell memory_cells[] = {
		{12, 2071, 2069, 2070},
		{13, 2068, 2066, 2067},
		{14, 2065, 2063, 2064},
		{15, 2062, 2060, 2061},
		{16, 2059, 2057, 2058},
		{17, 2056, 2054, 2055},
		{18, 2053, 2051, 2052},
		{19, 2050, 2048, 2049}};
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
	pin_output_config output_pins[8];
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(memory_cells); ++index)
	{
		output_pins[index].pin = memory_cells[index].pin;

		if (memory_cells[index].pin == 13 || memory_cells[index].pin == 14 ||
			memory_cells[index].pin == 15 || memory_cells[index].pin == 16 ||
			memory_cells[index].pin == 17 || memory_cells[index].pin == 18)
		{
			output_pins[index].flags = OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED;
		}
		else
		{
			output_pins[index].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;
		}

		if (!jed_get_fuse(jed, memory_cells[index].polarity_fuse))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_epl16rp4_pins - configures the pins for
    a EPL16RP4
-------------------------------------------------*/

static void config_epl16rp4_pins(const pal_data* pal, const jed_data* jed)
{
	typedef struct _memory_cell memory_cell;
	struct _memory_cell
	{
		UINT16 pin;
		UINT16 polarity_fuse; /* 0 - active low?, 1 - active high? */
		UINT16 or_fuse; /* 0 - intact? */
		UINT16 xor_fuse; /* 0 - intact? */
	};

	static memory_cell memory_cells[] = {
		{12, 2071, 2069, 2070},
		{13, 2068, 2066, 2067},
		{14, 2065, 2063, 2064},
		{15, 2062, 2060, 2061},
		{16, 2059, 2057, 2058},
		{17, 2056, 2054, 2055},
		{18, 2053, 2051, 2052},
		{19, 2050, 2048, 2049}};
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
	pin_output_config output_pins[8];
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(memory_cells); ++index)
	{
		output_pins[index].pin = memory_cells[index].pin;

		if (memory_cells[index].pin == 14 || memory_cells[index].pin == 15 ||
			memory_cells[index].pin == 16 || memory_cells[index].pin == 17)
		{
			output_pins[index].flags = OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED;
		}
		else
		{
			output_pins[index].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;
		}

		if (!jed_get_fuse(jed, memory_cells[index].polarity_fuse))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}
#endif



/*-------------------------------------------------
    config_pal10p8_pins - configures the pins for
    a PAL10P8
-------------------------------------------------*/

static void config_pal10p8_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11};
	pin_output_config output_pins[8];
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(output_pins); ++index)
	{
		output_pins[index].pin = index + 12;
		output_pins[index].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE;

		if (!jed_get_fuse(jed, 327 - index))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal12p6_pins - configures the pins for
    a PAL12P6A
-------------------------------------------------*/

static void config_pal12p6_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 19};
	pin_output_config output_pins[6];
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(output_pins); ++index)
	{
		output_pins[index].pin = index + 13;
		output_pins[index].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE;

		if (!jed_get_fuse(jed, 389 - index))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal14p4_pins - configures the pins for
    a PAL14P4
-------------------------------------------------*/

static void config_pal14p4_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 18, 19};
	pin_output_config output_pins[4];
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(output_pins); ++index)
	{
		output_pins[index].pin = index + 14;
		output_pins[index].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE;

		if (!jed_get_fuse(jed, 451 - index))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal16p2_pins - configures the pins for
    a PAL16P2
-------------------------------------------------*/

static void config_pal16p2_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13, 14, 17, 18, 19};
	pin_output_config output_pins[2];
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(output_pins); ++index)
	{
		output_pins[index].pin = index + 15;
		output_pins[index].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE;

		if (!jed_get_fuse(jed, 513 - index))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal16p8_pins - configures the pins for
    a PAL16P8
-------------------------------------------------*/

static void config_pal16p8_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 13, 14, 15, 16, 17, 18};
	pin_output_config output_pins[8];
	UINT16 output_pin_count, index;

	output_pin_count = 0;

	for (index = 0; index < pal->pinfuserowscount; ++index)
	{
		if (does_output_enable_fuse_row_allow_output(pal, jed, pal->pinfuserows[index].fuserowoutputenable))
		{
			output_pins[output_pin_count].pin = pal->pinfuserows[index].pin;
			output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

			if (!jed_get_fuse(jed, 2055 - index))
			{
				output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
			}
			else
			{
				output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
			}

			++output_pin_count;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_pal16rp4_pins - configures the pins for
    a PAL16RP4
-------------------------------------------------*/

static void config_pal16rp4_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19};
	static UINT16 registered_pins[] = {14, 15, 16, 17};
	pin_output_config output_pins[8];
	UINT16 output_pin_count, index;

	output_pin_count = 0;

	if (does_output_enable_fuse_row_allow_output(pal, jed, 1792))
	{
		output_pins[output_pin_count].pin = 12;
		output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		if (!jed_get_fuse(jed, 2055))
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
		}

		++output_pin_count;
	}

	if (does_output_enable_fuse_row_allow_output(pal, jed, 1536))
	{
		output_pins[output_pin_count].pin = 13;
		output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		if (!jed_get_fuse(jed, 2054))
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
		}

		++output_pin_count;
	}

	for (index = 0; index < ARRAY_LENGTH(registered_pins); ++index)
	{
		output_pins[output_pin_count].pin = registered_pins[index];
		output_pins[output_pin_count].flags = OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED;

		if (!jed_get_fuse(jed, 2053 - index))
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
		}

		++output_pin_count;
	}

	if (does_output_enable_fuse_row_allow_output(pal, jed, 256))
	{
		output_pins[output_pin_count].pin = 18;
		output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		if (!jed_get_fuse(jed, 2049))
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
		}

		++output_pin_count;
	}

	if (does_output_enable_fuse_row_allow_output(pal, jed, 0))
	{
		output_pins[output_pin_count].pin = 19;
		output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		if (!jed_get_fuse(jed, 2048))
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
		}

		++output_pin_count;
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_pal16rp6_pins - configures the pins for
    a PAL16RP6
-------------------------------------------------*/

static void config_pal16rp6_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19};
	static UINT16 registered_pins[] = {13, 14, 15, 16, 17, 18};
	pin_output_config output_pins[8];
	UINT16 output_pin_count, index;

	output_pin_count = 0;

	if (does_output_enable_fuse_row_allow_output(pal, jed, 1792))
	{
		output_pins[output_pin_count].pin = 12;
		output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		if (!jed_get_fuse(jed, 2055))
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
		}

		++output_pin_count;
	}

	for (index = 0; index < ARRAY_LENGTH(registered_pins); ++index)
	{
		output_pins[output_pin_count].pin = registered_pins[index];
		output_pins[output_pin_count].flags = OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED;

		if (!jed_get_fuse(jed, 2054 - index))
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
		}

		++output_pin_count;
	}

	if (does_output_enable_fuse_row_allow_output(pal, jed, 0))
	{
		output_pins[output_pin_count].pin = 19;
		output_pins[output_pin_count].flags = OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_OUTPUT;

		if (!jed_get_fuse(jed, 2048))
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[output_pin_count].flags |= OUTPUT_ACTIVEHIGH;
		}

		++output_pin_count;
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, output_pin_count);
}



/*-------------------------------------------------
    config_pal16rp8_pins - configures the pins for
    a PAL16RP8
-------------------------------------------------*/

static void config_pal16rp8_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {2, 3, 4, 5, 6, 7, 8, 9, 12, 13, 14, 15, 16, 17, 18, 19};
	pin_output_config output_pins[] = {
		{12, OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{13, OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{14, OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{15, OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{16, OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{17, OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{18, OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED},
		{19, OUTPUT_REGISTERED | OUTPUT_FEEDBACK_REGISTERED}};
	UINT16 index;

	for (index = 0; index < ARRAY_LENGTH(output_pins); ++index)
	{
		if (!jed_get_fuse(jed, 2055 - index))
		{
			output_pins[index].flags |= OUTPUT_ACTIVELOW;
		}
		else
		{
			output_pins[index].flags |= OUTPUT_ACTIVEHIGH;
		}
	}

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal6l16_pins - configures the pins for
    a PAL6L16
-------------------------------------------------*/

static void config_pal6l16_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {4, 5, 6, 7, 8, 9};
	static pin_output_config output_pins[] = {
		{1, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{2, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{3, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{10, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{11, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{13, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{14, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{20, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{21, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{22, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{23, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal8l14_pins - configures the pins for
    a PAL8L14
-------------------------------------------------*/

static void config_pal8l14_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {3, 4, 5, 6, 7, 8, 9, 10};
	static pin_output_config output_pins[] = {
		{1, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{2, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{11, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{13, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{14, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{20, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{21, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{22, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{23, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal12h10_pins - configures the pins for
    a PAL12H10
-------------------------------------------------*/

static void config_pal12h10_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13};
	static pin_output_config output_pins[] = {
		{14, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{15, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{20, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{21, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{22, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{23, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal12l10_pins - configures the pins for
    a PAL12L10
-------------------------------------------------*/

static void config_pal12l10_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13};
	static pin_output_config output_pins[] = {
		{14, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{20, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{21, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{22, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{23, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal14h8_pins - configures the pins for
    a PAL14H8
-------------------------------------------------*/

static void config_pal14h8_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 23};
	static pin_output_config output_pins[] = {
		{15, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{20, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{21, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{22, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal14l8_pins - configures the pins for
    a PAL14L8
-------------------------------------------------*/

static void config_pal14l8_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 23};
	static pin_output_config output_pins[] = {
		{15, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{20, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{21, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{22, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal16h6_pins - configures the pins for
    a PAL16H6
-------------------------------------------------*/

static void config_pal16h6_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 22, 23};
	static pin_output_config output_pins[] = {
		{16, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{20, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{21, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal16l6_pins - configures the pins for
    a PAL16L6
-------------------------------------------------*/

static void config_pal16l6_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 22, 23};
	static pin_output_config output_pins[] = {
		{16, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{17, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{20, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{21, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal18h4_pins - configures the pins for
    a PAL18H4
-------------------------------------------------*/

static void config_pal18h4_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16, 21, 22, 23};
	static pin_output_config output_pins[] = {
		{17, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{20, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal18l4_pins - configures the pins for
    a PAL18L4
-------------------------------------------------*/

static void config_pal18l4_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16, 21, 22, 23};
	static pin_output_config output_pins[] = {
		{17, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{18, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{20, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal20c1_pins - configures the pins for
    a PAL20C1
-------------------------------------------------*/

static void config_pal20c1_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16, 17, 20, 21, 22, 23};
	static pin_output_config output_pins[] = {
		{18, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVEHIGH | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    config_pal20l2_pins - configures the pins for
    a PAL20L2
-------------------------------------------------*/

static void config_pal20l2_pins(const pal_data* pal, const jed_data* jed)
{
	static UINT16 input_pins[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14, 15, 16, 17, 20, 21, 22, 23};
	static pin_output_config output_pins[] = {
		{18, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE},
		{19, OUTPUT_ACTIVELOW | OUTPUT_COMBINATORIAL | OUTPUT_FEEDBACK_NONE}};

	set_input_pins(input_pins, ARRAY_LENGTH(input_pins));
	set_output_pins(output_pins, ARRAY_LENGTH(output_pins));
}



/*-------------------------------------------------
    is_gal16v8_product_term_enabled - determines if
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
    get_peel18cv8_pin_fuse_state - determines the
    fuse state of an input pin in a fuse row
-------------------------------------------------*/

static UINT16 get_peel18cv8_pin_fuse_state(const pal_data* pal, const jed_data* jed, UINT16 pin, UINT16 fuserow)
{
	const pin_fuse_columns* fuse_columns;
	int lowfusestate, highfusestate, tmpfusestate, swapfusestates;
	UINT16 cfgpin;

	/* Synchronous Preset or Asynchronous Clear fuse row? */

	if (fuserow == 2592 || fuserow == 2628)
	{
		return get_pin_fuse_state(pal, jed, pin, fuserow);
	}

	fuse_columns = find_fuse_columns(pal, pin);

	if (!fuse_columns)
	{
		fprintf(stderr, "Fuse column data missing for pin %d!\n", pin);

		return NO_FUSE_BLOWN;
	}

	cfgpin = find_pin_from_fuse_row(pal, fuserow);

	if (!cfgpin)
	{
		fprintf(stderr, "Pin from fuse row failed!  (Fuse row: %d)\n", fuserow);

		return get_pin_fuse_state(pal, jed, pin, fuserow);
	}

	lowfusestate = jed_get_fuse(jed, fuserow + fuse_columns->lowfusecolumn);
	highfusestate = jed_get_fuse(jed, fuserow + fuse_columns->highfusecolumn);
	swapfusestates = 0;

	if (is_output_pin(pin) && is_output_pin(cfgpin))
	{
		if (get_pin_output_flags(cfgpin) & OUTPUT_FEEDBACK_COMBINATORIAL)
		{
			if ((get_pin_output_flags(pin) & OUTPUT_ACTIVELOW) &&
				(get_pin_output_flags(pin) & OUTPUT_FEEDBACK_COMBINATORIAL))
			{
				swapfusestates = 1;
			}
		}
		else if (get_pin_output_flags(cfgpin) & OUTPUT_FEEDBACK_REGISTERED)
		{
			if ((get_pin_output_flags(pin) & OUTPUT_ACTIVELOW) &&
				(get_pin_output_flags(pin) & OUTPUT_FEEDBACK_REGISTERED))
			{
				swapfusestates = 1;
			}
		}
		else if (get_pin_output_flags(cfgpin) & OUTPUT_FEEDBACK_OUTPUT)
		{
			if ((get_pin_output_flags(pin) & OUTPUT_ACTIVELOW) &&
				(get_pin_output_flags(pin) & OUTPUT_FEEDBACK_REGISTERED))
			{
				swapfusestates = 1;
			}
		}
		else
		{
			fprintf(stderr, "Unknown output pin type!  (Fuse row: %d)\n", fuserow);
		}
	}

	if (swapfusestates)
	{
		tmpfusestate = lowfusestate;
		lowfusestate = highfusestate;
		highfusestate = tmpfusestate;
	}

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
		"  jedutil -convert <source.jed> <target.bin> [fuses] -- convert JEDEC to binary form\n"
		"  jedutil -convert <source.pla> <target.bin> [fuses] -- convert Berkeley standard PLA to binary form\n"
		"  jedutil -convert <source.bin> <target.jed> -- convert binary to JEDEC form\n"
		"  jedutil -view <source.jed> <device> -- dump JED logic equations\n"
		"  jedutil -view <source.bin> <device> -- dump binary logic equations\n"
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
	int src_is_jed, src_is_pla, dst_is_jed;
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

	/* does the source end in '.pla'? */
	src_is_pla = is_pla_file(srcfile);

	/* does the destination end in '.jed'? */
	dst_is_jed = is_jed_file(dstfile);

	/* error if neither or both are .jed */
	if (!src_is_jed && !src_is_pla && !dst_is_jed)
	{
		fprintf(stderr, "At least one of the filenames must end in .jed or .pla!\n");
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
	{
		free(srcbuf);
		return 1;
	}

	memset(&jed, 0, sizeof(jed));

	/* if the source is JED or PLA, convert to binary */
	if (src_is_jed || src_is_pla)
	{
		printf("Converting '%s' to binary form '%s'\n", srcfile, dstfile);

		/* read the fuse data */
		if (src_is_jed)
			err = jed_parse(srcbuf, srcbuflen, &jed);
		else if (src_is_pla)
			err = pla_parse(srcbuf, srcbuflen, &jed);

		switch (err)
		{
			case JEDERR_INVALID_DATA:   fprintf(stderr, "Fatal error: Invalid source file\n"); free(srcbuf); return 1;
			case JEDERR_BAD_XMIT_SUM:   fprintf(stderr, "Fatal error: Bad transmission checksum\n"); free(srcbuf); return 1;
			case JEDERR_BAD_FUSE_SUM:   fprintf(stderr, "Fatal error: Bad fusemap checksum\n"); free(srcbuf); return 1;
		}

		/* override the number of fuses */
		if (numfuses != 0)
			jed.numfuses = numfuses;

		/* print out data */
		printf("Source file read successfully\n");
		printf("  Total fuses = %d\n", jed.numfuses);

		/* generate the output */
		dstbuflen = jedbin_output(&jed, nullptr, 0);
		dstbuf = (UINT8 *)malloc(dstbuflen);
		if (!dstbuf)
		{
			fprintf(stderr, "Unable to allocate %d bytes for the target buffer!\n", (int)dstbuflen);
			free(srcbuf);
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
			case JEDERR_INVALID_DATA:   fprintf(stderr, "Fatal error: Invalid binary JEDEC file\n"); free(srcbuf); return 1;
		}

		/* print out data */
		printf("Source file read successfully\n");
		printf("  Total fuses = %d\n", jed.numfuses);

		/* generate the output */
		dstbuflen = jed_output(&jed, nullptr, 0);
		dstbuf = (UINT8 *)malloc(dstbuflen);
		if (!dstbuf)
		{
			fprintf(stderr, "Unable to allocate %d bytes for the target buffer!\n", (int)dstbuflen);
			free(srcbuf);
			return 1;
		}
		dstbuflen = jed_output(&jed, dstbuf, dstbuflen);
	}

	/* write the destination file */
	err = write_dest_file(dstfile);
	free(srcbuf);
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
	int result = 0;
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
	{
		result = 1;
		goto end;
	}

	/* if the source is JED, convert to binary */
	if (is_jed)
	{
		/* read the JEDEC data */
		err = jed_parse(srcbuf, srcbuflen, &jed);
		switch (err)
		{
			case JEDERR_INVALID_DATA:   fprintf(stderr, "Fatal error: Invalid .JED file\n"); result = 1; goto end;
			case JEDERR_BAD_XMIT_SUM:   fprintf(stderr, "Fatal error: Bad transmission checksum\n"); result = 1; goto end;
			case JEDERR_BAD_FUSE_SUM:   fprintf(stderr, "Fatal error: Bad fusemap checksum\n"); result = 1; goto end;
		}
	}
	else
	{
		/* read the binary data */
		err = jedbin_parse(srcbuf, srcbuflen, &jed);
		switch (err)
		{
			case JEDERR_INVALID_DATA:   fprintf(stderr, "Fatal error: Invalid binary JEDEC file\n"); result = 1; goto end;
		}
	}

	if (jed.numfuses != pal->numfuses)
	{
		fprintf(stderr, "Fuse count does not match this pal type.");
		result = 1;
		goto end;
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
		result = 1;
	}

end:
	free(srcbuf);
	return result;
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

	for (index = 0; index < ARRAY_LENGTH(paldata); ++index)
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

	for (index = 0; index < ARRAY_LENGTH(command_entries); ++index)
	{
		if (!strcmp(argv[1], command_entries[index].command))
			return command_entries[index].command_func(argc - 2, &argv[2]);
	}

	return print_usage();
}
