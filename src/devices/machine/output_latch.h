// license:BSD-3-Clause
// copyright-holders:smf
#ifndef MAME_MACHINE_OUTPUT_LATCH_H
#define MAME_MACHINE_OUTPUT_LATCH_H

#pragma once


#define MCFG_OUTPUT_LATCH_BIT0_HANDLER(_devcb) \
	downcast<output_latch_device &>(*device).set_bit_handler<0>(DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT1_HANDLER(_devcb) \
	downcast<output_latch_device &>(*device).set_bit_handler<1>(DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT2_HANDLER(_devcb) \
	downcast<output_latch_device &>(*device).set_bit_handler<2>(DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT3_HANDLER(_devcb) \
	downcast<output_latch_device &>(*device).set_bit_handler<3>(DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT4_HANDLER(_devcb) \
	downcast<output_latch_device &>(*device).set_bit_handler<4>(DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT5_HANDLER(_devcb) \
	downcast<output_latch_device &>(*device).set_bit_handler<5>(DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT6_HANDLER(_devcb) \
	downcast<output_latch_device &>(*device).set_bit_handler<6>(DEVCB_##_devcb);

#define MCFG_OUTPUT_LATCH_BIT7_HANDLER(_devcb) \
	downcast<output_latch_device &>(*device).set_bit_handler<7>(DEVCB_##_devcb);

class output_latch_device : public device_t
{
public:
	output_latch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	template <unsigned Bit, class Object> devcb_base &set_bit_handler(Object &&cb) { return m_bit_handlers[Bit].set_callback(std::forward<Object>(cb)); }
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
