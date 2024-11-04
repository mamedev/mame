// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    µPD7004

    10-bit 8 Channel A/D Converter

                   ___ ___
          CH4   1 |*  u   | 28  CH3
          CH5   2 |       | 27  CH2
          CH6   3 |       | 26  CH1
          CH7   4 |       | 25  CH0
         VREF   5 |       | 24  AGND
         DGND   6 |       | 23  AVDD
       DB7/SO   7 |       | 22  CS
       DB6/SI   8 |       | 21  RD/SCKI
    DB5/SHIFT   9 |       | 20  A0
     DB4/SCKO  10 |       | 19  WR/STB
     DB3/SOEN  11 |       | 18  MC
     DB2/CODE  12 |       | 17  CLOCK
     DB1/DEV1  13 |       | 16  EOC
     DB0/DEV0  14 |_______| 15  DVDD

***************************************************************************/

#ifndef MAME_DEVICES_MACHINE_UPD7004_H
#define MAME_DEVICES_MACHINE_UPD7004_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class upd7004_device : public device_t
{
public:
	// construction/destruction
	upd7004_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto eoc_callback() { return m_eoc_cb.bind(); }
	auto eoc_ff_callback() { return m_eoc_ff_cb.bind(); }
	template <int N> auto in_callback() { return m_in_cb[N].bind(); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(update_state);

private:
	// callbacks
	devcb_write_line m_eoc_cb;
	devcb_write_line m_eoc_ff_cb;
	devcb_read16::array<8> m_in_cb;

	enum state : int
	{
		STATE_IDLE,
		STATE_CONVERSION_START,
		STATE_CONVERSION_DONE
	};
	state m_state;

	emu_timer *m_cycle_timer;

	// state
	int m_div;
	bool m_code;
	int m_address;
	uint16_t m_sar;
};

// device type definition
DECLARE_DEVICE_TYPE(UPD7004, upd7004_device)

#endif // MAME_DEVICES_MACHINE_UPD7004_H
