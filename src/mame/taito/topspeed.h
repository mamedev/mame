// license:BSD-3-Clause
// copyright-holders:David Graves
/*************************************************************************

    Top Speed / Full Throttle

*************************************************************************/
#ifndef MAME_INCLUDES_TOPSPEED_H
#define MAME_INCLUDES_TOPSPEED_H

#pragma once

#include "sound/msm5205.h"
#include "sound/flt_vol.h"
#include "machine/taitoio.h"
#include "video/pc080sn.h"

class topspeed_state : public driver_device
{
public:
	topspeed_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_spritemap(*this, "spritemap")
		, m_raster_ctrl(*this, "raster_ctrl")
		, m_spriteram(*this, "spriteram")
		, m_sharedram(*this, "sharedram")
		, m_sndbank(*this, "sndbank")
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_subcpu(*this, "subcpu")
		, m_msm(*this, "msm%u", 1U)
		, m_pc080sn(*this, "pc080sn_%u", 1U)
		, m_tc0040ioc(*this, "tc0040ioc")
		, m_filter1l(*this, "filter1l")
		, m_filter1r(*this, "filter1r")
		, m_filter2(*this, "filter2")
		, m_filter3(*this, "filter3")
		, m_gfxdecode(*this, "gfxdecode")
		, m_gas(*this, "GAS")
		, m_brake(*this, "BRAKE")
		, m_steer(*this, "STEER")
		, m_msm_rom(*this, "adpcm_%u", 0U)
	{ }

	void topspeed(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(gas_pedal_r);
	DECLARE_CUSTOM_INPUT_MEMBER(brake_pedal_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_shared_ptr<u16> m_spritemap;
	required_shared_ptr<u16> m_raster_ctrl;
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_sharedram;
	required_memory_bank m_sndbank;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device_array<msm5205_device, 2> m_msm;
	required_device_array<pc080sn_device, 2> m_pc080sn;
	required_device<tc0040ioc_device> m_tc0040ioc;
	required_device<filter_volume_device> m_filter1l;
	required_device<filter_volume_device> m_filter1r;
	required_device<filter_volume_device> m_filter2;
	required_device<filter_volume_device> m_filter3;
	required_device<gfxdecode_device> m_gfxdecode;
	required_ioport m_gas;
	required_ioport m_brake;
	required_ioport m_steer;

	// Misc
	u16  m_cpua_ctrl = 0;
	s32  m_ioc220_port = 0;

	// ADPCM
	required_region_ptr_array<u8, 2> m_msm_rom;
	u16  m_msm_pos[2]{};
	u8   m_msm_reset[2]{};
	u8   m_msm_nibble[2]{};
	u8   m_msm2_vck = 0;
	u8   m_msm2_vck2 = 0;

#ifdef MAME_DEBUG
	u8   m_dislayer[5] = { 0, 0, 0, 0, 0 };
#endif

	void msm5205_update(int chip);

	void cpua_ctrl_w(u16 data);
	u8 input_bypass_r();
	u16 motor_r(offs_t offset);
	void motor_w(offs_t offset, u16 data);
	void coins_w(u8 data);

	void msm5205_command_w(offs_t offset, u8 data);
	DECLARE_WRITE_LINE_MEMBER(msm5205_1_vck);
	DECLARE_WRITE_LINE_MEMBER(z80ctc_to0);
	void volume_w(offs_t offset, u8 data);

	// video/topspeed.cpp
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cpua_map(address_map &map);
	void cpub_map(address_map &map);
	void z80_io(address_map &map);
	void z80_prg(address_map &map);
};

#endif // MAME_INCLUDES_TOPSPEED_H
