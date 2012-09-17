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

struct tc0140syt_state
{
	UINT8     slavedata[4];  /* Data on master->slave port (4 nibbles) */
	UINT8     masterdata[4]; /* Data on slave->master port (4 nibbles) */
	UINT8     mainmode;      /* Access mode on master cpu side */
	UINT8     submode;       /* Access mode on slave cpu side */
	UINT8     status;        /* Status data */
	UINT8     nmi_enabled;   /* 1 if slave cpu has nmi's enabled */
	UINT8     nmi_req;       /* 1 if slave cpu has a pending nmi */

	device_t *mastercpu;	/* this is the maincpu */
	device_t *slavecpu;	/* this is the audiocpu */
};

/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE tc0140syt_state *get_safe_token( device_t *device )
{
	assert(device != NULL);
	assert(device->type() == TC0140SYT);

	return (tc0140syt_state *)downcast<tc0140syt_device *>(device)->token();
}

INLINE const tc0140syt_interface *get_interface( device_t *device )
{
	assert(device != NULL);
	assert((device->type() == TC0140SYT));
	return (const tc0140syt_interface *) device->static_config();
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

static void interrupt_controller( device_t *device )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);

	if (tc0140syt->nmi_req && tc0140syt->nmi_enabled)
	{
		tc0140syt->slavecpu->execute().set_input_line(INPUT_LINE_NMI, PULSE_LINE);
		tc0140syt->nmi_req = 0;
	}
}

WRITE8_DEVICE_HANDLER( tc0140syt_port_w )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);
	data &= 0x0f;

	tc0140syt->mainmode = data;
	//logerror("taitosnd: Master cpu mode [%02x]\n", data);
	if (data > 4)
	{
		logerror("tc0140syt : error Master entering unknown mode[%02x]\n", data);
	}
}

WRITE8_DEVICE_HANDLER( tc0140syt_comm_w )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);

	data &= 0x0f;	/*this is important, otherwise ballbros won't work*/

	switch (tc0140syt->mainmode)
	{
		case 0x00:		// mode #0
			tc0140syt->slavedata[tc0140syt->mainmode ++] = data;
			//logerror("taitosnd: Master cpu written port 0, data %01x\n", data);
			break;

		case 0x01:		// mode #1
			tc0140syt->slavedata[tc0140syt->mainmode ++] = data;
			tc0140syt->status |= TC0140SYT_PORT01_FULL;
			tc0140syt->nmi_req = 1;
			//logerror("taitosnd: Master cpu sends 0/1 : %01x%01x\n", tc0140syt->slavedata[1], tc0140syt->slavedata[0]);
        	break;

		case 0x02:		// mode #2
			tc0140syt->slavedata[tc0140syt->mainmode ++] = data;
			//logerror("taitosnd: Master cpu written port 2, data %01\n", data);
			break;

		case 0x03:		// mode #3
			tc0140syt->slavedata[tc0140syt->mainmode ++] = data;
			tc0140syt->status |= TC0140SYT_PORT23_FULL;
			tc0140syt->nmi_req = 1;
			//logerror("taitosnd: Master cpu sends 2/3 : %01x%01x\n", tc0140syt->slavedata[3], tc0140syt->slavedata[2]);
			break;

		case 0x04:		// port status
			//logerror("taitosnd: Master issued control value %02x (PC = %08x) \n",data, space.device().safe_pc() );
			/* this does a hi-lo transition to reset the sound cpu */
			if (data)
				tc0140syt->slavecpu->execute().set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
			else
			{
				tc0140syt->slavecpu->execute().set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
				tc0140syt->mastercpu->execute().spin(); /* otherwise no sound in driftout */
			}
			break;

		default:
			//logerror("taitosnd: Master cpu written in mode [%02x] data[%02x]\n", tc0140syt->mainmode, data);
			break;
	}

}

READ8_DEVICE_HANDLER( tc0140syt_comm_r )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);

	switch( tc0140syt->mainmode )
	{
		case 0x00:		// mode #0
			//logerror("taitosnd: Master cpu read portdata %01x\n", tc0140syt->masterdata[0]);
			return tc0140syt->masterdata[tc0140syt->mainmode ++];

		case 0x01:		// mode #1
			//logerror("taitosnd: Master cpu receives 0/1 : %01x%01x\n", tc0140syt->masterdata[1], tc0140syt->masterdata[0]);
			tc0140syt->status &= ~TC0140SYT_PORT01_FULL_MASTER;
			return tc0140syt->masterdata[tc0140syt->mainmode ++];

		case 0x02:		// mode #2
			//logerror("taitosnd: Master cpu read masterdata %01x\n", tc0140syt->masterdata[2]);
			return tc0140syt->masterdata[tc0140syt->mainmode ++];

		case 0x03:		// mode #3
			//logerror("taitosnd: Master cpu receives 2/3 : %01x%01x\n", tc0140syt->masterdata[3], tc0140syt->masterdata[2]);
			tc0140syt->status &= ~TC0140SYT_PORT23_FULL_MASTER;
			return tc0140syt->masterdata[tc0140syt->mainmode ++];

		case 0x04:		// port status
			//logerror("tc0140syt : Master cpu read status : %02x\n", tc0140syt->status);
			return tc0140syt->status;

		default:
			//logerror("tc0140syt : Master cpu read in mode [%02x]\n", tc0140syt->mainmode);
			return 0;
	}
}

//SLAVE SIDE

WRITE8_DEVICE_HANDLER( tc0140syt_slave_port_w )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);

	data &= 0x0f;
	tc0140syt->submode = data;
	//logerror("taitosnd: Slave cpu mode [%02x]\n", data);
	if (data > 6)
		logerror("tc0140syt error : Slave cpu unknown mode[%02x]\n", data);
}

WRITE8_DEVICE_HANDLER( tc0140syt_slave_comm_w )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);

	data &= 0x0f;
	switch (tc0140syt->submode)
	{
		case 0x00:		// mode #0
			tc0140syt->masterdata[tc0140syt->submode ++] = data;
			//logerror("taitosnd: Slave cpu written port 0, data %01x\n", data);
			break;

		case 0x01:		// mode #1
			tc0140syt->masterdata[tc0140syt->submode ++] = data;
			tc0140syt->status |= TC0140SYT_PORT01_FULL_MASTER;
			//logerror("taitosnd: Slave cpu sends 0/1 : %01x%01x\n" , tc0140syt->masterdata[1] , tc0140syt->masterdata[0]);
			tc0140syt->slavecpu->execute().spin(); /* writing should take longer than emulated, so spin */
			break;

		case 0x02:		// mode #2
			//logerror("taitosnd: Slave cpu written port 2, data %01x\n", data);
			tc0140syt->masterdata[tc0140syt->submode ++] = data;
			break;

		case 0x03:		// mode #3
			tc0140syt->masterdata[tc0140syt->submode ++] = data;
			tc0140syt->status |= TC0140SYT_PORT23_FULL_MASTER;
			//logerror("taitosnd: Slave cpu sends 2/3 : %01x%01x\n" , tc0140syt->masterdata[3] , tc0140syt->masterdata[2]);
			tc0140syt->slavecpu->execute().spin(); /* writing should take longer than emulated, so spin */
			break;

		case 0x04:		// port status
			//tc0140syt->status = TC0140SYT_SET_OK;
			//logerror("tc0140syt : Slave cpu status ok.\n");
			break;

		case 0x05:		// nmi disable
			tc0140syt->nmi_enabled = 0;
			break;

		case 0x06:		// nmi enable
			tc0140syt->nmi_enabled = 1;
			break;

		default:
			//logerror("tc0140syt: Slave cpu written in mode [%02x] data[%02x]\n" , tc0140syt->submode, data & 0xff);
			break;
	}

	interrupt_controller(device);

}

READ8_DEVICE_HANDLER( tc0140syt_slave_comm_r )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);
	UINT8 res = 0;

	switch ( tc0140syt->submode )
	{
		case 0x00:		// mode #0
			//logerror("taitosnd: Slave cpu read slavedata %01x\n", tc0140syt->slavedata[0]);
			res = tc0140syt->slavedata[tc0140syt->submode ++];
			break;

		case 0x01:		// mode #1
			//logerror("taitosnd: Slave cpu receives 0/1 : %01x%01x PC=%4x\n", tc0140syt->slavedata[1] , tc0140syt->slavedata[0],space.device().safe_pc());
			tc0140syt->status &= ~TC0140SYT_PORT01_FULL;
			res = tc0140syt->slavedata[tc0140syt->submode ++];
			break;

		case 0x02:		// mode #2
			//logerror("taitosnd: Slave cpu read slavedata %01x\n", tc0140syt->slavedata[2]);
			res = tc0140syt->slavedata[tc0140syt->submode ++];
			break;

		case 0x03:		// mode #3
			//logerror("taitosnd: Slave cpu receives 2/3 : %01x%01x\n", tc0140syt->slavedata[3] , tc0140syt->slavedata[2]);
			tc0140syt->status &= ~TC0140SYT_PORT23_FULL;
			res = tc0140syt->slavedata[tc0140syt->submode ++];
			break;

		case 0x04:		// port status
			//logerror("tc0140syt : Slave cpu read status : %02x\n", tc0140syt->status);
			res = tc0140syt->status;
			break;

		default:
			//logerror("tc0140syt : Slave cpu read in mode [%02x]\n", tc0140syt->submode);
			res = 0;
			break;
	}

	interrupt_controller(device);

	return res;
}

/*****************************************************************************
    DEVICE INTERFACE
*****************************************************************************/

static DEVICE_START( tc0140syt )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);
	const tc0140syt_interface *intf = get_interface(device);

	/* use the given gfx set */
	tc0140syt->mastercpu = device->machine().device(intf->master);
	tc0140syt->slavecpu = device->machine().device(intf->slave);

	device->save_item(NAME(tc0140syt->mainmode));
	device->save_item(NAME(tc0140syt->submode));
	device->save_item(NAME(tc0140syt->status));
	device->save_item(NAME(tc0140syt->nmi_enabled));
	device->save_item(NAME(tc0140syt->nmi_req));
	device->save_item(NAME(tc0140syt->slavedata));
	device->save_item(NAME(tc0140syt->masterdata));
}

static DEVICE_RESET( tc0140syt )
{
	tc0140syt_state *tc0140syt = get_safe_token(device);
	int i;

	tc0140syt->mainmode = 0;
	tc0140syt->submode = 0;
	tc0140syt->status = 0;
	tc0140syt->nmi_enabled = 0;
	tc0140syt->nmi_req = 0;

	for (i = 0; i < 4; i++)
	{
		tc0140syt->slavedata[i] = 0;
		tc0140syt->masterdata[i] = 0;
	}
}

const device_type TC0140SYT = &device_creator<tc0140syt_device>;

tc0140syt_device::tc0140syt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC0140SYT, "Taito TC0140SYT", tag, owner, clock)
{
	m_token = global_alloc_array_clear(UINT8, sizeof(tc0140syt_state));
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void tc0140syt_device::device_config_complete()
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0140syt_device::device_start()
{
	DEVICE_START_NAME( tc0140syt )(this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0140syt_device::device_reset()
{
	DEVICE_RESET_NAME( tc0140syt )(this);
}


