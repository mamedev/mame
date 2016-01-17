// license:BSD-3-Clause
// copyright-holders:smf
#ifndef __BUFFER_H__
#define __BUFFER_H__

class input_buffer_device : public device_t
{
public:
	input_buffer_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	UINT8 read() { return m_input_data; }
	DECLARE_READ8_MEMBER(read) { return read(); }

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

	UINT8 m_input_data;
};

extern const device_type INPUT_BUFFER;

#endif
