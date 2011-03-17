enum
{
	NAMCO_CGANGPZL,
	NAMCO_EMERALDA,
	NAMCO_KNCKHEAD,
	NAMCO_BKRTMAQ,
	NAMCO_EXBANIA,
	NAMCO_QUIZTOU,
	NAMCO_SWCOURT,
	NAMCO_TINKLPIT,
	NAMCO_NUMANATH,
	NAMCO_FA,
	NAMCO_XDAY2
};

#define NA1_NVRAM_SIZE (0x800)
#define NAMCONA1_NUM_TILEMAPS 4

class namcona1_state : public driver_device
{
public:
	namcona1_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *videoram;
	UINT16 *spriteram;
	UINT16 *mpBank0;
	UINT16 *mpBank1;
	int mEnableInterrupts;
	int gametype;
	UINT8 nvmem[NA1_NVRAM_SIZE];
	UINT16 count;
	UINT32 keyval;
	UINT16 mcu_mailbox[8];
	UINT8 mcu_port4;
	UINT8 mcu_port5;
	UINT8 mcu_port6;
	UINT8 mcu_port8;
	UINT16 *workram;
	UINT16 *vreg;
	UINT16 *scroll;
	UINT16 *shaperam;
	UINT16 *cgram;
	tilemap_t *roz_tilemap;
	int roz_palette;
	tilemap_t *bg_tilemap[NAMCONA1_NUM_TILEMAPS];
	int tilemap_palette_bank[NAMCONA1_NUM_TILEMAPS];
	int palette_is_dirty;
	UINT8 mask_data[8];
	UINT8 conv_data[9];
};


/*----------- defined in video/namcona1.c -----------*/


extern WRITE16_HANDLER( namcona1_videoram_w );
extern READ16_HANDLER( namcona1_videoram_r );

extern READ16_HANDLER( namcona1_gfxram_r );
extern WRITE16_HANDLER( namcona1_gfxram_w );

extern READ16_HANDLER( namcona1_paletteram_r );
extern WRITE16_HANDLER( namcona1_paletteram_w );

extern SCREEN_UPDATE( namcona1 );
extern VIDEO_START( namcona1 );
