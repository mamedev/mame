// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari,Aaron Giles
/*************************************************************************

    Gottlieb Exterminator hardware

*************************************************************************/
#include "sound/dac.h"
#include "cpu/tms32010/tms32010.h"

class exterm_state : public driver_device
{
public:
	exterm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_audioslave(*this, "audioslave"),
		m_slave(*this, "slave"),
		m_dac(*this, "dac"),
		m_master_videoram(*this, "master_videoram"),
		m_slave_videoram(*this, "slave_videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_audioslave;
	required_device<tms34010_device> m_slave;
	required_device<dac_device> m_dac;

	required_shared_ptr<UINT16> m_master_videoram;
	required_shared_ptr<UINT16> m_slave_videoram;

	UINT8 m_aimpos[2];
	UINT8 m_trackball_old[2];
	UINT8 m_master_sound_latch;
	UINT8 m_slave_sound_latch;
	UINT8 m_sound_control;
	UINT8 m_dac_value[2];
	UINT16 m_last;

	DECLARE_WRITE16_MEMBER(exterm_host_data_w);
	DECLARE_READ16_MEMBER(exterm_host_data_r);
	DECLARE_READ16_MEMBER(exterm_input_port_0_r);
	DECLARE_READ16_MEMBER(exterm_input_port_1_r);
	DECLARE_WRITE16_MEMBER(exterm_output_port_0_w);
	DECLARE_WRITE16_MEMBER(sound_latch_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_rate_w);
	DECLARE_READ8_MEMBER(sound_master_latch_r);
	DECLARE_READ8_MEMBER(sound_slave_latch_r);
	DECLARE_READ8_MEMBER(sound_nmi_to_slave_r);
	DECLARE_WRITE8_MEMBER(sound_control_w);
	DECLARE_WRITE8_MEMBER(ym2151_data_latch_w);
	DECLARE_WRITE8_MEMBER(sound_slave_dac_w);
	DECLARE_PALETTE_INIT(exterm);
	TIMER_CALLBACK_MEMBER(sound_delayed_w);
	TIMER_DEVICE_CALLBACK_MEMBER(master_sound_nmi_callback);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg_master);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg_master);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg_slave);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg_slave);
	UINT16 exterm_trackball_port_r(int which, UINT16 mem_mask);
};
