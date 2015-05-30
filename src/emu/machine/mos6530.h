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

#ifndef __MIOT6530_H__
#define __MIOT6530_H__


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

struct mos6530_port
{
	UINT8 m_in;
	UINT8 m_out;
	UINT8 m_ddr;
};

/***************************************************************************
    MACROS / CONSTANTS
***************************************************************************/

class mos6530_device : public device_t
{
public:
	mos6530_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~mos6530_device() {}

	template<class _Object> static devcb_base &set_in_pa_callback(device_t &device, _Object object) { return downcast<mos6530_device &>(device).m_in_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pa_callback(device_t &device, _Object object) { return downcast<mos6530_device &>(device).m_out_pa_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_in_pb_callback(device_t &device, _Object object) { return downcast<mos6530_device &>(device).m_in_pb_cb.set_callback(object); }
	template<class _Object> static devcb_base &set_out_pb_callback(device_t &device, _Object object) { return downcast<mos6530_device &>(device).m_out_pb_cb.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	UINT8 porta_in_get();
	UINT8 portb_in_get();

	UINT8 porta_out_get();
	UINT8 portb_out_get();

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	// internal state
	devcb_read8    m_in_pa_cb;
	devcb_write8   m_out_pa_cb;

	devcb_read8    m_in_pb_cb;
	devcb_write8   m_out_pb_cb;

	mos6530_port    m_port[2];

	UINT8           m_irqstate;
	UINT8           m_irqenable;

	UINT8           m_timershift;
	UINT8           m_timerstate;
	emu_timer *     m_timer;

	UINT32          m_clock;

	void update_irqstate();
	UINT8 get_timer();

	void porta_in_set(UINT8 data, UINT8 mask);
	void portb_in_set(UINT8 data, UINT8 mask);

	enum
	{
		TIMER_END_CALLBACK
	};
};

extern const device_type MOS6530;


#define MCFG_MOS6530_IN_PA_CB(_devcb) \
	devcb = &mos6530_device::set_in_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_MOS6530_OUT_PA_CB(_devcb) \
	devcb = &mos6530_device::set_out_pa_callback(*device, DEVCB_##_devcb);

#define MCFG_MOS6530_IN_PB_CB(_devcb) \
	devcb = &mos6530_device::set_in_pb_callback(*device, DEVCB_##_devcb);

#define MCFG_MOS6530_OUT_PB_CB(_devcb) \
	devcb = &mos6530_device::set_out_pb_callback(*device, DEVCB_##_devcb);


#endif
