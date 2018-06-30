// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega 315-5338A

    I/O Controller

    Custom 100-pin QFP LSI. Supports 7 8-bit input/output ports and can
    directly interact with dual port RAM. Also supports a master/slave
    configuration where one controller acts as master and sends commands
    and data over a serial link to another controller.

    TODO:
    - Serial
    - Slave mode
    - and probably lots more

***************************************************************************/

#ifndef MAME_MACHINE_315_5338A_H
#define MAME_MACHINE_315_5338A_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_315_5338A_READ_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_read_callback(DEVCB_##_devcb);

#define MCFG_315_5338A_WRITE_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_write_callback(DEVCB_##_devcb);

#define MCFG_315_5338A_IN_PA_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_in_callback<0>(DEVCB_##_devcb);

#define MCFG_315_5338A_OUT_PA_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_out_callback<0>(DEVCB_##_devcb);

#define MCFG_315_5338A_IN_PB_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_in_callback<1>(DEVCB_##_devcb);

#define MCFG_315_5338A_OUT_PB_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_out_callback<1>(DEVCB_##_devcb);

#define MCFG_315_5338A_IN_PC_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_in_callback<2>(DEVCB_##_devcb);

#define MCFG_315_5338A_OUT_PC_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_out_callback<2>(DEVCB_##_devcb);

#define MCFG_315_5338A_IN_PD_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_in_callback<3>(DEVCB_##_devcb);

#define MCFG_315_5338A_OUT_PD_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_out_callback<3>(DEVCB_##_devcb);

#define MCFG_315_5338A_IN_PE_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_in_callback<4>(DEVCB_##_devcb);

#define MCFG_315_5338A_OUT_PE_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_out_callback<4>(DEVCB_##_devcb);

#define MCFG_315_5338A_IN_PF_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_in_callback<5>(DEVCB_##_devcb);

#define MCFG_315_5338A_OUT_PF_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_out_callback<5>(DEVCB_##_devcb);

#define MCFG_315_5338A_IN_PG_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_in_callback<6>(DEVCB_##_devcb);

#define MCFG_315_5338A_OUT_PG_CB(_devcb) \
	devcb = &downcast<sega_315_5338a_device &>(*device).set_out_callback<6>(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sega_315_5338a_device : public device_t
{
public:
	// construction/destruction
	sega_315_5338a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <class Object> devcb_base &set_read_callback(Object &&cb)
	{ return m_read_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_write_callback(Object &&cb)
	{ return m_write_cb.set_callback(std::forward<Object>(cb)); }

	template <int Port, class Object> devcb_base &set_out_callback(Object &&cb)
	{ return m_out_port_cb[Port].set_callback(std::forward<Object>(cb)); }

	template <int Port, class Object> devcb_base &set_in_callback(Object &&cb)
	{ return m_in_port_cb[Port].set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// callbacks
	devcb_read8 m_read_cb;
	devcb_write8 m_write_cb;
	devcb_read8 m_in_port_cb[7];
	devcb_write8 m_out_port_cb[7];

	uint8_t m_port_value[7];
	uint8_t m_port_config;
	uint8_t m_serial_output;
	uint16_t m_address;
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_315_5338A, sega_315_5338a_device)

#endif // MAME_MACHINE_315_5338A_H
