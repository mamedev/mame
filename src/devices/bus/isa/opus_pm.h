// license:BSD-3-Clause
// copyright-holders:Dave Rand, Patrick Mackinlay
/*********************************************************************

    Opus Systems PM (Personal Mainframe) coprocessor boards

    A family of NS32000 UNIX coprocessor cards for the IBM PC, all
    sharing one host interface and the Opus5 operating system (the
    Opus port of AT&T UNIX System V):

      opus_pm100   Opus 108PM   NS32016   8-bit ISA
      opus_pm110   Opus 110PM   NS32032   8-bit ISA

    Both carry an NS32081 FPU, an NS32082 MMU and an NS32201 TCU with
    up to 4MB of DRAM, and differ only in the CPU.  There is no boot
    ROM: the host monitor (OPMON.EXE) loads a stand-alone shell or
    kernel into low board memory through a 64KB ISA memory window and
    serves all I/O over a shared-memory command page at board physical
    address 0.

    Modelled from the surviving Opus5 source distribution and the
    OPMON driver kit.  The Opus software was preserved by Al Kossow;
    nearly all of the files this emulation was derived from came
    directly from the bitsavers.org archives.

*********************************************************************/

#ifndef MAME_BUS_ISA_OPUS_PM_H
#define MAME_BUS_ISA_OPUS_PM_H

#pragma once

#include "isa.h"

#include "cpu/ns32000/ns32000.h"
#include "machine/ns32081.h"
#include "machine/ns32082.h"

class isa8_opus_pm100_device_base : public device_t, public device_isa8_card_interface
{
protected:
	// for derived board variants (different CPU, same everything else)
	isa8_opus_pm100_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void remap(int space_id, offs_t start, offs_t end) override;

	offs_t window_phys(offs_t offset, bool write);  // window mapping (init: physical low; RUN: via 32082 MMU)

	void cpu_map(address_map &map) ATTR_COLD;

	// the CPU is the only thing that varies between board variants; the common
	// code only uses generic cpu_device operations, so keep the finder generic
	required_device<cpu_device> m_cpu;       // NS32016 (108PM) or NS32032 (110PM)
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;

private:
	// host side: the 64KB memory window
	virtual uint8_t window_r(offs_t offset) = 0;
	virtual void window_w(offs_t offset, uint8_t data) = 0;
	uint8_t csr_r(offs_t offset);                   // host status register (base+FFF0)
	void csr_w(offs_t offset, uint8_t data);
	uint8_t csr_strobe_r(offs_t reg);               // address strobes (base+FFF1..FFF7): read fires too
	void csr_strobe_w(offs_t reg, uint8_t data);
	void csr_strobe(offs_t reg);

	// 32016/32032 side I/O registers
	uint8_t slave_reg_r(offs_t offset);
	void slave_reg_w(offs_t offset, uint8_t data);

	void reset_card();
	void set_running(bool running);
	TIMER_CALLBACK_MEMBER(start_cpu);
	void set_host_irq(bool state);
	void update_host_irq();
	void irq_line_w(int line, int state);

	required_ioport m_seg;
	required_ioport m_irq;

	emu_timer *m_start_timer;  // reset-before-run hold before the CPU executes

	bool m_running;     // CSR stat bit 3 (st_init) is the inverse
	bool m_window_top;  // window covers the top page once the monitor is up
	bool m_irq_enb;     // CPU-to-host interrupt enable (eirq/run)
	bool m_irq_latch;   // CPU-to-host interrupt (R_C_IRQ)
	bool m_int_slave;   // host-to-CPU interrupt (int/R_C_ACK)
	int m_installed_irq;
};

class isa8_opus_pm100_device : public isa8_opus_pm100_device_base
{
public:
	isa8_opus_pm100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	virtual uint8_t window_r(offs_t offset) override;
	virtual void window_w(offs_t offset, uint8_t data) override;

	memory_share_creator<uint16_t> m_ram;
};

// Opus 110PM -- the same board with an NS32032 in place of the NS32016
class isa8_opus_pm110_device : public isa8_opus_pm100_device_base
{
public:
	isa8_opus_pm110_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual uint8_t window_r(offs_t offset) override;
	virtual void window_w(offs_t offset, uint8_t data) override;

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	memory_share_creator<uint32_t> m_ram;
};

DECLARE_DEVICE_TYPE(ISA8_OPUS_PM100, isa8_opus_pm100_device)
DECLARE_DEVICE_TYPE(ISA8_OPUS_PM110, isa8_opus_pm110_device)

#endif // MAME_BUS_ISA_OPUS_PM_H
