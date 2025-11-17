// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    AY-3-4592 Keyboard Encoder emulation

**********************************************************************
                            _____   _____
                   GND   1 |*    \_/     | 40  Vcc
        Data Output D1   2 |             | 39  AKD
        Data Output D2   3 |             | 38  nMATIN
        Data Output D3   4 |             | 37  nPOR
        Data Output D4   5 |             | 36  Clock
        Data Output D5   6 |             | 35  nKPD
        Data Output D6   7 |             | 34  SLI
        Data Output D7   8 |             | 33  ALI
        Data Output D8   9 |             | 32  Matrix Output X6
        Data Output D9  10 |  AY-3-4592  | 31  Matrix Output X7
        Data Output D10 11 |             | 30  Matrix Output X5
                 KBINH  12 |             | 29  Matrix Output X14
                 LO/RO  13 |             | 28  Matrix Output X15
       Matrix Input YA  14 |             | 27  Matrix Output X13
       Matrix Input YC  15 |             | 26  Matrix Output X12
       Matrix Input YB  16 |             | 25  Matrix Output X11
      Matrix Output X0  17 |             | 24  Matrix Output X10
      Matrix Output X1  18 |             | 23  Matrix Output X9
      Matrix Output X2  19 |             | 22  Matrix Output X8
      Matrix Output X3  20 |_____________| 21  Matrix Output X4

**********************************************************************/

#ifndef MAME_MACHINE_AY34592_H
#define MAME_MACHINE_AY34592_H

#pragma once

class ay34592_device : public device_t
{
public:
	ay34592_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N> auto x() { return m_read_x[N].bind(); }

	auto akd_cb() { return m_akd_cb.bind(); }
	auto sli_cb() { return m_sli_cb.bind(); }
	auto ali_cb() { return m_ali_cb.bind(); }
	auto x15_cb() { return m_x15_cb.bind(); }

	// keyboard data
	uint16_t data_r();

protected:
	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	devcb_read16::array<14> m_read_x;
	devcb_write_line m_akd_cb;
	devcb_write_line m_sli_cb;
	devcb_write_line m_ali_cb;
	devcb_write_line m_x15_cb;

	emu_timer *m_scan_timer;

	TIMER_CALLBACK_MEMBER(scan_matrix);

	uint16_t output_code(int mode, int x, int y);

	// internal state
	uint8_t m_matrix_addr;
	uint8_t m_keydown_addr;
	uint8_t m_opcode;
	uint8_t m_akd;
	uint16_t m_key_latch;
};


DECLARE_DEVICE_TYPE(AY34592, ay34592_device)

#endif // MAME_MACHINE_AY34592_H
