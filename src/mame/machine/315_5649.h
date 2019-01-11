// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega 315-5649

    I/O Controller

***************************************************************************/

#ifndef MAME_MACHINE_315_5649_H
#define MAME_MACHINE_315_5649_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_315_5649_IN_PA_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_in_port_callback<0>(DEVCB_##_devcb);

#define MCFG_315_5649_OUT_PA_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_out_port_callback<0>(DEVCB_##_devcb);

#define MCFG_315_5649_IN_PB_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_in_port_callback<1>(DEVCB_##_devcb);

#define MCFG_315_5649_OUT_PB_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_out_port_callback<1>(DEVCB_##_devcb);

#define MCFG_315_5649_IN_PC_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_in_port_callback<2>(DEVCB_##_devcb);

#define MCFG_315_5649_OUT_PC_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_out_port_callback<2>(DEVCB_##_devcb);

#define MCFG_315_5649_IN_PD_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_in_port_callback<3>(DEVCB_##_devcb);

#define MCFG_315_5649_OUT_PD_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_out_port_callback<3>(DEVCB_##_devcb);

#define MCFG_315_5649_IN_PE_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_in_port_callback<4>(DEVCB_##_devcb);

#define MCFG_315_5649_OUT_PE_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_out_port_callback<4>(DEVCB_##_devcb);

#define MCFG_315_5649_IN_PF_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_in_port_callback<5>(DEVCB_##_devcb);

#define MCFG_315_5649_OUT_PF_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_out_port_callback<5>(DEVCB_##_devcb);

#define MCFG_315_5649_IN_PG_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_in_port_callback<6>(DEVCB_##_devcb);

#define MCFG_315_5649_OUT_PG_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_out_port_callback<6>(DEVCB_##_devcb);

#define MCFG_315_5649_AN0_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_an_port_callback<0>(DEVCB_##_devcb);

#define MCFG_315_5649_AN1_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_an_port_callback<1>(DEVCB_##_devcb);

#define MCFG_315_5649_AN2_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_an_port_callback<2>(DEVCB_##_devcb);

#define MCFG_315_5649_AN3_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_an_port_callback<3>(DEVCB_##_devcb);

#define MCFG_315_5649_AN4_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_an_port_callback<4>(DEVCB_##_devcb);

#define MCFG_315_5649_AN5_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_an_port_callback<5>(DEVCB_##_devcb);

#define MCFG_315_5649_AN6_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_an_port_callback<6>(DEVCB_##_devcb);

#define MCFG_315_5649_AN7_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_an_port_callback<7>(DEVCB_##_devcb);

#define MCFG_315_5649_SERIAL_CH1_READ_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_serial_rd_callback<0>(DEVCB_##_devcb);

#define MCFG_315_5649_SERIAL_CH1_WRITE_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_serial_wr_callback<0>(DEVCB_##_devcb);

#define MCFG_315_5649_SERIAL_CH2_READ_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_serial_rd_callback<1>(DEVCB_##_devcb);

#define MCFG_315_5649_SERIAL_CH2_WRITE_CB(_devcb) \
	devcb = &downcast<sega_315_5649_device &>(*device).set_serial_wr_callback<1>(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class sega_315_5649_device : public device_t
{
public:
	// construction/destruction
	sega_315_5649_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <int N, class Object> devcb_base &set_in_port_callback(Object &&cb)
	{ return m_in_port_cb[N].set_callback(std::forward<Object>(cb)); }

	template <int N, class Object> devcb_base &set_out_port_callback(Object &&cb)
	{ return m_out_port_cb[N].set_callback(std::forward<Object>(cb)); }

	template <int N, class Object> devcb_base &set_an_port_callback(Object &&cb)
	{ return m_an_port_cb[N].set_callback(std::forward<Object>(cb)); }

	template <int N, class Object> devcb_base &set_serial_rd_callback(Object &&cb)
	{ return m_serial_rd_cb[N].set_callback(std::forward<Object>(cb)); }

	template <int N, class Object> devcb_base &set_serial_wr_callback(Object &&cb)
	{ return m_serial_wr_cb[N].set_callback(std::forward<Object>(cb)); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// callbacks
	devcb_read8 m_in_port_cb[7];
	devcb_write8 m_out_port_cb[7];
	devcb_read8 m_an_port_cb[8];
	devcb_read8 m_serial_rd_cb[2];
	devcb_write8 m_serial_wr_cb[2];

	uint8_t m_port_value[7];
	uint8_t m_port_config;
	int m_analog_channel;
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_315_5649, sega_315_5649_device)

#endif // MAME_MACHINE_315_5649_H
