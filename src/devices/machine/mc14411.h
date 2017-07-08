// license:BSD-3-Clause
// copyright-holders:Joakim Larsson Edstrom
/**********************************************************************
*
*   Motorola MC14411 - bit rate generator. It utilizes a frequency divider 
*   network to provide a wide range of output frequencies. A crystal controlled
*   oscillator is the clock source for the network. A 2 bit adress is used to 
*   select one of four multiple output clock rates.
*
*   - Single 5V sower supply
*   - Internal Oscillator Crystal Controlled for stability  (1.8432 MHz)
*   - Sixteen Different Output Clock Rates
*   - 50% Output Duty Cycle
*   - Programmable Time Bases for one of four Multiple Output Rates
*   - Buffered Output compatible with low power TTL
*   - Noice immunity = 45% of VDD Typlical
*   - Diode protection on All Inputs
*   - External Clock may be applied to pin 21
*   - Internal pullup on reset input
*                           _____   _____
*                    F1  1 |*    \_/     | 24  VDD
*                    F3  2 |             | 23  Rate Select A
*                    F5  3 |             | 22  Rate Select B
*                    F7  4 |             | 21  Xtal In
*                    F8  5 |             | 20  Xtal Out
*                   F10  6 |             | 19  F16
*                    F9  7 |   MC14411   | 18  F15
*                   F11  8 |             | 17  F2
*                   F14  9 |             | 16  F4
*                Reset* 10 |             | 15  F6 
*             Not Used  11 |             | 14  F12
*                  VSS  12 |_____________| 13  F13
*
*
* +----------+-------Output Rates (Hz)-------+
* | Output   | Rate Select B and A pins      |
* | Number   | X64 11| X16 10| X8 01 | X1 00 |
* |----------+-------------------------------+
* | Fl       | 614.4k| 153.6k|  76.8k|  9600 |
* | F2       | 46O.8k| 115.2k|  57.6k|  7200 |
* | F3       | 3O7.2k|  76.8k|  38.4k|  4800 |
* | F4       | 23O.4k|  57.6k|  28.8k|  3600 |
* | F5       | 153.6k|  38.4k|  19.2k|  2400 |
* | F6       | 115.2k|  28.8k|  14.4k|  1800 |
* | F7       |  76.8k|  19.2k|  9600 |  1200 |
* | FS       |  38.4k|  9600 |  4800 |   600 |
* | F9       |  19.2k|  4800 |  2400 |   300 |
* | F1O      |  12.8k|  3200 |  1600 |   200 |
* | Fll      |  9600 |  2400 |  1200 |   150 |
* | F12      | 8613.2| 2153.3| 1076.6|  134.5|
* | F13      | 7035.5| 1758.8|  879.4|  109.9|
* | F14      |  4800 |  1200 |   600 |    75 |
* | F15      | 921.6k| 921.6k| 921.6k| 921.6k|
* | F16      | 1.843M| 1.843M| 1.843M| 1.843M|
* +------------------------------------------+
*   - F16 is a buffered oscillator output
*
* The device is designed to work with 1.843MHz crystal so it is assumed that
* an external clock source attached to pin 21 is also fixed thus not need to
* interface through a callback interface.
**********************************************************************/

#ifndef MAME_MACHINE_MC14411_H
#define MAME_MACHINE_MC14411_H

#pragma once

//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************
#define MCFG_MC14411_ADD(_tag, _clock) MCFG_DEVICE_ADD(_tag, MC14411, _clock)

#define MCFG_MC14411_F1_CB(_devcb)  devcb = &mc14411_device::set_out_fx_cb(*device,  0, DEVCB_##_devcb);
#define MCFG_MC14411_F2_CB(_devcb)  devcb = &mc14411_device::set_out_fx_cb(*device,  1, DEVCB_##_devcb);
#define MCFG_MC14411_F3_CB(_devcb)  devcb = &mc14411_device::set_out_fx_cb(*device,  2, DEVCB_##_devcb);
#define MCFG_MC14411_F4_CB(_devcb)  devcb = &mc14411_device::set_out_fx_cb(*device,  3, DEVCB_##_devcb);
#define MCFG_MC14411_F5_CB(_devcb)  devcb = &mc14411_device::set_out_fx_cb(*device,  4, DEVCB_##_devcb);
#define MCFG_MC14411_F6_CB(_devcb)  devcb = &mc14411_device::set_out_fx_cb(*device,  5, DEVCB_##_devcb);
#define MCFG_MC14411_F7_CB(_devcb)  devcb = &mc14411_device::set_out_fx_cb(*device,  6, DEVCB_##_devcb);
#define MCFG_MC14411_F8_CB(_devcb)  devcb = &mc14411_device::set_out_fx_cb(*device,  7, DEVCB_##_devcb);
#define MCFG_MC14411_F9_CB(_devcb)  devcb = &mc14411_device::set_out_fx_cb(*device,  8, DEVCB_##_devcb);
#define MCFG_MC14411_F10_CB(_devcb) devcb = &mc14411_device::set_out_fx_cb(*device,  9, DEVCB_##_devcb);
#define MCFG_MC14411_F11_CB(_devcb) devcb = &mc14411_device::set_out_fx_cb(*device, 10, DEVCB_##_devcb);
#define MCFG_MC14411_F12_CB(_devcb) devcb = &mc14411_device::set_out_fx_cb(*device, 11, DEVCB_##_devcb);
#define MCFG_MC14411_F13_CB(_devcb) devcb = &mc14411_device::set_out_fx_cb(*device, 12, DEVCB_##_devcb);
#define MCFG_MC14411_F14_CB(_devcb) devcb = &mc14411_device::set_out_fx_cb(*device, 13, DEVCB_##_devcb);
#define MCFG_MC14411_F15_CB(_devcb) devcb = &mc14411_device::set_out_fx_cb(*device, 14, DEVCB_##_devcb);
#define MCFG_MC14411_F16_CB(_devcb) devcb = &mc14411_device::set_out_fx_cb(*device, 15, DEVCB_##_devcb);

#define MCFG_MC14411_RSA 0x01
#define MCFG_MC14411_RSB 0x02

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************
class mc14411_device : public device_t
{
public:
	// construction/destruction
	mc14411_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <class Object> static devcb_base &set_out_fx_cb(device_t &device, int index, Object &&cb) { return downcast<mc14411_device &>(device).m_out_fx_cbs[index].set_callback(std::forward<Object>(cb)); }

	DECLARE_WRITE_LINE_MEMBER(reset_w);
	DECLARE_WRITE8_MEMBER(rate_select_w);
	
protected:
	mc14411_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	// timers
	enum // indexes
	{
		F1 = 0,
		F2 = 1,
		F3 = 2,
		F4 = 3,
		F5 = 4,
		F6 = 5,
		F7 = 6,
		F8 = 7,
		F9 = 8,
		F10 = 9,
		F11 = 10,
		F12 = 11,
		F13 = 12,
		F14 = 13,
		F15 = 14,
		F16 = 15		
	};

	emu_timer *m_fx_timer[16];
	
	enum
	{
		TIMER_ID_RESET = 16
	};
	emu_timer *m_reset_timer;

	// F1-F16 Output line states
	uint32_t m_fx_state[16];

	// divider matrix
	static const int counter_divider[16][4];

	devcb_write_line m_out_fx_cbs[16];

	uint32_t m_divider;	// main divider to use, 0-3 column index into counter_divider 
	uint32_t m_reset;	// Reset line state
};

// device type definition
DECLARE_DEVICE_TYPE(MC14411, mc14411_device)

#endif // MAME_MACHINE_MC14411_H
