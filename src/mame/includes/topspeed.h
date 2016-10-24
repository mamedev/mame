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
	topspeed_state(const machine_config &mconfig, device_type type, const char *tag)
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
		m_steer(*this, "STEER") { }

	required_shared_ptr<uint16_t> m_spritemap;
	required_shared_ptr<uint16_t> m_raster_ctrl;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_sharedram;

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
	required_ioport m_steer;

	// Misc
	uint16_t  m_cpua_ctrl;
	int32_t   m_ioc220_port;

	// ADPCM
	uint8_t   *m_msm_rom[2];
	uint16_t  m_msm_pos[2];
	uint8_t   m_msm_reset[2];
	uint8_t   m_msm_nibble[2];
	uint8_t   m_msm2_vck;
	uint8_t   m_msm2_vck2;

#ifdef MAME_DEBUG
	uint8_t   m_dislayer[5];
#endif

	// drivers/topspeed.c
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void msm5205_update(int chip);

	void cpua_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t input_bypass_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint16_t motor_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void motor_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	ioport_value pedal_r(ioport_field &field, void *param);

	void sound_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void msm5205_command_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void msm5205_1_vck(int state);
	void z80ctc_to0(int state);
	void volume_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// video/topspeed.c
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_topspeed(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
