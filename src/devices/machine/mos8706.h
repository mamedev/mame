// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS 8706 Speech Glue Logic ASIC emulation

**********************************************************************
                            _____   _____
                  _RES   1 |*    \_/     | 28  Vdd
                  _IRQ   2 |             | 27  D0
                  R/_W   3 |             | 26  T6721A D0
                  phi0   4 |             | 25  D1
                   _CS   5 |             | 24  T6721A D1
                    A0   6 |             | 23  D2
                    A1   7 |   MOS8706   | 22  T6721A D2
                         8 |             | 21  D3
                  _EOS   9 |             | 20  T6721A D3
                   APD  10 |             | 19  D4
                  phi2  11 |             | 18  D5
                    DI  12 |             | 17  D6
                  DTRD  13 |             | 16  D7
                   GND  14 |_____________| 15  _WR

**********************************************************************/

#ifndef MAME_MACHINE_MOS8706_H
#define MAME_MACHINE_MOS8706_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> mos8706_device

class mos8706_device : public device_t
{
public:
	mos8706_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto command_handler() { return m_write_command.bind(); }
	auto di_handler() { return m_write_di.bind(); }
	auto irq_handler() { return m_write_irq.bind(); }

	void eos_w(int state);
	void apd_w(int state);
	void phi2_w(int state);
	void dtrd_w(int state);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void update_irq();

	devcb_write8 m_write_command;
	devcb_write_line m_write_di;
	devcb_write_line m_write_irq;

	uint8_t m_command;
	uint8_t m_control;
	uint8_t m_fifo[16];
	uint8_t m_head;
	uint8_t m_count;
	uint8_t m_bit;
	bool m_eos;
	bool m_apd;
	bool m_phi2;
	bool m_dtrd;
	bool m_irq;
};


// device type definition
DECLARE_DEVICE_TYPE(MOS8706, mos8706_device)


#endif // MAME_MACHINE_MOS8706_H
