#include "audio/decobsmt.h"
#include "video/bufsprite.h"

class deco32_state : public driver_device
{
public:
	deco32_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_decobsmt(*this, "decobsmt"),
		m_spriteram(*this, "spriteram")
    { }

	required_device<cpu_device> m_maincpu;
	optional_device<decobsmt_device> m_decobsmt;
	optional_device<buffered_spriteram32_device> m_spriteram;

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
	bitmap_ind16 *m_tilemap_alpha_bitmap;


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
	DECLARE_READ32_MEMBER(deco32_irq_controller_r);
	DECLARE_WRITE32_MEMBER(deco32_irq_controller_w);
	DECLARE_WRITE32_MEMBER(deco32_sound_w);
	DECLARE_READ32_MEMBER(deco32_71_r);
	DECLARE_READ32_MEMBER(captaven_prot_r);
	DECLARE_READ32_MEMBER(captaven_soundcpu_r);
	DECLARE_READ32_MEMBER(fghthist_control_r);
	DECLARE_WRITE32_MEMBER(fghthist_eeprom_w);
	DECLARE_READ32_MEMBER(dragngun_service_r);
	DECLARE_READ32_MEMBER(lockload_gun_mirror_r);
	DECLARE_READ32_MEMBER(dragngun_prot_r);
	DECLARE_READ32_MEMBER(tattass_prot_r);
	DECLARE_WRITE32_MEMBER(tattass_prot_w);
	DECLARE_WRITE32_MEMBER(tattass_control_w);
	DECLARE_READ32_MEMBER(nslasher_prot_r);
	DECLARE_WRITE32_MEMBER(nslasher_eeprom_w);
	DECLARE_WRITE32_MEMBER(nslasher_prot_w);
	DECLARE_READ32_MEMBER(deco32_spriteram_r);
	DECLARE_WRITE32_MEMBER(deco32_spriteram_w);
	DECLARE_WRITE32_MEMBER(deco32_buffer_spriteram_w);
	DECLARE_READ32_MEMBER(deco32_spriteram2_r);
	DECLARE_WRITE32_MEMBER(deco32_spriteram2_w);
	DECLARE_WRITE32_MEMBER(deco32_buffer_spriteram2_w);
	DECLARE_WRITE32_MEMBER(deco32_pf1_rowscroll_w);
	DECLARE_WRITE32_MEMBER(deco32_pf2_rowscroll_w);
	DECLARE_WRITE32_MEMBER(deco32_pf3_rowscroll_w);
	DECLARE_WRITE32_MEMBER(deco32_pf4_rowscroll_w);
	DECLARE_READ8_MEMBER(latch_r);
	DECLARE_WRITE32_MEMBER(deco32_pri_w);
	DECLARE_WRITE32_MEMBER(deco32_ace_ram_w);
	DECLARE_WRITE32_MEMBER(deco32_nonbuffered_palette_w);
	DECLARE_WRITE32_MEMBER(deco32_buffered_palette_w);
	DECLARE_WRITE32_MEMBER(deco32_palette_dma_w);
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
	DECLARE_READ32_MEMBER(dragngun_lightgun_r);
	DECLARE_WRITE32_MEMBER(dragngun_lightgun_w);
	DECLARE_WRITE32_MEMBER(dragngun_sprite_control_w);
	DECLARE_WRITE32_MEMBER(dragngun_spriteram_dma_w);
};



/*----------- defined in video/deco32.c -----------*/

VIDEO_START( captaven );
VIDEO_START( fghthist );
VIDEO_START( dragngun );
VIDEO_START( lockload );
VIDEO_START( nslasher );

SCREEN_VBLANK( captaven );
SCREEN_VBLANK( dragngun );

SCREEN_UPDATE_IND16( captaven );
SCREEN_UPDATE_RGB32( fghthist );
SCREEN_UPDATE_RGB32( dragngun );
SCREEN_UPDATE_RGB32( nslasher );


WRITE32_HANDLER( deco32_pf1_data_w );
WRITE32_HANDLER( deco32_pf2_data_w );
WRITE32_HANDLER( deco32_pf3_data_w );
WRITE32_HANDLER( deco32_pf4_data_w );


