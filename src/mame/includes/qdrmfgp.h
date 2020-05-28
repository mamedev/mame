// license:BSD-3-Clause
// copyright-holders:Hau
#ifndef MAME_INCLUDES_QDRMFGP_H
#define MAME_INCLUDES_QDRMFGP_H

#pragma once

#include "bus/ata/ataintf.h"
#include "machine/timer.h"
#include "sound/k054539.h"
#include "machine/k053252.h"
#include "video/konami_helper.h"
#include "video/k054156_k054157_k056832.h"
#include "emupal.h"

class qdrmfgp_state : public driver_device
{
public:
	qdrmfgp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_nvram(*this, "nvram"),
		m_workram(*this, "workram"),
		m_k056832(*this, "k056832"),
		m_k054539(*this, "k054539"),
		m_k053252(*this, "k053252"),
		m_ata(*this, "ata"),
		m_inputs_port(*this, "INPUTS"),
		m_dsw_port(*this, "DSW"),
		m_palette(*this, "palette"),
		m_sndram(*this, "sndram")
	{
	}

	void qdrmfgp(machine_config &config);
	void qdrmfgp2(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(battery_sensor_r);

protected:
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint16_t> m_nvram;
	required_shared_ptr<uint16_t> m_workram;
	required_device<k056832_device> m_k056832;
	required_device<k054539_device> m_k054539;
	required_device<k053252_device> m_k053252;
	required_device<ata_interface_device> m_ata;
	required_ioport m_inputs_port;
	required_ioport m_dsw_port;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_sndram;

	uint16_t m_control;
	int32_t m_gp2_irq_control;
	int32_t m_pal;
	emu_timer *m_gp2_timer;

	DECLARE_WRITE16_MEMBER(gp_control_w);
	DECLARE_WRITE16_MEMBER(gp2_control_w);
	DECLARE_READ16_MEMBER(v_rom_r);
	DECLARE_READ16_MEMBER(gp2_vram_r);
	DECLARE_READ16_MEMBER(gp2_vram_mirror_r);
	DECLARE_WRITE16_MEMBER(gp2_vram_w);
	DECLARE_WRITE16_MEMBER(gp2_vram_mirror_w);
	DECLARE_READ16_MEMBER(sndram_r);
	DECLARE_WRITE16_MEMBER(sndram_w);
	DECLARE_READ16_MEMBER(gp2_ide_std_r);
	DECLARE_READ16_MEMBER(inputs_r);

	DECLARE_MACHINE_START(qdrmfgp);
	DECLARE_VIDEO_START(qdrmfgp);
	DECLARE_MACHINE_START(qdrmfgp2);
	DECLARE_VIDEO_START(qdrmfgp2);

	uint32_t screen_update_qdrmfgp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(qdrmfgp2_interrupt);
	TIMER_CALLBACK_MEMBER(gp2_timer_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(qdrmfgp_interrupt);
	DECLARE_WRITE_LINE_MEMBER(ide_interrupt);
	DECLARE_WRITE_LINE_MEMBER(gp2_ide_interrupt);
	DECLARE_WRITE_LINE_MEMBER(k054539_irq1_gen);
	K056832_CB_MEMBER(qdrmfgp_tile_callback);
	K056832_CB_MEMBER(qdrmfgp2_tile_callback);

	void qdrmfgp2_map(address_map &map);
	void qdrmfgp_k054539_map(address_map &map);
	void qdrmfgp_map(address_map &map);
};

#endif // MAME_INCLUDES_QDRMFGP_H
