// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Simple 156 based board

*************************************************************************/

#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "deco16ic.h"
#include "decospr.h"
#include "emupal.h"

class simpl156_state : public driver_device
{
public:
	simpl156_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_deco_tilegen(*this, "tilegen"),
		m_eeprom(*this, "eeprom"),
		m_okimusic(*this, "okimusic"),
		m_sprgen(*this, "spritegen"),
		m_palette(*this, "palette"),
		m_rowscroll(*this, "rowscroll_%u", 1U, 0x1000U, ENDIANNESS_LITTLE),
		m_mainram(*this, "mainram", 0x4000U, ENDIANNESS_LITTLE),
		m_systemram(*this, "systemram"),
		m_spriteram(*this, "spriteram", 0x1000U, ENDIANNESS_LITTLE)
	{ }

	void joemacr(machine_config &config);
	void magdrop(machine_config &config);
	void chainrec(machine_config &config);
	void mitchell156(machine_config &config);
	void magdropp(machine_config &config);

	void init_simpl156();
	void init_joemacr();
	void init_charlien();
	void init_prtytime();
	void init_osman();
	void init_chainrec();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(pri_callback);

	void eeprom_w(u32 data);
	u32 spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, u32 data, u32 mem_mask);
	u32 mainram_r(offs_t offset);
	void mainram_w(offs_t offset, u32 data, u32 mem_mask);
	template<unsigned Layer> u32 rowscroll_r(offs_t offset);
	template<unsigned Layer> void rowscroll_w(offs_t offset, u32 data, u32 mem_mask);
	u32 joemacr_speedup_r();
	u32 chainrec_speedup_r();
	u32 prtytime_speedup_r();
	u32 charlien_speedup_r();
	u32 osman_speedup_r();

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_interrupt(int state);

	void base_map(address_map &map) ATTR_COLD;
	void chainrec_map(address_map &map) ATTR_COLD;
	void joemacr_map(address_map &map) ATTR_COLD;
	void magdrop_map(address_map &map) ATTR_COLD;
	void magdropp_map(address_map &map) ATTR_COLD;
	void mitchell156_map(address_map &map) ATTR_COLD;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<deco16ic_device> m_deco_tilegen;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<okim6295_device> m_okimusic;
	required_device<decospr_device> m_sprgen;
	required_device<palette_device> m_palette;

	/* memory pointers */
	memory_share_array_creator<u16, 2> m_rowscroll;
	memory_share_creator<u16> m_mainram;
	required_shared_ptr<u32> m_systemram;
	memory_share_creator<u16> m_spriteram;

	size_t m_spriteram_size = 0;
};
