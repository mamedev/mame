// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    MOS Technology 6530 Memory, I/O, Timer Array emulation

**********************************************************************
                            _____   _____
                   Vss   1 |*    \_/     | 40  PA1
                   PA0   2 |             | 39  PA2
                  phi2   3 |             | 38  PA3
                   RS0   4 |             | 37  PA4
                    A9   5 |             | 36  PA5
                    A8   6 |             | 35  PA6
                    A7   7 |             | 34  PA7
                    A6   8 |             | 33  DB0
                   R/W   9 |             | 32  DB1
                    A5  10 |   MCS6530   | 31  DB2
                    A4  11 |             | 30  DB3
                    A3  12 |             | 29  DB4
                    A2  13 |             | 28  DB5
                    A1  14 |             | 27  DB6
                    A0  15 |             | 26  DB7
                  _RES  16 |             | 25  PB0
               IRQ/PB7  17 |             | 24  PB1
               CS1/PB6  18 |             | 23  PB2
               CS2/PB5  19 |             | 22  PB3
                   Vcc  20 |_____________| 21  PB4

**********************************************************************/

#ifndef MAME_MACHINE_MOS6530_H
#define MAME_MACHINE_MOS6530_H

#pragma once


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class mos6530_device : public device_t
{
public:
	mos6530_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto in_pa_callback() { return m_in_pa_cb.bind(); }
	auto out_pa_callback() { return m_out_pa_cb.bind(); }
	auto in_pb_callback() { return m_in_pb_cb.bind(); }
	auto out_pb_callback() { return m_out_pb_cb.bind(); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	uint8_t porta_in_get();
	uint8_t portb_in_get();

	uint8_t porta_out_get();
	uint8_t portb_out_get();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	enum
	{
		TIMER_END_CALLBACK
	};

	struct mos6530_port
	{
		uint8_t m_in;
		uint8_t m_out;
		uint8_t m_ddr;
	};

	// internal state
	devcb_read8    m_in_pa_cb;
	devcb_write8   m_out_pa_cb;

	devcb_read8    m_in_pb_cb;
	devcb_write8   m_out_pb_cb;

	mos6530_port    m_port[2];

	uint8_t           m_irqstate;
	uint8_t           m_irqenable;

	uint8_t           m_timershift;
	uint8_t           m_timerstate;
	emu_timer *     m_timer;

	uint32_t          m_clock;

	void update_irqstate();
	uint8_t get_timer();

	void porta_in_set(uint8_t data, uint8_t mask);
	void portb_in_set(uint8_t data, uint8_t mask);
};

DECLARE_DEVICE_TYPE(MOS6530, mos6530_device)

#endif // MAME_MACHINE_MOS6530_H
