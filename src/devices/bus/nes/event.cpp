// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***********************************************************************************************************


 NES/Famicom cartridge emulation for Nintendo NES-EVENT PCB


 Here we emulate the following PCBs

 * Nintendo NES-EVENT [mapper 105]

 ***********************************************************************************************************/


#include "emu.h"
#include "event.h"


#ifdef NES_PCB_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG_MMC(x) do { if (VERBOSE) logerror x; } while (0)


//-------------------------------------------------
//  constructor
//-------------------------------------------------

DEFINE_DEVICE_TYPE(NES_EVENT, nes_event_device, "nes_event", "NES Cart Event PCB")


nes_event_device::nes_event_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: nes_sxrom_device(mconfig, NES_EVENT, tag, owner, clock)
	, m_dsw(*this, "DIPSW")
	, m_nwc_init(0)
	, event_timer(nullptr)
	, m_timer_count(0)
	, m_timer_on(0)
	, m_timer_enabled(0)
{
}


void nes_event_device::device_start()
{
	nes_sxrom_device::device_start();

	event_timer = timer_alloc(TIMER_EVENT);
	event_timer->adjust(attotime::never);

	save_item(NAME(m_nwc_init));

	save_item(NAME(m_timer_count));
	save_item(NAME(m_timer_on));
	save_item(NAME(m_timer_enabled));
}

void nes_event_device::pcb_reset()
{
	nes_sxrom_device::pcb_reset();
	prg32(0);

	m_nwc_init = 2;

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

 MMC1 variant with repurposed register at $a000 and a
 lot of discrete components

 iNES: mapper 105

 In MAME: Supported.

 -------------------------------------------------*/

void nes_event_device::set_chr()
{
	// no CHR switching, there are only 8KB VRAM from the cart
}

void nes_event_device::set_prg()
{
//  printf("enter with %d and reg1 0x%x - reg3 0x%x\n", m_nwc_init, m_reg[1], m_reg[3]);
	// reg[1] is different from base MMC1!
	// bit 0 is ignored, bit1/bit3 are used for PRG switch, bit4 is used for the timer
	// initially PRG is fixed, until bit4 of reg1 is set to 1 and then to 0
	switch (m_nwc_init)
	{
		case 2:
			if (BIT(m_reg[1], 4)) m_nwc_init--;
			return;
		case 1:
			if (!BIT(m_reg[1], 4)) m_nwc_init--;
			return;
	}

	// PRG mode 1 works similarly to base MMC1, but only acts on the higher 128KB (2nd PRG ROM)
	if (BIT(m_reg[1], 3))
		nes_sxrom_device::set_prg(0x08, 0x07);
	else
		prg32(BIT(m_reg[1], 1, 2));

	// after the init procedure above, bit4 of m_reg[1] is used to init IRQ, by setting and then clearing the bit
	// however, there are (bankswitch related?) writes with bit4 cleared before the one 'enabling' the timer, so
	// we need the additional m_timer_enabled variable, to avoid starting the timer before its time...
	if (BIT(m_reg[1], 4))
	{
		m_timer_count = 0;
		m_timer_enabled = 1;
		set_irq_line(CLEAR_LINE);
	}
	else if (!m_timer_on && m_timer_enabled)
	{
		event_timer->adjust(attotime::zero, 0, clocks_to_attotime(1));
		m_timer_on = 1;
	}
}


//-------------------------------------------------
//  Dipswitch
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
	if (id == TIMER_EVENT && m_timer_on)
	{
		if (++m_timer_count >= (0x10 | m_dsw->read()) << 25)
		{
			set_irq_line(ASSERT_LINE);
			event_timer->reset();
		}
	}
}
