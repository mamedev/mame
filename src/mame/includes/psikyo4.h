// license:BSD-3-Clause
// copyright-holders:David Haywood, Paul Priest
/*************************************************************************

    Psikyo PS6807 (PS4)

*************************************************************************/

#include "cpu/sh/sh2.h"
#include "sound/ymf278b.h"
#include "machine/eepromser.h"
#include "screen.h"

#define MASTER_CLOCK 57272700   // main oscillator frequency

class psikyo4_state : public driver_device
{
public:
	psikyo4_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_vidregs(*this, "vidregs"),
		m_bgpen_1(*this, "bgpen_1"),
		m_bgpen_2(*this, "bgpen_2"),
		m_paletteram(*this, "paletteram"),
		m_ymf_bank(*this, "ymfbank%u", 0),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "lpalette"),
		m_palette2(*this, "rpalette"),
		m_lscreen(*this, "lscreen"),
		m_rscreen(*this, "rscreen"),
		m_keys(*this, "KEY.%u", 0)
	{ }

	/* memory pointers */
	required_shared_ptr<uint32_t> m_spriteram;
	required_shared_ptr<uint32_t> m_vidregs;
	required_shared_ptr<uint32_t> m_bgpen_1;
	required_shared_ptr<uint32_t> m_bgpen_2;
	required_shared_ptr<uint32_t> m_paletteram;

	required_memory_bank_array<4> m_ymf_bank;
	uint8_t m_ymf_max_bank;
	uint8_t m_io_select;

	/* video-related */
	double         m_oldbrt1;
	double         m_oldbrt2;

	/* devices */
	required_device<sh2_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<palette_device> m_palette2;
	required_device<screen_device> m_lscreen;
	required_device<screen_device> m_rscreen;
	optional_ioport_array<8> m_keys;

	DECLARE_WRITE32_MEMBER(ps4_paletteram32_RRRRRRRRGGGGGGGGBBBBBBBBxxxxxxxx_dword_w);
	DECLARE_WRITE32_MEMBER(ps4_bgpen_1_dword_w);
	DECLARE_WRITE32_MEMBER(ps4_bgpen_2_dword_w);
	DECLARE_WRITE32_MEMBER(ps4_screen1_brt_w);
	DECLARE_WRITE32_MEMBER(ps4_screen2_brt_w);
	DECLARE_WRITE32_MEMBER(ps4_vidregs_w);
	DECLARE_WRITE32_MEMBER(io_select_w);
	DECLARE_CUSTOM_INPUT_MEMBER(mahjong_ctrl_r);
	DECLARE_WRITE32_MEMBER(ps4_eeprom_w);
	DECLARE_READ32_MEMBER(ps4_eeprom_r);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_psikyo4_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_psikyo4_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(psikyosh_interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t scr);
	void ps4big(machine_config &config);
	void ps4small(machine_config &config);
	void ps4_map(address_map &map);
	void ps4_ymf_map(address_map &map);
};
