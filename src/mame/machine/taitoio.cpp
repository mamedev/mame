// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

TC0220IOC
---------
A simple I/O interface with integrated watchdog.
It has four address inputs, which would suggest 16 bytes of addressing space,
but only the first 8 seem to be used.

000 R  IN00-07 (DSA)
000  W watchdog reset
001 R  IN08-15 (DSB)
002 R  IN16-23 (1P)
002  W unknown. Usually written on startup: initialize?
003 R  IN24-31 (2P)
004 RW coin counters and lockout
005  W unknown
006  W unknown
007 R  INB0-7 (coin)


TC0510NIO
---------
Newer version of the I/O chip

000 R  DSWA
000  W watchdog reset
001 R  DSWB
001  W unknown (ssi)
002 R  1P
003 R  2P
003  W unknown (yuyugogo, qzquest and qzchikyu use it a lot)
004 RW coin counters and lockout
005  W unknown
006  W unknown (koshien and pulirula use it a lot)
007 R  coin


TC0640FIO
---------
Newer version of the I/O chip ?


***************************************************************************/

#include "emu.h"
#include "machine/taitoio.h"


/***************************************************************************/
/*                                                                         */
/*                              TC0220IOC                                  */
/*                                                                         */
/***************************************************************************/

const device_type TC0220IOC = &device_creator<tc0220ioc_device>;

tc0220ioc_device::tc0220ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC0220IOC, "Taito TC0220IOC", tag, owner, clock, "tc0220ioc", __FILE__),
	m_read_0_cb(*this),
	m_read_1_cb(*this),
	m_read_2_cb(*this),
	m_read_3_cb(*this),
	m_read_7_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0220ioc_device::device_start()
{
	m_read_0_cb.resolve_safe(0);
	m_read_1_cb.resolve_safe(0);
	m_read_2_cb.resolve_safe(0);
	m_read_3_cb.resolve_safe(0);
	m_read_7_cb.resolve_safe(0);

	save_item(NAME(m_regs));
	save_item(NAME(m_port));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0220ioc_device::device_reset()
{
	m_port = 0;

	for (int i = 0; i < 8; i++)
		m_regs[i] = 0;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_MEMBER( tc0220ioc_device::read )
{
	switch (offset)
	{
		case 0x00:
			return m_read_0_cb(0);

		case 0x01:
			return m_read_1_cb(0);

		case 0x02:
			return m_read_2_cb(0);

		case 0x03:
			return m_read_3_cb(0);

		case 0x04:  /* coin counters and lockout */
			return m_regs[4];

		case 0x07:
			return m_read_7_cb(0);

		default:
//logerror("PC %06x: warning - read TC0220IOC address %02x\n",space.device().safe_pc(),offset);
			return 0xff;
	}
}

WRITE8_MEMBER( tc0220ioc_device::write )
{
	m_regs[offset] = data;
	switch (offset)
	{
		case 0x00:
			machine().watchdog_reset();
			break;

		case 0x04:  /* coin counters and lockout, hi nibble irrelevant */

			coin_lockout_w(machine(), 0, ~data & 0x01);
			coin_lockout_w(machine(), 1, ~data & 0x02);
			coin_counter_w(machine(), 0, data & 0x04);
			coin_counter_w(machine(), 1, data & 0x08);

//if (data & 0xf0)
//logerror("PC %06x: warning - write %02x to TC0220IOC address %02x\n",space.device().safe_pc(),data,offset);

			break;

		default:
//logerror("PC %06x: warning - write %02x to TC0220IOC address %02x\n",space.device().safe_pc(),data,offset);
			break;
	}
}

READ8_MEMBER( tc0220ioc_device::port_r )
{
	return m_port;
}

WRITE8_MEMBER( tc0220ioc_device::port_w )
{
	m_port = data;
}

READ8_MEMBER( tc0220ioc_device::portreg_r )
{
	return read(space, m_port);
}

WRITE8_MEMBER( tc0220ioc_device::portreg_w )
{
	write(space, m_port, data);
}

/***************************************************************************/
/*                                                                         */
/*                              TC0510NIO                                  */
/*                                                                         */
/***************************************************************************/


const device_type TC0510NIO = &device_creator<tc0510nio_device>;

tc0510nio_device::tc0510nio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC0510NIO, "Taito TC0510NIO", tag, owner, clock, "tc0510nio", __FILE__),
	m_read_0_cb(*this),
	m_read_1_cb(*this),
	m_read_2_cb(*this),
	m_read_3_cb(*this),
	m_read_7_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0510nio_device::device_start()
{
	m_read_0_cb.resolve_safe(0);
	m_read_1_cb.resolve_safe(0);
	m_read_2_cb.resolve_safe(0);
	m_read_3_cb.resolve_safe(0);
	m_read_7_cb.resolve_safe(0);

	save_item(NAME(m_regs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0510nio_device::device_reset()
{
	for (int i = 0; i < 8; i++)
		m_regs[i] = 0;
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_MEMBER( tc0510nio_device::read )
{
	switch (offset)
	{
		case 0x00:
			return m_read_0_cb(0);

		case 0x01:
			return m_read_1_cb(0);

		case 0x02:
			return m_read_2_cb(0);

		case 0x03:
			return m_read_3_cb(0);

		case 0x04:  /* coin counters and lockout */
			return m_regs[4];

		case 0x07:
			return m_read_7_cb(0);

		default:
//logerror("PC %06x: warning - read TC0510NIO address %02x\n",space.device().safe_pc(),offset);
			return 0xff;
	}
}

WRITE8_MEMBER( tc0510nio_device::write )
{
	m_regs[offset] = data;

	switch (offset)
	{
		case 0x00:
			machine().watchdog_reset();
			break;

		case 0x04:  /* coin counters and lockout */
			coin_lockout_w(machine(), 0, ~data & 0x01);
			coin_lockout_w(machine(), 1, ~data & 0x02);
			coin_counter_w(machine(), 0, data & 0x04);
			coin_counter_w(machine(), 1, data & 0x08);
			break;

		default:
//logerror("PC %06x: warning - write %02x to TC0510NIO address %02x\n",space.device().safe_pc(),data,offset);
			break;
	}
}

READ16_MEMBER( tc0510nio_device::halfword_r )
{
	return read(space, offset);
}

WRITE16_MEMBER( tc0510nio_device::halfword_w )
{
	if (ACCESSING_BITS_0_7)
		write(space, offset, data & 0xff);
	else
	{
		/* driftout writes the coin counters here - bug? */
//logerror("CPU #0 PC %06x: warning - write to MSB of TC0510NIO address %02x\n",space.device().safe_pc(),offset);
		write(space, offset, (data >> 8) & 0xff);
	}
}

READ16_MEMBER( tc0510nio_device::halfword_wordswap_r )
{
	return halfword_r(space, offset ^ 1, mem_mask);
}

WRITE16_MEMBER( tc0510nio_device::halfword_wordswap_w )
{
	halfword_w(space, offset ^ 1,data, mem_mask);
}


/***************************************************************************/
/*                                                                         */
/*                              TC0640FIO                                  */
/*                                                                         */
/***************************************************************************/


const device_type TC0640FIO = &device_creator<tc0640fio_device>;

tc0640fio_device::tc0640fio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, TC0640FIO, "Taito TC0640FIO", tag, owner, clock, "tc0640fio", __FILE__),
	m_read_0_cb(*this),
	m_read_1_cb(*this),
	m_read_2_cb(*this),
	m_read_3_cb(*this),
	m_read_7_cb(*this)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0640fio_device::device_start()
{
	m_read_0_cb.resolve_safe(0);
	m_read_1_cb.resolve_safe(0);
	m_read_2_cb.resolve_safe(0);
	m_read_3_cb.resolve_safe(0);
	m_read_7_cb.resolve_safe(0);

	save_item(NAME(m_regs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0640fio_device::device_reset()
{
	for (int i = 0; i < 8; i++)
		m_regs[i] = 0;
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

READ8_MEMBER( tc0640fio_device::read )
{
	switch (offset)
	{
		case 0x00:
			return m_read_0_cb(0);

		case 0x01:
			return m_read_1_cb(0);

		case 0x02:
			return m_read_2_cb(0);

		case 0x03:
			return m_read_3_cb(0);

		case 0x04:  /* coin counters and lockout */
			return m_regs[4];

		case 0x07:
			return m_read_7_cb(0);

		default:
//logerror("PC %06x: warning - read TC0640FIO address %02x\n",space.device().safe_pc(),offset);
			return 0xff;
	}
}

WRITE8_MEMBER( tc0640fio_device::write )
{
	m_regs[offset] = data;
	switch (offset)
	{
		case 0x00:
			machine().watchdog_reset();
			break;

		case 0x04:  /* coin counters and lockout */
			coin_lockout_w(machine(), 0, ~data & 0x01);
			coin_lockout_w(machine(), 1, ~data & 0x02);
			coin_counter_w(machine(), 0, data & 0x04);
			coin_counter_w(machine(), 1, data & 0x08);
			break;

		default:
//logerror("PC %06x: warning - write %02x to TC0640FIO address %02x\n",space.device().safe_pc(),data,offset);
			break;
	}
}

READ16_MEMBER( tc0640fio_device::halfword_r )
{
	return read(space, offset);
}

WRITE16_MEMBER( tc0640fio_device::halfword_w )
{
	if (ACCESSING_BITS_0_7)
		write(space, offset, data & 0xff);
	else
	{
		write(space, offset, (data >> 8) & 0xff);
//logerror("CPU #0 PC %06x: warning - write to MSB of TC0640FIO address %02x\n",space.device().safe_pc(),offset);
	}
}

READ16_MEMBER( tc0640fio_device::halfword_byteswap_r )
{
	return halfword_r(space, offset, mem_mask) << 8;
}

WRITE16_MEMBER( tc0640fio_device::halfword_byteswap_w )
{
	if (ACCESSING_BITS_8_15)
		write(space, offset, (data >> 8) & 0xff);
	else
	{
		write(space, offset, data & 0xff);
//logerror("CPU #0 PC %06x: warning - write to LSB of TC0640FIO address %02x\n",space.device().safe_pc(),offset);
	}
}
