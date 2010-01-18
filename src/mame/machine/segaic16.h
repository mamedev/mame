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

/* multiply chip */
READ16_HANDLER( segaic16_multiply_0_r );
READ16_HANDLER( segaic16_multiply_1_r );
READ16_HANDLER( segaic16_multiply_2_r );
WRITE16_HANDLER( segaic16_multiply_0_w );
WRITE16_HANDLER( segaic16_multiply_1_w );
WRITE16_HANDLER( segaic16_multiply_2_w );

/* divide chip */
READ16_HANDLER( segaic16_divide_0_r );
READ16_HANDLER( segaic16_divide_1_r );
READ16_HANDLER( segaic16_divide_2_r );
WRITE16_HANDLER( segaic16_divide_0_w );
WRITE16_HANDLER( segaic16_divide_1_w );
WRITE16_HANDLER( segaic16_divide_2_w );

/* compare/timer chip */
void segaic16_compare_timer_init(int which, void (*sound_write_callback)(running_machine *, UINT8), void (*timer_ack_callback)(running_machine *));
int segaic16_compare_timer_clock(int which);
READ16_HANDLER( segaic16_compare_timer_0_r );
READ16_HANDLER( segaic16_compare_timer_1_r );
WRITE16_HANDLER( segaic16_compare_timer_0_w );
WRITE16_HANDLER( segaic16_compare_timer_1_w );

