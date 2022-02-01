// license:BSD-3-Clause
// copyright-holders:MetalliC


// Communication board used by Sega in Model3, NAOMI and Hikaru, uses mostly same design
// interface to main board LATTICE ICs: Model3 - 315-5958, Hikaru - 315-5958A, NAOMI - 315-6194A


// TODO:
// does IRQ 2 correct ? or it fired at any IO reg write by host
// IRQ 5 source (data frame exchange cycle start signal on MASTER), can be some timer or 'token acquired' event ?
// how exactly comm RAM bank flipping works ?
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

//#define VERBOSE 1
#include "logmacro.h"

#define M68K_TAG     "m3commcpu"

//////// Model 3 (main CPU @ C00xxxxx) and Hikaru (MMctrl bank 0E) interface
void m3comm_device::m3_map(address_map &map)
{
	map(0x0000000, 0x000ffff).rw(FUNC(m3comm_device::m3_comm_ram_r), FUNC(m3comm_device::m3_comm_ram_w));
	map(0x0010000, 0x00101ff).rw(FUNC(m3comm_device::m3_ioregs_r), FUNC(m3comm_device::m3_ioregs_w)).umask32(0xffff0000);
	map(0x0020000, 0x003ffff).rw(FUNC(m3comm_device::m3_m68k_ram_r), FUNC(m3comm_device::m3_m68k_ram_w)).umask32(0xffff0000);
}


/*************************************
 *  M3COMM Memory Map
 *************************************/
void m3comm_device::m3comm_mem(address_map &map)
{
	map(0x0000000, 0x000ffff).ram().share("m68k_ram");
	map(0x0040000, 0x00400ff).rw(FUNC(m3comm_device::ctrl_r), FUNC(m3comm_device::ctrl_w));
	map(0x0080000, 0x008ffff).bankrw("comm_ram");
	map(0x00C0000, 0x00C00ff).rw(FUNC(m3comm_device::ioregs_r), FUNC(m3comm_device::ioregs_w));
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DEFINE_DEVICE_TYPE(M3COMM, m3comm_device, "m3comm", "Model 3 Communication Board")

//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void m3comm_device::device_add_mconfig(machine_config &config)
{
	M68000(config, m_commcpu, 10000000); // random
	m_commcpu->set_addrmap(AS_PROGRAM, &m3comm_device::m3comm_mem);

	RAM(config, RAM_TAG).set_default_size("128K");
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  m3comm_device - constructor
//-------------------------------------------------

m3comm_device::m3comm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, M3COMM, tag, owner, clock),
	m_line_rx(OPEN_FLAG_WRITE | OPEN_FLAG_CREATE ),
	m_line_tx(OPEN_FLAG_READ),
	m68k_ram(*this, "m68k_ram"),
	m_commcpu(*this, M68K_TAG),
	m_ram(*this, RAM_TAG)
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
	timer = timer_alloc(TIMER_IRQ5);
	timer->adjust(attotime::from_usec(10000));
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

uint16_t swapb16(uint16_t data)
{
	return (data << 8) | (data >> 8);
}


void m3comm_device::device_timer(emu_timer &timer, device_timer_id id, int param)
{
	if(id != TIMER_IRQ5)
		return;

	m_commcpu->set_input_line(M68K_IRQ_5, ASSERT_LINE);
	timer.adjust(attotime::from_usec(10000));   // there is it from actually ??????
}

///////////// Internal MMIO

uint16_t m3comm_device::ctrl_r(offs_t offset, uint16_t mem_mask)
{
	switch (offset) {
	case 0x00 / 2:
		return m_commbank;
	default:
		LOG("M3COMM CtrlRead from %04x mask %04x unimplemented!\n", offset * 2, mem_mask);
		return 0;
	}
}
void m3comm_device::ctrl_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset) {
	case 0x00 / 2:      // Communication RAM bank switch (flipped in IRQ2 and IRQ5 handlers)
		m_commbank = data;
		membank("comm_ram")->set_base(m_ram->pointer() + ((m_commbank & 1) ? 0x10000 : 0));
		break;
	case 0x40 / 2:      // IRQ 5 ACK
		m_commcpu->set_input_line(M68K_IRQ_5, CLEAR_LINE);
		break;
	case 0xA0 / 2:      // IRQ 2 ACK
		m_commcpu->set_input_line(M68K_IRQ_2, CLEAR_LINE);
		break;
	case 0x80 / 2:      // LEDs
	case 0xC0 / 2:      // possible node unique ID and broadcast flag (7FFF) ????
	case 0xE0 / 2:      // unknown, conditionally cleared in IRQ6 (receive complete) handler
		break;
	default:
		LOG("M3COMM CtrlWrite to %04x %04x mask %04x\n", offset * 2, data, mem_mask);
	}
}

uint16_t m3comm_device::ioregs_r(offs_t offset, uint16_t mem_mask)
{
	switch (offset) {
	case 0x00 / 2:  // UNK, Model3 host wait it to be NZ then write 0
			// perhaps Model3 IO regs 0-80 mapped not to M68K C0000-80 ?
		return 1;
	case 0x10 / 2:  // receive result/status
		return 5; // dbg random
	case 0x18 / 2:  // transmit result/status
		return 5; // dbg random
	case 0x82 / 2: // IRQ/status ?
		return 0xA0;
	case 0x88 / 2:
		return m_status0;
	case 0x8A / 2:
		return m_status1;
	default:
		LOG("M3COMM IOread from %02x mask %04x\n", offset * 2, mem_mask);
		return 0;
	}
}
void m3comm_device::ioregs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (offset) {
	case 0x14 / 2:  // written 80 at data receive enable, 0 then 1 at IRQ6 handler
		if ((data & 0xFF) != 0x80)
			m_commcpu->set_input_line(M68K_IRQ_6, CLEAR_LINE);
		break;      // it seems one of these ^v is IRQ6 ON/ACK, another is data transfer enable
	case 0x16 / 2:  // written 8C at data receive enable, 0 at IRQ6 handler
		if ((data & 0xFF) == 0x8C) {
			LOG("M3COMM Receive offs %04x size %04x\n", recv_offset, recv_size);
/*
            if (!m_line_rx.is_open())
            {
                LOG("M3COMM: listen on %s\n", m_localhost);
                m_line_rx.open(m_localhost);
            }
            if (m_line_rx.is_open())
            {
                uint8_t *commram = (uint8_t*)membank("comm_ram")->base();
                m_line_rx.read(&commram[recv_offset], recv_size);
            }
*/
		m_commcpu->set_input_line(M68K_IRQ_6, ASSERT_LINE); // debug hack
		}
		break;
	case 0x1A / 2:  // written 80 at data transmit enable, 0 at IRQ4 handler
		break;      // it seems one of these ^v is IRQ4 ON/ACK, another is data transfer enable
	case 0x1C / 2:  // written 8C at data transmit enable, 0 at IRQ4 handler
		if ((data & 0xFF) == 0x8C) {
			LOG("M3COMM Send offs %04x size %04x\n", send_offset, send_size);
/*
            if (!m_line_tx.is_open())
            {
                LOG("M3COMM: connect to %s\n", m_remotehost);
                m_line_tx.open(m_remotehost);
            }
            if (m_line_tx.is_open())
            {
                uint8_t *commram = (uint8_t*)membank("comm_ram")->base();
                m_line_tx.write(&commram[send_offset], send_size);
            }
*/
		}
		m_commcpu->set_input_line(M68K_IRQ_4, ((data & 0xFF) == 0x8C) ? ASSERT_LINE : CLEAR_LINE);  // debug hack
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
		COMBINE_DATA(&m_status0);
		break;
	case 0x8A / 2:
		COMBINE_DATA(&m_status1);
		break;
	case 0xC0 / 2:
		m_commcpu->set_input_line(INPUT_LINE_RESET, data ? CLEAR_LINE : ASSERT_LINE);
		break;
	default:
		LOG("M3COMM IOwrite to %02x %04x mask %04x\n", offset * 2, data, mem_mask);
		return;
	}
}

////////////// Model3 interface

uint16_t m3comm_device::m3_m68k_ram_r(offs_t offset)
{
	uint16_t value = m68k_ram[offset];        // FIXME endian
	return swapb16(value);
}
void m3comm_device::m3_m68k_ram_w(offs_t offset, uint16_t data)
{
	m68k_ram[offset] = swapb16(data);       // FIXME endian
}
uint8_t m3comm_device::m3_comm_ram_r(offs_t offset)
{
	uint8_t *commram = (uint8_t*)membank("comm_ram")->base();
	return commram[offset ^ 3];
}
void m3comm_device::m3_comm_ram_w(offs_t offset, uint8_t data)
{
	uint8_t *commram = (uint8_t*)membank("comm_ram")->base();
	commram[offset ^ 3] = data;
}
uint16_t m3comm_device::m3_ioregs_r(offs_t offset, uint16_t mem_mask)
{
	uint16_t value = ioregs_r(offset, swapb16(mem_mask));
	return swapb16(value);
}
void m3comm_device::m3_ioregs_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	uint16_t value = swapb16(data);
	ioregs_w(offset, value, swapb16(mem_mask));

	// guess, can be asserted at any reg write
	if (offset == (0x88 / 2))
		m_commcpu->set_input_line(M68K_IRQ_2, ASSERT_LINE);
}

////////////// NAOMI interface

uint16_t m3comm_device::naomi_r(offs_t offset)
{
	switch (offset)
	{
	case 0:         // 5F7018
		return naomi_control;
	case 1:         // 5F701C
		return naomi_offset;
	case 2:         // 5F7020
	{
//      LOG("M3COMM read @ %08x\n", (naomi_control << 16) | naomi_offset);
		uint16_t value;
		if (naomi_control & 1)
			value = m68k_ram[naomi_offset / 2];     // FIXME endian
		else {
			uint16_t *commram = (uint16_t*)membank("comm_ram")->base();

			value = commram[naomi_offset / 2];      // FIXME endian
		}
		naomi_offset += 2;
		return value;
	}
	case 3:         // 5F7024
		return m_status0;
	case 4:         // 5F7028
		return m_status1;
	default:
		return 0;
	}
}
void m3comm_device::naomi_w(offs_t offset, uint16_t data)
{
	switch (offset)
	{
	case 0:         // 5F7018
					// bit 0: access RAM is 0 - communication RAM / 1 - M68K RAM
					// bit 1: comm RAM bank (seems R/O for SH4)
					// bit 5: M68K Reset
					// bit 6: ???
					// bit 7: ??? nlCbIntr reset this bit at each VBLANK-IN during game run (but not during handshake), might be M68K IRQ 5 or 2
					// bit 14: G1 DMA bus master 0 - active / 1 - disabled
					// bit 15: 0 - enable / 1 - disable this device ???
//      LOG("M3COMM control write %04x\n", data);
		naomi_control = data;
		m_commcpu->set_input_line(INPUT_LINE_RESET, (naomi_control & 0x20) ? CLEAR_LINE : ASSERT_LINE);
		break;
	case 1:         // 5F701C
		naomi_offset = data;
		break;
	case 2:         // 5F7020
//      LOG("M3COMM write @ %08x %04x\n", (naomi_control << 16) | naomi_offset, data);
		if (naomi_control & 1)
			m68k_ram[naomi_offset / 2] = data;      // FIXME endian
		else {
			uint16_t *commram = (uint16_t*)membank("comm_ram")->base();
			commram[naomi_offset / 2] = data;       // FIXME endian
		}
		naomi_offset += 2;
		break;
	case 3:         // 5F7024
		m_status0 = data;
		break;
	case 4:         // 5F7028
		m_status1 = data;
		break;
	}
}
