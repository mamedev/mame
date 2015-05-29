// license:GPL-2.0+
// copyright-holders:Dirk Best
/***************************************************************************

    CR-511-B CD-ROM drive

    CD-ROM drive with a custom MKE/Panasonic interface as used in the
    Commodore CDTV and early SoundBlaster cards.

     1 _RESET      2  GND
     3  EFFK       4  SCCK
     5  SBCP       6  SCOR
     7  GND        8  GND
     9  C16M      10  GND
    11 _XAEN      12  GND
    13  EMPASIS   14  D0
    15  LRCK      16  DSCK
    17 _MUTE      18  GND
    19 _BUSY      20 _STCH
    21 _ENABLE    22  DRQ
    23 _HWR       24  GND
    25 _DTEN      26 _HRD
    27 _STEN      28 _CMD
    29 _EOP       30  GND
    31  DB7       32  GND
    33  DB6       34  DB5
    35  DB4       36  GND
    37  DB3       38  DB2
    39  DB1       40  DB0

***************************************************************************/

#pragma once

#ifndef __CR511B_H__
#define __CR511B_H__

#include "emu.h"
#include "imagedev/chd_cd.h"
#include "sound/cdda.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CR511B_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, CR511B, 0)
#define MCFG_CR511B_STCH_HANDLER(_devcb) \
	devcb = &cr511b_device::set_stch_handler(*device, DEVCB_##_devcb);

#define MCFG_CR511B_STEN_HANDLER(_devcb) \
	devcb = &cr511b_device::set_sten_handler(*device, DEVCB_##_devcb);

#define MCFG_CR511B_DRQ_HANDLER(_devcb) \
	devcb = &cr511b_device::set_drq_handler(*device, DEVCB_##_devcb);

#define MCFG_CR511B_DTEN_HANDLER(_devcb) \
	devcb = &cr511b_device::set_dten_handler(*device, DEVCB_##_devcb);

#define MCFG_CR511B_SCOR_HANDLER(_devcb) \
	devcb = &cr511b_device::set_scor_handler(*device, DEVCB_##_devcb);

#define MCFG_CR511B_XAEN_HANDLER(_devcb) \
	devcb = &cr511b_device::set_xaen_handler(*device, DEVCB_##_devcb);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cr511b_device

class cr511b_device : public device_t
{
public:
	// construction/destruction
	cr511b_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// callbacks
	template<class _Object> static devcb_base &set_stch_handler(device_t &device, _Object object)
		{ return downcast<cr511b_device &>(device).m_stch_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_sten_handler(device_t &device, _Object object)
		{ return downcast<cr511b_device &>(device).m_sten_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_drq_handler(device_t &device, _Object object)
		{ return downcast<cr511b_device &>(device).m_drq_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_dten_handler(device_t &device, _Object object)
		{ return downcast<cr511b_device &>(device).m_dten_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_scor_handler(device_t &device, _Object object)
		{ return downcast<cr511b_device &>(device).m_scor_handler.set_callback(object); }

	template<class _Object> static devcb_base &set_xaen_handler(device_t &device, _Object object)
		{ return downcast<cr511b_device &>(device).m_xaen_handler.set_callback(object); }

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER ( write );

	DECLARE_WRITE_LINE_MEMBER( enable_w );
	DECLARE_WRITE_LINE_MEMBER( cmd_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	enum
	{
		STATUS_DOOR_CLOSED = 0x80,
		STATUS_MEDIA       = 0x40,
		STATUS_MOTOR       = 0x20,
		STATUS_ERROR       = 0x10,
		STATUS_SUCCESS     = 0x08,
		STATUS_PLAYING     = 0x04,
		STATUS_DOOR_LOCKED = 0x02,
		STATUS_READY       = 0x01
	};

	required_device<cdrom_image_device> m_cdrom;
	required_device<cdda_device> m_cdda;

	devcb_write_line m_stch_handler;
	devcb_write_line m_sten_handler;
	devcb_write_line m_drq_handler;
	devcb_write_line m_dten_handler;
	devcb_write_line m_scor_handler;
	devcb_write_line m_xaen_handler;

	emu_timer *m_frame_timer;

	//bool m_motor;

	// state of lines
	int m_enabled;
	int m_cmd;

	// data transfer
	//UINT8 m_sector_buffer[CD_MAX_SECTOR_DATA];
};

// device type definition
extern const device_type CR511B;

#endif
