// license:LGPL-2.1+
// copyright-holders:Michael Zapf
/****************************************************************************

    IDE adapter card
    designed by Thierry Nouspikel in 2001, revised in 2004

    The IDE card is quite simple, since it only implements PIO transfer. A DMA
    support was also included, requiring a separate card, which did not become
    available.

    Detailed descriptions can be found on Thierry Nouspikel's website. [1]

    The card includes a clock chip to timestamp files, and a SRAM for the DSR.

    SRAM: 512 KiB (may be battery-backed)

    Four variants of the clock chip (since the 2004 revision):
    - RTC-65271 (external SRAM, unbuffered)
    - BQ4847 (external SRAM, buffered)
    - BQ4842 (internal SRAM, 128K)
    - BQ4847 (internal SRAM, 512K)

    The card does not contain any ROM. The firmware must be loaded into the
    card or saved on the IDE drive. It is part of the IDEAL software package [2]
    ("IDE Access Layer").

    DIP switches
    - SW1: SP3T switch, located on card area outside of the box
           selects 16 bit (TI) or 21 bit (Geneve) address decoding or disables
           the card

    - SW2: DPDT switch, located at center of the card
           selects TI (LSB/MSB) vs. Geneve byte order (MSB/LSB)

    - SW3: 4xDIP, located near front edge, lower edge
           A resets the RTC65271 chip to clear interrupts that occured while
              the power was off (clock continues running)
              Not implemented since the clock does not run outside of MAME
           B selects whether clock or SRAM is mapped into the 4000 address space
             on powerup. SRAM should only be mapped when it is battery-backed.
           (Switches A and B may have changed position between revisions, or the
           existing PCBs do not match the specification.)
           C, D not used

    - SW4: 16-position rotary switch, located near front edge
           selects second digit for CRU base address (1x00, x=0..F)

    The card supports a battery-backed or normal SRAM of 512KiB size. The battery
    power may be taken from the clock chip (which offers a battery holder).

    Suggested configuration procedure:
    (The IDELOAD program is part of the IDEAL package.)

    1) Clock chips BQ4847, BQ4842, BQ4852
    The IDELOAD program must be used to load the firmware into the SRAM.
    Bootstrap code cannot be stored in the RTC. In order to activate the DSR
    on next system startup, set the DIP switch to boot from SRAM.

    2) Clock chip RTC65271
    The IDELOAD program must be used to load the firmware into the SRAM and
    to install the bootstrap code in the clock memory. The bootstrap code
    must be inactive until the IDEAL files have been copied on the hard disk.
    Once this is done, the bootstrap code must be activated; it will load the
    IDEAL files into the SRAM on each power-up of the system.

    Memory map
    ----------

    CRU bit 0 == 1:
       CRU bit 1 == DIP switch setting:  (bit 1==0 on power-up, switch==0 when closed)
          RTC65271:
             4000-401F: XRAM (32 bytes of page given in 4080)
             402x: RTC data register (mirrored)
             403x: RTC indirect address register (mirrored)
             408x,409x: Page register for XRAM (mirrored)
             40Ax: mirror of 402x
             40Bx: mirror of 403x

          BQ4842/4852: (switch should be open (1) on power-up)
             4000-403F: not mapped
             4080-40BF: not mapped

          BQ4847:
             4020-403F: Registers (even addresses, mirrored on address+1)

          4040-404E: CS1Fx read  (IDE register group 1), mirrored at 40C0-40CE
          4050-405E: CS1Fx write (IDE register group 1), mirrored at 40D0-40DE
          4060-406E: CS3Fx read  (IDE register group 2), mirrored at 40E0-40FE
          4070-407E: CS3Fx write (IDE register group 2), mirrored at 40F0-40FE

       4000-40FF: SRAM (CRU bit 1 != DIP switch setting)
       4100-4FFF: SRAM

    6000-7FFF: SRAM (CRU bit 4 == 1)  [RAMBO support]

    BQ4842/52: Clock registers are located at upper end of SRAM
       BQ4842: page 0F (mirrored 1F, 2F, 3F)
       BQ4852: page 3F

    Write SRAM:
       CRU bit 2 == 0:
          normal write
       CRU bit 2 == 1:
          Set SRAM page nn/2 (address 40nn, mirrored in 4000-5FFF, 6000-7FFF)
       CRU bit 5 == 1:
          SRAM write protect

    Original version by Raphael Nabet
    Rewritten by Michael Zapf

    References

    [1] Th. Nouspikel: IDE Interface card version 2
        https://www.unige.ch/medecine/nouspikel/ti99/ide2.htm

    [2] Th. Nouspikel: Description of the IDEAL software.
        https://www.unige.ch/medecine/nouspikel/ti99/ideal.htm

*****************************************************************************/

#include "emu.h"
#include "tn_ide.h"

#define LOG_WARN       (1U<<1)
#define LOG_CRU        (1U<<2)
#define LOG_RTC        (1U<<3)
#define LOG_XRAM       (1U<<4)
#define LOG_SRAM       (1U<<5)
#define LOG_ATA        (1U<<6)
#define LOG_SRAMH      (1U<<7)

#define VERBOSE ( LOG_GENERAL | LOG_WARN )

#include "logmacro.h"

DEFINE_DEVICE_TYPE(TI99_IDE, bus::ti99::peb::nouspikel_ide_card_device, "ti99_ide", "Nouspikel IDE interface card")

#define CLOCK65_TAG "rtc65271"
#define CLOCK47_TAG "bq4847"
#define CLOCK42_TAG "bq4842"
#define CLOCK52_TAG "bq4852"
#define ATA_TAG "ata"
#define LATCH_TAG "crulatch"
#define ATALATCHEV_TAG "atalatch_even"
#define ATALATCHODD_TAG "atalatch_odd"
#define RAM512_TAG "sram512"

namespace bus::ti99::peb {

enum
{
	MODE_OFF = 0,
	MODE_GENEVE,
	MODE_TI
};

enum
{
	RTC65 = 0,
	RTC47,
	RTC42,
	RTC52
};

nouspikel_ide_card_device::nouspikel_ide_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, TI99_IDE, tag, owner, clock),
	device_ti99_peribox_card_interface(mconfig, *this),
	m_rtc65(*this, CLOCK65_TAG),
	m_rtc47(*this, CLOCK47_TAG),
	m_rtc42(*this, CLOCK42_TAG),
	m_rtc52(*this, CLOCK52_TAG),
	m_ata(*this, ATA_TAG),
	m_sram(*this, RAM512_TAG),
	m_crulatch(*this, LATCH_TAG),
	m_latch0_7(*this, ATALATCHEV_TAG),
	m_latch8_15(*this, ATALATCHODD_TAG),
	m_ideint(false),
	m_mode(MODE_OFF),
	m_page(0),
	m_rtctype(0),
	m_genmod(false)
{
}

void nouspikel_ide_card_device::readz(offs_t offset, uint8_t *value)
{
	bool mmap = false;
	bool sramsel = false;
	bool xramsel = false;
	bool rtcsel = false;
	bool cs1fx = false;
	bool cs3fx = false;

	decode(offset, mmap, sramsel, xramsel, rtcsel, cs1fx, cs3fx);

	bool idesel = cs1fx || cs3fx;

	if (xramsel || rtcsel)
	{
		// Swap the address bits (TI numbering vs. standard)
		// A8->A5, A15->A4, A14->A3, A13->A2, A12->A1, A11->A0
		// .... .... 5..0 1234
		// .... .... ..54 3210

		// int addr = ((offset & 0x80)>>2) | ((offset & 1)<<4) | ((offset & 2)<<2)
		//          | (offset & 4) | ((offset & 8)>>2) | ((offset & 16)>>4);
		// *value = m_rtc->read(xramsel, addr);
		// LOGMASKED(LOG_RTC, "rtc %04x (%02x, %s) -> %02x\n", offset&0xffff, addr, xramsel? "xram" : "rtc", *value);

		// However, We take the simple way and keep the address as is.
		// This makes debugging less tedious.

		if (rtcsel)   // 4020-403F
		{
			if (m_rtctype==RTC65)
			{
				if ((offset&0x0010)!=0)
				{
					*value = m_rtc65->read(0, 1);
					LOGMASKED(LOG_RTC, "rtc65 read -> %02x\n", *value);
				}
			}
			else
			{
				if (m_rtctype==RTC47)
				{
					*value = m_rtc47->read((offset & 0x1e)>>1);
					LOGMASKED(LOG_RTC, "rtc reg %02d (%04x) -> %02x\n", (offset & 0x1e)>>1, offset & 0xffff, *value);
				}
				// No reaction for RTC42, RTC52
			}
		}
		else
		{
			if (m_rtctype==RTC65)   // xram, only for 65271, unmapped for others
			{
				int addr = (offset & 0x1f) | ((offset&0x80)>>2);
				*value = m_rtc65->read(1, addr);
				LOGMASKED(LOG_XRAM, "xram %02x -> %02x\n", addr, *value);
			}
		}
	}

	if (sramsel)
	{
		int page = m_page;
		// When addressing in 4000-4fff, and bit 3 = 0, lock page to 0
		if (((offset & 0x3000)==0x0000) && m_crulatch->q3_r()==0)
			page = 0;

		offs_t addr = (offset & 0x1fff) | (page<<13);

		if (m_rtctype==RTC65 || m_rtctype==RTC47)
		{
			*value = m_sram->read(addr);    // external SRAM
		}
		else
		{
			// The BQ4842/52 offer SRAM by themselves
			if (m_rtctype==RTC42)
				*value = m_rtc42->read(addr);
			else
				*value = m_rtc52->read(addr);
		}
		if (m_mode==MODE_TI)
		{
			if ((offset & 1)==0)
				LOGMASKED(LOG_SRAM, "sram %04x (%02x) -> %04x\n", offset&0xffff, page, (*value<<8) | m_sram->read(addr+1));
		}
	}

	if (idesel)
	{
		// Don't let the debugger mess with the latches
		if (machine().side_effects_disabled())
		{
			*value = 0;
			return;
		}

		int reg = (offset >> 1)&7;
		bool even = ((offset & 1)==0);

		// Geneve writes even/odd, TI writes odd/even
		bool first = (even != (m_mode==MODE_TI));

		m_latch0_7->leba_w(first? 0:1);
		m_latch8_15->leba_w(first? 0:1);

		uint16_t atavalue = 0;

		// On the first read, get the 16-bit value
		// but only when addressing in the area 4040-404F / 4060-406F
		// (check A11=0). That way, Read-before-Write does not interfere
		if (first && ((offset & 0x0010)==0))
		{
			if (cs1fx)
				atavalue = m_ata->cs0_r(reg);
			else
				atavalue = m_ata->cs1_r(reg);
			LOGMASKED(LOG_ATA, "%s %02x -> %04x\n", cs1fx? "cs1" : "cs3", reg, atavalue);
		}

		// Load latches (no change during second access)
		m_latch0_7->b_w(atavalue&0xff);
		m_latch8_15->b_w((atavalue >> 8)&0xff);

		// Activate the respective latch
		m_latch0_7->oeba_w(even? 0:1);
		m_latch8_15->oeba_w(even? 1:0);

		// Only one of them delivers a value, the other is Z
		m_latch0_7->outputa_rz(*value);
		m_latch8_15->outputa_rz(*value);

		// Reads in the upper half are RBW and should be ignored
		if ((offset & 0x0010)==0)
			LOGMASKED(LOG_ATA, "ata %04x -> %02x\n", offset&0xffff, *value);
	}
}

void nouspikel_ide_card_device::write(offs_t offset, uint8_t data)
{
	bool mmap = false;
	bool sramsel = false;
	bool xramsel = false;
	bool rtcsel = false;
	bool cs1fx = false;
	bool cs3fx = false;

	decode(offset, mmap, sramsel, xramsel, rtcsel, cs1fx, cs3fx);
	bool idesel = cs1fx || cs3fx;

	if (xramsel || rtcsel)
	{
		// Swap the address bits (TI numbering vs. standard)
		// Actually, this is almost irrelevant for the RTC access, since only
		// A0 determines the mode.
		// A8->A5, A15->A4, A14->A3, A13->A2, A12->A1, A11->A0
		// .... .... 5..0 1234
		// .... .... ..54 3210

		// int addr = ((offset & 0x80)>>2) | ((offset & 1)<<4) | ((offset & 2)<<2)
		//          | (offset & 4) | ((offset & 8)>>2) | ((offset & 16)>>4);

		// LOGMASKED(LOG_RTC, "rtc %04x (%02x, %s) <- %02x\n", offset&0xffff, addr, xramsel? "xram" : "rtc", data);
		// m_rtc->write(xramsel, addr, data);

		// See above (read), don't swap the lines.

		if (rtcsel)
		{
			if (m_rtctype == RTC65)
			{
				if ((offset&0x0010)==0)
				{
					m_rtc65->write(0, 0, data);
					LOGMASKED(LOG_RTC, "rtc set <- %02x\n", data);
				}
				else
				{
					m_rtc65->write(0, 1, data);
					LOGMASKED(LOG_RTC, "rtc write <- %02x\n", data);
				}
			}
			else
			{
				if (m_rtctype == RTC47)
				{
					LOGMASKED(LOG_RTC, "rtc reg %02d (%04x) <- %02x\n", (offset & 0x1e)>>1, offset & 0xffff, data);
					m_rtc47->write((offset & 0x1e)>>1, data);
				}
				// No reaction for RTC42, RTC52
			}
		}
		else
		{
			if (m_rtctype==RTC65)
			{
				int addr = (offset & 0x1f) | ((offset&0x80)>>2);
				m_rtc65->write(1, addr, data);

				if (addr & 0x20)
					LOGMASKED(LOG_XRAM, "xram set page %02x\n", data);
				else
					LOGMASKED(LOG_XRAM, "xram %02x <- %02x\n", addr & 0x1f, data);
			}
		}
	}

	if (sramsel)
	{
		if (m_crulatch->q2_r()==1)
		{
			m_page = (offset & 0x007e)>>1;
			LOGMASKED(LOG_SRAM, "sram page set %02x (%04x)\n", m_page, offset&0xffff);
		}

		// Software must ensure that CRU bit 5 is 1 (SRAM write protect)
		// when bit 2 is 1 (page select)
		if (m_crulatch->q5_r()==0)
		{
			int page = m_page;

			// When addressing in 4000-4fff, and bit 3 = 0, lock page to 0
			if (((offset & 0x3000)==0x0000) && m_crulatch->q3_r()==0)
				page = 0;

			offs_t addr = (offset & 0x1fff) | (page<<13);

			if (m_rtctype==RTC65 || m_rtctype==RTC47)
			{
				m_sram->write(addr, data);
			}
			else
			{
				if (m_rtctype==RTC42)
					m_rtc42->write(addr, data);
				else
					m_rtc52->write(addr, data);
			}

			LOGMASKED(LOG_SRAM, "sram %04x (%02x) <- %02x\n", offset&0xffff, page, data);
			if ((offset & 0xfff0)==0x5ff0)
				LOGMASKED(LOG_SRAMH, "sram %04x (%02x) <- %02x\n", offset&0xffff, page, data);
		}
	}

	if (idesel)
	{
		// Don't let the debugger mess with the latches
		if (machine().side_effects_disabled())
		{
			return;
		}
		LOGMASKED(LOG_ATA, "ata %04x <- %02x\n", offset&0xffff, data);

		bool even = ((offset & 1)==0);
		m_latch0_7->leab_w(even? 0:1);
		m_latch8_15->leab_w(even? 1:0);

		// Load the value into the respective latch
		m_latch0_7->a_w(data);
		m_latch8_15->a_w(data);

		// Geneve writes even/odd, TI writes odd/even
		bool first = (even != (m_mode==MODE_TI));

		// Output on second access
		m_latch0_7->oeab_w(first? 1:0);
		m_latch8_15->oeab_w(first? 1:0);

		// No output during the first access
		int reg = (offset >> 1)&7;
		uint8_t out = 0;
		m_latch8_15->outputb_rz(out);
		uint16_t atavalue = (out << 8);
		m_latch0_7->outputb_rz(out);
		atavalue |= out;

		if (!first)
		{
			LOGMASKED(LOG_ATA, "%s %02x <- %04x\n", cs1fx? "cs1" : "cs3", reg, atavalue);

			if (cs1fx)
				m_ata->cs0_w(reg, atavalue);
			else
				m_ata->cs1_w(reg, atavalue);
		}
	}
}

void nouspikel_ide_card_device::decode(offs_t offset, bool& mmap, bool& sramsel, bool& xramsel, bool& rtcsel, bool& cs1fx, bool& cs3fx)
{
	bool inspace = false;

	// In a normal Geneve, assume AME=1, AMD=0
	if (!m_genmod) offset = ((offset & 0x07ffff) | 0x100000);

	// A0=0
	if (m_mode == MODE_TI) inspace = ((offset & 0x8000)==0);
	else
	{
		// AME=1, AMD=0, AMC/AMB/AMA=111, A0=0
		if (m_mode == MODE_GENEVE) inspace = ((offset & 0x1f8000)==0x170000);
		// else mode=off
	}

	// mmap = 0x4000 - 0x40ff (if bit 1 == DIP setting)
	// sramsel = 0x4100 - 0x4fff (if bit 0 = 1) or 0x6000 - 0x7fff (if bit 4 = 1)

	// A0 is not checked again (subsumed in inspace)

	mmap = ((offset & 0x7f00)==0x4000) && (m_crulatch->q0_r()==1)
			&& ((m_crulatch->q1_r()!=0) == m_srammap) && inspace;
	sramsel = ((((offset & 0x6000)==0x4000) && !mmap && (m_crulatch->q0_r()==1))
				|| (((offset & 0x6000)==0x6000) && (m_crulatch->q4_r()==1))) && inspace;

	xramsel = false;
	rtcsel = false;
	cs1fx = false;
	cs3fx = false;

	if (mmap)
	{
		xramsel = ((offset & 0x60)==0x00);  // 4000-401F, 4080  (only 65271)
		rtcsel = ((offset & 0x60)==0x20);   // 4020-403F  (65271 and 4847)
		cs1fx = ((offset & 0x60)==0x40);    // 4040-405F
		cs3fx = ((offset & 0x60)==0x60);    // 4060-407F
	}
}

/*
    CRU read access to the LS251 multiplexer.
*/
void nouspikel_ide_card_device::crureadz(offs_t offset, uint8_t *value)
{
	uint8_t bit = 0;

	if ((offset & 0xff00)==m_cru_base)
	{
		switch ((offset>>1) & 0x07)
		{
		case 0:
			bit = m_ideint? 1:0;
			break;
		case 1:
			bit = m_srammap? 1:0;
			break;
		case 2:
			if (m_rtctype==RTC65)
				bit = (m_rtc65->intrq_r()==ASSERT_LINE)? 0:1;
			else
			{
				if (m_rtctype==RTC47)
					bit = (m_rtc47->intrq_r()==ASSERT_LINE)? 0:1;
				else
				{
					if (m_rtctype==RTC42)
						bit = (m_rtc42->intrq_r()==ASSERT_LINE)? 0:1;
					else
						bit = (m_rtc52->intrq_r()==ASSERT_LINE)? 0:1;
				}
			}
			break;
		case 3:
			bit = 1;
			break;
		case 4:
			bit = m_crulatch->q4_r();
			break;
		case 5:
			bit = m_crulatch->q5_r();
			break;
		case 6:
		case 7:
			break;
		}
		*value = bit;
		LOGMASKED(LOG_CRU, "cru %04x (bit %d) -> %d\n", offset, (offset & 0xff)>>1, bit);
	}
}

/*
    CRU write access to the latch.
*/
void nouspikel_ide_card_device::cruwrite(offs_t offset, uint8_t data)
{
	if ((offset & 0xff00)==m_cru_base)
	{
		// LOGMASKED(LOG_CRU, "cru %04x (bit %d) <- %d\n", offset, (offset & 0xff)>>1, data);
		int bitnumber = (offset >> 1) & 0x07;
		m_crulatch->write_bit(bitnumber, data&1);

#if 0
		// Just debugging
		switch (bitnumber)
		{
		case 0:
			LOGMASKED(LOG_CRU, "Turn card %s\n", (data&1)? "on" : "off");
			break;
		case 1:
			LOGMASKED(LOG_CRU, "Map %s at 4000-40FF\n", ((data&1)==m_srammap)? "register" : "SRAM");
			break;
		case 2:
			LOGMASKED(LOG_CRU, "%s SRAM page\n", (data&1)? "Enable switch" : "Fixed");
			break;
		case 3:
			LOGMASKED(LOG_CRU, "%s\n", (data&1)? "Same page at 4000-5FFF" : "Fix page 0 at 4000-4FFF");
			break;
		case 4:
			LOGMASKED(LOG_CRU, "%s RAMBO\n", (data&1)? "Enable" : "Disable");
			break;
		case 5:
			LOGMASKED(LOG_CRU, "Write %s SRAM\n", (data&1)? "protect" : "enable");
			break;
		case 6:
			LOGMASKED(LOG_CRU, "%s IDE interrupt\n", (data&1)? "Enable" : "Disable");
			break;
		case 7:
			LOGMASKED(LOG_CRU, "%s\n", (data&1)? "Reset drives" : "Normal operation");
			break;
		}
#endif
	}
}

WRITE_LINE_MEMBER(nouspikel_ide_card_device::clock_interrupt_callback)
{
	m_slot->set_inta(state);
}

WRITE_LINE_MEMBER(nouspikel_ide_card_device::ide_interrupt_callback)
{
	m_ideint = (state==ASSERT_LINE);
	if (m_crulatch->q6_r()==1) m_slot->set_inta(state);
}

WRITE_LINE_MEMBER(nouspikel_ide_card_device::resetdr_callback)
{
	if (m_crulatch->q6_r()==1 && (state==0))
		// not implemented
		LOGMASKED(LOG_ATA, "Drive reset\n");
}

void nouspikel_ide_card_device::device_add_mconfig(machine_config &config)
{
	// Choice of RTC chips
	RTC65271(config, m_rtc65, 0);
	BQ4847(config, m_rtc47, 0);
	BQ4842(config, m_rtc42, 0);
	BQ4852(config, m_rtc52, 0);

	m_rtc65->interrupt_cb().set(FUNC(nouspikel_ide_card_device::clock_interrupt_callback));
	m_rtc47->interrupt_cb().set(FUNC(nouspikel_ide_card_device::clock_interrupt_callback));
	m_rtc42->interrupt_cb().set(FUNC(nouspikel_ide_card_device::clock_interrupt_callback));
	m_rtc52->interrupt_cb().set(FUNC(nouspikel_ide_card_device::clock_interrupt_callback));

	ATA_INTERFACE(config, m_ata).options(ata_devices, "hdd", nullptr, false);
	m_ata->irq_handler().set(FUNC(nouspikel_ide_card_device::ide_interrupt_callback));

	TTL74543(config, m_latch0_7, 0);
	m_latch0_7->set_ceab_pin_value(0);
	m_latch0_7->set_ceba_pin_value(0);

	TTL74543(config, m_latch8_15, 0);
	m_latch8_15->set_ceab_pin_value(0);
	m_latch8_15->set_ceba_pin_value(0);

	LS259(config, m_crulatch);
	m_crulatch->q_out_cb<7>().set(FUNC(nouspikel_ide_card_device::resetdr_callback));

	BUFF_RAM(config, RAM512_TAG, 0).set_size(512*1024);
}

void nouspikel_ide_card_device::device_start()
{
	save_item(NAME(m_ideint));
	save_item(NAME(m_page));
}

void nouspikel_ide_card_device::device_reset()
{
	int rtype[] = { RTC65, RTC47, RTC42, RTC52 };

	m_page = 0;
	m_ideint = false;
	m_cru_base = (ioport("CRUIDE")->read() << 8) | 0x1000;
	m_mode = ioport("MODE")->read();
	m_srammap = (ioport("MAPMODE")->read()!=0);
	m_rtctype = rtype[ioport("RTC")->read()];
	m_genmod = (ioport("GENMOD")->read() != 0);

	// The 65271 option does not support buffered SRAM; only the BQ4847
	// can drive a buffered external RAM; the other two chips have internal SRAM
	m_sram->set_buffered(m_rtctype == RTC47);

	// Only activate the selected RTC
	m_rtc47->connect_osc(ioport("RTC")->read()==1);
	m_rtc42->connect_osc(ioport("RTC")->read()==2);
	m_rtc52->connect_osc(ioport("RTC")->read()==3);
}

INPUT_CHANGED_MEMBER( nouspikel_ide_card_device::mode_changed )
{
	// Card mode changed
	if (param==0)
		m_srammap = (newval != 0);
	else
		m_mode = newval;
}

INPUT_PORTS_START( tn_ide )

	PORT_START("RTC")
	PORT_CONFNAME(0x03, 1, "RTC chip")
		PORT_CONFSETTING(0, "RTC-65271")
		PORT_CONFSETTING(1, "BQ4847 (ext SRAM)")
		PORT_CONFSETTING(2, "BQ4842 (128K)")
		PORT_CONFSETTING(3, "BQ4852 (512K)")

	// When used in a normal Geneve, AME/AMD lines are set to (1,0)
	PORT_START("GENMOD")
	PORT_CONFNAME(0x01, 0, "Genmod decoding")
		PORT_CONFSETTING(0, DEF_STR( Off ))
		PORT_CONFSETTING(1, DEF_STR( On ))

	// The switch should be open (1) on powerup for BQ clock chips
	PORT_START("MAPMODE")
	PORT_DIPNAME(0x1, 1, "Map at boot time") PORT_CHANGED_MEMBER(DEVICE_SELF, nouspikel_ide_card_device, mode_changed, 0)
		PORT_DIPSETTING(0, "RTC")
		PORT_DIPSETTING(1, "SRAM")

	// Set to off as default, because random contents in SRAM may lead to lockup
	PORT_START("MODE")
	PORT_DIPNAME(0x3, MODE_OFF, "Card mode") PORT_CHANGED_MEMBER(DEVICE_SELF, nouspikel_ide_card_device, mode_changed, 1)
		PORT_DIPSETTING(MODE_OFF, "Off")
		PORT_DIPSETTING(MODE_GENEVE, "Geneve")
		PORT_DIPSETTING(MODE_TI, "TI")

	PORT_START( "CRUIDE" )
	PORT_DIPNAME( 0xf, 0x0, "IDE CRU base" )
		PORT_DIPSETTING( 0x0, "1000" )
		PORT_DIPSETTING( 0x1, "1100" )
		PORT_DIPSETTING( 0x2, "1200" )
		PORT_DIPSETTING( 0x3, "1300" )
		PORT_DIPSETTING( 0x4, "1400" )
		PORT_DIPSETTING( 0x5, "1500" )
		PORT_DIPSETTING( 0x6, "1600" )
		PORT_DIPSETTING( 0x7, "1700" )
		PORT_DIPSETTING( 0x8, "1800" )
		PORT_DIPSETTING( 0x9, "1900" )
		PORT_DIPSETTING( 0xa, "1A00" )
		PORT_DIPSETTING( 0xb, "1B00" )
		PORT_DIPSETTING( 0xc, "1C00" )
		PORT_DIPSETTING( 0xd, "1D00" )
		PORT_DIPSETTING( 0xe, "1E00" )
		PORT_DIPSETTING( 0xf, "1F00" )
INPUT_PORTS_END

ioport_constructor nouspikel_ide_card_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(tn_ide);
}

} // end namespace bus::ti99::peb
