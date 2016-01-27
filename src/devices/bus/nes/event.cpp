// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nintendo NES-EVENT PCB


 Here we emulate the following PCBs

 * Nintendo NES-EVENT [mapper 105]

 ***********************************************************************************************************/


#include "emu.h"
#include "event.h"

#include "cpu/m6502/m6502.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

const device_type NES_EVENT = &device_creator<nes_event_device>;


nes_event_device::nes_event_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
					: nes_sxrom_device(mconfig, NES_EVENT, "NES Cart Event PCB", tag, owner, clock, "nes_event", __FILE__),
						m_dsw(*this, "DIPSW"),
	m_nwc_init(0),
	event_timer(nullptr),
	m_timer_count(0),
	m_timer_on(0),
	m_timer_enabled(0)
				{
}


void nes_event_device::device_start()
{
	common_start();
	event_timer = timer_alloc(TIMER_EVENT);
	event_timer->adjust(attotime::never);
	timer_freq = machine().device<cpu_device>("maincpu")->cycles_to_attotime(1);

	save_item(NAME(m_latch));
	save_item(NAME(m_count));
	save_item(NAME(m_reg));
	save_item(NAME(m_reg_write_enable));
	save_item(NAME(m_nwc_init));

	save_item(NAME(m_timer_count));
	save_item(NAME(m_timer_on));
	save_item(NAME(m_timer_enabled));
}

void nes_event_device::pcb_reset()
{
	m_latch = 0;
	m_count = 0;
	m_reg[0] = 0x0f;
	m_reg[1] = m_reg[2] = m_reg[3] = 0;
	m_reg_write_enable = 1;
	m_nwc_init = 2;

	set_nt_mirroring(PPU_MIRROR_HORZ);
	chr8(0, CHRRAM);
	prg32(0);
	m_timer_count = 0;
	m_timer_enabled = 0;
	m_timer_on = 0;
}



/*-------------------------------------------------
 mapper specific handlers
 -------------------------------------------------*/

/*-------------------------------------------------

 Event PCB

 Games: Nintento World Championships

 MMC-1 variant with repurposed register at $a000 and a
 lot of discrete components

 iNES: mapper 105

 In MESS: Supported.

 -------------------------------------------------*/

void nes_event_device::set_chr()
{
	// no CHR switching, there are only 8KB VRAM from the cart
}

void nes_event_device::set_prg()
{
//  printf("enter with %d and reg1 0x%x - reg3 0x%x\n", m_nwc_init, m_reg[1], m_reg[3]);
	// reg[1] is different from base MMC-1!
	// bit 0 is ignored, bit1/bit3 are used for PRG switch, bit4 is used for the timer
	UINT8 temp = (m_reg[1] >> 1) & 7;

	// initially PRG is fixed, until bit4 of reg1 is set to 1 and then to 0
	switch (m_nwc_init)
	{
		case 2:
			if (m_reg[1] & 0x10) m_nwc_init--;
			return;
		case 1:
			if (~m_reg[1] & 0x10) m_nwc_init--;
			return;
	}

	if (temp < 4)
		prg32(temp);
	else
	{
		// else PRG works similarly to base MMC-1, but only acts on the higher 128KB (2nd PRG ROM)
		switch (m_reg[0] & 0x0c)
		{
			case 0x00:
			case 0x04:
				prg32(0x04 | ((m_reg[3] >> 1) & 0x03));
				break;
			case 0x08:
				prg16_89ab(0x08 | 0x00);
				prg16_cdef(0x08 | (m_reg[3] & 0x07));
				break;
			case 0x0c:
				prg16_89ab(0x08 | (m_reg[3] & 0x07));
				prg16_cdef(0x08 | 0x07);
				break;
		}
	}

	// after the init procedure above, bit4 of m_reg[1] is used to init IRQ, by setting and then clearing the bit
	// however, there are (bankswitch related?) writes with bit4 cleared before the one 'enabling' the timer, so
	// we need the additional m_timer_enabled variable, to avoid starting the timer before its time...
	if (m_reg[1] & 0x10)
	{
		m_timer_enabled = 1;
		m_maincpu->set_input_line(M6502_IRQ_LINE, CLEAR_LINE);
	}
	else
	{
		if (!m_timer_on && m_timer_enabled)
		{
			m_timer_count = 0x20000000 | ((m_dsw->read() & 0x0f) << 25);
			event_timer->adjust(attotime::zero, 0, timer_freq);
			m_timer_on = 1;
		}
	}
}

void nes_event_device::update_regs(int reg)
{
	switch (reg)
	{
		case 0:
			switch (m_reg[0] & 0x03)
			{
				case 0: set_nt_mirroring(PPU_MIRROR_LOW); break;
				case 1: set_nt_mirroring(PPU_MIRROR_HIGH); break;
				case 2: set_nt_mirroring(PPU_MIRROR_VERT); break;
				case 3: set_nt_mirroring(PPU_MIRROR_HORZ); break;
			}
			set_prg();
			break;
		case 1:
			set_prg();
			break;
		case 2:
			set_chr();
			break;
		case 3:
			set_prg();
			break;
	}
}

//-------------------------------------------------
//  Dipswicth
//-------------------------------------------------

static INPUT_PORTS_START( nwc_dsw )
	PORT_START("DIPSW")
	PORT_DIPNAME( 0x0f, 0x04, "Timer" ) PORT_DIPLOCATION("SW:!1,!2,!3,!4")
	PORT_DIPSETTING( 0x00, "5:00.4" )
	PORT_DIPSETTING( 0x01, "5:19.2" )
	PORT_DIPSETTING( 0x02, "5:38.0" )
	PORT_DIPSETTING( 0x03, "5:56.7" )
	PORT_DIPSETTING( 0x04, "6:15.5" )
	PORT_DIPSETTING( 0x05, "6:34.3" )
	PORT_DIPSETTING( 0x06, "6:53.1" )
	PORT_DIPSETTING( 0x07, "7:11.9" )
	PORT_DIPSETTING( 0x08, "7:30.6" )
	PORT_DIPSETTING( 0x09, "7:49.4" )
	PORT_DIPSETTING( 0x0a, "8:08.2" )
	PORT_DIPSETTING( 0x0b, "8:27.0" )
	PORT_DIPSETTING( 0x0c, "8:45.8" )
	PORT_DIPSETTING( 0x0d, "9:04.5" )
	PORT_DIPSETTING( 0x0e, "9:23.3" )
	PORT_DIPSETTING( 0x0f, "9:42.1" )
INPUT_PORTS_END



ioport_constructor nes_event_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( nwc_dsw );
}


//-------------------------------------------------
//  device_timer - handler timer events
//-------------------------------------------------

void nes_event_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	if (id == TIMER_EVENT)
	{
		m_timer_count--;
		if (!m_timer_count)
		{
			m_maincpu->set_input_line(M6502_IRQ_LINE, HOLD_LINE);
			event_timer->reset();
		}
	}
}
