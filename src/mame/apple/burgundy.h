// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    burgundy.h

    "Burgundy" stereo 16-bit audio CODEC (iMac, Blue & White G3)

***************************************************************************/

#ifndef MAME_APPLE_BURGUNDY_H
#define MAME_APPLE_BURGUNDY_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> awacs_macrisc_device

class burgundy_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	burgundy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto dma_output() { return m_output_cb.bind(); }
	auto dma_input() { return m_input_cb.bind(); }

	virtual u32 read_macrisc(offs_t offset);
	virtual void write_macrisc(offs_t offset, u32 data);

protected:
	enum {
		ACTIVE_OUT = 0x01,
		ACTIVE_IN = 0x02
	};

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream) override;

	devcb_read32 m_output_cb;
	devcb_write32 m_input_cb;

	sound_stream *m_stream;

	u32 m_registers[256];
	u8 m_active;
	u16 m_phase;
	u16 m_reg_addr, m_cur_byte, m_last_byte;
	u32 m_codec_status, m_last_codec_control;
	u8 m_counter;
};

// device type definition
DECLARE_DEVICE_TYPE(BURGUNDY, burgundy_device)

#endif // MAME_APPLE_BURGUNDY_H
