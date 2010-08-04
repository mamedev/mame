

#define MCU_INITIAL_SEED	0x81


class chaknpop_state : public driver_data_t
{
public:
	static driver_data_t *alloc(running_machine &machine) { return auto_alloc_clear(&machine, chaknpop_state(machine)); }

	chaknpop_state(running_machine &machine)
		: driver_data_t(machine) { }

	/* memory pointers */
	UINT8 *  mcu_ram;
	UINT8 *  tx_ram;
	UINT8 *  spr_ram;
	UINT8 *  attr_ram;
	size_t   spr_ram_size;

	/* mcu-related */
	UINT8 mcu_seed;
	UINT8 mcu_select;
	UINT8 mcu_result;


	/* video-related */
	tilemap_t  *tx_tilemap;
	UINT8    *vram1, *vram2, *vram3, *vram4;
	UINT8    gfxmode;
	UINT8    flip_x, flip_y;
};



/*----------- defined in machine/chaknpop.c -----------*/

READ8_HANDLER( chaknpop_mcu_port_a_r );
READ8_HANDLER( chaknpop_mcu_port_b_r );
READ8_HANDLER( chaknpop_mcu_port_c_r );
WRITE8_HANDLER( chaknpop_mcu_port_a_w );
WRITE8_HANDLER( chaknpop_mcu_port_b_w );
WRITE8_HANDLER( chaknpop_mcu_port_c_w );


/*----------- defined in video/chaknpop.c -----------*/

PALETTE_INIT( chaknpop );
VIDEO_START( chaknpop );
VIDEO_UPDATE( chaknpop );

READ8_HANDLER( chaknpop_gfxmode_r );
WRITE8_HANDLER( chaknpop_gfxmode_w );
WRITE8_HANDLER( chaknpop_txram_w );
WRITE8_HANDLER( chaknpop_attrram_w );
