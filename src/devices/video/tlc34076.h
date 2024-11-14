// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/***************************************************************************

    tlc34076.h

    Basic implementation of the TLC34076 palette chip and similar
    compatible chips.

***************************************************************************/

#ifndef MAME_VIDEO_TLC34076_H
#define MAME_VIDEO_TLC34076_H

#pragma once


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

class tlc34076_device : public device_t, public device_palette_interface
{
public:
	enum tlc34076_bits
	{
		TLC34076_6_BIT = 6,
		TLC34076_8_BIT = 8
	};

	// construction/destruction
	tlc34076_device(const machine_config &mconfig, const char *tag, device_t *owner, tlc34076_bits bits)
		: tlc34076_device(mconfig, tag, owner, (uint32_t)0)
	{
		set_bits(bits);
	}

	tlc34076_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	void set_bits(tlc34076_bits bits) { m_dacbits = bits; }

	// public interface
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_palette_interface overrides
	virtual uint32_t palette_entries() const noexcept override { return 0x100; }

private:
	// internal helpers
	void update_pen(uint8_t i);

	// internal state
	std::unique_ptr<uint8_t[]> m_local_paletteram[3];
	uint8_t m_regs[0x10];
	uint8_t m_palettedata[3];
	uint8_t m_writeindex;
	uint8_t m_readindex;
	uint8_t m_dacbits;
};


DECLARE_DEVICE_TYPE(TLC34076, tlc34076_device)

#endif // MAME_VIDEO_TLC34076_H
