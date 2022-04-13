// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Roland MB87013 QD (Quick Disk) Drive Interface Adapter

    This device uses a MB89251 (Intel 8251 compatible USART) to serialize
    and deserialize the disk data. The two ICs appear to be clocked at the
    same 6.5 MHz rate, and they have address, data, RD and WR signals in
    common.

    The OP4 and RTS pins are usually bridged.

***************************************************************************/

#include "emu.h"
#include "mb87013.h"

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(MB87013, mb87013_device, "mb87013", "Roland MB87013 QDC")


//**************************************************************************
//  DEVICE IMPLEMENTATION
//**************************************************************************

//-------------------------------------------------
//  mb87013_device - constructor
//-------------------------------------------------

mb87013_device::mb87013_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, MB87013, tag, owner, clock)
	, m_sio_rd_callback(*this)
	, m_sio_wr_callback(*this)
	, m_txc_callback(*this)
	, m_rxc_callback(*this)
	, m_rxd_callback(*this)
	, m_dsr_callback(*this)
	, m_op4_callback(*this)
{
}


//-------------------------------------------------
//  device_resolve_objects - resolve objects that
//  may be needed for other devices to set
//  initial conditions at start time
//-------------------------------------------------

void mb87013_device::device_resolve_objects()
{
	m_sio_rd_callback.resolve_safe(0);
	m_sio_wr_callback.resolve_safe();
	m_txc_callback.resolve_safe();
	m_rxc_callback.resolve_safe();
	m_rxd_callback.resolve_safe();
	m_dsr_callback.resolve_safe();
	m_op4_callback.resolve_safe();
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void mb87013_device::device_start()
{
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void mb87013_device::device_reset()
{
}


//-------------------------------------------------
//  read - CPU read from QDC/SIO
//-------------------------------------------------

u8 mb87013_device::read(offs_t offset)
{
	if (!BIT(offset, 1))
		return m_sio_rd_callback(offset);

	// TODO
	if (BIT(offset, 0))
		logerror("%s: Reading data from control register or CRC register (LSB)\n", machine().describe_context());
	else
		logerror("%s: Reading data from CRC register (MSB)\n", machine().describe_context());
	return 0;
}


//-------------------------------------------------
//  write - CPU write to QDC/SIO
//-------------------------------------------------

void mb87013_device::write(offs_t offset, u8 data)
{
	if (!BIT(offset, 1))
	{
		// TODO: A1 = 0 & A0 = 0 writes data to both devices
		m_sio_wr_callback(offset, data);
		return;
	}

	// TODO
	if (BIT(offset, 0))
		logerror("%s: Writing %02X to control register\n", machine().describe_context(), data);
	else
		logerror("%s: Writing %02X to data register\n", machine().describe_context(), data);
}


//-------------------------------------------------
//  dtr_w - SIO control line write to enable
//  MFM modulator
//-------------------------------------------------

WRITE_LINE_MEMBER(mb87013_device::dtr_w)
{
}


//-------------------------------------------------
//  txd_w - SIO line write of data to be MFM
//  modulated by QDC
//-------------------------------------------------

WRITE_LINE_MEMBER(mb87013_device::txd_w)
{
}


//-------------------------------------------------
//  rts_w - line write to enable transfer of
//  data through RxC and RxD
//-------------------------------------------------

WRITE_LINE_MEMBER(mb87013_device::rts_w)
{
}
