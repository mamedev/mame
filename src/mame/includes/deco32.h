#include "audio/decobsmt.h"

class deco32_state : public driver_device
{
public:
	deco32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_decobsmt(*this, "decobsmt")
    { }

	required_device<cpu_device> m_maincpu;
	optional_device<decobsmt_device> m_decobsmt;

	UINT32 *m_ram;
	int m_raster_enable;
	timer_device *m_raster_irq_timer;
	UINT8 m_nslasher_sound_irq;
	int m_strobe;
	int m_tattass_eprom_bit;
	int m_lastClock;
	char m_buffer[32];
	int m_bufPtr;
	int m_pendingCommand;
	int m_readBitCount;
	int m_byteAddr;

	int m_ace_ram_dirty;
	int m_has_ace_ram;
	UINT32 *m_ace_ram;

	UINT8 *m_dirty_palette;

	int m_pri;
	bitmap_t *m_tilemap_alpha_bitmap;


	UINT16 m_spriteram16[0x1000];
	UINT16 m_spriteram16_buffered[0x1000];
	UINT16 m_spriteram16_2[0x1000];
	UINT16 m_spriteram16_2_buffered[0x1000];
	UINT16    m_pf1_rowscroll[0x1000];
	UINT16    m_pf2_rowscroll[0x1000];
	UINT16    m_pf3_rowscroll[0x1000];
	UINT16    m_pf4_rowscroll[0x1000];
	// we use the pointers below to store a 32-bit copy..
	UINT32 *m_pf1_rowscroll32;
	UINT32 *m_pf2_rowscroll32;
	UINT32 *m_pf3_rowscroll32;
	UINT32 *m_pf4_rowscroll32;

	device_t *m_deco_tilegen1;
	device_t *m_deco_tilegen2;
	UINT8 m_irq_source;
};

class dragngun_state : public deco32_state
{
public:
	dragngun_state(const machine_config &mconfig, device_type type, const char *tag)
		: deco32_state(mconfig, type, tag) { }

	UINT32 *m_dragngun_sprite_layout_0_ram;
	UINT32 *m_dragngun_sprite_layout_1_ram;
	UINT32 *m_dragngun_sprite_lookup_0_ram;
	UINT32 *m_dragngun_sprite_lookup_1_ram;
	UINT32 m_dragngun_sprite_ctrl;
	int m_dragngun_lightgun_port;
};



/*----------- defined in video/deco32.c -----------*/

VIDEO_START( captaven );
VIDEO_START( fghthist );
VIDEO_START( dragngun );
VIDEO_START( lockload );
VIDEO_START( nslasher );

SCREEN_EOF( captaven );
SCREEN_EOF( dragngun );

SCREEN_UPDATE( captaven );
SCREEN_UPDATE( fghthist );
SCREEN_UPDATE( dragngun );
SCREEN_UPDATE( nslasher );


WRITE32_HANDLER( deco32_pf1_data_w );
WRITE32_HANDLER( deco32_pf2_data_w );
WRITE32_HANDLER( deco32_pf3_data_w );
WRITE32_HANDLER( deco32_pf4_data_w );

WRITE32_HANDLER( deco32_nonbuffered_palette_w );
WRITE32_HANDLER( deco32_buffered_palette_w );
WRITE32_HANDLER( deco32_palette_dma_w );

WRITE32_HANDLER( deco32_pri_w );
WRITE32_HANDLER( dragngun_sprite_control_w );
WRITE32_HANDLER( dragngun_spriteram_dma_w );
WRITE32_HANDLER( deco32_ace_ram_w );
