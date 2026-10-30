// license:BSD-3-Clause
// copyright-holders:Dave Rand
/**********************************************************************

    Xerox Low Profile Keyboard (LPK, product #G25) — HLE

    The 16/8 (Xerox Professional Computer) "RX" monitor expects the Low
    Profile Keyboard, which is *position encoded* rather than ASCII
    translated like the standard 820/820-II keyboard.  Per the 9R80758
    Technical Reference (Appendix J, "Position encoded keyboard handler"):

      - Same strobed parallel interface to the system as the standard
        keyboard (one byte on the bus + KBSTB strobe -> Z80 PIO port B).
        The bus is read complemented by the host (kbpio_pb_r ^ 0xff), so
        this device presents the *true* byte value and lets the driver
        invert, exactly as the standard keyboard does.

      - Each key transition emits TWO strobed bytes (three for the mouse,
        not modelled): a cmd/status byte then a scan code.  The host ISR
        is a state machine that assembles them.

      - cmd/status bits: 7 cmd, 6 up-stroke (1=release/break, 0=make),
        5 mouse-Yneg, 4 mouse-Xneg, 3 mouse, 2 ctrl, 1 shift, 0 lock.

      - Scan codes are IBM PC/XT set-1 (the host ROM holds the
        scancode->ASCII tables).

    This is a high-level emulation: we have the wire protocol and the
    scancode set, but not the LPK's own controller ROM, so the bytes are
    synthesised directly from the MAME key matrix.

*********************************************************************/

#ifndef MAME_XEROX_XEROX_LPK_H
#define MAME_XEROX_XEROX_LPK_H

#pragma once

#include "x820kb.h"


class xerox_lpk_device : public xerox820_keyboard_device
{
public:
	xerox_lpk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual uint8_t read() override { return m_bus; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	// cmd/status bit positions (Tech Ref Appendix J)
	static constexpr uint8_t CST_CMD   = 0x80;
	static constexpr uint8_t CST_BREAK = 0x40; // up-stroke
	static constexpr uint8_t CST_CTRL  = 0x04;
	static constexpr uint8_t CST_SHIFT = 0x02;
	static constexpr uint8_t CST_LOCK  = 0x01;

	static constexpr int MATRIX_ROWS = 14;

	TIMER_CALLBACK_MEMBER(scan_tick);

	void queue(uint8_t b);
	void send_key(uint8_t scancode, bool make, bool shift, bool ctrl);
	uint8_t modifiers(bool shift, bool ctrl) const;

	required_ioport_array<MATRIX_ROWS> m_rows;
	ioport_value m_state[MATRIX_ROWS];

	uint8_t m_bus;          // current byte presented to the host
	bool    m_lock;         // local LOCK toggle state
	bool    m_strobe;       // KBSTB level; toggled once per byte (one edge each)

	// transmit FIFO of true-value bytes to strobe out, one per tick
	uint8_t m_fifo[256];
	int     m_fifo_head, m_fifo_tail;

	emu_timer *m_scan_timer; // polls the matrix + paces the FIFO
};

DECLARE_DEVICE_TYPE(XEROX_LPK, xerox_lpk_device)

#endif // MAME_XEROX_XEROX_LPK_H
