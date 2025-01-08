// license:BSD-3-Clause
// copyright-holders:Couriersud
/**********************************************************************

    8 bit latch interface and emulation

    Generic emulation of 74LS174/175, 74LS259 and other latches.
    Apart from providing synched latch operation, these
    latches can be configured to read their input bitwise from other
    devices as well.

    Please see audio/dkong.c for examples.

**********************************************************************/

#ifndef MAME_MACHINE_LATCH8_H
#define MAME_MACHINE_LATCH8_H

#pragma once


class latch8_device : public device_t
{
public:
	latch8_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// Write bit to discrete node
	template <unsigned N> auto write_cb() { return m_write_cb[N].bind(); }

	// Upon read, replace bits by reading from another device handler
	template <unsigned N> auto read_cb() { return m_read_cb[N].bind(); }

	// Bit mask specifying bits to be masked *out*
	void set_maskout(uint32_t maskout) { m_maskout = maskout; }

	// Bit mask specifying bits to be inverted
	void set_xorvalue(uint32_t xorvalue) { m_xorvalue = xorvalue; }

	// Bit mask specifying bits not needing cpu synchronization.
	void set_nosync(uint32_t nosync) { m_nosync = nosync; }

	// write & read full byte
	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// reset the latch
	void reset_w(offs_t offset, uint8_t data);

	// read bit x
	// FIXME: does not honour read callbacks or XOR mask
	int bit0_r() { return BIT(m_value, 0); }
	int bit1_r() { return BIT(m_value, 1); }
	int bit2_r() { return BIT(m_value, 2); }
	int bit3_r() { return BIT(m_value, 3); }
	int bit4_r() { return BIT(m_value, 4); }
	int bit5_r() { return BIT(m_value, 5); }
	int bit6_r() { return BIT(m_value, 6); }
	int bit7_r() { return BIT(m_value, 7); }

	// read inverted bit
	// FIXME: does not honour read callbacks or XOR mask
	int bit0_q_r() { return BIT(~m_value, 0); }
	int bit1_q_r() { return BIT(~m_value, 1); }
	int bit2_q_r() { return BIT(~m_value, 2); }
	int bit3_q_r() { return BIT(~m_value, 3); }
	int bit4_q_r() { return BIT(~m_value, 4); }
	int bit5_q_r() { return BIT(~m_value, 5); }
	int bit6_q_r() { return BIT(~m_value, 6); }
	int bit7_q_r() { return BIT(~m_value, 7); }

	// write bit x from data into bit determined by offset
	// latch = (latch & ~(1<<offset)) | (((data >> x) & 0x01) << offset)
	void bit0_w(offs_t offset, uint8_t data);
	void bit1_w(offs_t offset, uint8_t data);
	void bit2_w(offs_t offset, uint8_t data);
	void bit3_w(offs_t offset, uint8_t data);
	void bit4_w(offs_t offset, uint8_t data);
	void bit5_w(offs_t offset, uint8_t data);
	void bit6_w(offs_t offset, uint8_t data);
	void bit7_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_validity_check(validity_checker &valid) const override;

	TIMER_CALLBACK_MEMBER( timerproc );
	void update(uint8_t new_val, uint8_t mask);
	template <int Bit> void bitx_w(offs_t offset, uint8_t data);

private:
	devcb_write_line::array<8> m_write_cb;
	devcb_read_line::array<8> m_read_cb;

	// internal state
	uint8_t            m_value;
	bool               m_has_write;
	bool               m_has_read;

	// only for byte reads, does not affect bit reads and node_map
	uint32_t           m_maskout;
	uint32_t           m_xorvalue;  // after mask
	uint32_t           m_nosync;
};

DECLARE_DEVICE_TYPE(LATCH8, latch8_device)

#endif // MAME_MACHINE_LATCH8_H
