// license:BSD-3-Clause
// copyright-holders:r09
/****************************************************************************

    Fujitsu FMT-121 SCSI Card

    An expansion card for the dedicated SCSI card slot on the FM Towns
    Model 1 and 2 computers. It contains a Fujitsu MB673522U SCSI controller
    and an external 50-pin Centronics connector.

                          |||||||||||||||||||||
+-------------------------|                   |-------------------------+
|                     CN1 +-------------------+                         |
|                                                                       |
|                                                 74LS04N               |
|       74LS14N       74LS240N      SN74LS06N                           |
|                                                            MB413      |
|   MB463                                                               |
|                                                            74AS00N    |
|                           +-----------+                               |
|   MB463                   |           |                               |
|                           | MB673522U |                    MB412      |
|                           +-----------+                               |
|   MB463                                                    74LS240N   |
|                             CN3                                       |
|              ..........................................               |
|              .                                        .               |
|              ..........................................               |
|                             CN2                                       |
|              +----------------------------------------+               |
|              |                                        |               |
+--------------|                                        |---------------+
               ||||||||||||||||||||||||||||||||||||||||||

    CN1: 30-pin MFC-30LFD DIP connector
    CN2: 50-pin Centronics connector
    CN3: solder pads for internal 50-pin connector (not present)

****************************************************************************/

#include "emu.h"
#include "fmt121.h"

#include "bus/scsi/scsi.h"
#include "bus/scsi/scsihd.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(FMT121, fmt121_device, "fmt121", "FMT-121 SCSI Card")

//**************************************************************************
//  DEVICE DEFINITION
//**************************************************************************

//-------------------------------------------------
//  fmt121_device - construction
//-------------------------------------------------

fmt121_device::fmt121_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, FMT121, tag, owner, clock)
	, fmt_scsi_card_interface(mconfig, *this)
	, m_scsi_ctlr(*this, "fmscsi")
{
}


//-------------------------------------------------
//  device_add_mconfig - add device-specific
//  machine configuration
//-------------------------------------------------

void fmt121_device::device_add_mconfig(machine_config &config)
{
	scsi_port_device &scsi(SCSI_PORT(config, "scsi", 0));
	scsi.set_slot_device(1, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_0));
	scsi.set_slot_device(2, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_1));
	scsi.set_slot_device(3, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_2));
	scsi.set_slot_device(4, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_3));
	scsi.set_slot_device(5, "harddisk", SCSIHD, DEVICE_INPUT_DEFAULTS_NAME(SCSI_ID_4));

	FMSCSI(config, m_scsi_ctlr, 0);
	m_scsi_ctlr->set_scsi_port("scsi");
	m_scsi_ctlr->irq_handler().set(FUNC(fmt121_device::irq_w));
	m_scsi_ctlr->drq_handler().set(FUNC(fmt121_device::drq_w));
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void fmt121_device::device_start()
{
}

//**************************************************************************
//  FMT121 INTERFACE
//**************************************************************************

//-------------------------------------------------
//  fmt_scsi_read - I/O read access
//-------------------------------------------------

u8 fmt121_device::fmt_scsi_read(offs_t offset)
{
	return m_scsi_ctlr->fmscsi_r(offset);
}


//-------------------------------------------------
//  fmt_scsi_write - I/O write access
//-------------------------------------------------

void fmt121_device::fmt_scsi_write(offs_t offset, u8 data)
{
	m_scsi_ctlr->fmscsi_w(offset, data);
}

//-------------------------------------------------
//  fmt_scsi_data_read - data read access
//-------------------------------------------------

u8 fmt121_device::fmt_scsi_data_read()
{
	return m_scsi_ctlr->fmscsi_data_r();
}

//-------------------------------------------------
//  fmt_scsi_data_write - data write access
//-------------------------------------------------

void fmt121_device::fmt_scsi_data_write(u8 data)
{
	m_scsi_ctlr->fmscsi_data_w(data);
}


void fmt121_device::irq_w(int state)
{
	m_slot->irq_w(state);
}


void fmt121_device::drq_w(int state)
{
	m_slot->drq_w(state);
}
