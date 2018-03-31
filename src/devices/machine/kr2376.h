// license:BSD-3-Clause
// copyright-holders:Roberto Lavarone
/**********************************************************************

    SMC KR2376 Keyboard Encoder emulation

**********************************************************************
                            _____   _____
                   Vcc   1 |*    \_/     | 40  Frequency Control A
   Frequency Control B   2 |             | 39  X0
   Frequency Control C   3 |             | 38  X1
           Shift Input   4 |             | 37  X2
         Control Input   5 |             | 36  X3
   Parity Invert Input   6 |             | 35  X4
         Parity Output   7 |             | 34  X5
        Data Output B8   8 |             | 33  X6
        Data Output B7   9 |             | 32  X7
        Data Output B6  10 |   KR2376    | 31  Y0
        Data Output B5  11 |             | 30  Y1
        Data Output B4  12 |             | 29  Y2
        Data Output B3  13 |             | 28  Y3
        Data Output B2  14 |             | 27  Y4
        Data Output B1  15 |             | 26  Y5
         Strobe Output  16 |             | 25  Y6
                Ground  17 |             | 24  Y7
                   Vgg  18 |             | 23  Y8
  Strobe Control Input  19 |             | 22  Y9
          Invert Input  20 |_____________| 21  Y10

**********************************************************************/

#ifndef MAME_MACHINE_KR2376_H
#define MAME_MACHINE_KR2376_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_KR2376_MATRIX_X0(_cb) \
	devcb = &downcast<kr2376_device &>(*device).set_x_cb<0>(DEVCB_##_cb);
#define MCFG_KR2376_MATRIX_X1(_cb) \
	devcb = &downcast<kr2376_device &>(*device).set_x_cb<1>(DEVCB_##_cb);
#define MCFG_KR2376_MATRIX_X2(_cb) \
	devcb = &downcast<kr2376_device &>(*device).set_x_cb<2>(DEVCB_##_cb);
#define MCFG_KR2376_MATRIX_X3(_cb) \
	devcb = &downcast<kr2376_device &>(*device).set_x_cb<3>(DEVCB_##_cb);
#define MCFG_KR2376_MATRIX_X4(_cb) \
	devcb = &downcast<kr2376_device &>(*device).set_x_cb<4>(DEVCB_##_cb);
#define MCFG_KR2376_MATRIX_X5(_cb) \
	devcb = &downcast<kr2376_device &>(*device).set_x_cb<5>(DEVCB_##_cb);
#define MCFG_KR2376_MATRIX_X6(_cb) \
	devcb = &downcast<kr2376_device &>(*device).set_x_cb<6>(DEVCB_##_cb);
#define MCFG_KR2376_MATRIX_X7(_cb) \
	devcb = &downcast<kr2376_device &>(*device).set_x_cb<7>(DEVCB_##_cb);
#define MCFG_KR2376_SHIFT_CB(_cb) \
	devcb = &downcast<kr2376_device &>(*device).set_shift_cb(DEVCB_##_cb);
#define MCFG_KR2376_CONTROL_CB(_cb) \
	devcb = &downcast<kr2376_device &>(*device).set_control_cb(DEVCB_##_cb);
#define MCFG_KR2376_STROBE_CB(_cb) \
	devcb = &downcast<kr2376_device &>(*device).set_strobe_cb(DEVCB_##_cb);

class kr2376_device : public device_t
{
public:
	/*
	 * Input pins
	 */
	enum input_pin_t
	{
		KR2376_DSII=20,         /* DSII  - Pin 20 - Data & Strobe Invert Input */
		KR2376_PII=6            /* PII   - Pin  6 - Parity Invert Input */
	};

	enum output_pin_t
	{
		KR2376_SO=16,           /* SO    - Pin 16 - Strobe Output */
		KR2376_PO=7             /* PO    - Pin  7 - Parity Output */
	};

	kr2376_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	template <unsigned N, class Object> devcb_base &set_x_cb(Object &&cb) { return m_read_x[N].set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_shift_cb(Object &&cb) { return m_read_shift.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_control_cb(Object &&cb) { return m_read_control.set_callback(std::forward<Object>(cb)); }
	template <class Object> devcb_base &set_strobe_cb(Object &&cb) { return m_write_strobe.set_callback(std::forward<Object>(cb)); }

	/* keyboard data */
	DECLARE_READ8_MEMBER( data_r );

	/* Set an input pin */
	void set_input_pin( input_pin_t pin, int data );

	/* Get an output pin */
	int get_output_pin( output_pin_t pin );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual uint8_t key_codes(int mode, int x, int y) { return 0x00; }

private:
	// internal state
	int m_pins[41];

	int m_ring11;                     /* sense input scan counter */
	int m_ring8;                      /* drive output scan counter */

	int m_strobe;                     /* strobe output */
	int m_strobe_old;
	int m_parity;
	int m_data;

	/* timers */
	emu_timer *m_scan_timer;          /* keyboard scan timer */
	devcb_read16 m_read_x[8];
	devcb_read_line m_read_shift, m_read_control;
	devcb_write_line m_write_strobe;

	enum
	{
		TIMER_SCAN_TICK
	};

	void change_output_lines();
	void clock_scan_counters();
	void detect_keypress();
};

class kr2376_st_device : public kr2376_device
{
public:
	// construction/destruction
	kr2376_st_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	virtual uint8_t key_codes(int mode, int x, int y) override;
};

//class kr2376_12_device : public kr2376_device
//{
//public:
//  // construction/destruction
//  kr2376_12_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
//protected:
//  virtual uint8_t key_codes(int mode, int x, int y) override;
//};

DECLARE_DEVICE_TYPE(KR2376_ST, kr2376_st_device)
//DECLARE_DEVICE_TYPE(KR2376_12, kr2376_12_device)

#endif // MAME_MACHINE_KR2376_H
