// license:BSD-3-Clause
// copyright-holders:David Graves
/*************************************************************************

    Top Speed / Full Throttle

*************************************************************************/

#include "sound/msm5205.h"
#include "sound/flt_vol.h"
#include "machine/taitoio.h"
#include "video/pc080sn.h"

class topspeed_state : public driver_device
{
public:
	topspeed_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_spritemap(*this, "spritemap"),
		m_raster_ctrl(*this, "raster_ctrl"),
		m_spriteram(*this, "spriteram"),
		m_sharedram(*this, "sharedram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "subcpu"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_pc080sn_1(*this, "pc080sn_1"),
		m_pc080sn_2(*this, "pc080sn_2"),
		m_tc0220ioc(*this, "tc0220ioc"),
		m_filter1l(*this, "filter1l"),
		m_filter1r(*this, "filter1r"),
		m_filter2(*this, "filter2"),
		m_filter3(*this, "filter3"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	required_shared_ptr<UINT16> m_spritemap;
	required_shared_ptr<UINT16> m_raster_ctrl;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_sharedram;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<msm5205_device> m_msm1;
	required_device<msm5205_device> m_msm2;
	required_device<pc080sn_device> m_pc080sn_1;
	required_device<pc080sn_device> m_pc080sn_2;
	required_device<tc0220ioc_device> m_tc0220ioc;
	required_device<filter_volume_device> m_filter1l;
	required_device<filter_volume_device> m_filter1r;
	required_device<filter_volume_device> m_filter2;
	required_device<filter_volume_device> m_filter3;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	// Misc
	UINT16  m_cpua_ctrl;
	INT32   m_ioc220_port;

	// ADPCM
	UINT8   *m_msm_rom[2];
	UINT16  m_msm_pos[2];
	UINT8   m_msm_reset[2];
	UINT8   m_msm_nibble[2];
	UINT8   m_msm2_vck;
	UINT8   m_msm2_vck2;

#ifdef MAME_DEBUG
	UINT8   m_dislayer[5];
#endif

	// drivers/topspeed.c
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void msm5205_update(int chip);

	DECLARE_WRITE16_MEMBER(cpua_ctrl_w);
	DECLARE_READ8_MEMBER(input_bypass_r);
	DECLARE_READ16_MEMBER(motor_r);
	DECLARE_WRITE16_MEMBER(motor_w);
	DECLARE_CUSTOM_INPUT_MEMBER(pedal_r);

	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE8_MEMBER(msm5205_command_w);
	DECLARE_WRITE_LINE_MEMBER(msm5205_1_vck);
	DECLARE_WRITE_LINE_MEMBER(z80ctc_to0);
	DECLARE_WRITE8_MEMBER(volume_w);

	// video/topspeed.c
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_topspeed(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
