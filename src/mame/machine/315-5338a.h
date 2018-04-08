// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega 315-5338A

    I/O Controller

    Custom 100-pin QFP LSI. Supports 8 analog channels, 3 digital 8-bit
    input ports, 1 digital 8-bit output port and a serial mode (and
    probably more).

	TODO:
	- Serial/remote mode

***************************************************************************/

#ifndef MAME_MACHINE_315_5338A_H
#define MAME_MACHINE_315_5338A_H

#pragma once

#include "emu.h"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_315_5338A_AN0_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_an_callback(DEVCB_##_devcb, 0);

#define MCFG_315_5338A_AN1_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_an_callback(DEVCB_##_devcb, 1);

#define MCFG_315_5338A_AN2_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_an_callback(DEVCB_##_devcb, 2);

#define MCFG_315_5338A_AN3_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_an_callback(DEVCB_##_devcb, 3);

#define MCFG_315_5338A_AN4_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_an_callback(DEVCB_##_devcb, 4);

#define MCFG_315_5338A_AN5_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_an_callback(DEVCB_##_devcb, 5);

#define MCFG_315_5338A_AN6_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_an_callback(DEVCB_##_devcb, 6);

#define MCFG_315_5338A_AN7_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_an_callback(DEVCB_##_devcb, 7);

#define MCFG_315_5338A_DI0_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_di_callback(DEVCB_##_devcb, 0);

#define MCFG_315_5338A_DI1_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_di_callback(DEVCB_##_devcb, 1);

#define MCFG_315_5338A_DI2_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_di_callback(DEVCB_##_devcb, 2);

#define MCFG_315_5338A_DO_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_do_callback(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sega_315_5338a_device : public device_t
{
public:
	// construction/destruction
	sega_315_5338a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <class Object> devcb_base &set_an_callback(Object &&cb, int index)
	{ return m_an_cb[index].set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_di_callback(Object &&cb, int index)
	{ return m_di_cb[index].set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_do_callback(Object &&cb)
	{ return m_do_cb.set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// callbacks
	devcb_read8 m_an_cb[8];
	devcb_read8 m_di_cb[3];
	devcb_write8 m_do_cb;

	uint8_t m_out;
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_315_5338A, sega_315_5338a_device)

#endif // MAME_MACHINE_315_5338A_H
