// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_OUTPUT_LATCH_H
#define MAME_MACHINE_OUTPUT_LATCH_H

#pragma once


class output_latch_device : public device_t
{
public:
	output_latch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <unsigned Bit> auto bit_handler() { return m_bit_handlers[Bit].bind(); }

	void write(uint8_t data);
	DECLARE_WRITE8_MEMBER(bus_w) { write(data); }

protected:
	virtual void device_resolve_objects() override;
	virtual void device_start() override;

private:
	devcb_write_line m_bit_handlers[8];

	int m_bits[8];
};

DECLARE_DEVICE_TYPE(OUTPUT_LATCH, output_latch_device)

#endif // MAME_MACHINE_OUTPUT_LATCH_H
