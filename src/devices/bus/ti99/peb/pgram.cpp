// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/*******************************************************************************
    PGRAM(+) Memory expansion and GROM simulator

    The PGRAM card is a battery-buffered RAM card. It also contains a circuitry
    to simulate GROMs (TMC0430), which can also be programmed, thus better
    called GRAMs.

    Memory chips of the board:

    u21: 43256LP (SRAM 32Kx8) -- GROMs 4-7
    u22:  6264LP (SRAM  8Kx8) -- GROM  3
    u23: 43256LP (SRAM 32Kx8) -- 16 KiB DSR (driver) / 16 KiB RAM

    The PGRAM+ is an extension of this card, offering 192 KiB memory:

    u21: e.g. BS62LV1027 (SRAM 128Kx8) -- GROMs 4-7 (4 banks)
    u22: 43256LP (SRAM  32Kx8) -- GROM  3 (4 banks)
    u23: 43256LP (SRAM  32Kx8) -- 16 KiB DSR (driver) / 16 KiB RAM

    The socket for u21 is already prepared for the larger package of the
    128Kx8 SRAM chip (32 pins). No modifications are needed; the smaller chip
    can simply be plugged into the larger socket. For u22, a jumper is included
    on the board for connecting the address line to the bus (PGRAM+)
    or to high level (PGRAM).

    A real-time clock is also included on the board:
    u1: MM58167A

    The GROM address counter is implemented by four 74LS161 chips.

    A 3V battery is used to buffer the SRAMs.

    The 74LS259 latch (u14) stores CRU settings and controls select lines.

    Memory/port mapping
    -------------------
    The card can be configured to respond to CRU addresses 1000-1700 with the
    exception of 1100, which is used by most disk controller cards. The
    address is set by one of seven switches (SW1).

    CRU bits (relative to start address):

    0 - Map DSR into CPU space 4000-5FFF
    1 - Enable GRAM and RAM
    2 - Write protect GRAM/RAM
    3 - DSR/RAM bank selection
    4 - DSR/RAM bank selection (see below)

    The GRAM simulation responds to the usual GROM address base 9800. However,
    only GROMs 3-7 are simulated; GROMs 0-2 are located in the console. Setting
    the GROM address is done by writing both bytes of the 16-bit address to
    address 9C02 (high/low). Reading the address is not possible, but this is
    provided by the console GROMs. Reading the contents of the GRAM simulation
    is done by reading a byte from address 9800, which also advances the address
    counter. Writing to the GRAM is achieved by writing to address 9C00.

    For the PGRAM+ card, four GROM banks are available:
    Bank 0: 98x0/9Cx0/9Cx2
    Bank 1: 98x4/9Cx4/9Cx4
    Bank 2: 98x8/9Cx8/9Cx8
    Bank 3: 98xC/9CxC/9CxC

    Due to incomplete decoding, the second digit from the right may take any
    value.

    The real-time clock is mapped to addresses 8640-865E (even addresses only).
    8640 = Register 0
    865e = Register 15

    The 32K RAM circuit (u23) is used for the DSR (device service routine, i.e.
    the driver) and 16 KiB RAM. The DSR area is located in the lower half of the
    address space, while the RAM area is located in the upper half.

    The DSR area is mapped as two selectable 8K blocks into 4000-5FFF.
    The RAM area is mapped as two selectable 8K blocks into 6000-7FFF.

    +------------+   0000
    | DSR bank 0 |
    +------------+   2000
    | DSR bank 1 |
    +------------+   4000
    | RAM bank 0 |
    +------------+   6000
    | RAM bank 1 |
    +------------+   7FFF

    The selection of either bank of the DSR space is done by writing a value
    to an address of 4000/4004/4008... for selecting bank 0, and to 4002/4006/...
    for selecting bank 1. For the RAM area, the same scheme is applied to
    6000/6004/... and 6002/6006/... The selection is buffered by the flipflop
    u3 for both RAM and DSR, hence it should make no difference whether to
    write to 6000 or to 4000.

    CRU bits 3 and 4 are fed into the PRE* and CLR* inputs of the flipflop.

    Bits 3  4
         0  0    -> not recommended (typically, bit 4 wins)
         0  1    -> set lower 8K
         1  0    -> set higher 8K
         1  1    -> allow switching by write access

    The PGRAM+ card does not allow for expanding this area. This is indeed a
    limitation, as it does not allow us to store several cartridges with
    a GROM and ROM part into the PGRAM+.

    Switch SW2 deactivates the card in case the contents prevent a proper
    startup of the TI console.

    Michael Zapf
    March 2020

*******************************************************************************/

#include "emu.h"
#include "pgram.h"

#define LOG_WARN       (1U<<1)
#define LOG_WP         (1U<<2)
#define LOG_DSR        (1U<<3)
#define LOG_RAM        (1U<<4)
#define LOG_GRAM       (1U<<5)
#define LOG_GRAMADDR   (1U<<6)
#define LOG_CRU        (1U<<7)
#define LOG_BANK       (1U<<8)

#define VERBOSE ( LOG_GENERAL | LOG_WARN )

#include "logmacro.h"

#define GRAM4567_TAG "u21_gram4567"
#define GRAM3_TAG "u22_gram3"
#define DSRRAM_TAG "u23_dsrram"
#define CLOCK_TAG "u1_rtc"
#define BANKFF_TAG "u3_bankff"
#define CRULATCH_TAG "u14_latch"

#define COUNT0_TAG "u13_counter"
#define COUNT1_TAG "u11_counter"
#define COUNT2_TAG "u12_counter"
#define COUNT3_TAG "u10_counter"

DEFINE_DEVICE_TYPE_NS(TI99_PGRAM, bus::ti99::peb, pgram_device, "ti99_pgram", "PGRAM(+) memory card")

namespace bus::ti99::peb {

pgram_device::pgram_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock):
	  device_t(mconfig, TI99_PGRAM, tag, owner, clock),
	  device_ti99_peribox_card_interface(mconfig, *this),
	  m_gram3(*this, GRAM3_TAG),
	  m_gram4567(*this, GRAM4567_TAG),
	  m_dsrram(*this, DSRRAM_TAG),
	  m_clock(*this, CLOCK_TAG),
	  m_crulatch(*this, CRULATCH_TAG),
	  m_bankff(*this, BANKFF_TAG),
	  m_count0(*this, COUNT0_TAG),
	  m_count1(*this, COUNT1_TAG),
	  m_count2(*this, COUNT2_TAG),
	  m_count3(*this, COUNT3_TAG),
	  m_lowbyte(false)
{
}

/*
    Read access to the card. This comprises reading from the RAM circuit for
    DSR and free space, for GRAM reading, and the real-time clock.

    Decoding is done by the 74ls138 and 74ls139 circuits on the board.
*/
void pgram_device::readz(offs_t offset, uint8_t *value)
{
	if (!m_active) return;

	// GROMs: 1001 1w.. .... ..00
	if ((offset & 0xfc03)==0x9800)
	{
		gram_read((offset>>2)&0x03, value);
	}
	else
	{
		// DSR/RAM: 01.
		if ((offset & 0xc000)==0x4000)
		{
			dsr_ram_read(offset & 0x3fff, value);
		}
		else
		{
			// RTC: 1000 0110 01.. ...0
			if ((offset & 0xffc1)==0x8640)
			{
				uint8_t val = m_clock->read((offset >> 1) & 0x001f);
				*value = val;
			}
		}
	}
}

/*
    Write access to the card. Same comments apply as for the read access.
*/
void pgram_device::write(offs_t offset, uint8_t data)
{
	if (!m_active) return;

	// GROMs: 1001 11.. .... ..a0
	if ((offset & 0xfc01)==0x9c00)
	{
		if ((offset & 0x0002)!=0)
			set_gram_address(data);
		else
			gram_write((offset>>2)&0x03, data);
	}
	else
	{
		// DSR/RAM: 01.
		if ((offset & 0xc000)==0x4000)
			dsr_ram_write(offset & 0x3fff, data);
		else
		{
			// RTC: 1000 0110 01.. ...0
			if ((offset & 0xffc1)==0x8640)
				m_clock->write((offset >> 1) & 0x001f, data);
		}
	}
}

/*
    Read access to the 32K RAM circuit (u23). This one is used to provide the
    DSR (driver) in the memory area 4000-5fff, and to provide free space in the
    area 6000-7fff. A second bank can be selected for both DSR and RAM using
    CRU bits 3 and 4.
*/
void pgram_device::dsr_ram_read(offs_t offset, uint8_t *value)
{
	// ..0. .... .... .... = DSR
	// ..1. .... .... .... = RAM
	bool dsr = ((offset & 0x2000)==0);

	// Note: On power-up, the ls259 latch is reset, and bits 4 and 3 lead 0
	// to both PRE* and CLR* of the 7474. This would normally cause the bank
	// select line to be 1 (unstable).

	// Reset the clock
	m_bankff->clock_w(1);

	// Latch may have changed; update FF asynchronously
	m_bankff->clear_w(m_crulatch->q3_r());
	m_bankff->preset_w(m_crulatch->q4_r());

	offs_t base = (offset & 0x2000)<<1;   // DSR:0000, RAM:4000
	offs_t address = base | (offset & 0x1fff) | (m_bankff->output_r()? 0x2000 : 0);

	if ((dsr && m_crulatch->q0_r()) || (!dsr && m_crulatch->q1_r()))
	{
		*value = m_dsrram->read(address);

		if (address&1)
		{
			uint16_t b0 = m_dsrram->read(address&0xfffe) << 8 | *value;

			if (dsr)
				LOGMASKED(LOG_DSR, "%04x (bank %d) -> %04x\n",(offset&0xfffe)|0x4000, m_bankff->output_r()? 1:0, b0);
			else
				LOGMASKED(LOG_RAM, "%04x (bank %d) -> %04x\n",(offset&0xfffe)|0x4000, m_bankff->output_r()? 1:0, b0);
		}
	}
}

/*
    Write access to the 32K RAM circuit (u23). See above for information about
    the DSR and RAM space.
    A flipflop (7474) is used to control access to the banks of both DSR and
    RAM. The state of the flipflop is controlled by CRU bits 3 and 4. When both
    bits are set, the flipflop can be set or reset by address line 14 (2^1).
*/
void pgram_device::dsr_ram_write(offs_t offset, uint8_t data)
{
	// ..0. .... .... .... = DSR
	// ..1. .... .... .... = RAM
	bool dsr = ((offset & 0x2000)==0);

	if ((dsr && m_crulatch->q0_r()) || (!dsr && m_crulatch->q1_r()))
	{
		// Get the current settings of the flipflop from the latch
		m_bankff->clear_w(m_crulatch->q3_r());
		m_bankff->preset_w(m_crulatch->q4_r());

		int oldff = m_bankff->output_r();

		// A14 is led to the flipflop D input
		m_bankff->d_w((offset & 2)>>1);  // A14
		m_bankff->clock_w(0);

		if (m_crulatch->q2_r()==0)  // not write-protected
		{
			offs_t base = (offset & 0x2000)<<1;
			offs_t address = base | (offset & 0x1fff) | (m_bankff->output_r()? 0x2000 : 0);

			m_dsrram->write(address, data);
			if (base==0)
				LOGMASKED(LOG_DSR, "%04x (bank %d) <- %02x\n", offset|0x4000, m_bankff->output_r()? 1:0, data);
			else
				LOGMASKED(LOG_RAM, "%04x (bank %d) <- %02x\n", offset|0x4000, m_bankff->output_r()? 1:0, data);
		}

		m_bankff->clock_w(1);
		if (m_bankff->output_r() != oldff)
			LOGMASKED(LOG_BANK, "Switch to bank %d (%04x)\n", m_bankff->output_r(), offset|0x4000);
	}
}

/*
    Read access to the GRAM emulation. The GRAM space is split between
    two memory circuits, one for G3 (8K) and one for the other four GRAMs (32K).
    For PGRAM+, we habe 32K + 128K (four banks).
    When the GROM address is outside the area for G3-G7, the access is ignored.
    GRAM access requires setting CRU bit 1 before.
*/
void pgram_device::gram_read(offs_t offset, uint8_t *value)
{
	// Don't let the debugger mess with the GRAM emulation
	if (machine().side_effects_disabled())
	{
		*value = 0;
		return;
	}

	if (m_crulatch->q1_r())
	{
		// Reset the clock
		m_bankff->clock_w(1);

		clock_gram_counter(0);
		uint16_t gaddress = get_gram_address();

		// GROM3: 011. .... .... ....
		// Checked by u9
		if ((gaddress & 0xe000)==0x6000)
		{
			offs_t gfull = (gaddress & 0x1fff) | (m_pgramplus? (offset << 13) : 0);
			*value = m_gram3->read(gfull);
			LOGMASKED(LOG_GRAM, "gram(%04x,%04x) -> %02x\n", gaddress, gfull, *value);
		}

		// GROM4-7: 1... .... .... ....
		// Checked by u9 + u8
		if ((gaddress & 0x8000)!=0)
		{
			offs_t gfull = (gaddress & 0x7fff) | (m_pgramplus? (offset << 15) : 0);
			*value = m_gram4567->read(gfull);
			LOGMASKED(LOG_GRAM, "gram(%04x,%05x) -> %02x\n", gaddress, gfull, *value);
		}
		clock_gram_counter(1);
	}
}

/*
    Write access to the GRAM emulation. Same comments apply as for gram_read.
*/
void pgram_device::gram_write(offs_t offset, uint8_t data)
{
	// Don't let the debugger mess with the GRAM emulation
	if (machine().side_effects_disabled())
	{
		return;
	}

	if (m_crulatch->q1_r())
	{
		clock_gram_counter(0);
		uint16_t gaddress = get_gram_address();

		// GROM3: 011. .... .... ....
		if ((gaddress & 0xe000)==0x6000)
		{
			offs_t gfull = (gaddress & 0x1fff) | (m_pgramplus? (offset << 13) : 0);
			m_gram3->write(gfull, data);
			LOGMASKED(LOG_GRAM, "gram(%04x,%04x) <- %02x\n", gaddress, gfull, data);
		}

		// GROM4-7: 1... .... .... ....
		if ((gaddress & 0x8000)!=0)
		{
			offs_t gfull = (gaddress & 0x7fff) | (m_pgramplus? (offset << 15) : 0);
			m_gram4567->write(gfull, data);
			LOGMASKED(LOG_GRAM, "gram(%04x,%05x) <- %02x\n", gaddress, gfull, data);
		}
		clock_gram_counter(1);
	}
}

/*
    Set the address digits in the counters. Address setting is done by two
    consecutive byte writes, where the currently stored digits in u13 and u11
    are transferred to u12 and u10, and u13 and u11 get their new values
    from the data bus.

    D0-D3 -> u11 -> u10
    D4-D7 -> u13 -> u12
*/
void pgram_device::set_gram_address(uint8_t data)
{
	if (m_crulatch->q1_r())
	{
		set_load_gram_counter(0);
		clock_gram_counter(0);
		m_count0->p_w(data&0x0f);
		m_count1->p_w((data>>4)&0x0f);
		m_count2->p_w(m_count0->output_r());
		m_count3->p_w(m_count1->output_r());
		clock_gram_counter(1);
		set_load_gram_counter(1);
		m_lowbyte = !m_lowbyte;
	}
}

/*
    Set the clock signal for all four counter chips. Propagate carry bits.
*/
void pgram_device::clock_gram_counter(int state)
{
	if (m_crulatch->q1_r())
	{
		m_count1->cet_w(m_count0->tc_r());
		m_count2->cet_w(m_count1->tc_r());
		m_count3->cet_w(m_count2->tc_r());

		m_count0->clock_w(state);
		m_count1->clock_w(state);
		m_count2->clock_w(state);
		m_count3->clock_w(state);

		// Debugging
		if (state==1 && m_lowbyte)
		{
			uint16_t gaddr = get_gram_address();
			if (gaddr >= 0x6000) LOGMASKED(LOG_GRAMADDR, "gaddr=%04x\n", gaddr);
		}
	}
}

/*
    Convenience method to retrieve the current GRAM address.
*/
uint16_t pgram_device::get_gram_address()
{
	return (m_count3->output_r()<<12) | (m_count2->output_r()<<8) | (m_count1->output_r()<<4) | m_count0->output_r();
}

/*
    Set/reset the PE* bit of all 74LS161 curcuits.
*/
void pgram_device::set_load_gram_counter(int state)
{
	m_count0->pe_w(state);
	m_count1->pe_w(state);
	m_count2->pe_w(state);
	m_count3->pe_w(state);
}

/*
    CRU write access to the LS259 latch.
*/
void pgram_device::cruwrite(offs_t offset, uint8_t data)
{
	// ...1 0xxx .... ...0
	bool selected = ((offset & 0xff01)==m_crubase);
	if (selected)
	{
		uint8_t bit = (offset & 0xfe)>>1;
		LOGMASKED(LOG_CRU, "cru %04x (bit %d) <- %d\n", offset, bit , data);
		if (bit==3 || bit==4) LOGMASKED(LOG_BANK, "cru bit %d <- %d\n", bit, data);
	}

	m_crulatch->enable_w(selected? 0 : 1);
	m_crulatch->write_abcd((offset>>1)&0x07, data);
}

/*
    Device construction.
*/
void pgram_device::device_add_mconfig(machine_config& config)
{
	// 4-bit counters
	TTL74161(config, m_count0); //  u13
	TTL74161(config, m_count1); //  u11
	TTL74161(config, m_count2); //  u12
	TTL74161(config, m_count3); //  u10

	// Hardwired connections
	m_count0->set_cet_pin_value(1);

	m_count0->set_cep_pin_value(1);
	m_count1->set_cep_pin_value(1);
	m_count2->set_cep_pin_value(1);
	m_count3->set_cep_pin_value(1);

	// CRU latch
	LS259(config, m_crulatch); // u14

	// Bank switch
	TTL7474(config, m_bankff, 0);

	// We allocate the space for PGRAM+
	BUFF_RAM(config, GRAM4567_TAG, 0).set_size(128*1024);
	BUFF_RAM(config, GRAM3_TAG, 0).set_size(32*1024);
	BUFF_RAM(config, DSRRAM_TAG, 0).set_size(32*1024);

	// Real-time clock
	MM58167(config, CLOCK_TAG, 32.768_kHz_XTAL);
}

void pgram_device::device_start()
{
}

void pgram_device::device_reset()
{
	m_active = (ioport("SW2")->read()==1);
	m_crubase = (ioport("SW1")->read()<<8) | 0x1000;
	m_pgramplus = (ioport("SIZE")->read()==1);
}

INPUT_CHANGED_MEMBER( pgram_device::sw1_changed )
{
	// CRU base changed
	m_crubase = (newval << 8) | 0x1000;
}

INPUT_CHANGED_MEMBER( pgram_device::sw2_changed )
{
	// Activation switch changed
	m_active = (newval==1);
}

INPUT_PORTS_START( pgram_switches )
	PORT_START( "SIZE" )
	PORT_CONFNAME( 0x01, 0x00, "Memory size")  // cannot be changed during runtime
		PORT_CONFSETTING(0x00, "PGRAM 72K")
		PORT_CONFSETTING(0x01, "PGRAM+ 192K")

	PORT_START( "SW2" )
	PORT_DIPNAME( 0x01, 0x01, "Activate switch" ) PORT_CHANGED_MEMBER(DEVICE_SELF, pgram_device, sw2_changed, 0)
		PORT_DIPSETTING(0x00, DEF_STR( Off ))
		PORT_DIPSETTING(0x01, DEF_STR( On ))

	PORT_START( "SW1" )
	PORT_DIPNAME( 0x07, 0x00, "CRU base" ) PORT_CHANGED_MEMBER(DEVICE_SELF, pgram_device, sw1_changed, 0)
		PORT_DIPSETTING( 0x00, "1000")
		PORT_DIPSETTING( 0x02, "1200")
		PORT_DIPSETTING( 0x03, "1300")
		PORT_DIPSETTING( 0x04, "1400")
		PORT_DIPSETTING( 0x05, "1500")
		PORT_DIPSETTING( 0x06, "1600")
		PORT_DIPSETTING( 0x07, "1700")
INPUT_PORTS_END

ioport_constructor pgram_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( pgram_switches );
}

} // end namespace bus::ti99::peb
