// license:BSD-3-Clause
// copyright-holders:Luca Elia, David Haywood
/***************************************************************************

    ESD 16 Bit Games

***************************************************************************/

#include "machine/eepromser.h"
#include "video/decospr.h"

class esd16_state : public driver_device
{
public:
	esd16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_scroll_0(*this, "scroll_0"),
		m_scroll_1(*this, "scroll_1"),
		m_spriteram(*this, "spriteram"),
		m_head_layersize(*this, "head_layersize"),
		m_headpanic_platform_x(*this, "platform_x"),
		m_headpanic_platform_y(*this, "platform_y"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_sprgen(*this, "spritegen"),
		m_eeprom(*this, "eeprom")
		{}

	/* memory pointers */
	required_shared_ptr<UINT16> m_vram_0;
	required_shared_ptr<UINT16> m_vram_1;
	required_shared_ptr<UINT16> m_scroll_0;
	required_shared_ptr<UINT16> m_scroll_1;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_head_layersize;
	required_shared_ptr<UINT16> m_headpanic_platform_x;
	required_shared_ptr<UINT16> m_headpanic_platform_y;

	/* video-related */
	tilemap_t       *m_tilemap_0_16x16;
	tilemap_t       *m_tilemap_1_16x16;
	tilemap_t       *m_tilemap_0;
	tilemap_t       *m_tilemap_1;
	int           m_tilemap0_color;
	int           m_tilemap1_color;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<decospr_device> m_sprgen;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;

	DECLARE_WRITE16_MEMBER(esd16_sound_command_w);
	DECLARE_WRITE16_MEMBER(hedpanic_platform_w);
	DECLARE_READ16_MEMBER(esd_eeprom_r);
	DECLARE_WRITE16_MEMBER(esd_eeprom_w);
	DECLARE_WRITE8_MEMBER(esd16_sound_rombank_w);
	DECLARE_READ8_MEMBER(esd16_sound_command_r);
	DECLARE_WRITE16_MEMBER(esd16_vram_0_w);
	DECLARE_WRITE16_MEMBER(esd16_vram_1_w);
	DECLARE_WRITE16_MEMBER(esd16_tilemap0_color_w);
	DECLARE_WRITE16_MEMBER(esd16_tilemap0_color_jumppop_w);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_0_16x16);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILE_GET_INFO_MEMBER(get_tile_info_1_16x16);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	UINT32 screen_update_hedpanic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECOSPR_PRIORITY_CB_MEMBER(hedpanic_pri_callback);
};
