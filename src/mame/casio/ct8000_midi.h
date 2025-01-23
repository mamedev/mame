// license:BSD-3-Clause
// copyright-holders: Devin Acker

#ifndef MAME_CASIO_CT8000_MIDI_H
#define MAME_CASIO_CT8000_MIDI_H

#pragma once

#include "diserial.h"

class ct8000_midi_device : public device_t, public device_serial_interface
{
public:
	ct8000_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto &set_base_program(u8 base) { m_base_program = base; return *this; }

	auto int_cb() { return m_int_cb.bind(); }

	u8 data_r();
	void data_w(u8 data) { m_in_data = data; }

	int ack_r() { return m_in_strobe; }
	void ack_w(int state);

	void strobe_w(int state);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void rcv_complete() override;

private:
	static constexpr unsigned NOTE_MIN = 12; // C0
	static constexpr unsigned NOTE_MAX = 107; // B7

	enum
	{
		CMD_EFFECTS = 0x10,
		CMD_STOP    = 0x24,
		CMD_NOTE    = 0x28,
		CMD_TONE    = 0xe1,
	};

	enum
	{
		FX_REVERB  = 0x01,
		FX_VIBRATO = 0x02,
		FX_PEDAL   = 0x04, // only affects the local keyboard, so unused here
		FX_CHORUS  = 0x08,
		FX_SUSTAIN = 0x10
	};

	void midi_message();

	void send_nibble(u8 data);
	void send_cmd(u8 cmd);
	void send_cmd(u8 cmd, u8 data);
	void send_note(u8 note, u8 on);
	void update_control(u8 control);
	void update_sustain(u8 value);

	required_ioport m_channel;
	devcb_write_line m_int_cb;

	u8 m_base_program;

	u8 m_status;
	u8 m_midi_data[2];
	u8 m_midi_data_pos, m_midi_data_size;
	u8 m_midi_notes_on[NOTE_MAX - NOTE_MIN + 1];
	u8 m_sustain;

	u8 m_ack;
	u8 m_int;
	u8 m_out_data[0x1000];
	u16 m_out_data_read, m_out_data_write;
	u16 m_out_data_space;
	u8 m_out_data_controls;

	u8 m_in_data;
	u8 m_in_strobe;
};

DECLARE_DEVICE_TYPE(CT8000_MIDI, ct8000_midi_device)

#endif // MAME_CASIO_CT8000_MIDI_H
