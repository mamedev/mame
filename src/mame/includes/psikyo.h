/*************************************************************************

    Psikyo Games

*************************************************************************/

class psikyo_state : public driver_device
{
public:
	psikyo_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT32 *       vram_0;
	UINT32 *       vram_1;
	UINT32 *       vregs;
	UINT32 *       spritebuf1;
	UINT32 *       spritebuf2;
	UINT32 *       bootleg_spritebuffer;
//      UINT32 *       paletteram;  // currently this uses generic palette handling
//  UINT32 *       spriteram;   // currently this uses generic buffered spriteram
//  size_t         spriteram_size;

	/* video-related */
	tilemap_t        *tilemap_0_size0, *tilemap_0_size1, *tilemap_0_size2, *tilemap_0_size3;
	tilemap_t        *tilemap_1_size0, *tilemap_1_size1, *tilemap_1_size2, *tilemap_1_size3;
	int            tilemap_0_bank, tilemap_1_bank;
	int            ka302c_banking;

	/* misc */
	UINT8          soundlatch;
	int            z80_nmi, mcu_status;

	/* devices */
	device_t *audiocpu;

	/* game-specific */
	// 1945 MCU
	UINT8          s1945_mcu_direction, s1945_mcu_latch1, s1945_mcu_latch2, s1945_mcu_inlatch, s1945_mcu_index;
	UINT8          s1945_mcu_latching, s1945_mcu_mode, s1945_mcu_control, s1945_mcu_bctrl;
	const UINT8    *s1945_mcu_table;
};


/*----------- defined in video/psikyo.c -----------*/

void psikyo_switch_banks(running_machine *machine, int tmap, int bank);

WRITE32_HANDLER( psikyo_vram_0_w );
WRITE32_HANDLER( psikyo_vram_1_w );

VIDEO_START( sngkace );
VIDEO_START( psikyo );
VIDEO_UPDATE( psikyo );
VIDEO_UPDATE( psikyo_bootleg );
VIDEO_EOF( psikyo );
