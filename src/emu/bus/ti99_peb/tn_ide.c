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
#include "peribox.h"
#include "machine/ataintf.h"
#include "tn_ide.h"

#define CRU_BASE 0x1000

#define BUFFER_TAG "ram"

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

nouspikel_ide_interface_device::nouspikel_ide_interface_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: ti_expansion_card_device(mconfig, TI99_IDE, "Nouspikel IDE interface card", tag, owner, clock, "ti99_ide", __FILE__),
	m_ata(*this, "ata")
{
}

/*
    CRU read
*/
READ8Z_MEMBER(nouspikel_ide_interface_device::crureadz)
{
	UINT8 reply = 0;
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
	UINT8 reply = 0;
	if (space.debugger_access()) return;

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
					reply = m_rtc->xram_r(machine().driver_data()->generic_space(),(addr & 0x1f) | 0x20);
				else
					/* RTC RAM read */
					reply = m_rtc->xram_r(machine().driver_data()->generic_space(),addr);
				break;
			case 1:     /* RTC registers */
				if (addr & 0x10)
					/* register data */
					reply = m_rtc->rtc_r(machine().driver_data()->generic_space(),1);
				else
					/* register select */
					reply = m_rtc->rtc_r(machine().driver_data()->generic_space(),0);
				break;
			case 2:     /* IDE registers set 1 (CS1Fx) */
				if (m_tms9995_mode ? (!(addr & 1)) : (addr & 1))
				{   /* first read triggers 16-bit read cycle */
					m_input_latch = (! (addr & 0x10)) ? m_ata->read_cs0(space, (addr >> 1) & 0x7, 0xffff) : 0;
				}

				/* return latched input */
				/*reply = (addr & 1) ? input_latch : (input_latch >> 8);*/
				/* return latched input - bytes are swapped in 2004 IDE card */
				reply = ((addr & 1) ? (m_input_latch >> 8) : m_input_latch) & 0xff;
				break;
			case 3:     /* IDE registers set 2 (CS3Fx) */
				if (m_tms9995_mode ? (!(addr & 1)) : (addr & 1))
				{   /* first read triggers 16-bit read cycle */
					m_input_latch = (! (addr & 0x10)) ? m_ata->read_cs1(space, (addr >> 1) & 0x7, 0xffff) : 0;
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
				reply = m_ram[addr+0x2000 * m_cur_page];
			else
				reply = m_ram[addr];
		}
		*value = reply;
	}
}

/*
    Memory write. The controller is 16 bit, so we need to demultiplex again.
*/
WRITE8_MEMBER(nouspikel_ide_interface_device::write)
{
	if (space.debugger_access()) return;

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
					m_rtc->xram_w(machine().driver_data()->generic_space(),(addr & 0x1f) | 0x20, data);
				else
					/* RTC RAM write */
					m_rtc->xram_w(machine().driver_data()->generic_space(),addr, data);
				break;
			case 1:     /* RTC registers */
				if (addr & 0x10)
					/* register data */
					m_rtc->rtc_w(machine().driver_data()->generic_space(),1, data);
				else
					/* register select */
					m_rtc->rtc_w(machine().driver_data()->generic_space(),0, data);
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
					m_ata->write_cs0(space, (addr >> 1) & 0x7, m_output_latch, 0xffff);
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
					m_ata->write_cs1(space, (addr >> 1) & 0x7, m_output_latch, 0xffff);
				}
				break;
			}
		}
		else
		{   /* sram */
			if (! (m_cru_register & cru_reg_wp))
			{
				if ((m_cru_register & cru_reg_page_0) || (addr >= 0x1000))
					m_ram[addr+0x2000 * m_cur_page] = data;
				else
					m_ram[addr] = data;
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
	m_rtc = subdevice<rtc65271_device>("ide_rtc");

	m_ram = memregion(BUFFER_TAG)->base();
	m_sram_enable_dip = false; // TODO: what is this?
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

MACHINE_CONFIG_FRAGMENT( tn_ide )
	MCFG_DEVICE_ADD( "ide_rtc", RTC65271, 0 )
	MCFG_RTC65271_INTERRUPT_CB(WRITELINE(nouspikel_ide_interface_device, clock_interrupt_callback))
	MCFG_ATA_INTERFACE_ADD( "ata", ata_devices, "hdd", NULL, false)
	MCFG_ATA_INTERFACE_IRQ_HANDLER(WRITELINE(nouspikel_ide_interface_device, ide_interrupt_callback))
MACHINE_CONFIG_END

ROM_START( tn_ide )
	ROM_REGION(0x80000, BUFFER_TAG, 0)  /* RAM buffer 512 KiB */
	ROM_FILL(0x0000, 0x80000, 0x00)
ROM_END

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

machine_config_constructor nouspikel_ide_interface_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( tn_ide );
}

const rom_entry *nouspikel_ide_interface_device::device_rom_region() const
{
	return ROM_NAME( tn_ide );
}

ioport_constructor nouspikel_ide_interface_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tn_ide);
}

const device_type TI99_IDE = &device_creator<nouspikel_ide_interface_device>;
