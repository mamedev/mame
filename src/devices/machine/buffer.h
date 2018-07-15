// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_DEVICES_MACHINE_BUFFER_H
#define MAME_DEVICES_MACHINE_BUFFER_H

class input_buffer_device : public device_t
{
public:
	input_buffer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	uint8_t read() { return m_input_data; }
	DECLARE_READ8_MEMBER(bus_r) { return read(); }

	DECLARE_WRITE_LINE_MEMBER(write_bit0) { if (state) m_input_data |= 0x01; else m_input_data &= ~0x01; }
	DECLARE_WRITE_LINE_MEMBER(write_bit1) { if (state) m_input_data |= 0x02; else m_input_data &= ~0x02; }
	DECLARE_WRITE_LINE_MEMBER(write_bit2) { if (state) m_input_data |= 0x04; else m_input_data &= ~0x04; }
	DECLARE_WRITE_LINE_MEMBER(write_bit3) { if (state) m_input_data |= 0x08; else m_input_data &= ~0x08; }
	DECLARE_WRITE_LINE_MEMBER(write_bit4) { if (state) m_input_data |= 0x10; else m_input_data &= ~0x10; }
	DECLARE_WRITE_LINE_MEMBER(write_bit5) { if (state) m_input_data |= 0x20; else m_input_data &= ~0x20; }
	DECLARE_WRITE_LINE_MEMBER(write_bit6) { if (state) m_input_data |= 0x40; else m_input_data &= ~0x40; }
	DECLARE_WRITE_LINE_MEMBER(write_bit7) { if (state) m_input_data |= 0x80; else m_input_data &= ~0x80; }

protected:
	virtual void device_start() override;

	uint8_t m_input_data;
};

DECLARE_DEVICE_TYPE(INPUT_BUFFER, input_buffer_device)

#endif // MAME_DEVICES_MACHINE_BUFFER_H
