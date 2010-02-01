/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/

/* open bus read helpers */
READ16_HANDLER( segaic16_open_bus_r );

/* memory mapping chip */
typedef struct _segaic16_memory_map_entry segaic16_memory_map_entry;
struct _segaic16_memory_map_entry
{
	UINT8			regbase;			/* register offset for this region */
	offs_t			regoffs;			/* offset within the region for this entry */
	offs_t			length;				/* length in bytes of this entry */
	offs_t			mirror;				/* maximal mirror values (will be truncated) */
	offs_t			romoffset;			/* offset within REGION_CPU0, or ~0 for independent entries */
	read16_space_func	read;				/* read handler */
	const char *	readbank;			/* bank for reading */
	write16_space_func	write;				/* write handler */
	const char *	writebank;			/* bank for writing */
	UINT16 **		base;				/* pointer to memory base */
	const char *	name;				/* friendly name for debugging */
};

void segaic16_memory_mapper_init(running_device *cpu, const segaic16_memory_map_entry *entrylist, void (*sound_w_callback)(running_machine *, UINT8), UINT8 (*sound_r_callback)(running_machine *));
void segaic16_memory_mapper_reset(running_machine *machine);
void segaic16_memory_mapper_config(running_machine *machine, const UINT8 *map_data);
void segaic16_memory_mapper_set_decrypted(running_machine *machine, UINT8 *decrypted);
READ8_HANDLER( segaic16_memory_mapper_r );
WRITE8_HANDLER( segaic16_memory_mapper_w );
READ16_HANDLER( segaic16_memory_mapper_lsb_r );
WRITE16_HANDLER( segaic16_memory_mapper_lsb_w );

/*** Sega 16-bit Devices ***/

#include "devcb.h"

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef void (*_315_5250_sound_callback)(running_machine *, UINT8);
typedef void (*_315_5250_timer_ack_callback)(running_machine *);

typedef struct _ic_315_5250_interface ic_315_5250_interface;
struct _ic_315_5250_interface
{
	_315_5250_sound_callback          sound_write_callback;
	_315_5250_timer_ack_callback      timer_ack_callback;
};


/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DEVICE_GET_INFO( ic_315_5248 );
DEVICE_GET_INFO( ic_315_5249 );
DEVICE_GET_INFO( ic_315_5250 );

/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define _315_5248 DEVICE_GET_INFO_NAME( ic_315_5248 )

#define MDRV_315_5248_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, _315_5248, 0)

#define _315_5249 DEVICE_GET_INFO_NAME( ic_315_5249 )

#define MDRV_315_5249_ADD(_tag) \
	MDRV_DEVICE_ADD(_tag, _315_5249, 0)

#define _315_5250 DEVICE_GET_INFO_NAME( ic_315_5250 )

#define MDRV_315_5250_ADD(_tag, _interface) \
	MDRV_DEVICE_ADD(_tag, _315_5250, 0) \
	MDRV_DEVICE_CONFIG(_interface)


/***************************************************************************
    DEVICE I/O FUNCTIONS
***************************************************************************/

/* multiply chip */
READ16_DEVICE_HANDLER( segaic16_multiply_r );
WRITE16_DEVICE_HANDLER( segaic16_multiply_w );

/* divide chip */
READ16_DEVICE_HANDLER( segaic16_divide_r );
WRITE16_DEVICE_HANDLER( segaic16_divide_w );

/* compare/timer chip */
int segaic16_compare_timer_clock( running_device *device );
READ16_DEVICE_HANDLER( segaic16_compare_timer_r );
WRITE16_DEVICE_HANDLER( segaic16_compare_timer_w );
