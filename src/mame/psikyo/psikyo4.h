// license:BSD-3-Clause
// copyright-holders:David Haywood, Paul Priest
/*************************************************************************

    Psikyo PS6807 (PS4)

*************************************************************************/

#include "cpu/sh/sh7604.h"
#include "sound/ymopl.h"
#include "machine/eepromser.h"
#include "emupal.h"
#include "screen.h"

class psikyo4_state : public driver_device
{
public:
	psikyo4_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_vidregs(*this, "vidregs"),
		m_bgpen(*this, "bgpen_%u", 1U),
		m_paletteram(*this, "paletteram"),
		m_gfxbank(*this, "gfxbank"),
		m_ymf_bank(*this, "ymfbank%u", 0),
		m_maincpu(*this, "maincpu"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, {"lpalette", "rpalette"}),
		m_lscreen(*this, "lscreen"),
		m_rscreen(*this, "rscreen"),
		m_system(*this, "SYSTEM"),
		m_keys(*this, "KEY.%u", 0)
	{ }

	void ps4big(machine_config &config);
	void ps4small(machine_config &config);

	ioport_value system_r();
	template <int P> ioport_value mahjong_ctrl_r();

private:
	/* memory pointers */
	required_shared_ptr<uint32_t> m_spriteram;
	required_shared_ptr<uint32_t> m_vidregs;
	required_shared_ptr_array<uint32_t, 2> m_bgpen;
	required_shared_ptr<uint32_t> m_paletteram;

	required_memory_bank m_gfxbank;
	required_memory_bank_array<4> m_ymf_bank;

	uint16_t m_gfx_max_bank = 0;
	uint8_t m_ymf_max_bank = 0;
	uint8_t m_io_select = 0;

	/* video-related */
	double         m_oldbrt[2]{};

	/* devices */
	required_device<sh7604_device> m_maincpu;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device_array<palette_device, 2> m_palette;
	required_device<screen_device> m_lscreen;
	required_device<screen_device> m_rscreen;
	optional_ioport m_system;
	optional_ioport_array<8> m_keys;

	void paletteram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Screen> void bgpen_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	template<int Screen> void screen_brt_w(uint8_t data);
	void vidregs_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void ymf_bank_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void io_select_w(uint8_t data);
	void eeprom_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	template<int Screen> uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint32_t scr);
	void ps4_map(address_map &map) ATTR_COLD;
	void ps4_ymf_map(address_map &map) ATTR_COLD;
};
