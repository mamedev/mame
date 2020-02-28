// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Simple 156 based board

*************************************************************************/

#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decospr.h"
#include "emupal.h"

class simpl156_state : public driver_device
{
public:
	simpl156_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_deco_tilegen(*this, "tilegen"),
		m_eeprom(*this, "eeprom"),
		m_okimusic(*this, "okimusic") ,
		m_mainram(*this, "mainram"),
		m_systemram(*this, "systemram"),
		m_sprgen(*this, "spritegen"),
		m_palette(*this, "palette") { }

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

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<deco16ic_device> m_deco_tilegen;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<okim6295_device> m_okimusic;
	/* memory pointers */
	std::unique_ptr<u16[]>  m_rowscroll[2];
	required_shared_ptr<u32> m_mainram;
	required_shared_ptr<u32> m_systemram;
	optional_device<decospr_device> m_sprgen;
	required_device<palette_device> m_palette;
	std::unique_ptr<u16[]> m_spriteram;
	size_t m_spriteram_size;
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(pri_callback);

	void eeprom_w(u32 data);
	u32 spriteram_r(offs_t offset);
	void spriteram_w(offs_t offset, u32 data, u32 mem_mask);
	u32 mainram_r(offs_t offset);
	void mainram_w(offs_t offset, u32 data, u32 mem_mask);
	template<unsigned Layer> u32 rowscroll_r(offs_t offset);
	template<unsigned Layer> void rowscroll_w(offs_t offset, u32 data, u32 mem_mask);
	DECLARE_READ32_MEMBER(joemacr_speedup_r);
	DECLARE_READ32_MEMBER(chainrec_speedup_r);
	DECLARE_READ32_MEMBER(prtytime_speedup_r);
	DECLARE_READ32_MEMBER(charlien_speedup_r);
	DECLARE_READ32_MEMBER(osman_speedup_r);

	virtual void video_start() override;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	DECLARE_WRITE_LINE_MEMBER(vblank_interrupt);

	void chainrec_map(address_map &map);
	void joemacr_map(address_map &map);
	void magdrop_map(address_map &map);
	void magdropp_map(address_map &map);
	void mitchell156_map(address_map &map);
};
