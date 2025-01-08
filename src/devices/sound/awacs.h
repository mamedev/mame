// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    awacs.h

    AWACS/Singer style 16-bit audio I/O for '040 and PowerPC Macs

***************************************************************************/

#ifndef MAME_SOUND_AWACS_H
#define MAME_SOUND_AWACS_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> awacs_device

class awacs_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	awacs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto irq_out_cb() { return m_irq_out_cb.bind(); }
	auto irq_in_cb() { return m_irq_in_cb.bind(); }

	auto dma_output() { return m_output_cb.bind(); }
	auto dma_input() { return m_input_cb.bind(); }

	auto port_input() { return m_input_port_cb.bind(); }
	auto port_output() { return m_output_port_cb.bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	enum {
		ACTIVE_OUT = 0x01,
		ACTIVE_IN = 0x02
	};

	static const u8 divider[4];

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	devcb_write_line m_irq_out_cb, m_irq_in_cb;
	devcb_read32 m_output_cb;
	devcb_write32 m_input_cb;
	devcb_read8 m_input_port_cb;
	devcb_write8 m_output_port_cb;

	sound_stream *m_stream;

	attotime m_last_sample;
	u8 m_ctrl0, m_ctrl1, m_codec0, m_codec1, m_codec2, m_out_irq, m_in_irq, m_active;
	u16 m_ext_address, m_ext_data, m_buffer_size, m_phase;
	bool m_extend, m_ext_command, m_input_buffer, m_output_buffer;

	void update_irq();
};


// device type definition
DECLARE_DEVICE_TYPE(AWACS, awacs_device)

#endif // MAME_SOUND_AWACS_H
