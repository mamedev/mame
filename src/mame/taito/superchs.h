// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
#ifndef MAME_TAITO_SUPERCHS_H
#define MAME_TAITO_SUPERCHS_H

#pragma once

#include "machine/eepromser.h"
#include "tc0480scp.h"
#include "emupal.h"

class superchs_state : public driver_device
{
public:
	superchs_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ram(*this,"ram"),
		m_spriteram(*this,"spriteram"),
		m_shared_ram(*this,"shared_ram"),
		m_spritemap(*this,"spritemap"),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_tc0480scp(*this, "tc0480scp"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_volume(*this, "SOUND")
	{ }

	void superchs(machine_config &config);
	void chase3(machine_config &config);
	void init_superchs();

protected:
	virtual void video_start() override ATTR_COLD;

private:
	struct schs_tempsprite
	{
		u8 gfx = 0;
		u32 code = 0, color = 0;
		bool flipx = 0, flipy = 0;
		int x = 0, y = 0;
		int zoomx = 0, zoomy = 0;
		u32 primask = 0;
	};

	required_shared_ptr<u32> m_ram;
	required_shared_ptr<u32> m_spriteram;
	required_shared_ptr<u16> m_shared_ram;
	required_region_ptr<u16> m_spritemap;

	std::unique_ptr<schs_tempsprite[]> m_spritelist;

	u16 shared_ram_r(offs_t offset);
	void shared_ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void cpua_ctrl_w(offs_t offset, u32 data, u32 mem_mask = ~0);
	void coin_word_w(u8 data);
	u8 volume_r();
	u32 main_cycle_r();
	u16 sub_cycle_r();
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const u32 *primasks, int x_offs, int y_offs);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<tc0480scp_device> m_tc0480scp;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_ioport m_volume;
	void chase3_cpub_map(address_map &map) ATTR_COLD;
	void superchs_cpub_map(address_map &map) ATTR_COLD;
	void superchs_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TAITO_SUPERCHS_H
