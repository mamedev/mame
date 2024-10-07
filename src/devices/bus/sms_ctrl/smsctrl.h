// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/**********************************************************************

    Sega 7-bit I/O port emulation

    Pin                  1      2      3      4      5      6      7      8      9     10
                        Up    Down   Left   Right          TL     TH            TR
                                                           TxD                  RXD
                        PC0    PC1    PC2    PC3           PC4    PC6           PC5
    SC-3000              I      I      I      I     N/C     I     N/C    GND     I
    SG-1000              I      I      I      I     N/C     I     GND    GND     I
    SG-1000 Mark III     I      I      I      I     +5V     I     GND    GND     I
    Master System        I      I      I      I     +5V     I     I/O    GND    I/O
    Mega Drive          I/O    I/O    I/O    I/O    +5V    I/O    I/O    GND    I/O
    Game Gear           I/O    I/O    I/O    I/O    +5V    I/O    I/O    GND    I/O    N/C

    * Male DE-9 connector on most systems.
    * Female DE-9 connector for Mega Drive EXP port.
    * 10-pin tongue connector on Game Gear (Hoshiden HDC-0492).
    * TH/PC6 can latch the VDP's horizontal counter and/or raise
      interrupts on some systems.
    * Mega Drive and Game Gear can route TL/PC4 and TR/PC5 to a
      UART for serial communication.

**********************************************************************/
#ifndef MAME_BUS_SMS_CTRL_SMSCTRL_H
#define MAME_BUS_SMS_CTRL_SMSCTRL_H

#pragma once

#include "screen.h"

#include <utility>


//**************************************************************************
//  FORWARD DECLARATIONS
//**************************************************************************

class device_sms_control_interface;



//**************************************************************************
//  CLASS DECLARATIONS
//**************************************************************************

class sms_control_port_device : public device_t, public device_single_card_slot_interface<device_sms_control_interface>
{
public:
	template <typename T>
	sms_control_port_device(machine_config const &mconfig, char const *tag, device_t *owner, T &&opts, char const *dflt) :
		sms_control_port_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	sms_control_port_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
	virtual ~sms_control_port_device();

	auto th_handler() { return m_th_handler.bind(); } // 0 for pulled low, 1 for pulled high or high impedance
	template <typename T> void set_screen(T &&tag) { m_screen.set_tag(std::forward<T>(tag)); }

	// Bit  Pin  Signal
	// 0    1    Up
	// 1    2    Down
	// 2    3    Left
	// 3    4    Right
	// 4    6    TL
	// 5    9    TR
	//
	// return bits set for pulled high or high impedance
	// return bits clear for pulled low
	u8 in_r();

	// Bit  Pin  Signal
	// 0    1    Up
	// 1    2    Down
	// 2    3    Left
	// 3    4    Right
	// 4    6    TL
	// 5    9    TR
	// 6    7    TH
	//
	// mem_mask bits set for lines driven by low impedance output
	// mem_mask bits clear for lines with high-impedance pull-up/pull-down
	void out_w(u8 data, u8 mem_mask);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	optional_device<screen_device> m_screen;
	devcb_write_line m_th_handler;

	device_sms_control_interface *m_controller;

	friend class device_sms_control_interface;
};


class device_sms_control_interface : public device_interface
{
public:
	virtual ~device_sms_control_interface();

	virtual u8 in_r();
	virtual void out_w(u8 data, u8 mem_mask);

protected:
	device_sms_control_interface(machine_config const &mconfig, device_t &device);

	template <typename T> void configure_screen(T &&act) { if (m_port) act(std::as_const(m_port->m_screen)); }

	screen_device *screen() const { return m_port ? m_port->m_screen.target() : nullptr; }
	void th_w(int state) { if (m_port) m_port->m_th_handler(state); }

private:
	sms_control_port_device *const m_port;
};



//**************************************************************************
//  INLINE MEMBER FUNCTIONS
//**************************************************************************

inline u8 sms_control_port_device::in_r()
{
	return m_controller ? m_controller->in_r() : 0xff;
}


inline void sms_control_port_device::out_w(u8 data, u8 mem_mask)
{
	if (m_controller)
		m_controller->out_w(data, mem_mask);
}



//**************************************************************************
//  DEVICE TYPE DECLARATIONS
//**************************************************************************

DECLARE_DEVICE_TYPE(SMS_CONTROL_PORT, sms_control_port_device)

#endif // MAME_BUS_SMS_CTRL_SMSCTRL_H
