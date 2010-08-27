/***************************************************************************

    i8243.c

    Intel 8243 Port Expander

    Copyright Aaron Giles

***************************************************************************/

#include "emu.h"
#include "i8243.h"
#include "devhelpr.h"

//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

GENERIC_DEVICE_CONFIG_SETUP(i8243, "I8243")

//-------------------------------------------------
//  static_set_read_handler - configuration helper
//  to set the read handler
//-------------------------------------------------

void i8243_device_config::static_set_read_handler(device_config *device, read8_device_func callback)
{
	i8243_device_config *i8243 = downcast<i8243_device_config *>(device);
	if(callback != NULL)
	{
		i8243->m_readhandler.type = DEVCB_TYPE_SELF;
		i8243->m_readhandler.readdevice = callback;
	}
	else
	{
		i8243->m_readhandler.type = DEVCB_TYPE_NULL;
	}
}


//-------------------------------------------------
//  static_set_write_handler - configuration helper
//  to set the write handler
//-------------------------------------------------

void i8243_device_config::static_set_write_handler(device_config *device, write8_device_func callback)
{
	i8243_device_config *i8243 = downcast<i8243_device_config *>(device);
	if(callback != NULL)
	{
		i8243->m_writehandler.type = DEVCB_TYPE_SELF;
		i8243->m_writehandler.writedevice = callback;
	}
	else
	{
		i8243->m_writehandler.type = DEVCB_TYPE_NULL;
	}
}



/***************************************************************************
    LIVE DEVICE
***************************************************************************/

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

const device_type I8243 = i8243_device_config::static_alloc_device_config;

//-------------------------------------------------
//  i8243_device - constructor
//-------------------------------------------------

i8243_device::i8243_device(running_machine &_machine, const i8243_device_config &config)
    : device_t(_machine, config),
      m_config(config)
{

}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void i8243_device::device_start()
{
	devcb_resolve_read8(&m_readhandler, &m_config.m_readhandler, this);
	devcb_resolve_write8(&m_writehandler, &m_config.m_writehandler, this);
}


//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8243_device::device_reset()
{
	m_p2 = 0x0f;
	m_p2out = 0x0f;
	m_prog = 1;
}


/*-------------------------------------------------
    i8243_p2_r - handle a read from port 2
-------------------------------------------------*/

READ8_DEVICE_HANDLER_TRAMPOLINE(i8243, i8243_p2_r)
{
	return m_p2out;
}


/*-------------------------------------------------
    i8243_p2_r - handle a write to port 2
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER_TRAMPOLINE(i8243, i8243_p2_w)
{
	m_p2 = data & 0x0f;
}


/*-------------------------------------------------
    i8243_prog_w - handle a change in the PROG
    line state
-------------------------------------------------*/

WRITE8_DEVICE_HANDLER_TRAMPOLINE(i8243, i8243_prog_w)
{
	/* only care about low bit */
	data &= 1;

	/* on high->low transition state, latch opcode/port */
	if(m_prog && !data)
	{
		m_opcode = m_p2;

		/* if this is a read opcode, copy result to p2out */
		if((m_opcode >> 2) == MCS48_EXPANDER_OP_READ)
		{
			if (m_readhandler.read != NULL)
			{
				m_p[m_opcode & 3] = devcb_call_read8(&m_readhandler, m_opcode & 3);
			}
			m_p2out = m_p[m_opcode & 3] & 0x0f;
		}
	}

	/* on low->high transition state, act on opcode */
	else if(!m_prog && data)
	{
		switch(m_opcode >> 2)
		{
			case MCS48_EXPANDER_OP_WRITE:
				m_p[m_opcode & 3] = m_p2 & 0x0f;
				devcb_call_write8(&m_writehandler, m_opcode & 3, m_p[m_opcode & 3]);
				break;

			case MCS48_EXPANDER_OP_OR:
				m_p[m_opcode & 3] |= m_p2 & 0x0f;
				devcb_call_write8(&m_writehandler, m_opcode & 3, m_p[m_opcode & 3]);
				break;

			case MCS48_EXPANDER_OP_AND:
				m_p[m_opcode & 3] &= m_p2 & 0x0f;
				devcb_call_write8(&m_writehandler, m_opcode & 3, m_p[m_opcode & 3]);
				break;
		}
	}

	/* remember the state */
	m_prog = data;
}
