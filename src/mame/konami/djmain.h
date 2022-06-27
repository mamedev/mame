// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_INCLUDES_DJMAIN_H
#define MAME_INCLUDES_DJMAIN_H

#pragma once

#include "bus/ata/ataintf.h"
#include "konami_helper.h"
#include "k054156_k054157_k056832.h"
#include "k055555.h"
#include "emupal.h"

class djmain_state : public driver_device
{
public:
	djmain_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_obj_ram(*this, "obj_ram")
		, m_maincpu(*this, "maincpu")
		, m_k056832(*this, "k056832")
		, m_k055555(*this, "k055555")
		, m_ata(*this, "ata")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_turntable(*this, "TT%u", 1U)
		, m_sndram(*this, "sndram")
		, m_leds(*this, "led%u", 0U)
	{
	}

	void djmainj(machine_config &config);
	void djmainu(machine_config &config);
	void djmaina(machine_config &config);

	void init_bm7thmix();
	void init_bm6thmix();
	void init_hmcompmx();
	void init_bmfinal();
	void init_hmcompm2();
	void init_bm5thmix();
	void init_bm4thmix();
	void init_bs4thmix();
	void init_beatmania();
	void init_bmdct();
	void init_bmcompm2();
	void init_bmcorerm();
	void init_bmclubmx();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	void sndram_bank_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t sndram_r(offs_t offset, uint32_t mem_mask = ~0);
	void sndram_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t obj_ctrl_r(offs_t offset);
	void obj_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t obj_rom_r(offs_t offset, uint32_t mem_mask = ~0);
	void v_ctrl_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	uint32_t v_rom_r(offs_t offset, uint32_t mem_mask = ~0);
	uint8_t inp1_r(offs_t offset);
	uint8_t inp2_r(offs_t offset);
	uint32_t turntable_r(offs_t offset, uint32_t mem_mask = ~0);
	void turntable_select_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void light_ctrl_1_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void light_ctrl_2_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void unknown590000_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void unknown802000_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);
	void unknownc02000_w(offs_t offset, uint32_t data, uint32_t mem_mask = ~0);

	uint32_t screen_update_djmain(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vb_interrupt);
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);
	void draw_sprites( bitmap_rgb32 &bitmap, const rectangle &cliprect);
	K056832_CB_MEMBER(tile_callback);
	void k054539_map(address_map &map);
	void maincpu_djmain(address_map &map);
	void maincpu_djmaina(address_map &map);
	void maincpu_djmainj(address_map &map);
	void maincpu_djmainu(address_map &map);

	required_shared_ptr<uint32_t> m_obj_ram;
	required_device<cpu_device> m_maincpu;
	required_device<k056832_device> m_k056832;
	required_device<k055555_device> m_k055555;
	required_device<ata_interface_device> m_ata;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_ioport_array<2> m_turntable;
	required_shared_ptr<uint8_t> m_sndram;
	output_finder<3> m_leds;

	int m_sndram_bank = 0;
	int m_turntable_select = 0;
	uint8_t m_turntable_last_pos[2]{};
	uint16_t m_turntable_pos[2]{};
	uint8_t m_pending_vb_int = 0U;
	uint16_t m_v_ctrl = 0U;
	uint32_t m_obj_regs[0xa0/4]{};
	const uint8_t *m_ata_user_password = nullptr;
	const uint8_t *m_ata_master_password = nullptr;
};

#endif // MAME_INCLUDES_DJMAIN_H
