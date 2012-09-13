class liberate_state : public driver_device
{
public:
	liberate_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_paletteram(*this, "paletteram"),
		m_bg_vram(*this, "bg_vram"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_scratchram(*this, "scratchram"){ }

	optional_shared_ptr<UINT8> m_paletteram;
	optional_shared_ptr<UINT8> m_bg_vram; /* prosport */
	required_shared_ptr<UINT8> m_colorram;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	optional_shared_ptr<UINT8> m_scratchram;
	UINT8 *m_charram;	/* prosoccr */

	UINT8 m_io_ram[16];

	int m_bank;
	int m_latch;
	UINT8 m_gfx_rom_readback;
	int m_background_color;
	int m_background_disable;

	tilemap_t *m_back_tilemap;
	tilemap_t *m_fix_tilemap;

	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	DECLARE_READ8_MEMBER(deco16_bank_r);
	DECLARE_READ8_MEMBER(deco16_io_r);
	DECLARE_WRITE8_MEMBER(deco16_bank_w);
	DECLARE_READ8_MEMBER(prosoccr_bank_r);
	DECLARE_READ8_MEMBER(prosoccr_charram_r);
	DECLARE_WRITE8_MEMBER(prosoccr_charram_w);
	DECLARE_WRITE8_MEMBER(prosoccr_char_bank_w);
	DECLARE_WRITE8_MEMBER(prosoccr_io_bank_w);
	DECLARE_READ8_MEMBER(prosport_charram_r);
	DECLARE_WRITE8_MEMBER(prosport_charram_w);
	DECLARE_WRITE8_MEMBER(deco16_io_w);
	DECLARE_WRITE8_MEMBER(prosoccr_io_w);
	DECLARE_WRITE8_MEMBER(prosport_io_w);
	DECLARE_WRITE8_MEMBER(liberate_videoram_w);
	DECLARE_WRITE8_MEMBER(liberate_colorram_w);
	DECLARE_WRITE8_MEMBER(prosport_bg_vram_w);
	DECLARE_WRITE8_MEMBER(prosport_paletteram_w);
	DECLARE_DRIVER_INIT(yellowcb);
	DECLARE_DRIVER_INIT(liberate);
	DECLARE_DRIVER_INIT(prosport);
	TILEMAP_MAPPER_MEMBER(back_scan);
	TILEMAP_MAPPER_MEMBER(fix_scan);
	TILE_GET_INFO_MEMBER(get_back_tile_info);
	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	TILE_GET_INFO_MEMBER(prosport_get_back_tile_info);
	DECLARE_MACHINE_START(liberate);
	DECLARE_MACHINE_RESET(liberate);
	DECLARE_VIDEO_START(liberate);
	DECLARE_PALETTE_INIT(liberate);
	DECLARE_VIDEO_START(prosport);
	DECLARE_VIDEO_START(boomrang);
	DECLARE_VIDEO_START(prosoccr);
};


/*----------- defined in video/liberate.c -----------*/


SCREEN_UPDATE_IND16( prosoccr );
SCREEN_UPDATE_IND16( prosport );
SCREEN_UPDATE_IND16( liberate );
SCREEN_UPDATE_IND16( boomrang );






