// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    atariscom.h

    Atari sound communications device.

***************************************************************************/

#ifndef MAME_ATARI_ATARISCOM_H
#define MAME_ATARI_ATARISCOM_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define PORT_ATARI_COMM_SOUND_TO_MAIN_READY(_tag) \
	PORT_READ_LINE_DEVICE_MEMBER(_tag, atari_sound_comm_device, sound_to_main_ready)

#define PORT_ATARI_COMM_MAIN_TO_SOUND_READY(_tag) \
	PORT_READ_LINE_DEVICE_MEMBER(_tag, atari_sound_comm_device, main_to_sound_ready)


// ======================> atari_sound_comm_device

// device type definition
DECLARE_DEVICE_TYPE(ATARI_SOUND_COMM, atari_sound_comm_device)

class atari_sound_comm_device : public device_t
{
public:
	// construction/destruction
	template <typename T>
	atari_sound_comm_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&cputag)
		: atari_sound_comm_device(mconfig, tag, owner, (u32)0)
	{
		m_sound_cpu.set_tag(std::forward<T>(cputag));
	}

	atari_sound_comm_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto int_callback() { return m_main_int_cb.bind(); }

	// getters
	int main_to_sound_ready() { return m_main_to_sound_ready ? ASSERT_LINE : CLEAR_LINE; }
	int sound_to_main_ready() { return m_sound_to_main_ready ? ASSERT_LINE : CLEAR_LINE; }

	// main cpu accessors (forward internally to the atari_sound_comm_device)
	void main_command_w(u8 data);
	u8 main_response_r();
	void sound_reset_w(u16 data = 0);

	// sound cpu accessors
	void sound_cpu_reset() { m_sound_reset_timer->adjust(attotime::zero, 1); }
	void sound_response_w(u8 data);
	u8 sound_command_r();

protected:
	// sound I/O helpers
	TIMER_CALLBACK_MEMBER(delayed_sound_reset);
	TIMER_CALLBACK_MEMBER(delayed_sound_write);
	TIMER_CALLBACK_MEMBER(delayed_6502_write);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// configuration state
	devcb_write_line   m_main_int_cb;

	// internal state
	required_device<cpu_device> m_sound_cpu;
	emu_timer       *m_sound_reset_timer;
	emu_timer       *m_sound_write_timer;
	emu_timer       *m_6502_write_timer;
	bool             m_main_to_sound_ready;
	bool             m_sound_to_main_ready;
	u8               m_main_to_sound_data;
	u8               m_sound_to_main_data;
};


#endif // MAME_ATARI_ATARISCOM_H
