// license:BSD-3-Clause
// copyright-holders:David Haywood, Paul Priest
/*************************************************************************

    Psikyo PS6807 (PS4)

*************************************************************************/

#define MASTER_CLOCK 57272700   // main oscillator frequency

#include "emu.h"
#include "cpu/sh2/sh2.h"
#include "sound/ymf278b.h"
#include "machine/eepromser.h"

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

	memory_bank *m_ymf_bank[4];
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

	void ps4_paletteram32_RRRRRRRRGGGGGGGGBBBBBBBBxxxxxxxx_dword_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void ps4_bgpen_1_dword_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void ps4_bgpen_2_dword_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void ps4_screen1_brt_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void ps4_screen2_brt_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void ps4_vidregs_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void io_select_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	ioport_value mahjong_ctrl_r(ioport_field &field, void *param);
	void ps4_eeprom_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t ps4_eeprom_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_psikyo4_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_psikyo4_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void psikyosh_interrupt(device_t &device);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t scr);
};
