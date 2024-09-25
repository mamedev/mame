// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    awacs_macrisc.h

    AWACS/Screamer 16-bit audio I/O for "MacRisc" architecture Macs (PCI-based)

***************************************************************************/

#ifndef MAME_APPLE_AWACS_MACRISC_H
#define MAME_APPLE_AWACS_MACRISC_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> awacs_macrisc_device

class awacs_macrisc_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	awacs_macrisc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto dma_output() { return m_output_cb.bind(); }
	auto dma_input() { return m_input_cb.bind(); }

	virtual u32 read_macrisc(offs_t offset);
	virtual void write_macrisc(offs_t offset, u32 data);

protected:
	awacs_macrisc_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	enum {
		ACTIVE_OUT = 0x01,
		ACTIVE_IN = 0x02
	};

	static const u8 divider[4];

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	devcb_read32 m_output_cb;
	devcb_write32 m_input_cb;

	sound_stream *m_stream;

	u32 m_registers[16];
	u8 m_active;
	u16 m_phase;
};

class screamer_device : public awacs_macrisc_device
{
public:
	screamer_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u32 read_macrisc(offs_t offset) override;
	virtual void write_macrisc(offs_t offset, u32 data) override;

protected:
	virtual void device_reset() override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(AWACS_MACRISC, awacs_macrisc_device)
DECLARE_DEVICE_TYPE(SCREAMER, screamer_device)

#endif // MAME_APPLE_AWACS_MACRISC_H
