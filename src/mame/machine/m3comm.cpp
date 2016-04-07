// license:BSD-3-Clause
// copyright-holders:MetalliC


// Communication board used by Sega in Model3, NAOMI and Hikaru, uses mostly same design
// interface to main board LATTICE ICs: Model3 - 315-5958, Hikaru - 315-5958A, NAOMI - 315-6194A


// TODO:
// Find out sources of IRQ 2 (flip comm RAM bank) and IRQ 5 (data frame exchange cycle start signal on MASTER)
// Is there any IRQs can be fired to host systems ?
// Implement NAOMI G1-DMA mode
// find out and hook actual networking exchange, some sort of token ring ???

/*

MODEL3 COMMUNICATION BOARD
837-11861  171-7053B
SEGA 1995
|---------------------------------------------------------------------------------------------------|
|                                                                                                   |
|   LATTICE                     NKK N341256SJ-20     NKK N341256SJ-20     NKK N341256SJ-20    40MHz |
|   PLSI 2032   JP3                                                                      JP1        |
|   (315-5958)  JP4             NKK N341256SJ-20     NKK N341256SJ-20     NKK N341256SJ-20          |
|                                                                                             JP2   |
|                                                                                                   |
|   PALCE16V8                                                                                       |
|   315-6075       68000FN12       315-5804             315-5917              315-5917              |
|                                  (QFP144)             (QFP80)               (QFP80)               |
|   PALCE16V8                                                                                       |
|   315-6074                                                                                        |
|  LEDx7                                                                                            |
|---------------------------------------------------------------------------------------------------|
JP1: 1-2
JP2: 2-3
JP3: not shorted
JP4: shorted


HIKARU COMMUNICATION BOARD
837-13404  171-7641B
|-----------------------------------|
| 40MHz      LATTICE     315-5917   |
|            PLSI2032               |
|            (315-5958A)    315-5917|
|   PAL                             |
|                          62256    |
|             315-5804     62256    |
|                          62256    |
|                          62256    |
|  68000          PAL               |
|  62256                            |
|  62256          PAL               |
|-----------------------------------|
Notes:
      62256 - 32k x8 SRAM


NAOMI COMMUNICATION BOARD
840-0001E
837-13489  171-7704B
SEGA 1998
|--------------------------------------------------------------------|
|         CN1                                  CN3                   |
| 40MHz        256KbSRAM  315-5917    256KbSRAM  315-5917  256KbSRAM |
|              256KbSRAM  (QFP80)     256KbSRAM  (QFP80)   256KbSRAM |
|                                                                    |
|                                                                    |
|    315-5804        68000FN12        LATTICE                        |
|    (QFP144)                         M5-128/104                     |
|                                     12YC/1-15YI/1                  |
|                                     (315-6194A)                    |
|          CN2                                                       |
|--------------------------------------------------------------------|

*/


#include "emu.h"
#include "emuopts.h"
#include "machine/m3comm.h"

#define M68K_TAG     "m3commcpu"

//////// Model 3 (main CPU @ C00xxxxx) and Hikaru (MMctrl bank 0E) interface
DEVICE_ADDRESS_MAP_START(m3_map, 32, m3comm_device)
	AM_RANGE(0x0000000, 0x000ffff) AM_READWRITE8(m3_comm_ram_r, m3_comm_ram_w, 0xffffffff)
	AM_RANGE(0x0010000, 0x00101ff) AM_READWRITE16(m3_ioregs_r, m3_ioregs_w, 0xffff0000)
	AM_RANGE(0x0020000, 0x003ffff) AM_READWRITE16(m3_m68k_ram_r, m3_m68k_ram_w, 0xffff0000)
ADDRESS_MAP_END


/*************************************
 *  M3COMM Memory Map
 *************************************/
static ADDRESS_MAP_START( m3comm_mem, AS_PROGRAM, 16, m3comm_device )
	AM_RANGE(0x0000000, 0x000ffff) AM_RAM AM_SHARE("m68k_ram")
	AM_RANGE(0x0040000, 0x00400ff) AM_READWRITE(ctrl_r, ctrl_w)
	AM_RANGE(0x0080000, 0x008ffff) AM_RAMBANK("comm_ram")
	AM_RANGE(0x00C0000, 0x00C00ff) AM_READWRITE(ioregs_r, ioregs_w)
ADDRESS_MAP_END


MACHINE_CONFIG_FRAGMENT( m3comm )
	MCFG_CPU_ADD(M68K_TAG, M68000, 10000000) // random
	MCFG_CPU_PROGRAM_MAP(m3comm_mem)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("128K")
MACHINE_CONFIG_END


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type M3COMM = &device_creator<m3comm_device>;

//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor m3comm_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( m3comm );
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m3comm_device - constructor
//-------------------------------------------------

m3comm_device::m3comm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, M3COMM, "MODEL-3 COMMUNICATION BD", tag, owner, clock, "m3comm", __FILE__),
	m68k_ram(*this, "m68k_ram"),
	m_commcpu(*this, M68K_TAG),
	m_ram(*this, RAM_TAG),
	m_line_rx(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE ),
	m_line_tx(OPEN_FLAG_READ)
{
	// prepare localhost "filename"
	m_localhost[0] = 0;
	strcat(m_localhost, "socket.");
	strcat(m_localhost, mconfig.options().comm_localhost());
	strcat(m_localhost, ":");
	strcat(m_localhost, mconfig.options().comm_localport());

	// prepare remotehost "filename"
	m_remotehost[0] = 0;
	strcat(m_remotehost, "socket.");
	strcat(m_remotehost, mconfig.options().comm_remotehost());
	strcat(m_remotehost, ":");
	strcat(m_remotehost, mconfig.options().comm_remoteport());
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m3comm_device::device_start()
{
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void m3comm_device::device_reset()
{
	naomi_control = 0xC000;
	naomi_offset = 0;
	m_status0 = 0;
	m_status1 = 0;
	m_commbank = 0;
	membank("comm_ram")->set_base(m_ram->pointer());
}

void m3comm_device::device_reset_after_children()
{
	m_commcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}

/////////////

UINT16 swapb16(UINT16 data)
{
	return (data << 8) | (data >> 8);
}

///////////// Internal MMIO

READ16_MEMBER(m3comm_device::ctrl_r)
{
	switch (offset) {
	case 0x00 / 2:
		return m_commbank;
	default:
		logerror("M3COMM CtrlRead from %04x mask %04x unimplemented!\n", offset * 2, mem_mask);
		return 0;
	}
}
WRITE16_MEMBER(m3comm_device::ctrl_w)
{
	switch (offset) {
	case 0x00 / 2:		// Communication RAM bank switch (flipped in IRQ2 and IRQ5 handlers)
		m_commbank = data;
		membank("comm_ram")->set_base(m_ram->pointer() + ((m_commbank & 1) ? 0x10000 : 0));
		break;
	case 0x40 / 2:		// IRQ 5 ACK
		m_commcpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
		break;
	case 0xA0 / 2:		// IRQ 2 ACK
		m_commcpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
		break;
	case 0x80 / 2:		// LEDs
	case 0xC0 / 2:		// possible node unique ID and broadcast flag (7FFF) ????
	case 0xE0 / 2:		// unknown, conditionally cleared in IRQ6 (receive complete) handler
		break;
	default:
		logerror("M3COMM CtrlWrite to %04x %04x mask %04x\n", offset * 2, data, mem_mask);
	}
}

READ16_MEMBER(m3comm_device::ioregs_r)
{
	switch (offset) {
	case 0x10 / 2:	// receive result/status
		return 5; // dbg random
	case 0x18 / 2:	// transmit result/status
		return 5; // dbg random
	case 0x88 / 2:
		return m_status0;
	case 0x8A / 2:
		return m_status1;
	default:
		logerror("M3COMM IOread from %02x mask %04x\n", offset * 2, mem_mask);
		return 0;
	}
}
WRITE16_MEMBER(m3comm_device::ioregs_w)
{
	switch (offset) {
	case 0x14 / 2:	// written 80 at data receive enable, 0 then 1 at IRQ6 handler
		break;		// it seems one of these ^v is IRQ6 ON/ACK, another is data transfer enable
	case 0x16 / 2:  // written 8C at data receive enable, 0 at IRQ6 handler
		if ((data & 0xFF) == 0x8C) {
			logerror("M3COMM Receive offs %04x size %04x\n", recv_offset, recv_size);
		}
		m_commcpu->set_input_line(M68K_IRQ_6, ((data & 0xFF) == 0x8C) ? ASSERT_LINE : CLEAR_LINE);	// debug hack
		break;
	case 0x1A / 2:	// written 80 at data transmit enable, 0 at IRQ4 handler
		break;		// it seems one of these ^v is IRQ4 ON/ACK, another is data transfer enable
	case 0x1C / 2:	// written 8C at data transmit enable, 0 at IRQ4 handler
		if ((data & 0xFF) == 0x8C) {
			logerror("M3COMM Send offs %04x size %04x\n", send_offset, send_size);
		}
		m_commcpu->set_input_line(M68K_IRQ_4, ((data & 0xFF) == 0x8C) ? ASSERT_LINE : CLEAR_LINE);	// debug hack
		break;
	case 0x40 / 2:
		recv_offset = (recv_offset >> 8) | (data << 8);
		break;
	case 0x42 / 2:
		recv_size = (recv_size >> 8) | (data << 8);
		break;
	case 0x44 / 2:
		send_offset = (send_offset >> 8) | (data << 8);
		break;
	case 0x46 / 2:
		send_size = (send_size >> 8) | (data << 8);
		break;
	case 0x88 / 2:
		m_status0 = (m_status0 & ~mem_mask) | (data & mem_mask);
		break;
	case 0x8A / 2:
		m_status1 = (m_status1 & ~mem_mask) | (data & mem_mask);
		break;
	case 0xC0 / 2:
		m_commcpu->set_input_line(INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
		break;
	default:
		logerror("M3COMM IOwrite to %02x %04x mask %04x\n", offset * 2, data, mem_mask);
		return;
	}
}

////////////// Model3 interface

READ16_MEMBER(m3comm_device::m3_m68k_ram_r)
{
	UINT16 value = m68k_ram[offset];
	return swapb16(value);
}
WRITE16_MEMBER(m3comm_device::m3_m68k_ram_w)
{
	m68k_ram[offset] = swapb16(data);
}
READ8_MEMBER(m3comm_device::m3_comm_ram_r)
{
	UINT8 *commram = (UINT8*)membank("comm_ram")->base();
	return commram[offset ^ 3];
}
WRITE8_MEMBER(m3comm_device::m3_comm_ram_w)
{
	UINT8 *commram = (UINT8*)membank("comm_ram")->base();
	commram[offset ^ 3] = data;
}
READ16_MEMBER(m3comm_device::m3_ioregs_r)
{
	UINT16 value = ioregs_r(space, offset, swapb16(mem_mask));
	return swapb16(value);
}
WRITE16_MEMBER(m3comm_device::m3_ioregs_w)
{
	UINT16 value = swapb16(data);
	ioregs_w(space, offset, value, swapb16(mem_mask));
}

////////////// NAOMI inerface

READ16_MEMBER(m3comm_device::naomi_r)
{
	switch (offset)
	{
	case 0:			// 5F7018
		return naomi_control;
	case 1:			// 5F701C
		return naomi_offset;
	case 2:			// 5F7020
	{
//		logerror("M3COMM read @ %08x\n", (naomi_control << 16) | naomi_offset);
		UINT16 value;
		if (naomi_control & 1)
			value = m68k_ram[naomi_offset / 2];
		else {
			UINT16 *commram = (UINT16*)membank("comm_ram")->base();

			value = commram[naomi_offset / 2];
		}
		naomi_offset += 2;
		return value;
	}
	case 3:			// 5F7024
		return m_status0;
	case 4:			// 5F7028
		return m_status1;
	default:
		return 0;
	}
}
WRITE16_MEMBER(m3comm_device::naomi_w)
{
	switch (offset)
	{
	case 0:			// 5F7018
					// bit 0: access RAM is 0 - communication RAM / 1 - M68K RAM
					// bit 1: comm RAM bank ??? (not really used)
					// bit 5: M68K Reset
					// bit 6: ???
					// bit 7: ???
					// bit 14: G1 DMA bus master 0 - active / 1 - disabled
					// bit 15: 0 - enable / 1 - disable this device ???
//		logerror("M3COMM control write %04x\n", data);
		naomi_control = data;
		m_commcpu->set_input_line(INPUT_LINE_RESET, (naomi_control & 0x20) ? CLEAR_LINE : ASSERT_LINE);
		break;
	case 1:			// 5F701C
		naomi_offset = data;
		break;
	case 2:			// 5F7020
//		logerror("M3COMM write @ %08x %04x\n", (naomi_control << 16) | naomi_offset, data);
		if (naomi_control & 1)
			m68k_ram[naomi_offset / 2] = data;
		else {
			UINT16 *commram = (UINT16*)membank("comm_ram")->base();
			commram[naomi_offset / 2] = data;
		}
		naomi_offset += 2;
		break;
	case 3:			// 5F7024
		m_status0 = data;
		break;
	case 4:			// 5F7028
		m_status1 = data;
		break;
	}
}
