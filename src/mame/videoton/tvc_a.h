// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*****************************************************************************
 *
 *   includes/tvc.h
 *
 ****************************************************************************/

#ifndef MAME_VIDEOTON_TVC_A_H
#define MAME_VIDEOTON_TVC_A_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> tvc_sound_device

class tvc_sound_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	tvc_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto sndint_wr_callback() { return m_write_sndint.bind(); }

	void write(offs_t offset, uint8_t data);
	void reset_divider();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

	TIMER_CALLBACK_MEMBER(trigger_int);

private:
	sound_stream *      m_stream;
	int                 m_freq;
	int                 m_enabled;
	int                 m_volume;
	int                 m_incr;
	int                 m_signal;
	uint8_t             m_ports[3];
	emu_timer *         m_sndint_timer;
	devcb_write_line    m_write_sndint;
};

// device type definition
DECLARE_DEVICE_TYPE(TVC_SOUND, tvc_sound_device)

#endif // MAME_VIDEOTON_TVC_A_H
