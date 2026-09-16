// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, R. Belmont
/***************************************************************************

    h8_cpu_base.h

    Formal definition of the interface that H8 family peripherals expect
    a host CPU to implement.

***************************************************************************/

#ifndef MAME_CPU_H8_H8_CPU_BASE_H
#define MAME_CPU_H8_H8_CPU_BASE_H

#pragma once

struct h8_dma_state;
struct h8_dtc_state;


class h8_cpu_base : public cpu_device
{
public:
	// Digital I/O ports (h8_port_device).
	virtual u8   do_read_port(int port) = 0;
	virtual void do_write_port(int port, u8 data, u8 ddr) = 0;

	// Interrupt controller (h8_intc_device).
	virtual void set_irq(int irq_vector, int irq_level, bool irq_nmi) = 0;
	virtual bool trigger_dma(int vector) = 0;

	// Serial communication interface (h8_sci_device).
	virtual void do_sci_tx(int sci, int state) = 0;
	virtual void do_sci_clk(int sci, int state) = 0;

	// Cycle / clock queries
	virtual u64  system_clock() const = 0;
	virtual u64  now_as_cycles() const = 0;
	virtual void internal_update() = 0;
	virtual bool access_is_dma() const = 0;
	virtual int  standby() = 0;
	virtual u64  standby_time() = 0;

	// A/D converter (h8_adc_device).
	virtual u16 do_read_adc(int port) = 0;

	// DMA controller (h8_dma_device).
	virtual void set_dma_channel(h8_dma_state *state) = 0;
	virtual void update_active_dma_channel() = 0;

	// Data transfer controller (h8_dtc_device).
	virtual void set_current_dtc(h8_dtc_state *state) = 0;
	virtual void request_state(int state) = 0;

protected:
	h8_cpu_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: cpu_device(mconfig, type, tag, owner, clock)
	{
	}
};

#endif // MAME_CPU_H8_H8_CPU_BASE_H
