// license:BSD-3-Clause
// copyright-holders:David Haywood
/*************************************************************************

    Simple 156 based board

*************************************************************************/

#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "video/deco16ic.h"
#include "video/decospr.h"

class simpl156_state : public driver_device
{
public:
	simpl156_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_deco_tilegen1(*this, "tilegen1"),
		m_eeprom(*this, "eeprom"),
		m_okimusic(*this, "okimusic") ,
		m_mainram(*this, "mainram"),
		m_systemram(*this, "systemram"),
		m_sprgen(*this, "spritegen"),
		m_palette(*this, "palette") { }

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<okim6295_device> m_okimusic;
	/* memory pointers */
	std::unique_ptr<uint16_t[]>  m_pf1_rowscroll;
	std::unique_ptr<uint16_t[]>  m_pf2_rowscroll;
	required_shared_ptr<uint32_t> m_mainram;
	required_shared_ptr<uint32_t> m_systemram;
	optional_device<decospr_device> m_sprgen;
	required_device<palette_device> m_palette;
	std::unique_ptr<uint16_t[]> m_spriteram;
	size_t m_spriteram_size;
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(pri_callback);

	void simpl156_eeprom_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t simpl156_spriteram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void simpl156_spriteram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t simpl156_mainram_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void simpl156_mainram_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t simpl156_pf1_rowscroll_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void simpl156_pf1_rowscroll_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t simpl156_pf2_rowscroll_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void simpl156_pf2_rowscroll_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t joemacr_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t chainrec_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t prtytime_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t charlien_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t osman_speedup_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void init_simpl156();
	void init_joemacr();
	void init_charlien();
	void init_prtytime();
	void init_osman();
	void init_chainrec();
	virtual void video_start() override;
	uint32_t screen_update_simpl156(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void simpl156_vbl_interrupt(device_t &device);
};
