// license:BSD-3-Clause
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

#ifndef MAME_MACHINE_CR511B_H
#define MAME_MACHINE_CR511B_H

#pragma once

#include "imagedev/cdromimg.h"
#include "sound/cdda.h"

class cr511b_device : public device_t
{
public:
	// construction/destruction
	cr511b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto stch_handler() { return m_stch_handler.bind(); }
	auto sten_handler() { return m_sten_handler.bind(); }
	auto drq_handler() { return m_drq_handler.bind(); }
	auto dten_handler() { return m_dten_handler.bind(); }
	auto scor_handler() { return m_scor_handler.bind(); }
	auto xaen_handler() { return m_xaen_handler.bind(); }

	uint8_t read();
	void write(uint8_t data);

	void enable_w(int state);
	void cmd_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

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

	//bool m_motor;

	// state of lines
	int m_enabled;
	int m_cmd;

	// data transfer
	//uint8_t m_sector_buffer[CD_MAX_SECTOR_DATA];
};

DECLARE_DEVICE_TYPE(CR511B, cr511b_device)

#endif // MAME_MACHINE_CR511B_H
