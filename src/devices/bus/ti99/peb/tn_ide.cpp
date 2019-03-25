// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    Thierry Nouspikel's IDE card emulation

    This card is just a prototype.  It has been designed by Thierry Nouspikel,
    and its description was published in 2001.  The card has been revised in
    2004.

    The specs have been published in <http://www.nouspikel.com/ti99/ide.html>.

    The IDE interface is quite simple, since it only implements PIO transfer.
    The card includes a clock chip to timestamp files, and an SRAM for the DSR.
    It should be possible to use a battery backed DSR SRAM, but since the clock
    chip includes 4kb of battery-backed RAM, a bootstrap loader can be saved in
    the clock SRAM in order to load the DSR from the HD when the computer
    starts.

    Raphael Nabet, 2002-2004.

    Michael Zapf
    September 2010: Rewritten as device
    February 2012: Rewritten as class

    FIXME: Completely untested and likely to be broken

*****************************************************************************/

#include "emu.h"
#include "tn_ide.h"

DEFINE_DEVICE_TYPE_NS(TI99_IDE, bus::ti99::peb, nouspikel_ide_interface_device, "ti99_ide", "Nouspikel IDE interface card")

namespace bus { namespace ti99 { namespace peb {

#define CRU_BASE 0x1000

#define RAMREGION "ram"

/* previously 0xff */
#define PAGE_MASK 0x3f

enum
{
	cru_reg_page_switching = 0x04,
	cru_reg_page_0 = 0x08,
	/*cru_reg_rambo = 0x10,*/   /* not emulated */
	cru_reg_wp = 0x20,
	cru_reg_int_en = 0x40,
	cru_reg_reset = 0x80
};

nouspikel_ide_interface_device::nouspikel_ide_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_IDE, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_cru_register(0),
	m_rtc(*this, "ide_rtc"),
	m_ata(*this, "ata"),
	m_clk_irq(false), m_sram_enable(false),
	m_sram_enable_dip(false), m_cur_page(0), m_tms9995_mode(false),
	m_input_latch(0), m_output_latch(0), m_ram(*this, RAMREGION)
{
}

/*
    CRU read
*/
READ8Z_MEMBER(nouspikel_ide_interface_device::crureadz)
{
	uint8_t reply = 0;
	if ((offset & 0xff00)==m_cru_base)
	{
		int bit = (offset >> 4) & 7;

		if (bit==0)
		{
			reply = m_cru_register & 0x30;
			reply |= 8; /* IDE bus IORDY always set */
			if (!m_clk_irq)
				reply |= 4;
			if (m_sram_enable_dip)
				reply |= 2;
			if (!m_ata_irq)
				reply |= 1;
		}
		*value = reply;
	}
}

/*
    CRU write
*/
WRITE8_MEMBER(nouspikel_ide_interface_device::cruwrite)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		int bit = (offset >>1) & 7;
		switch (bit)
		{
		case 0:
			m_selected = (data!=0);

		case 1:         // enable SRAM or registers in 0x4000-0x40ff
			m_sram_enable = (data!=0);
			break;

		case 2:         // enable SRAM page switching
		case 3:         // force SRAM page 0
		case 4:         // enable SRAM in 0x6000-0x7000 ("RAMBO" mode)
		case 5:         // write-protect RAM
		case 6:         // irq and reset enable
		case 7:         // reset drive
			if (data!=0)
				m_cru_register |= 1 << bit;
			else
				m_cru_register &= ~(1 << bit);

			if (bit == 6)
				m_slot->set_inta((m_cru_register & cru_reg_int_en) && m_ata_irq);

			if ((bit == 6) || (bit == 7))
				if ((m_cru_register & cru_reg_int_en) && !(m_cru_register & cru_reg_reset))
					m_ata->reset();
			break;
		}
	}
}

/*
    Memory read
*/
READ8Z_MEMBER(nouspikel_ide_interface_device::readz)
{
	uint8_t reply = 0;
	if (machine().side_effects_disabled()) return;

	if (((offset & m_select_mask)==m_select_value) && m_selected)
	{
		int addr = offset & 0x1fff;

		if ((addr <= 0xff) && (m_sram_enable == m_sram_enable_dip))
		{   /* registers */
			switch ((addr >> 5) & 0x3)
			{
			case 0:     /* RTC RAM */
				if (addr & 0x80)
					/* RTC RAM page register */
					reply = m_rtc->xram_r(machine().dummy_space(), (addr & 0x1f) | 0x20);
				else
					/* RTC RAM read */
					reply = m_rtc->xram_r(machine().dummy_space(), addr);
				break;
			case 1:     /* RTC registers */
				if (addr & 0x10)
					/* register data */
					reply = m_rtc->rtc_r(machine().dummy_space(), 1);
				else
					/* register select */
					reply = m_rtc->rtc_r(machine().dummy_space(), 0);
				break;
			case 2:     /* IDE registers set 1 (CS1Fx) */
				if (m_tms9995_mode ? (!(addr & 1)) : (addr & 1))
				{   /* first read triggers 16-bit read cycle */
					m_input_latch = (! (addr & 0x10)) ? m_ata->read_cs0((addr >> 1) & 0x7) : 0;
				}

				/* return latched input */
				/*reply = (addr & 1) ? input_latch : (input_latch >> 8);*/
				/* return latched input - bytes are swapped in 2004 IDE card */
				reply = ((addr & 1) ? (m_input_latch >> 8) : m_input_latch) & 0xff;
				break;
			case 3:     /* IDE registers set 2 (CS3Fx) */
				if (m_tms9995_mode ? (!(addr & 1)) : (addr & 1))
				{   /* first read triggers 16-bit read cycle */
					m_input_latch = (! (addr & 0x10)) ? m_ata->read_cs1((addr >> 1) & 0x7) : 0;
				}

				/* return latched input */
				/*reply = (addr & 1) ? input_latch : (input_latch >> 8);*/
				/* return latched input - bytes are swapped in 2004 IDE card */
				reply = ((addr & 1) ? (m_input_latch >> 8) : m_input_latch) & 0xff;
				break;
			}
		}
		else
		{   /* sram */
			if ((m_cru_register & cru_reg_page_0) || (addr >= 0x1000))
				reply = m_ram->pointer()[addr+0x2000 * m_cur_page];
			else
				reply = m_ram->pointer()[addr];
		}
		*value = reply;
	}
}

/*
    Memory write. The controller is 16 bit, so we need to demultiplex again.
*/
WRITE8_MEMBER(nouspikel_ide_interface_device::write)
{
	if (machine().side_effects_disabled()) return;

	if (((offset & m_select_mask)==m_select_value) && m_selected)
	{
		if (m_cru_register & cru_reg_page_switching)
		{
			m_cur_page = (offset >> 1) & PAGE_MASK;
		}

		int addr = offset & 0x1fff;

		if ((addr <= 0xff) && (m_sram_enable == m_sram_enable_dip))
		{   /* registers */
			switch ((addr >> 5) & 0x3)
			{
			case 0:     /* RTC RAM */
				if (addr & 0x80)
					/* RTC RAM page register */
					m_rtc->xram_w(machine().dummy_space(), (addr & 0x1f) | 0x20, data);
				else
					/* RTC RAM write */
					m_rtc->xram_w(machine().dummy_space(), addr, data);
				break;
			case 1:     /* RTC registers */
				if (addr & 0x10)
					/* register data */
					m_rtc->rtc_w(machine().dummy_space(), 1, data);
				else
					/* register select */
					m_rtc->rtc_w(machine().dummy_space(), 0, data);
				break;
			case 2:     /* IDE registers set 1 (CS1Fx) */
/*
                if (addr & 1)
                    m_output_latch = (m_output_latch & 0xff00) | data;
                else
                    m_output_latch = (m_output_latch & 0x00ff) | (data << 8);
*/
				/* latch write - bytes are swapped in 2004 IDE card */
				if (addr & 1)
					m_output_latch = (m_output_latch & 0x00ff) | (data << 8);
				else
					m_output_latch = (m_output_latch & 0xff00) | data;

				if (m_tms9995_mode ? (addr & 1) : (!(addr & 1)))
				{   /* second write triggers 16-bit write cycle */
					m_ata->write_cs0((addr >> 1) & 0x7, m_output_latch);
				}
				break;
			case 3:     /* IDE registers set 2 (CS3Fx) */
/*
                if (addr & 1)
                    m_output_latch = (m_output_latch & 0xff00) | data;
                else
                    m_output_latch = (m_output_latch & 0x00ff) | (data << 8);
*/
				/* latch write - bytes are swapped in 2004 IDE card */
				if (addr & 1)
					m_output_latch = (m_output_latch & 0x00ff) | (data << 8);
				else
					m_output_latch = (m_output_latch & 0xff00) | data;

				if (m_tms9995_mode ? (addr & 1) : (!(addr & 1)))
				{   /* second write triggers 16-bit write cycle */
					m_ata->write_cs1((addr >> 1) & 0x7, m_output_latch);
				}
				break;
			}
		}
		else
		{   /* sram */
			if (! (m_cru_register & cru_reg_wp))
			{
				if ((m_cru_register & cru_reg_page_0) || (addr >= 0x1000))
					m_ram->pointer()[addr+0x2000 * m_cur_page] = data;
				else
					m_ram->pointer()[addr] = data;
			}
		}
	}
}

void nouspikel_ide_interface_device::do_inta(int state)
{
	m_slot->set_inta(state);
}

/*
    ti99_ide_interrupt()
    IDE interrupt callback
*/
WRITE_LINE_MEMBER(nouspikel_ide_interface_device::ide_interrupt_callback)
{
	m_ata_irq = state;
	if (m_cru_register & cru_reg_int_en)
		do_inta(state);
}

/*
    clk_interrupt_callback()
    clock interrupt callback
*/
WRITE_LINE_MEMBER(nouspikel_ide_interface_device::clock_interrupt_callback)
{
	m_clk_irq = (state!=0);
	m_slot->set_inta(state);
}

void nouspikel_ide_interface_device::device_start()
{
	m_sram_enable_dip = false; // TODO: what is this?

	save_item(NAME(m_ata_irq));
	save_item(NAME(m_cru_register));
	save_item(NAME(m_clk_irq));
	save_item(NAME(m_sram_enable));
	save_item(NAME(m_sram_enable_dip));
	save_item(NAME(m_cur_page));
	save_item(NAME(m_tms9995_mode));
	save_item(NAME(m_input_latch));
	save_item(NAME(m_output_latch));
}

void nouspikel_ide_interface_device::device_reset()
{
	m_cur_page = 0;
	m_sram_enable = false;
	m_cru_register = 0;

	if (m_genmod)
	{
		m_select_mask = 0x1fe000;
		m_select_value = 0x174000;
	}
	else
	{
		m_select_mask = 0x7e000;
		m_select_value = 0x74000;
	}
	m_selected = false;

	m_cru_base = ioport("CRUIDE")->read();
	m_clk_irq = false;

	m_tms9995_mode =  false; // (device->type()==TMS9995);
}

INPUT_PORTS_START( tn_ide )
	PORT_START( "CRUIDE" )
	PORT_DIPNAME( 0x1f00, 0x1000, "IDE CRU base" )
		PORT_DIPSETTING( 0x1000, "1000" )
		PORT_DIPSETTING( 0x1100, "1100" )
		PORT_DIPSETTING( 0x1200, "1200" )
		PORT_DIPSETTING( 0x1300, "1300" )
		PORT_DIPSETTING( 0x1400, "1400" )
		PORT_DIPSETTING( 0x1500, "1500" )
		PORT_DIPSETTING( 0x1600, "1600" )
		PORT_DIPSETTING( 0x1700, "1700" )
		PORT_DIPSETTING( 0x1800, "1800" )
		PORT_DIPSETTING( 0x1900, "1900" )
		PORT_DIPSETTING( 0x1a00, "1A00" )
		PORT_DIPSETTING( 0x1b00, "1B00" )
		PORT_DIPSETTING( 0x1c00, "1C00" )
		PORT_DIPSETTING( 0x1d00, "1D00" )
		PORT_DIPSETTING( 0x1e00, "1E00" )
		PORT_DIPSETTING( 0x1f00, "1F00" )
INPUT_PORTS_END

void nouspikel_ide_interface_device::device_add_mconfig(machine_config &config)
{
	RTC65271(config, m_rtc, 0);
	m_rtc->interrupt_cb().set(FUNC(nouspikel_ide_interface_device::clock_interrupt_callback));

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
	m_ata->irq_handler().set(FUNC(nouspikel_ide_interface_device::ide_interrupt_callback));

	RAM(config, m_ram);
	m_ram->set_default_size("512K");
	m_ram->set_default_value(0);
}

ioport_constructor nouspikel_ide_interface_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tn_ide);
}

} } } // end namespace bus::ti99::peb

