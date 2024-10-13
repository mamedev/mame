// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari,Aaron Giles,Jonathan Gevaryahu,hap,Robbbert
/***************************************************************************

    8080-based black and white sound hardware

****************************************************************************/
#ifndef MAME_MIDW8080_8080BW_A_H
#define MAME_MIDW8080_8080BW_A_H

#pragma once

#include "machine/timer.h"
#include "sound/discrete.h"
#include "sound/samples.h"
#include "sound/sn76477.h"

class cane_audio_device : public device_t
{
public:
	cane_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void sh_port_1_w(u8 data);
	void music_w(u8 data);
	void sn76477_en_w(u8 data);
	void sn76477_dis_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	TIMER_DEVICE_CALLBACK_MEMBER(vco_voltage_timer);

	required_device<timer_device> m_vco_timer;
	required_device<sn76477_device> m_sn;
	required_device<discrete_sound_device> m_discrete;

	attotime m_vco_rc_chargetime;
};

DECLARE_DEVICE_TYPE(CANE_AUDIO, cane_audio_device)

#endif // MAME_MIDW8080_8080BW_A_H
