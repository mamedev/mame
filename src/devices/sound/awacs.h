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

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void set_dma_base(address_space &space, int offset0, int offset1);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	sound_stream *m_stream;

	// inline data
	uint8_t m_regs[0x100];

	int m_play_ptr, m_buffer_size, m_buffer_num;
	bool m_playback_enable;

	address_space *m_dma_space;
	int m_dma_offset_0, m_dma_offset_1;

	emu_timer *m_timer;
};


// device type definition
DECLARE_DEVICE_TYPE(AWACS, awacs_device)

#endif // MAME_SOUND_AWACS_H
