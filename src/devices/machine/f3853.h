// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    Fairchild F3853 SRAM interface with integrated interrupt
    controller and timer

    This chip is a timer shift register, basically the same as in the
    F3851.

****************************************************************************
                            _____   _____
                   Vgg   1 |*    \_/     | 40  Vdd
                   PHI   2 |             | 39  ROMC4
                 WRITE   3 |             | 38  ROMC3
              _INT REQ   4 |             | 37  ROMC2
               _PRI IN   5 |             | 36  ROMC1
            _RAM WRITE   6 |             | 35  ROMC0
              _EXT INT   7 |             | 34  CPU READ
                 ADDR7   8 |             | 33  REG DR
                 ADDR6   9 |             | 32  ADDR15
                 ADDR5  10 |    F3853    | 31  ADDR14
                 ADDR4  11 |             | 30  ADDR13
                 ADDR3  12 |             | 29  ADDR12
                 ADDR2  13 |             | 28  ADDR11
                 ADDR1  14 |             | 27  ADDR10
                 ADDR0  15 |             | 26  ADDR9
                   DB0  16 |             | 25  ADDR8
                   DB1  17 |             | 24  DB7
                   DB2  18 |             | 23  DB6
                   DB3  19 |             | 22  DB5
                   Vss  20 |_____________| 21  DB4

***************************************************************************/

#ifndef MAME_MACHINE_F3853_H
#define MAME_MACHINE_F3853_H

#pragma once


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

// ======================> f3853_device

class f3853_device : public device_t
{
public:
	// construction/destruction
	f3853_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto int_req_callback() { return m_int_req_callback.bind(); }
	auto pri_out_callback() { return m_pri_out_callback.bind(); }
	template<typename Object> void set_int_daisy_chain_callback(Object &&cb) { m_int_daisy_chain_callback = std::forward<Object>(cb); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	DECLARE_WRITE_LINE_MEMBER(ext_int_w);
	DECLARE_WRITE_LINE_MEMBER(pri_in_w);

	TIMER_CALLBACK_MEMBER(timer_callback);

	IRQ_CALLBACK_MEMBER(int_acknowledge);

protected:
	// device-level overrides
	virtual void device_resolve_objects() override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	uint16_t timer_interrupt_vector() const { return m_int_vector & ~uint16_t(0x0080); }
	uint16_t external_interrupt_vector() const { return m_int_vector | uint16_t(0x0080); }

	void set_interrupt_request_line();
	void timer_start(uint8_t value);

	devcb_write_line m_int_req_callback;
	devcb_write_line m_pri_out_callback;
	device_irq_acknowledge_delegate m_int_daisy_chain_callback;

	uint16_t m_int_vector; // Bit 7 is set to 0 for timer interrupts, 1 for external interrupts
	bool m_external_enable;
	bool m_timer_enable;

	bool m_request_flipflop;

	bool m_priority_line;              /* inverted level*/
	bool m_external_interrupt_line;    /* inverted level */

	emu_timer *m_timer;

	uint8_t m_value_to_cycle[0x100];
};


// device type definition
DECLARE_DEVICE_TYPE(F3853, f3853_device)

#endif // MAME_MACHINE_F3853_H
