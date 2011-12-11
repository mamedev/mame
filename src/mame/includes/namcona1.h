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
	namcona1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_mcu(*this,"mcu")
		{ }

	UINT16 *m_videoram;
	UINT16 *m_spriteram;
	UINT16 *m_mpBank0;
	UINT16 *m_mpBank1;
	int m_mEnableInterrupts;
	int m_gametype;
	UINT8 m_nvmem[NA1_NVRAM_SIZE];
	UINT16 m_count;
	UINT32 m_keyval;
	UINT16 m_mcu_mailbox[8];
	UINT8 m_mcu_port4;
	UINT8 m_mcu_port5;
	UINT8 m_mcu_port6;
	UINT8 m_mcu_port8;
	UINT16 *m_workram;
	UINT16 *m_vreg;
	UINT16 *m_scroll;
	UINT16 *m_shaperam;
	UINT16 *m_cgram;
	tilemap_t *m_roz_tilemap;
	int m_roz_palette;
	tilemap_t *m_bg_tilemap[NAMCONA1_NUM_TILEMAPS];
	int m_tilemap_palette_bank[NAMCONA1_NUM_TILEMAPS];
	int m_palette_is_dirty;
	UINT8 m_mask_data[8];
	UINT8 m_conv_data[9];

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_mcu;
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
