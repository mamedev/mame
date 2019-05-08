// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

TC0040IOC
---------
Taito's first custom I/O interface differs a bit from its successors,
being a 64-pin DIP with an address port and a data port. Besides digital
inputs, coin-related outputs and an integrated watchdog, this chip also
handles steering wheel and trackball input conversion in various games (not
emulated here yet).


TC0220IOC
---------
A simple I/O interface with integrated watchdog in a 80-pin flat package.
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
Newer QFP100 version of the I/O chip

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

#include <algorithm>

/***************************************************************************/
/*                                                                         */
/*                              TC0040IOC                                  */
/*                                                                         */
/***************************************************************************/

DEFINE_DEVICE_TYPE(TC0040IOC, tc0040ioc_device, "tc0040ioc", "Taito TC0040IOC")

tc0040ioc_device::tc0040ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TC0040IOC, tag, owner, clock),
	m_port(0),
	m_watchdog(*this, "watchdog"),
	m_read_0_cb(*this),
	m_read_1_cb(*this),
	m_read_2_cb(*this),
	m_read_3_cb(*this),
	m_write_4_cb(*this),
	m_read_7_cb(*this)
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void tc0040ioc_device::device_start()
{
	m_read_0_cb.resolve_safe(0);
	m_read_1_cb.resolve_safe(0);
	m_read_2_cb.resolve_safe(0);
	m_read_3_cb.resolve_safe(0);
	m_write_4_cb.resolve_safe();
	m_read_7_cb.resolve_safe(0);

	save_item(NAME(m_regs));
	save_item(NAME(m_port));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0040ioc_device::device_reset()
{
	m_port = 0;

	std::fill(std::begin(m_regs), std::end(m_regs), 0);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tc0040ioc_device::device_add_mconfig(machine_config &config)
{
	WATCHDOG_TIMER(config, m_watchdog);
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

u8 tc0040ioc_device::read(offs_t offset)
{
	if (offset & 1)
		return watchdog_r();
	else
		return portreg_r();
}

void tc0040ioc_device::write(offs_t offset, u8 data)
{
	if (offset & 1)
		port_w(data);
	else
		portreg_w(data);
}

u8 tc0040ioc_device::watchdog_r()
{
	if (!machine().side_effects_disabled())
		m_watchdog->watchdog_reset();
	return 0;
}

// only used now for "input bypass" hacks
u8 tc0040ioc_device::port_r()
{
	return m_port;
}

void tc0040ioc_device::port_w(u8 data)
{
	m_port = data;
}

u8 tc0040ioc_device::portreg_r()
{
	switch (m_port)
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
//logerror("%s: warning - read TC0040IOC address %02x\n",m_maincpu->pc(),m_port);
			return 0xff;
	}
}

void tc0040ioc_device::portreg_w(u8 data)
{
	if (m_port < ARRAY_LENGTH(m_regs))
		m_regs[m_port] = data;
	switch (m_port)
	{
		case 0x04:  /* coin counters and lockout, hi nibble irrelevant */
			m_write_4_cb(data & 0x0f);

//if (data & 0xf0)
//logerror("%s: warning - write %02x to TC0040IOC address %02x\n",m_maincpu->pc(),data,m_port);

			break;

		default:
//logerror("%s: warning - write %02x to TC0040IOC address %02x\n",m_maincpu->pc(),data,m_port);
			break;
	}
}


/***************************************************************************/
/*                                                                         */
/*                              TC0220IOC                                  */
/*                                                                         */
/***************************************************************************/

DEFINE_DEVICE_TYPE(TC0220IOC, tc0220ioc_device, "tc0220ioc", "Taito TC0220IOC")

tc0220ioc_device::tc0220ioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TC0220IOC, tag, owner, clock),
	m_watchdog(*this, "watchdog"),
	m_read_0_cb(*this),
	m_read_1_cb(*this),
	m_read_2_cb(*this),
	m_read_3_cb(*this),
	m_write_3_cb(*this),
	m_write_4_cb(*this),
	m_read_7_cb(*this)
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
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
	m_write_3_cb.resolve_safe();
	m_write_4_cb.resolve_safe();
	m_read_7_cb.resolve_safe(0);

	save_item(NAME(m_regs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0220ioc_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tc0220ioc_device::device_add_mconfig(machine_config &config)
{
	WATCHDOG_TIMER(config, m_watchdog);
//  m_watchdog->set_time(attotime::from_msec(3200));
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

u8 tc0220ioc_device::read(offs_t offset)
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
//logerror("%s: warning - read TC0220IOC address %02x\n",m_maincpu->pc(),offset);
			return 0xff;
	}
}

void tc0220ioc_device::write(offs_t offset, u8 data)
{
	m_regs[offset] = data;
	switch (offset)
	{
		case 0x00:
			m_watchdog->watchdog_reset();
			break;

		case 0x03:
			m_write_3_cb(data);
			break;

		case 0x04:  /* coin counters and lockout, hi nibble irrelevant */
			m_write_4_cb(data & 0x0f);

//if (data & 0xf0)
//logerror("%s: warning - write %02x to TC0220IOC address %02x\n",m_maincpu->pc(),data,offset);

			break;

		default:
//logerror("%s: warning - write %02x to TC0220IOC address %02x\n",m_maincpu->pc(),data,offset);
			break;
	}
}

/***************************************************************************/
/*                                                                         */
/*                              TC0510NIO                                  */
/*                                                                         */
/***************************************************************************/


DEFINE_DEVICE_TYPE(TC0510NIO, tc0510nio_device, "tc0510nio", "Taito TC0510NIO")

tc0510nio_device::tc0510nio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TC0510NIO, tag, owner, clock),
	m_watchdog(*this, "watchdog"),
	m_read_0_cb(*this),
	m_read_1_cb(*this),
	m_read_2_cb(*this),
	m_read_3_cb(*this),
	m_write_3_cb(*this),
	m_write_4_cb(*this),
	m_read_7_cb(*this)
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
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
	m_write_3_cb.resolve_safe();
	m_write_4_cb.resolve_safe();
	m_read_7_cb.resolve_safe(0);

	save_item(NAME(m_regs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0510nio_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tc0510nio_device::device_add_mconfig(machine_config &config)
{
	WATCHDOG_TIMER(config, m_watchdog);
}

/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

u8 tc0510nio_device::read(offs_t offset)
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
//logerror("%s: warning - read TC0510NIO address %02x\n",m_maincpu->pc(),offset);
			return 0xff;
	}
}

void tc0510nio_device::write(offs_t offset, u8 data)
{
	m_regs[offset] = data;

	switch (offset)
	{
		case 0x00:
			m_watchdog->watchdog_reset();
			break;

		case 0x03:
			m_write_3_cb(data);
			break;

		case 0x04:  /* coin counters and lockout */
			m_write_4_cb(data & 0x0f);
			break;

		default:
//logerror("%s: warning - write %02x to TC0510NIO address %02x\n",m_maincpu->pc(),data,offset);
			break;
	}
}

u16 tc0510nio_device::halfword_r(offs_t offset)
{
	return read(offset);
}

void tc0510nio_device::halfword_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		write(offset, data & 0xff);
	else
	{
		/* driftout writes the coin counters here - bug? */
//logerror("CPU #0 %s: warning - write to MSB of TC0510NIO address %02x\n",m_maincpu->pc(),offset);
		write(offset, (data >> 8) & 0xff);
	}
}

u16 tc0510nio_device::halfword_wordswap_r(offs_t offset)
{
	return halfword_r(offset ^ 1);
}

void tc0510nio_device::halfword_wordswap_w(offs_t offset, u16 data, u16 mem_mask)
{
	halfword_w(offset ^ 1,data, mem_mask);
}


/***************************************************************************/
/*                                                                         */
/*                              TC0640FIO                                  */
/*                                                                         */
/***************************************************************************/


DEFINE_DEVICE_TYPE(TC0640FIO, tc0640fio_device, "tc0640fio", "Taito TC0640FIO")

tc0640fio_device::tc0640fio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, TC0640FIO, tag, owner, clock),
	m_watchdog(*this, "watchdog"),
	m_read_0_cb(*this),
	m_read_1_cb(*this),
	m_read_2_cb(*this),
	m_read_3_cb(*this),
	m_write_4_cb(*this),
	m_read_7_cb(*this)
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
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
	m_write_4_cb.resolve_safe();
	m_read_7_cb.resolve_safe(0);

	save_item(NAME(m_regs));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void tc0640fio_device::device_reset()
{
	std::fill(std::begin(m_regs), std::end(m_regs), 0);
}

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void tc0640fio_device::device_add_mconfig(machine_config &config)
{
	WATCHDOG_TIMER(config, m_watchdog);
}


/*****************************************************************************
    DEVICE HANDLERS
*****************************************************************************/

u8 tc0640fio_device::read(offs_t offset)
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
//logerror("%s: warning - read TC0640FIO address %02x\n",m_maincpu->pc(),offset);
			return 0xff;
	}
}

void tc0640fio_device::write(offs_t offset, u8 data)
{
	m_regs[offset] = data;
	switch (offset)
	{
		case 0x00:
			m_watchdog->watchdog_reset();
			break;

		case 0x04:  /* coin counters and lockout */
			m_write_4_cb(data & 0x0f);
			break;

		default:
//logerror("%s: warning - write %02x to TC0640FIO address %02x\n",m_maincpu->pc(),data,offset);
			break;
	}
}

u16 tc0640fio_device::halfword_r(offs_t offset)
{
	return read(offset);
}

void tc0640fio_device::halfword_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_0_7)
		write(offset, data & 0xff);
	else
	{
		write(offset, (data >> 8) & 0xff);
//logerror("CPU #0 %s: warning - write to MSB of TC0640FIO address %02x\n",m_maincpu->pc(),offset);
	}
}

u16 tc0640fio_device::halfword_byteswap_r(offs_t offset)
{
	return halfword_r(offset) << 8;
}

void tc0640fio_device::halfword_byteswap_w(offs_t offset, u16 data, u16 mem_mask)
{
	if (ACCESSING_BITS_8_15)
		write(offset, (data >> 8) & 0xff);
	else
	{
		write(offset, data & 0xff);
//logerror("CPU #0 %s: warning - write to LSB of TC0640FIO address %02x\n",m_maincpu->pc(),offset);
	}
}
