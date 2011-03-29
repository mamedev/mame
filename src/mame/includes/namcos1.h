#define NAMCOS1_MAX_BANK 0x400

/* Bank handler definitions */
typedef struct
{
	read8_space_func bank_handler_r;
	write8_space_func bank_handler_w;
	int           bank_offset;
	UINT8 *bank_pointer;
} bankhandler;

class namcos1_state : public driver_device
{
public:
	namcos1_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int dac0_value;
	int dac1_value;
	int dac0_gain;
	int dac1_gain;
	UINT8 *paletteram;
	UINT8 *triram;
	UINT8 *s1ram;
	bankhandler bank_element[NAMCOS1_MAX_BANK];
	bankhandler active_bank[16];
	int key_id;
	int key_reg;
	int key_rng;
	int key_swap4_arg;
	int key_swap4;
	int key_bottom4;
	int key_top4;
	unsigned int key_quotient;
	unsigned int key_reminder;
	unsigned int key_numerator_high_word;
	UINT8 key[8];
	int mcu_patch_data;
	int reset;
	int wdog;
	int chip[16];
	UINT8 *videoram;
	UINT8 cus116[0x10];
	UINT8 *spriteram;
	UINT8 playfield_control[0x20];
	tilemap_t *bg_tilemap[6];
	UINT8 *tilemap_maskdata;
	int copy_sprites;
	UINT8 drawmode_table[16];
};


/*----------- defined in drivers/namcos1.c -----------*/

void namcos1_init_DACs(running_machine &machine);


/*----------- defined in machine/namcos1.c -----------*/

WRITE8_HANDLER( namcos1_bankswitch_w );
WRITE8_HANDLER( namcos1_subcpu_bank_w );

WRITE8_HANDLER( namcos1_cpu_control_w );
WRITE8_HANDLER( namcos1_watchdog_w );
WRITE8_HANDLER( namcos1_sound_bankswitch_w );

WRITE8_HANDLER( namcos1_mcu_bankswitch_w );
WRITE8_HANDLER( namcos1_mcu_patch_w );

MACHINE_RESET( namcos1 );

DRIVER_INIT( shadowld );
DRIVER_INIT( dspirit );
DRIVER_INIT( quester );
DRIVER_INIT( blazer );
DRIVER_INIT( pacmania );
DRIVER_INIT( galaga88 );
DRIVER_INIT( ws );
DRIVER_INIT( berabohm );
DRIVER_INIT( alice );
DRIVER_INIT( bakutotu );
DRIVER_INIT( wldcourt );
DRIVER_INIT( splatter );
DRIVER_INIT( faceoff );
DRIVER_INIT( rompers );
DRIVER_INIT( blastoff );
DRIVER_INIT( ws89 );
DRIVER_INIT( dangseed );
DRIVER_INIT( ws90 );
DRIVER_INIT( pistoldm );
DRIVER_INIT( soukobdx );
DRIVER_INIT( puzlclub );
DRIVER_INIT( tankfrce );
DRIVER_INIT( tankfrc4 );

/*----------- defined in video/namcos1.c -----------*/

READ8_HANDLER( namcos1_videoram_r );
WRITE8_HANDLER( namcos1_videoram_w );
WRITE8_HANDLER( namcos1_paletteram_w );
READ8_HANDLER( namcos1_spriteram_r );
WRITE8_HANDLER( namcos1_spriteram_w );

VIDEO_START( namcos1 );
SCREEN_UPDATE( namcos1 );
SCREEN_EOF( namcos1 );
