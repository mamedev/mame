#include "emu.h"
#include "cpu/z80/z80.h"
#include "taitosnd.h"


/**********************************************************************************************

    It seems like 1 nibble commands are only for control purposes.
    2 nibble commands are the real messages passed from one board to the other.

**********************************************************************************************/

#define TC0140SYT_PORT01_FULL         (0x01)
#define TC0140SYT_PORT23_FULL         (0x02)
#define TC0140SYT_PORT01_FULL_MASTER  (0x04)
#define TC0140SYT_PORT23_FULL_MASTER  (0x08)


// device type definition
const device_type TC0140SYT = &device_creator<tc0140syt_device>;


//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  tc0140syt_device - constructor
//-------------------------------------------------

tc0140syt_device::tc0140syt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC0140SYT, "Taito TC0140SYT", tag, owner, clock),
		m_mainmode(0),
		m_submode(0),
		m_status(0),
		m_nmi_enabled(0),
		m_nmi_req(0),
		m_mastercpu(NULL),
		m_slavecpu(NULL)
{
	memset(m_slavedata, 0, sizeof(UINT8)*4);
	memset(m_masterdata, 0, sizeof(UINT8)*4);
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0140syt_device::device_start()
{
	const tc0140syt_interface *intf = reinterpret_cast<const tc0140syt_interface*>(static_config());

	/* use the given gfx set */
	m_mastercpu = machine().device(intf->master);
	m_slavecpu = machine().device(intf->slave);

	save_item(NAME(m_mainmode));
	save_item(NAME(m_submode));
	save_item(NAME(m_status));
	save_item(NAME(m_nmi_enabled));
	save_item(NAME(m_nmi_req));
	save_item(NAME(m_slavedata));
	save_item(NAME(m_masterdata));
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0140syt_device::device_reset()
{
	int i;

	m_mainmode = 0;
	m_submode = 0;
	m_status = 0;
	m_nmi_enabled = 0;
	m_nmi_req = 0;

	for (i = 0; i < 4; i++)
	{
		m_slavedata[i] = 0;
		m_masterdata[i] = 0;
	}
}


//-------------------------------------------------
//  DEVICE HANDLERS
//-------------------------------------------------

void tc0140syt_device::interrupt_controller( )
{
	if (m_nmi_req && m_nmi_enabled)
	{
		m_slavecpu->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		m_nmi_req = 0;
	}
}


//-------------------------------------------------
//  MASTER SIDE
//-------------------------------------------------

WRITE8_MEMBER( tc0140syt_device::tc0140syt_port_w )
{
	data &= 0x0f;
	m_mainmode = data;

	if (data > 4)
	{
		logerror("tc0140syt : error Master entering unknown mode[%02x]\n", data);
	}
}

WRITE8_MEMBER( tc0140syt_device::tc0140syt_comm_w )
{
	data &= 0x0f; /* this is important, otherwise ballbros won't work */

	switch (m_mainmode)
	{
		case 0x00: // mode #0
			m_slavedata[m_mainmode++] = data;
			break;

		case 0x01: // mode #1
			m_slavedata[m_mainmode++] = data;
			m_status |= TC0140SYT_PORT01_FULL;
			m_nmi_req = 1;
			interrupt_controller();
			break;

		case 0x02: // mode #2
			m_slavedata[m_mainmode++] = data;
			break;

		case 0x03: // mode #3
			m_slavedata[m_mainmode++] = data;
			m_status |= TC0140SYT_PORT23_FULL;
			m_nmi_req = 1;
			interrupt_controller();
			break;

		case 0x04: // port status
			/* this does a hi-lo transition to reset the sound cpu */
			if (data)
				m_slavecpu->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			else
			{
				m_slavecpu->execute().set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				m_mastercpu->execute().spin(); /* otherwise no sound in driftout */
			}
			break;

		default:
			break;
	}
}

READ8_MEMBER( tc0140syt_device::tc0140syt_comm_r )
{
	UINT8 res = 0;

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

WRITE8_MEMBER( tc0140syt_device::tc0140syt_slave_port_w )
{
	data &= 0x0f;
	m_submode = data;

	if (data > 6)
	{
		logerror("tc0140syt error : Slave cpu unknown mode[%02x]\n", data);
	}
}

WRITE8_MEMBER( tc0140syt_device::tc0140syt_slave_comm_w )
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
			m_slavecpu->execute().spin(); /* writing should take longer than emulated, so spin */
			break;

		case 0x02: // mode #2
			m_masterdata[m_submode++] = data;
			break;

		case 0x03: // mode #3
			m_masterdata[m_submode++] = data;
			m_status |= TC0140SYT_PORT23_FULL_MASTER;
			m_slavecpu->execute().spin(); /* writing should take longer than emulated, so spin */
			break;

		case 0x04: // port status
			//m_status = TC0140SYT_SET_OK;
			break;

		case 0x05: // nmi disable
			m_nmi_enabled = 0;
			break;

		case 0x06: // nmi enable
			m_nmi_enabled = 1;
			interrupt_controller();
			break;

		default:
			break;
	}
}

READ8_MEMBER( tc0140syt_device::tc0140syt_slave_comm_r )
{
	UINT8 res = 0;

	switch (m_submode)
	{
		case 0x00: // mode #0
			res = m_slavedata[m_submode++];
			break;

		case 0x01: // mode #1
			m_status &= ~TC0140SYT_PORT01_FULL;
			res = m_slavedata[m_submode++];
			break;

		case 0x02: // mode #2
			res = m_slavedata[m_submode++];
			break;

		case 0x03: // mode #3
			m_status &= ~TC0140SYT_PORT23_FULL;
			res = m_slavedata[m_submode++];
			break;

		case 0x04: // port status
			res = m_status;
			break;

		default:
			break;
	}

	return res;
}
