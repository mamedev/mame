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


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_F3853_EXT_INPUT_CB(_class, _method) \
	downcast<f3853_device &>(*device).set_interrupt_req_callback(f3853_device::interrupt_req_delegate(&_class::_method, #_class "::" #_method, this));

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

#define F3853_INTERRUPT_REQ_CB(_name) void _name(uint16_t addr, int level)


// ======================> f3853_device

class f3853_device :  public device_t
{
public:
	typedef device_delegate<void (uint16_t addr, int level)> interrupt_req_delegate;

	// construction/destruction
	f3853_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename Object> void set_interrupt_req_callback(Object &&cb) { m_interrupt_req_cb = std::forward<Object>(cb); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void set_external_interrupt_in_line(int level);
	void set_priority_in_line(int level);

	TIMER_CALLBACK_MEMBER(timer_callback);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_post_load() override { }
	virtual void device_clock_changed() override { }

private:
	uint16_t interrupt_vector() const { return m_low | (uint16_t(m_high) << 8); }
	uint16_t timer_interrupt_vector() const { return interrupt_vector() & ~uint16_t(0x0080); }
	uint16_t external_interrupt_vector() const { return interrupt_vector() | uint16_t(0x0080); }

	void set_interrupt_request_line();
	void timer_start(uint8_t value);

	interrupt_req_delegate m_interrupt_req_cb;
	uint8_t m_high;
	uint8_t m_low; // Bit 7 is set to 0 for timer interrupts, 1 for external interrupts
	int32_t m_external_enable;
	int32_t m_timer_enable;

	int32_t m_request_flipflop;

	int32_t m_priority_line;              /* inverted level*/
	int32_t m_external_interrupt_line;    /* inverted level */

	emu_timer *m_timer;

	uint8_t m_value_to_cycle[0x100];
};


// device type definition
DECLARE_DEVICE_TYPE(F3853, f3853_device)

#endif // MAME_MACHINE_F3853_H
