// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    GAYLE

    Gate array used in the Amiga 600 and Amiga 1200 computers.

***************************************************************************/

#pragma once

#ifndef __GAYLE_H__
#define __GAYLE_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_GAYLE_ADD(_tag, _clock, _id) \
	MCFG_DEVICE_ADD(_tag, GAYLE, _clock) \
	gayle_device::set_id(*device, _id);

#define MCFG_GAYLE_INT2_HANDLER(_devcb) \
	devcb = &gayle_device::set_int2_handler(*device, DEVCB_##_devcb);

#define MCFG_GAYLE_CS0_READ_HANDLER(_devcb) \
	devcb = &gayle_device::set_cs0_read_handler(*device, DEVCB_##_devcb);

#define MCFG_GAYLE_CS0_WRITE_HANDLER(_devcb) \
	devcb = &gayle_device::set_cs0_write_handler(*device, DEVCB_##_devcb);

#define MCFG_GAYLE_CS1_READ_HANDLER(_devcb) \
	devcb = &gayle_device::set_cs1_read_handler(*device, DEVCB_##_devcb);

#define MCFG_GAYLE_CS1_WRITE_HANDLER(_devcb) \
	devcb = &gayle_device::set_cs1_write_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> gayle_device

class gayle_device : public device_t
{
public:
	// construction/destruction
	gayle_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// callbacks
	template<class _Object> static devcb_base &set_int2_handler(device_t &device, _Object object)
		{ return downcast<gayle_device &>(device).m_int2_w.set_callback(object); }

	template<class _Object> static devcb_base &set_cs0_read_handler(device_t &device, _Object object)
		{ return downcast<gayle_device &>(device).m_cs0_read.set_callback(object); }

	template<class _Object> static devcb_base &set_cs0_write_handler(device_t &device, _Object object)
		{ return downcast<gayle_device &>(device).m_cs0_write.set_callback(object); }

	template<class _Object> static devcb_base &set_cs1_read_handler(device_t &device, _Object object)
		{ return downcast<gayle_device &>(device).m_cs1_read.set_callback(object); }

	template<class _Object> static devcb_base &set_cs1_write_handler(device_t &device, _Object object)
		{ return downcast<gayle_device &>(device).m_cs1_write.set_callback(object); }

	// interface
	DECLARE_WRITE_LINE_MEMBER( ide_interrupt_w );

	DECLARE_READ16_MEMBER( gayle_r );
	DECLARE_WRITE16_MEMBER( gayle_w );
	DECLARE_READ16_MEMBER( gayle_id_r );
	DECLARE_WRITE16_MEMBER( gayle_id_w );

	// inline configuration
	static void set_id(device_t &device, UINT8 id);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	enum
	{
		GAYLE_CS = 0,   // interrupt status
		GAYLE_IRQ,      // interrupt change
		GAYLE_INTEN,    // interrupt enable register
		GAYLE_CFG       // config register
	};

	devcb_write_line m_int2_w;

	devcb_read16 m_cs0_read;
	devcb_write16 m_cs0_write;
	devcb_read16 m_cs1_read;
	devcb_write16 m_cs1_write;

	UINT8 m_gayle_id;
	int m_gayle_id_count;
	UINT8 m_gayle_reg[4];
};

// device type definition
extern const device_type GAYLE;

#endif
