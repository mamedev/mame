/***************************************************************************

    SMS OMTI 5100

    license: MAME, GPL-2.0+
    copyright-holders: Dirk Best

    SCSI/SASI Intelligent Data Controller

***************************************************************************/

#pragma once

#ifndef __OMTI5100_H__
#define __OMTI5100_H__

#include "emu.h"
#include "cpu/z8/z8.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_OMTI5100_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, OMTI5100, 0)

#define MCFG_OMTI5100_BSY_HANDLER(_devcb) \
	devcb = &omti5100_device::set_bsy_handler(*device, DEVCB_##_devcb);

#define MCFG_OMTI5100_CD_HANDLER(_devcb) \
	devcb = &omti5100_device::set_cd_handler(*device, DEVCB_##_devcb);

#define MCFG_OMTI5100_IO_HANDLER(_devcb) \
	devcb = &omti5100_device::set_io_handler(*device, DEVCB_##_devcb);

#define MCFG_OMTI5100_REQ_HANDLER(_devcb) \
	devcb = &omti5100_device::set_req_handler(*device, DEVCB_##_devcb);

#define MCFG_OMTI5100_MSG_HANDLER(_devcb) \
	devcb = &omti5100_device::set_msg_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> omti5100_device

class omti5100_device : public device_t
{
public:
	// construction/destruction
	omti5100_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// callbacks
	template<class _Object> static devcb_base &set_bsy_handler(device_t &device, _Object object)
		{ return downcast<omti5100_device &>(device).m_bsy_w.set_callback(object); }

	template<class _Object> static devcb_base &set_cd_handler(device_t &device, _Object object)
		{ return downcast<omti5100_device &>(device).m_cd_w.set_callback(object); }

	template<class _Object> static devcb_base &set_io_handler(device_t &device, _Object object)
		{ return downcast<omti5100_device &>(device).m_io_w.set_callback(object); }

	template<class _Object> static devcb_base &set_req_handler(device_t &device, _Object object)
		{ return downcast<omti5100_device &>(device).m_req_w.set_callback(object); }

	template<class _Object> static devcb_base &set_msg_handler(device_t &device, _Object object)
		{ return downcast<omti5100_device &>(device).m_msg_w.set_callback(object); }

	// data
	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );
	DECLARE_READ_LINE_MEMBER( parity_r );
	DECLARE_WRITE_LINE_MEMBER( parity_w );

	// control
	DECLARE_WRITE_LINE_MEMBER( rst_w );
	DECLARE_WRITE_LINE_MEMBER( sel_w );
	DECLARE_WRITE_LINE_MEMBER( ack_w );

protected:
	// device_t overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_start();
	virtual void device_reset();

private:
//  required_device<z8681_device> m_cpu;

	devcb_write_line m_bsy_w;
	devcb_write_line m_cd_w;
	devcb_write_line m_io_w;
	devcb_write_line m_req_w;
	devcb_write_line m_msg_w;
};

// device type definition
extern const device_type OMTI5100;

#endif // __OMTI5100_H__
