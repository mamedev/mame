// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn,Zsolt Vasvari,Aaron Giles
/*************************************************************************

    Gottlieb Exterminator hardware

*************************************************************************/

#include "cpu/tms32010/tms32010.h"
#include "cpu/tms34010/tms34010.h"
#include "machine/gen_latch.h"
#include "machine/timer.h"

class exterm_state : public driver_device
{
public:
	exterm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_audioslave(*this, "audioslave"),
		m_soundlatch(*this, "soundlatch%u", 1),
		m_slave(*this, "slave"),
		m_master_videoram(*this, "master_videoram"),
		m_slave_videoram(*this, "slave_videoram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_audioslave;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;
	required_device<tms34010_device> m_slave;

	required_shared_ptr<uint16_t> m_master_videoram;
	required_shared_ptr<uint16_t> m_slave_videoram;

	uint8_t m_aimpos[2];
	uint8_t m_trackball_old[2];
	uint8_t m_sound_control;
	uint16_t m_last;

	virtual void machine_start() override;
	DECLARE_WRITE16_MEMBER(exterm_host_data_w);
	DECLARE_READ16_MEMBER(exterm_host_data_r);
	DECLARE_READ16_MEMBER(exterm_input_port_0_r);
	DECLARE_READ16_MEMBER(exterm_input_port_1_r);
	DECLARE_WRITE16_MEMBER(exterm_output_port_0_w);
	DECLARE_WRITE8_MEMBER(sound_latch_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_rate_w);
	DECLARE_READ8_MEMBER(sound_nmi_to_slave_r);
	DECLARE_WRITE8_MEMBER(sound_control_w);
	DECLARE_WRITE8_MEMBER(ym2151_data_latch_w);
	DECLARE_PALETTE_INIT(exterm);
	TIMER_DEVICE_CALLBACK_MEMBER(master_sound_nmi_callback);
	TMS340X0_SCANLINE_IND16_CB_MEMBER(scanline_update);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg_master);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg_master);
	TMS340X0_TO_SHIFTREG_CB_MEMBER(to_shiftreg_slave);
	TMS340X0_FROM_SHIFTREG_CB_MEMBER(from_shiftreg_slave);
	uint16_t exterm_trackball_port_r(int which, uint16_t mem_mask);
	void exterm(machine_config &config);
	void master_map(address_map &map);
	void slave_map(address_map &map);
	void sound_master_map(address_map &map);
	void sound_slave_map(address_map &map);
};
