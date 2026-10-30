// license:BSD-3-Clause
// copyright-holders:Dave Rand
/*********************************************************************

    Definicon Systems DSI-32 coprocessor board (1985)

    NS32032 CPU + NS32081 FPU + NS32082 MMU with on-board DRAM and
    a SCN2681 DUART, presented to the PC as a 64K memory window
    (segment E000h or D000h) into the 32032's address space, paged
    by an 8-bit latch.  The host loads the bootstrap, the 32IO
    kernel and the user program through the window, then releases
    the 32032 from reset via the control port; thereafter the two
    processors converse through a mailbox page at 32032 address
    2000h, with the DUART's output port driving the PC's IRQ2.

    The board carries no firmware of any kind: an uninitialized
    CPU is held spinning by the DIAG vector PAL until the loader
    (LOAD.EXE under MS-DOS, LOAD.CMD under Concurrent CP/M)
    takes over.

    References: BYTE August/September 1985 (Marshall, Scolaro,
    Rand, King, Williams); Definicon loader sources LOAD.C,
    32K.ASM, IOPROC.ASM, 32KH.INC (October 1984); 32IO.A32.

*********************************************************************/

#ifndef MAME_BUS_ISA_DSI32_H
#define MAME_BUS_ISA_DSI32_H

#pragma once

#include "isa.h"

#include "cpu/ns32000/ns32000.h"
#include "machine/mc68681.h"
#include "machine/ns32081.h"
#include "machine/ns32082.h"


class isa8_dsi32_device : public device_t, public device_isa8_card_interface
{
public:
	isa8_dsi32_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void remap(int space_id, offs_t start, offs_t end) override;

private:
	void cpu_map(address_map &map) ATTR_COLD;

	// host side
	uint8_t window_r(offs_t offset);
	void window_w(offs_t offset, uint8_t data);
	void control_w(uint8_t data);
	void page_w(uint8_t data);

	// 32032 side
	void duart_op_w(uint8_t data);

	void update_hold();

	required_device<ns32032_device> m_cpu;
	required_device<ns32081_device> m_fpu;
	required_device<ns32082_device> m_mmu;
	required_device<scn2681_device> m_duart;
	required_ioport m_jumpers;

	uint8_t m_control;      // bus flags port: b0 DIAG, b1 INT32K, b2 RESET, b3 RFSH INHIBIT
	uint8_t m_page;
	bool m_alt;         // segment latch: A16-A23 of the window's 32032 address
};

DECLARE_DEVICE_TYPE(ISA8_DSI32, isa8_dsi32_device)

#endif // MAME_BUS_ISA_DSI32_H
