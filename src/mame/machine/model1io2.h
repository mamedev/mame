// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Sega Model 1 I/O Board (Advanced)

***************************************************************************/

#ifndef MAME_MACHINE_MODEL1IO2_H
#define MAME_MACHINE_MODEL1IO2_H

#pragma once

#include "machine/eepromser.h"
#include "video/hd44780.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_MODEL1IO2_READ_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_read_callback(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_WRITE_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_write_callback(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_IN0_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_in_callback<0>(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_IN1_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_in_callback<1>(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_IN2_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_in_callback<2>(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_DRIVE_READ_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_drive_read_callback(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_DRIVE_WRITE_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_drive_write_callback(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_AN0_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_an_callback<0>(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_AN1_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_an_callback<1>(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_AN2_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_an_callback<2>(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_AN3_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_an_callback<3>(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_AN4_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_an_callback<4>(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_AN5_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_an_callback<5>(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_AN6_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_an_callback<6>(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_AN7_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_an_callback<7>(DEVCB_##_devcb);

#define MCFG_MODEL1IO2_OUTPUT_CB(_devcb) \
	devcb = &downcast<model1io2_device &>(*device).set_output_callback(DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class model1io2_device : public device_t
{
public:
	// construction/destruction
	model1io2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <class Object> devcb_base &set_read_callback(Object &&cb)
	{ return m_read_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_write_callback(Object &&cb)
	{ return m_write_cb.set_callback(std::forward<Object>(cb)); }

	template <int Index, class Object> devcb_base &set_in_callback(Object &&cb)
	{ return m_in_cb[Index].set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_drive_read_callback(Object &&cb)
	{ return m_drive_read_cb.set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_drive_write_callback(Object &&cb)
	{ return m_drive_write_cb.set_callback(std::forward<Object>(cb)); }

	template <int Index, class Object> devcb_base &set_an_callback(Object &&cb)
	{ return m_an_cb[Index].set_callback(std::forward<Object>(cb)); }

	template <class Object> devcb_base &set_output_callback(Object &&cb)
	{ return m_output_cb.set_callback(std::forward<Object>(cb)); }

	void mem_map(address_map &map);
	void io_map(address_map &map);

	DECLARE_PALETTE_INIT(lcd);
	HD44780_PIXEL_UPDATE(lcd_pixel_update);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

private:
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<hd44780_device> m_lcd;
	output_finder<> m_led_comm_err;

	DECLARE_READ8_MEMBER(io_r);
	DECLARE_WRITE8_MEMBER(io_w);
	DECLARE_READ8_MEMBER(io_pa_r);
	DECLARE_READ8_MEMBER(io_pb_r);
	DECLARE_READ8_MEMBER(io_pc_r);
	DECLARE_WRITE8_MEMBER(io_pd_w);
	DECLARE_READ8_MEMBER(io_pe_r);
	DECLARE_WRITE8_MEMBER(io_pe_w);
	DECLARE_WRITE8_MEMBER(io_pf_w);
	DECLARE_WRITE8_MEMBER(io_pg_w);
	DECLARE_READ8_MEMBER(fpga_r);
	DECLARE_WRITE8_MEMBER(fpga_w);

	ioport_value analog0_r();
	ioport_value analog1_r();
	ioport_value analog2_r();
	ioport_value analog3_r();

	devcb_read8 m_read_cb;
	devcb_write8 m_write_cb;
	devcb_read8 m_in_cb[3];
	devcb_read8 m_drive_read_cb;
	devcb_write8 m_drive_write_cb;
	devcb_read8 m_an_cb[8];
	devcb_write8 m_output_cb;

	bool m_secondary_controls;
	uint8_t m_lcd_data;
};

// device type definition
DECLARE_DEVICE_TYPE(SEGA_MODEL1IO2, model1io2_device)

#endif // MAME_MACHINE_MODEL1IO2_H
