// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
#include "cpu/m68000/m68000.h"
#include "video/bufsprite.h"
#include "machine/eepromser.h"

class gaelco2_state : public driver_device
{
public:
	gaelco2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spriteram(*this,"spriteram"),
		m_vregs(*this, "vregs"),
		m_snowboar_protection(*this, "snowboar_prot"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_generic_paletteram_16(*this, "paletteram") { }

	required_device<m68000_device> m_maincpu;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_vregs;
	optional_shared_ptr<UINT16> m_snowboar_protection;
	optional_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT16> m_generic_paletteram_16;

	UINT32 snowboard_latch;


	UINT16 *m_videoram;
	tilemap_t *m_pant[2];
	int m_dual_monitor;

	DECLARE_READ16_MEMBER(dallas_kludge_r);
	DECLARE_WRITE16_MEMBER(gaelco2_coin_w);
	DECLARE_WRITE16_MEMBER(gaelco2_coin2_w);
	DECLARE_WRITE16_MEMBER(touchgo_coin_w);
	DECLARE_READ16_MEMBER(snowboar_protection_r);
	DECLARE_WRITE16_MEMBER(snowboar_protection_w);
	DECLARE_WRITE16_MEMBER(gaelco2_vram_w);
	DECLARE_WRITE16_MEMBER(gaelco2_palette_w);
	DECLARE_DRIVER_INIT(touchgo);
	DECLARE_DRIVER_INIT(snowboar);
	DECLARE_DRIVER_INIT(alighunt);
	TILE_GET_INFO_MEMBER(get_tile_info_gaelco2_screen0);
	TILE_GET_INFO_MEMBER(get_tile_info_gaelco2_screen1);
	TILE_GET_INFO_MEMBER(get_tile_info_gaelco2_screen0_dual);
	TILE_GET_INFO_MEMBER(get_tile_info_gaelco2_screen1_dual);
	DECLARE_VIDEO_START(gaelco2);
	DECLARE_VIDEO_START(gaelco2_dual);
	UINT32 screen_update_gaelco2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_gaelco2_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_gaelco2_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE16_MEMBER(gaelco2_eeprom_cs_w);
	DECLARE_WRITE16_MEMBER(gaelco2_eeprom_sk_w);
	DECLARE_WRITE16_MEMBER(gaelco2_eeprom_data_w);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int mask, int xoffs);
	UINT32 dual_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int index);
	void gaelco2_ROM16_split_gfx(const char *src_reg, const char *dst_reg, int start, int length, int dest1, int dest2);
};


class bang_state : public gaelco2_state
{
public:
	bang_state(const machine_config &mconfig, device_type type, const char *tag)
		: gaelco2_state(mconfig, type, tag)
		, m_light0_x(*this, "LIGHT0_X")
		, m_light0_y(*this, "LIGHT0_Y")
		, m_light1_x(*this, "LIGHT1_X")
		, m_light1_y(*this, "LIGHT1_Y")
	{}

	required_ioport m_light0_x;
	required_ioport m_light0_y;
	required_ioport m_light1_x;
	required_ioport m_light1_y;

	int m_clr_gun_int;

	DECLARE_READ16_MEMBER(p1_gun_x);
	DECLARE_READ16_MEMBER(p1_gun_y);
	DECLARE_READ16_MEMBER(p2_gun_x);
	DECLARE_READ16_MEMBER(p2_gun_y);
	DECLARE_WRITE16_MEMBER(bang_clr_gun_int_w);
	TIMER_DEVICE_CALLBACK_MEMBER(bang_irq);
	DECLARE_DRIVER_INIT(bang);
};


class wrally2_state : public gaelco2_state
{
public:
	wrally2_state(const machine_config &mconfig, device_type type, const char *tag)
		: gaelco2_state(mconfig, type, tag)
		, m_analog0(*this, "ANALOG0")
		, m_analog1(*this, "ANALOG1")
	{}

	required_ioport m_analog0;
	required_ioport m_analog1;

	UINT8 m_analog_ports[2];

	DECLARE_WRITE16_MEMBER(wrally2_coin_w);
	DECLARE_WRITE16_MEMBER(wrally2_adc_clk);
	DECLARE_WRITE16_MEMBER(wrally2_adc_cs);
	DECLARE_CUSTOM_INPUT_MEMBER(wrally2_analog_bit_r);
};
