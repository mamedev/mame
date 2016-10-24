// license:BSD-3-Clause
// copyright-holders:smf
#ifndef __BUFFER_H__
#define __BUFFER_H__

class input_buffer_device : public device_t
{
public:
	input_buffer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t read() { return m_input_data; }
	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff) { return read(); }

	void write_bit0(int state) { if (state) m_input_data |= 0x01; else m_input_data &= ~0x01; }
	void write_bit1(int state) { if (state) m_input_data |= 0x02; else m_input_data &= ~0x02; }
	void write_bit2(int state) { if (state) m_input_data |= 0x04; else m_input_data &= ~0x04; }
	void write_bit3(int state) { if (state) m_input_data |= 0x08; else m_input_data &= ~0x08; }
	void write_bit4(int state) { if (state) m_input_data |= 0x10; else m_input_data &= ~0x10; }
	void write_bit5(int state) { if (state) m_input_data |= 0x20; else m_input_data &= ~0x20; }
	void write_bit6(int state) { if (state) m_input_data |= 0x40; else m_input_data &= ~0x40; }
	void write_bit7(int state) { if (state) m_input_data |= 0x80; else m_input_data &= ~0x80; }

protected:
	virtual void device_start() override;

	uint8_t m_input_data;
};

extern const device_type INPUT_BUFFER;

#endif
