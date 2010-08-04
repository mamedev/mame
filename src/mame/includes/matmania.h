
class matmania_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, matmania_state(machine)); }

	matmania_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *         videoram;
	UINT8 *         videoram2;
	UINT8 *         videoram3;
	UINT8 *         colorram;
	UINT8 *         colorram2;
	UINT8 *         colorram3;
	UINT8 *         scroll;
	UINT8 *         pageselect;
	UINT8 *         spriteram;
	UINT8 *         paletteram;
	size_t          videoram_size;
	size_t          videoram2_size;
	size_t          videoram3_size;
	size_t          spriteram_size;

	/* video-related */
	bitmap_t        *tmpbitmap;
	bitmap_t        *tmpbitmap2;

	/* mcu */
	/* maniach 68705 protection */
	UINT8           port_a_in, port_a_out, ddr_a;
	UINT8           port_b_in, port_b_out, ddr_b;
	UINT8           port_c_in, port_c_out, ddr_c;
	UINT8           from_main, from_mcu;
	int             mcu_sent, main_sent;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *mcu;
};

/*----------- defined in machine/maniach.c -----------*/

READ8_HANDLER( maniach_68705_port_a_r );
WRITE8_HANDLER( maniach_68705_port_a_w );
READ8_HANDLER( maniach_68705_port_b_r );
WRITE8_HANDLER( maniach_68705_port_b_w );
READ8_HANDLER( maniach_68705_port_c_r );
WRITE8_HANDLER( maniach_68705_port_c_w );
WRITE8_HANDLER( maniach_68705_ddr_a_w );
WRITE8_HANDLER( maniach_68705_ddr_b_w );
WRITE8_HANDLER( maniach_68705_ddr_c_w );
WRITE8_HANDLER( maniach_mcu_w );
READ8_HANDLER( maniach_mcu_r );
READ8_HANDLER( maniach_mcu_status_r );


/*----------- defined in video/matmania.c -----------*/

WRITE8_HANDLER( matmania_paletteram_w );
PALETTE_INIT( matmania );
VIDEO_UPDATE( maniach );
VIDEO_START( matmania );
VIDEO_UPDATE( matmania );
