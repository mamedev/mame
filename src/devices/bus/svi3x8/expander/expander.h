// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    SVI 318/328 Expander Slot

    50-pin slot

     1  +5V         2  /CNTRL2
     3  +12V        4  -12V
     5  /CNTRL1     6  /WAIT
     7  /RST        8  CPUCLK
     9  A15        10  A14
    11  A13        12  A12
    13  A11        14  A10
    15  A9         16  A8
    17  A7         18  A6
    19  A5         20  A4
    21  A3         22  A2
    23  A1         24  A0
    25  /RFSH      26  /EXCSR
    27  /M1        28  /EXCSW
    29  /WR        30  /MREQ
    31  /IORQ      32  /RD
    33  D0         34  D1
    35  D2         36  D3
    37  D4         38  D5
    39  D6         40  D7
    41  CSOUND     42  /INT
    43  /RAMDIS    44  /ROMDIS
    45  /BK32      46  /BK31
    47  /BK22      48  /BK21
    49  GND        50  GND

***************************************************************************/

#pragma once

#ifndef __SVI3X8_EXPANDER_H__
#define __SVI3X8_EXPANDER_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SVI_EXPANDER_BUS_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SVI_EXPANDER, 0) \
	MCFG_DEVICE_SLOT_INTERFACE(svi_expander_modules, NULL, false)
#define MCFG_SVI_EXPANDER_INT_HANDLER(_devcb) \
	devcb = &svi_expander_device::set_int_handler(*device, DEVCB_##_devcb);

#define MCFG_SVI_EXPANDER_ROMDIS_HANDLER(_devcb) \
	devcb = &svi_expander_device::set_romdis_handler(*device, DEVCB_##_devcb);

#define MCFG_SVI_EXPANDER_RAMDIS_HANDLER(_devcb) \
	devcb = &svi_expander_device::set_ramdis_handler(*device, DEVCB_##_devcb);

#define MCFG_SVI_EXPANDER_CTRL1_HANDLER(_devcb) \
	devcb = &svi_expander_device::set_ctrl1_handler(*device, DEVCB_##_devcb);

#define MCFG_SVI_EXPANDER_CTRL2_HANDLER(_devcb) \
	devcb = &svi_expander_device::set_ctrl2_handler(*device, DEVCB_##_devcb);

#define MCFG_SVI_EXPANDER_EXCSR_HANDLER(_devcb) \
	devcb = &svi_expander_device::set_excsr_handler(*device, DEVCB_##_devcb);

#define MCFG_SVI_EXPANDER_EXCSW_HANDLER(_devcb) \
	devcb = &svi_expander_device::set_excsw_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class device_svi_expander_interface;

// ======================> svi_expander_device

class svi_expander_device : public device_t, public device_slot_interface
{
public:
	// construction/destruction
	svi_expander_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	virtual ~svi_expander_device();

	// callbacks
	template<class _Object> static devcb_base &set_int_handler(device_t &device, _Object object)
		{ return downcast<svi_expander_device &>(device).m_int_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_romdis_handler(device_t &device, _Object object)
		{ return downcast<svi_expander_device &>(device).m_romdis_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_ramdis_handler(device_t &device, _Object object)
		{ return downcast<svi_expander_device &>(device).m_ramdis_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_ctrl1_handler(device_t &device, _Object object)
		{ return downcast<svi_expander_device &>(device).m_ctrl1_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_ctrl2_handler(device_t &device, _Object object)
		{ return downcast<svi_expander_device &>(device).m_ctrl2_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_excsr_handler(device_t &device, _Object object)
		{ return downcast<svi_expander_device &>(device).m_excsr_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_excsw_handler(device_t &device, _Object object)
		{ return downcast<svi_expander_device &>(device).m_excsw_handler.set_callback(object); }

	// called from cart device
	DECLARE_WRITE_LINE_MEMBER( int_w ) { m_int_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( romdis_w ) { m_romdis_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( ramdis_w ) { m_ramdis_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( ctrl1_w ) { m_ctrl1_handler(state); }
	DECLARE_WRITE_LINE_MEMBER( ctrl2_w ) { m_ctrl2_handler(state); }

	DECLARE_READ8_MEMBER( excs_r ) { return m_excsr_handler(space, offset); }
	DECLARE_WRITE8_MEMBER( excs_w ) { m_excsw_handler(space, offset, data); }

	// called from host
	DECLARE_READ8_MEMBER( mreq_r );
	DECLARE_WRITE8_MEMBER( mreq_w );
	DECLARE_READ8_MEMBER( iorq_r );
	DECLARE_WRITE8_MEMBER( iorq_w );

	DECLARE_WRITE_LINE_MEMBER( bk21_w );
	DECLARE_WRITE_LINE_MEMBER( bk22_w );
	DECLARE_WRITE_LINE_MEMBER( bk31_w );
	DECLARE_WRITE_LINE_MEMBER( bk32_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	device_svi_expander_interface *m_module;

	devcb_write_line m_int_handler;
	devcb_write_line m_romdis_handler;
	devcb_write_line m_ramdis_handler;
	devcb_write_line m_ctrl1_handler;
	devcb_write_line m_ctrl2_handler;

	devcb_read8 m_excsr_handler;
	devcb_write8 m_excsw_handler;
};

// ======================> device_svi_expander_interface

class device_svi_expander_interface : public device_slot_card_interface
{
public:
	// construction/destruction
	device_svi_expander_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_svi_expander_interface();

	virtual DECLARE_READ8_MEMBER( mreq_r ) { return 0xff; };
	virtual DECLARE_WRITE8_MEMBER( mreq_w ){};
	virtual DECLARE_READ8_MEMBER( iorq_r ) { return 0xff; };
	virtual DECLARE_WRITE8_MEMBER( iorq_w ){};

	virtual void bk21_w(int state) {};
	virtual void bk22_w(int state) {};
	virtual void bk31_w(int state) {};
	virtual void bk32_w(int state) {};

protected:
	svi_expander_device *m_expander;
};

// device type definition
extern const device_type SVI_EXPANDER;

// include here so drivers don't need to
#include "modules.h"

#endif // __SVI3X8_EXPANDER_H__
