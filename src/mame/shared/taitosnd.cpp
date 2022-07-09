// license:BSD-3-Clause
// copyright-holders:Philip Bennett
/**********************************************************************************************

    Taito TC0140SYT

    TODO:
     - Add pinout and description
     - Create a separate implementation for the PC060HA

    General rule seems to be that TC0140SYT supports a YM2610,
    whereas PC060HA goes with a YM2203 or YM2151.

    The PC060HA has been decapped and verified to be a ULA.

**********************************************************************************************/

#include "emu.h"
#include "taitosnd.h"

#include "cpu/z80/z80.h"


/**********************************************************************************************

    It seems like 1 nibble commands are only for control purposes.
    2 nibble commands are the real messages passed from one board to the other.

**********************************************************************************************/

static constexpr u8 TC0140SYT_PORT01_FULL =        0x01;
static constexpr u8 TC0140SYT_PORT23_FULL =        0x02;
static constexpr u8 TC0140SYT_PORT01_FULL_MASTER = 0x04;
static constexpr u8 TC0140SYT_PORT23_FULL_MASTER = 0x08;


// device type definition
DEFINE_DEVICE_TYPE(TC0140SYT, tc0140syt_device, "tc0140syt", "Taito TC0140SYT")
DEFINE_DEVICE_TYPE(PC060HA, pc060ha_device, "pc060ha", "Taito PC060HA CIU")


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tc0140syt_device - constructor
//-------------------------------------------------

tc0140syt_device::tc0140syt_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, m_mainmode(0)
	, m_submode(0)
	, m_status(0)
	, m_nmi_enabled(0)
	, m_mastercpu(*this, finder_base::DUMMY_TAG)
	, m_slavecpu(*this, finder_base::DUMMY_TAG)
{
	std::fill(std::begin(m_slavedata), std::end(m_slavedata), 0);
	std::fill(std::begin(m_masterdata), std::end(m_masterdata), 0);
}

tc0140syt_device::tc0140syt_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tc0140syt_device(mconfig, TC0140SYT, tag, owner, clock)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0140syt_device::device_start()
{
	save_item(NAME(m_mainmode));
	save_item(NAME(m_submode));
	save_item(NAME(m_status));
	save_item(NAME(m_nmi_enabled));
	save_item(NAME(m_slavedata));
	save_item(NAME(m_masterdata));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0140syt_device::device_reset()
{
	m_mainmode = 0;
	m_submode = 0;
	m_status = 0;
	m_nmi_enabled = 0;

	for (u8 i = 0; i < 4; i++)
	{
		m_slavedata[i] = 0;
		m_masterdata[i] = 0;
	}
}


//-------------------------------------------------
//  DEVICE HANDLERS
//-------------------------------------------------

void tc0140syt_device::update_nmi()
{
	u32 nmi_pending = m_status & (TC0140SYT_PORT23_FULL | TC0140SYT_PORT01_FULL);
	u32 state = (nmi_pending && m_nmi_enabled) ? ASSERT_LINE : CLEAR_LINE;

	m_slavecpu->set_input_line(INPUT_LINE_NMI, state);
}


//-------------------------------------------------
//  MASTER SIDE
//-------------------------------------------------

void tc0140syt_device::master_port_w(u8 data)
{
	data &= 0x0f;
	m_mainmode = data;

	if (data > 4)
	{
		logerror("tc0140syt : error Master entering unknown mode[%02x]\n", data);
	}
}

void tc0140syt_device::master_comm_w(u8 data)
{
	machine().scheduler().synchronize(); // let slavecpu catch up (after we return and the main cpu finishes what it's doing)
	data &= 0x0f; /* this is important, otherwise ballbros won't work */

	switch (m_mainmode)
	{
		case 0x00: // mode #0
			m_slavedata[m_mainmode++] = data;
			break;

		case 0x01: // mode #1
			m_slavedata[m_mainmode++] = data;
			m_status |= TC0140SYT_PORT01_FULL;
			update_nmi();
			break;

		case 0x02: // mode #2
			m_slavedata[m_mainmode++] = data;
			break;

		case 0x03: // mode #3
			m_slavedata[m_mainmode++] = data;
			m_status |= TC0140SYT_PORT23_FULL;
			update_nmi();
			break;

		case 0x04: // port status
			/* this does a hi-lo transition to reset the sound cpu */
			m_slavecpu->set_input_line(INPUT_LINE_RESET, data ? ASSERT_LINE : CLEAR_LINE);
			break;

		default:
			break;
	}
}

u8 tc0140syt_device::master_comm_r()
{
	machine().scheduler().synchronize(); // let slavecpu catch up (after we return and the main cpu finishes what it's doing)
	u8 res = 0;

	switch (m_mainmode)
	{
		case 0x00: // mode #0
			res = m_masterdata[m_mainmode++];
			break;

		case 0x01: // mode #1
			m_status &= ~TC0140SYT_PORT01_FULL_MASTER;
			res = m_masterdata[m_mainmode++];
			break;

		case 0x02: // mode #2
			res = m_masterdata[m_mainmode++];
			break;

		case 0x03: // mode #3
			m_status &= ~TC0140SYT_PORT23_FULL_MASTER;
			res = m_masterdata[m_mainmode++];
			break;

		case 0x04: // port status
			res = m_status;
			break;

		default:
			break;
	}

	return res;
}


//-------------------------------------------------
//  SLAVE SIDE
//-------------------------------------------------

void tc0140syt_device::slave_port_w(u8 data)
{
	data &= 0x0f;
	m_submode = data;

	if (data > 6)
	{
		logerror("tc0140syt error : Slave cpu unknown mode[%02x]\n", data);
	}
}

void tc0140syt_device::slave_comm_w(u8 data)
{
	data &= 0x0f;

	switch (m_submode)
	{
		case 0x00: // mode #0
			m_masterdata[m_submode++] = data;
			break;

		case 0x01: // mode #1
			m_masterdata[m_submode++] = data;
			m_status |= TC0140SYT_PORT01_FULL_MASTER;
			break;

		case 0x02: // mode #2
			m_masterdata[m_submode++] = data;
			break;

		case 0x03: // mode #3
			m_masterdata[m_submode++] = data;
			m_status |= TC0140SYT_PORT23_FULL_MASTER;
			break;

		case 0x04: // port status
			//m_status = TC0140SYT_SET_OK;
			break;

		case 0x05: // NMI disable
			m_nmi_enabled = 0;
			update_nmi();
			break;

		case 0x06: // NMI enable
			m_nmi_enabled = 1;
			update_nmi();
			break;

		default:
			break;
	}
}

u8 tc0140syt_device::slave_comm_r()
{
	u8 res = 0;

	switch (m_submode)
	{
		case 0x00: // mode #0
			res = m_slavedata[m_submode++];
			break;

		case 0x01: // mode #1
			m_status &= ~TC0140SYT_PORT01_FULL;
			res = m_slavedata[m_submode++];
			update_nmi();
			break;

		case 0x02: // mode #2
			res = m_slavedata[m_submode++];
			break;

		case 0x03: // mode #3
			m_status &= ~TC0140SYT_PORT23_FULL;
			res = m_slavedata[m_submode++];
			update_nmi();
			break;

		case 0x04: // port status
			res = m_status;
			break;

		default:
			break;
	}

	return res;
}


//-------------------------------------------------
//  pc060ha_device - constructor
//-------------------------------------------------

pc060ha_device::pc060ha_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: tc0140syt_device(mconfig, PC060HA, tag, owner, clock)
{
}
