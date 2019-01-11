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
		, m_steer(*this, "STEER")
		, m_msm_rom(*this, "adpcm_%u", 0U)
	{ }

	void topspeed(machine_config &config);

	DECLARE_CUSTOM_INPUT_MEMBER(pedal_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_shared_ptr<uint16_t> m_spritemap;
	required_shared_ptr<uint16_t> m_raster_ctrl;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_sharedram;

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
	required_ioport m_steer;

	// Misc
	uint16_t  m_cpua_ctrl;
	int32_t   m_ioc220_port;

	// ADPCM
	required_region_ptr_array<uint8_t, 2> m_msm_rom;
	uint16_t  m_msm_pos[2];
	uint8_t   m_msm_reset[2];
	uint8_t   m_msm_nibble[2];
	uint8_t   m_msm2_vck;
	uint8_t   m_msm2_vck2;

#ifdef MAME_DEBUG
	uint8_t   m_dislayer[5];
#endif

	void msm5205_update(int chip);

	DECLARE_WRITE16_MEMBER(cpua_ctrl_w);
	DECLARE_READ8_MEMBER(input_bypass_r);
	DECLARE_READ16_MEMBER(motor_r);
	DECLARE_WRITE16_MEMBER(motor_w);
	DECLARE_WRITE8_MEMBER(coins_w);

	DECLARE_WRITE8_MEMBER(msm5205_command_w);
	DECLARE_WRITE_LINE_MEMBER(msm5205_1_vck);
	DECLARE_WRITE_LINE_MEMBER(z80ctc_to0);
	DECLARE_WRITE8_MEMBER(volume_w);

	// video/topspeed.c
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_topspeed(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void cpua_map(address_map &map);
	void cpub_map(address_map &map);
	void z80_io(address_map &map);
	void z80_prg(address_map &map);
};

#endif // MAME_INCLUDES_TOPSPEED_H
