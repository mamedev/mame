// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
#ifndef MAME_INCLUDES_SUPERCHS_H
#define MAME_INCLUDES_SUPERCHS_H

#pragma once

#include "machine/eepromser.h"
#include "video/tc0480scp.h"
#include "emupal.h"

class superchs_state : public driver_device
{
public:
	superchs_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_ram(*this,"ram"),
		m_spriteram(*this,"spriteram"),
		m_shared_ram(*this,"shared_ram"),
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
	virtual void video_start() override;

private:
	struct schs_tempsprite
	{
		int gfx;
		int code,color;
		int flipx,flipy;
		int x,y;
		int zoomx,zoomy;
		int primask;
	};

	required_shared_ptr<uint32_t> m_ram;
	required_shared_ptr<uint32_t> m_spriteram;
	required_shared_ptr<uint32_t> m_shared_ram;

	std::unique_ptr<schs_tempsprite[]> m_spritelist;
	uint32_t m_mem[2];

	DECLARE_READ16_MEMBER(shared_ram_r);
	DECLARE_WRITE16_MEMBER(shared_ram_w);
	DECLARE_WRITE32_MEMBER(cpua_ctrl_w);
	DECLARE_WRITE8_MEMBER(coin_word_w);
	DECLARE_READ8_MEMBER(volume_r);
	DECLARE_READ32_MEMBER(main_cycle_r);
	DECLARE_READ16_MEMBER(sub_cycle_r);
	uint32_t screen_update_superchs(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, const int *primasks, int x_offs, int y_offs);
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
	required_device<tc0480scp_device> m_tc0480scp;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_ioport m_volume;
	void chase3_cpub_map(address_map &map);
	void superchs_cpub_map(address_map &map);
	void superchs_map(address_map &map);
};

#endif // MAME_INCLUDES_SUPERCHS_H
