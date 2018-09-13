// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes, Nigel Barnes
/******************************************************************************
    BBC Model B

    MESS Driver By:

    Gordon Jefferyes
    mess_bbc@romvault.com

******************************************************************************/

#include <ctype.h>
#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "sound/tms5220.h"
#include "machine/6522via.h"
#include "machine/wd_fdc.h"
#include "includes/bbc.h"
#include "machine/mc146818.h"
#include "bus/centronics/ctronics.h"
#include "imagedev/cassette.h"


/****************************************
  BBC Model A memory handling functions
****************************************/

WRITE8_MEMBER(bbc_state::page_selecta_w)
{
	/* for the Model A just address the 4 on board ROM sockets */
	m_swrbank = data & 0x03;
	m_swrbank_ram = false;

	m_bank4->set_entry(m_swrbank);
}


/****************************************
  BBC Model B memory handling functions
****************************************/

WRITE8_MEMBER(bbc_state::page_selectb_w)
{
	static const unsigned short swramtype[4][16] = {
		// TODO: move sideways RAM boards to slot devices
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // 0: none
		{ 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1 }, // 1: 128K (bank 8 to 15) Solidisk sideways ram userport bank latch (not implemented)
		{ 0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0 }, // 2: 64K (banks 4 to 7) for Acorn sideways ram FE30 bank latch
		{ 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1 }, // 3: 128K (banks 8 to 15) for Acorn sideways ram FE30 bank latch
	};

	/* the Model B address all 16 of the ROM sockets */
	m_swrbank = data & 0x0f;
	m_swrbank_ram = swramtype[m_swramtype][m_swrbank] ? true : false;

	m_bank4->set_entry(m_swrbank);
}

WRITE8_MEMBER(bbc_state::bbc_memoryb4_w)
{
	if (m_swrbank_ram)
	{
		m_region_opt->base()[(m_swrbank << 14) + offset] = data;
	}
}


/****************************************
  BBC Model B+ memory handling functions
****************************************/

READ8_MEMBER(bbc_state::bbcbp_fetch_r)
{
	switch (offset & 0xf000)
	{
	case 0xa000:
		/* Code executing from sideways RAM between 0xa000-0xafff will access the shadow RAM (if selected) */
		if (m_vdusel && m_paged_ram)
		{
			m_bank2->set_base(m_region_maincpu->base() + 0xb000);
		}
		else
		{
			m_bank2->set_base(m_region_maincpu->base() + 0x3000);
		}
		break;

	case 0xc000:
	case 0xd000:
		/* Access shadow RAM if VDU drivers and shadow RAM selected */
		if (m_vdusel)
		{
			m_bank2->set_base(m_region_maincpu->base() + 0xb000);
		}
		else
		{
			m_bank2->set_base(m_region_maincpu->base() + 0x3000);
		}
		break;

	default:
		m_bank2->set_base(m_region_maincpu->base() + 0x3000);
		break;
	}
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

WRITE8_MEMBER(bbc_state::page_selectbp_w)
{
	/* the Model B+ addresses all 16 ROM sockets and extra 12K of RAM at 0x8000 and 20K of shadow RAM at 0x3000 */
	switch (offset & 0x07)
	{
	case 0x00:
		m_paged_ram = BIT(data,7);

		m_swrbank =  data & 0x0f;
		m_swrbank_ram = false;

		if (m_paged_ram)
		{
			/* if paged RAM then set 0x8000 to 0xafff to read from the RAM 8000 to 0xafff */
			m_bank4->set_entry(0x10);
		}
		else
		{
			/* if paged ROM then set the ROM to be read from 0x8000 to 0xafff */
			m_bank4->set_entry(m_swrbank);
		};

		/* set the ROM to be read from 0xb000 to 0xbfff */
		m_bank6->set_entry(m_swrbank);
		break;

	case 0x04:
		/* the video display should now use this flag to display the shadow RAM memory */
		m_vdusel = BIT(data,7);
		setvideoshadow(m_vdusel);
		break;
	}
}

WRITE8_MEMBER(bbc_state::page_selectbp128_w)
{
	/* the BBC B+ 128K has extra RAM mapped in replacing the ROM banks 0,1,c and d. */
	static const unsigned short swram_banks[16] = { 1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0 };

	switch (offset & 0x07)
	{
	case 0x00:
		m_paged_ram = BIT(data, 7);

		m_swrbank = data & 0x0f;
		m_swrbank_ram = swram_banks[m_swrbank] ? true : false;

		if (m_paged_ram)
		{
			/* if paged RAM then set 0x8000 to 0xafff to read from the RAM 8000 to 0xafff */
			m_bank4->set_entry(0x10);
		}
		else
		{
			/* if paged ROM then set the ROM to be read from 0x8000 to 0xafff */
			m_bank4->set_entry(m_swrbank);
		}

		/* set the ROM to be read from 0xb000 to 0xbfff */
		m_bank6->set_entry(m_swrbank);
		break;

	case 0x04:
		/* the video display should now use this flag to display the shadow RAM memory */
		m_vdusel = BIT(data, 7);
		setvideoshadow(m_vdusel);
		break;
	}
}

WRITE8_MEMBER(bbc_state::bbc_memorybp4_w)
{
	if (m_paged_ram)
	{
		m_region_maincpu->base()[offset + 0x8000] = data;
	}
	else if (m_swrbank_ram)
	{
		m_region_opt->base()[offset + (m_swrbank << 14)] = data;
	}
}

WRITE8_MEMBER(bbc_state::bbc_memorybp6_w)
{
	if (m_swrbank_ram)
	{
		m_region_opt->base()[offset + (m_swrbank << 14) + 0x3000] = data;
	}
}


/****************************************
  BBC Master memory handling functions
****************************************/

/*
  ROMSEL - &FE30 write only
    b7 RAM 1 = Page in ANDY &8000-&8FFF
           0 = Page in ROM  &8000-&8FFF
    b6     Not Used
    b5     Not Used
    b4     Not Used
    b3-b0  ROM/RAM Bank Select

  ACCCON - &FE34 read/write register
    b7 IRR 1 = Causes an IRQ to the processor
    b6 TST 1 = Selects &FC00-&FEFF read from OS-ROM
    b5 IFJ 1 = Internal 1MHz bus
           0 = External 1MHz bus
    b4 ITU 1 = Internal Tube
           0 = External Tube
    b3 Y   1 = Read/Write HAZEL &C000-&DFFF RAM
           0 = Read/Write ROM &C000-&DFFF OS-ROM
    b2 X   1 = Read/Write LYNNE
           0 = Read/Write main memory &3000-&8000
    b1 E   1 = Causes shadow if VDU code
           0 = Main all the time
    b0 D   1 = Display LYNNE as screen
           0 = Display main RAM screen

  HAZEL is the 8K of RAM used by the MOS, filing system, and other ROMs at &C000-&DFFF

  ANDY is the name of the 4K of RAM used by the MOS at &8000-&8FFF
*/

READ8_MEMBER(bbc_state::bbcm_fetch_r)
{
	if (m_acccon_x || (m_acccon_e && offset >= 0xc000 && offset <= 0xdfff))
	{
		m_bank2->set_base(m_region_maincpu->base() + 0xb000);
	}
	else
	{
		m_bank2->set_base(m_region_maincpu->base() + 0x3000);
	}
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

READ8_MEMBER(bbc_state::bbcm_acccon_r)
{
	return m_acccon;
}

WRITE8_MEMBER(bbc_state::bbcm_acccon_w)
{
	m_acccon = data;

	m_acccon_irr = BIT(data,7);
	m_acccon_tst = BIT(data,6);
	m_acccon_ifj = BIT(data,5);
	m_acccon_itu = BIT(data,4);
	m_acccon_y   = BIT(data,3);
	m_acccon_x   = BIT(data,2);
	m_acccon_e   = BIT(data,1);
	m_acccon_d   = BIT(data,0);

	/* Bit IRR causes Interrupt Request. */
	m_irqs->in_w<6>(m_acccon_irr);

	/* Bit Y causes 8K of RAM (HAZEL) to be overlayed on the MOS VDU drivers. */
	if (m_acccon_y)
	{
		m_bank7->set_base(m_region_maincpu->base() + 0x9000);
	}
	else
	{
		m_bank7->set_base(m_region_os->base());
	}

	/* Bit D causes the CRT controller to display the contents of LYNNE. */
	setvideoshadow(m_acccon_d);

	/* Bit X causes all accesses to 0x3000-0x7fff to be re-directed to LYNNE.*/
	if (m_acccon_x)
	{
		m_bank2->set_base(m_region_maincpu->base() + 0xb000);
	}
	else
	{
		m_bank2->set_base(m_region_maincpu->base() + 0x3000);
	}

	/* Bit TST controls paging of ROM reads in the 0xfc00-0xfeff region */
	/* if 0 the I/O is paged for both reads and writes */
	/* if 1 the ROM is paged in for reads but writes still go to I/O */
	if (m_acccon_tst)
	{
		m_bank8->set_base(m_region_os->base() + 0x3c00);
		space.install_read_bank(0xfc00, 0xfeff, "bank8");
	}
	else
	{
		space.install_read_handler(0xfc00, 0xfeff, read8_delegate(FUNC(bbc_state::bbcm_r),this));
	}
}

WRITE8_MEMBER(bbc_state::page_selectbm_w)
{
	m_paged_ram = (data & 0x80) >> 7;
	m_swrbank = data & 0x0f;

	if ((!m_lk19_ic37_paged_rom && (m_swrbank == 4 || m_swrbank == 5)) || (!m_lk18_ic41_paged_rom && (m_swrbank == 6 || m_swrbank == 7)))
	{
		/* links are set for RAM so make read/write */
		m_swrbank_ram = true;
	}
	else
	{
		/* links are set for ROM so make read-only */
		m_swrbank_ram = false;
	}

	if (m_paged_ram)
	{
		m_bank4->set_entry(0x10);
		m_bank5->set_entry(m_swrbank);
	}
	else
	{
		m_bank4->set_entry(m_swrbank);
		m_bank5->set_entry(m_swrbank);
	}
}

WRITE8_MEMBER(bbc_state::bbc_memorybm4_w)
{
	if (m_paged_ram)
	{
		m_region_maincpu->base()[offset + 0x8000] = data;
	}
	else if (m_swrbank_ram)
	{
		m_region_opt->base()[offset + (m_swrbank << 14)] = data;
	}
}


WRITE8_MEMBER(bbc_state::bbc_memorybm5_w)
{
	if (m_swrbank_ram)
	{
		m_region_opt->base()[offset + (m_swrbank << 14) + 0x1000] = data;
	}
}

WRITE8_MEMBER(bbc_state::bbc_memorybm7_w)
{
	if (m_acccon_y)
	{
		m_region_maincpu->base()[offset + 0x9000] = data;
	}
}


/******************************************************************************
&FC00-&FCFF FRED
&FC00-&FC03 Byte-Wide Expansion RAM
&FC08-&FC0F Ample M2000 MIDI Interface (see also FCF0)
&FC10-&FC13 Teletext
&FC14-&FC1F Prestel
&FC20-&FC27 IEEE 488 Interface
&FC28-&FC2F Acorn Expansion, currently unused
&FC30-&FC3F Cambridge Ring Interface
&FC40-&FC47 Winchester Disc Interface
&FC48-&FC7F Acorn Expansion, currently unused
&FC80-&FC8F Test Hardware
&FC90-&FCBF Acorn Expansion, currently unused
&FCC0-&FCFE User Applications
&FCF0-&FCF7 JGH/ETI MIDI Control (see also FC08)
&FCFC-&FCFF Page-Wide Expansion RAM

&FD00-&FDFF JIM
&FD00-&FDFF Page-wide expansion RAM window
&FD40-&FD4F Torch SASI/SCSI Hard Drive Access
&FDFE-&FDFF Reset Test vector

&FE00-&FEFF SHEILA          Read                    Write
&FE00-&FE07 6845 CRTC       Video controller        Video Controller         8 ( 2 bytes x  4 )
&FE08-&FE0F 6850 ACIA       Serial controller       Serial Controller        8 ( 2 bytes x  4 )
&FE10-&FE17 Serial ULA      -                       Serial system chip       8 ( 1 byte  x  8 )
&FE18-&FE1F uPD7002         A to D converter        A to D converter         8 ( 4 bytes x  2 )
&FE20-&FE23 Video ULA       -                       Video system chip        4 ( 2 bytes x  2 )
&FE24-&FE27 FDC Latch       1770 Control latch      1770 Control latch       4 ( 1 byte  x  4 )
&FE28-&FE2F 1770 registers  1770 Disc Controller    1770 Disc Controller     8 ( 4 bytes x  2 )
&FE30-&FE33 ROMSEL          -                       ROM Select               4 ( 1 byte  x  4 )
&FE34-&3FE7 ACCCON          ACCCON select reg.      ACCCON select reg        4 ( 1 byte  x  4 )
&FE38-&FE3F NC              -                       -
&FE40-&FE5F 6522 VIA        SYSTEM VIA              SYSTEM VIA              32 (16 bytes x  2 ) 1MHz
&FE60-&FE7F 6522 VIA        USER VIA                USER VIA                32 (16 bytes x  2 ) 1MHz
&FE80-&FE9F 8271 registers  8271 Disk Controller    8271 Disk Controller
&FEA0-&FEBF 68B54 ADLC      ECONET controller       ECONET controller       32 ( 4 bytes x  8 ) 2MHz
&FEC0-&FEDF 6854 ADLC       ECONET controller       ECONET controller       32 ( 4 bytes x  8 ) 2MHz
&FEE0-&FEFF Tube ULA        Tube system interface   Tube system interface   32 (32 bytes x  1 ) 2MHz
******************************************************************************/

READ8_MEMBER(bbc_state::bbcm_r)
{
	long myo;

	if (offset<=0x0ff) /* FRED */
	{
		return 0xff;
	};

	if ((offset>=0x100) && (offset<=0x1ff)) /* JIM */
	{
		return 0xff;
	};

	if ((offset>=0x200) && (offset<=0x2ff)) /* SHEILA */
	{
		myo = offset-0x200;
		if ((myo>=0x00) && (myo<=0x06) && (myo+0x01) & 1) return m_hd6845->status_r(space, myo-0x00);                     /* Video controller */
		if ((myo>=0x01) && (myo<=0x07) && (myo & 1))      return m_hd6845->register_r(space, myo-0x01);
		if ((myo>=0x08) && (myo<=0x0f))                   return m_acia ? m_acia->read(space, myo & 0x01) : 0xfe;         /* Serial controller */
		if ((myo>=0x10) && (myo<=0x17))                   return 0xfe;                                                    /* Serial System Chip */
		if ((myo>=0x18) && (myo<=0x1f))                   return m_upd7002 ? m_upd7002->read(space, myo-0x18) : 0xfe;     /* A to D converter */
		if ((myo>=0x20) && (myo<=0x23))                   return 0xfe;                                                    /* VideoULA */
		if ((myo>=0x24) && (myo<=0x27))                   return bbcm_wd177xl_read(space, myo-0x24);                      /* 177x Control Latch */
		if ((myo>=0x28) && (myo<=0x2f) && (m_wd1770))     return m_wd1770->read(myo-0x28);                         /* 1770 Controller */
		if ((myo>=0x28) && (myo<=0x2f) && (m_wd1772))     return m_wd1772->read(myo-0x28);                         /* 1772 Controller */
		if ((myo>=0x28) && (myo<=0x2f))                   return 0xfe;                                                    /* No Controller */
		if ((myo>=0x30) && (myo<=0x33))                   return 0xfe;
		if ((myo>=0x34) && (myo<=0x37))                   return bbcm_acccon_r(space, myo-0x34);                          /* ACCCON */
		if ((myo>=0x38) && (myo<=0x3f))                   return 0xfe;                                                    /* NC ?? */
		if ((myo>=0x40) && (myo<=0x5f))                   return m_via6522_0->read(myo-0x40);
		if ((myo>=0x60) && (myo<=0x7f))                   return m_via6522_1 ? m_via6522_1->read(myo-0x60) : 0xfe;
		if ((myo>=0x80) && (myo<=0x9f))                   return 0xfe;
		if ((myo>=0xa0) && (myo<=0xbf))                   return m_adlc ? m_adlc->read(space, myo & 0x03) : 0xfe;
		if ((myo>=0xc0) && (myo<=0xdf))                   return 0xff;
		if ((myo>=0xe0) && (myo<=0xff))
		{
			if (m_intube &&  m_acccon_itu)                  return m_intube->host_r(space, myo-0xe0);                       /* Internal TUBE */
			if (m_extube && !m_acccon_itu)                  return m_extube->host_r(space, myo-0xe0);                       /* External TUBE */
		}
	}
	return 0xfe;
}

WRITE8_MEMBER(bbc_state::bbcm_w)
{
	long myo;

	if ((offset>=0x200) && (offset<=0x2ff)) /* SHEILA */
	{
		myo=offset-0x200;
		if ((myo>=0x00) && (myo<=0x06) && (myo+0x01) & 1) m_hd6845->address_w(space, myo-0x00, data);                     /* Video Controller */
		if ((myo>=0x01) && (myo<=0x07) && (myo & 1))      m_hd6845->register_w(space, myo-0x01, data);
		if ((myo>=0x08) && (myo<=0x0f))                   if (m_acia) m_acia->write(space, myo & 0x01, data);             /* Serial controller */
		if ((myo>=0x10) && (myo<=0x17))                   bbc_SerialULA_w(space, myo-0x10, data);                         /* Serial System Chip */
		if ((myo>=0x18) && (myo<=0x1f) && (m_upd7002))    m_upd7002->write(space, myo-0x18, data);                        /* A to D converter */
		if ((myo>=0x20) && (myo<=0x23))                   bbc_videoULA_w(space, myo-0x20, data);                          /* VideoULA */
		if ((myo>=0x24) && (myo<=0x27) && (m_wd1770))     bbcm_wd1770l_write(space, myo-0x24, data);                      /* disc control latch */
		if ((myo>=0x28) && (myo<=0x2f) && (m_wd1770))     m_wd1770->write(myo-0x28, data);                         /* 1770 Controller */
		if ((myo>=0x24) && (myo<=0x27) && (m_wd1772))     bbcm_wd1772l_write(space, myo-0x24, data);                      /* disc control latch */
		if ((myo>=0x28) && (myo<=0x2f) && (m_wd1772))     m_wd1772->write(myo-0x28, data);                         /* 1772 Controller */
		if ((myo>=0x30) && (myo<=0x33))                   page_selectbm_w(space, myo-0x30, data);                         /* ROMSEL */
		if ((myo>=0x34) && (myo<=0x37))                   bbcm_acccon_w(space, myo-0x34, data);                           /* ACCCON */
		//if ((myo>=0x38) && (myo<=0x3f))                                                                                 /* NC ?? */
		if ((myo>=0x40) && (myo<=0x5f))                   m_via6522_0->write(myo-0x40, data);
		if ((myo>=0x60) && (myo<=0x7f) && (m_via6522_1))  m_via6522_1->write(myo-0x60, data);
		//if ((myo>=0x80) && (myo<=0x9f))
		if ((myo>=0xa0) && (myo<=0xbf) && (m_adlc))       m_adlc->write(space, myo & 0x03, data);
		//if ((myo>=0xc0) && (myo<=0xdf))
		if ((myo>=0xe0) && (myo<=0xff))
		{
			if (m_intube &&  m_acccon_itu)                  m_intube->host_w(space, myo-0xe0, data);                        /* Internal TUBE */
			if (m_extube && !m_acccon_itu)                  m_extube->host_w(space, myo-0xe0, data);                        /* External TUBE */
		}
	}
}


/******************************************************************************

System VIA 6522

PA0-PA7
Port A forms a slow data bus to the keyboard sound and speech processors
PortA   Keyboard
D0  Pin 8
D1  Pin 9
D2  Pin 10
D3  Pin 11
D4  Pin 5
D5  Pin 6
D6  Pin 7
D7  Pin 12

PB0-PB2 outputs
---------------
These 3 outputs form the address to an 8 bit addressable latch.
(IC32 74LS259)

PB3 output
----------
This output holds the data to be written to the selected
addressable latch bit.

PB4 and PB5 inputs
------------------
These are the inputs from the joystick FIRE buttons. They are
normally at logic 1 with no button pressed and change to 0
when a button is pressed.

PB6 and PB7 inputs from the speech processor (model B and B+)
-------------------------------------------------------------
PB6 is the speech processor 'ready' output and PB7 is from the
speech processor 'interrupt' output.

PB6 and PB7 outputs to Master CMOS RAM/RTC
------------------------------------------
PB6 operates the 146818 chip enable when set to '1'. PB7 operates
the 146818 address strobe line.

CA1 input
---------
This is the vertical sync input from the 6845. CA1 is set up to
interrupt the 6502 every 20ms (50Hz) as a vertical sync from
the video circuitry is detected. The operation system changes
the display flash colours on this interrupt so that they occur
during the screen blanking period.
----------------------------------------------------------------
This is required for a lot of time function within the machine
and must be triggered every 20ms. (Should check at some point
how this 20ms signal is made, and see if none standard shaped
screen modes change this time period.)

CB1 input
---------
The CB1 input is the end of conversion (EOC) signal from the
7002 analogue to digital converter. It can be used to interrupt
the 6502 whenever a conversion is complete.

CA2 input
---------
This input comes from the keyboard circuit, and is used to
generate an interrupt whenever a key is pressed. See the
keyboard circuit section for more details.

CB2 input
---------
This is the light pen strobe signal (LPSTB) from the light pen.
If also connects to the 6845 video processor,
CB2 can be programmed to interrupt the processor whenever
a light pen strobe occurs.
----------------------------------------------------------------
CB2 is not needed in the initial emulation
and should be set to logic low, should be mapped through to
a light pen emulator later.

IRQ output
This connects to the IRQ line of the 6502


The addressable latch
This 8 bit addressable latch is operated from port B lines 0-3.
PB0-PB2 are set to the required address of the output bit to be set.
PB3 is set to the value which should be programmed at that bit.
The function of the 8 output bits from this latch are:-

B0 - Write Enable to the sound generator IC
B1 - READ select on the speech processor
B2 - WRITE select on the speech processor
B3 - Keyboard write enable
B4,B5 - these two outputs define the number to be added to the
start of screen address in hardware to control hardware scrolling:-
Mode    Size    Start of screen  Number to add  B5      B4
0,1,2   20K     &3000                12K        1       1
3       16K     &4000                16K        0       0
4,5     10K     &5800 (or &1800)     22K        1       0
6       8K      &6000 (or &2000)     24K        0       1
B6 - Operates the CAPS lock LED  (Pin 17 keyboard connector)
B7 - Operates the SHIFT lock LED (Pin 16 keyboard connector)
******************************************************************************/


INTERRUPT_GEN_MEMBER(bbc_state::bbcb_keyscan)
{
	/* only do auto scan if keyboard is not enabled */
	if (m_b3_keyboard == 1)
	{
		/* KBD IC1 4 bit addressable counter */
		/* KBD IC3 4 to 10 line decoder */
		/* keyboard not enabled so increment counter */
		m_column = (m_column + 1) % 16;

		/* OS 0.1 programs CA2 to interrupt on negative edge and expects the keyboard to still work */
		//int set = (m_os01 ? 0 : 1);

		if (m_column < 13)
		{
			/* KBD IC4 8 input NAND gate */
			/* set the value of via_system ca2, by checking for any keys
			     being pressed on the selected m_column */
			if ((m_keyboard[m_column]->read() | 0x01) != 0xff)
			{
				m_via6522_0->write_ca2(1);
			}
			else
			{
				m_via6522_0->write_ca2(0);
			}
		}
		else
		{
			m_via6522_0->write_ca2(0);
		}
	}
}


int bbc_state::bbc_keyboard(address_space &space, int data)
{
	int bit;
	int row;
	int res;

	m_column = data & 0x0f;
	row = (data>>4) & 0x07;

	bit = 0;

	if (m_column < 13)
	{
		res = m_keyboard[m_column]->read();
	}
	else
	{
		res = 0xff;
	}

	/* Normal keyboard result */
	if ((res & (1<<row)) == 0)
	{
		bit = 1;
	}

	/* OS 0.1 programs CA2 to interrupt on negative edge and expects the keyboard to still work */
	//int set = (m_os01 ? 0 : 1);

	if ((res | 1) != 0xff)
	{
		m_via6522_0->write_ca2(1);
	}
	else
	{
		m_via6522_0->write_ca2(0);
	}

	return (data & 0x7f) | (bit<<7);
}


void bbc_state::bbcb_IC32_initialise(bbc_state *state)
{
	m_b0_sound = 0x01;             // Write Enable to the sound generator IC
	m_b1_speech_read = 0x01;       // READ select on the speech processor
	m_b2_speech_write = 0x01;      // WRITE select on the speech processor
	m_b3_keyboard = 0x01;          // Keyboard write enable
	m_b4_video0 = 0x01;            // These two outputs define the number to be added to the start of screen address
	m_b5_video1 = 0x01;            // in hardware to control hardware scrolling
	m_b6_caps_lock_led = 0x01;     // Operates the CAPS lock LED
	m_b7_shift_lock_led = 0x01;    // Operates the SHIFT lock LED
}


/* This the BBC Masters Real Time Clock and NVRAM IC */
void bbc_state::MC146818_set(address_space &space)
{
	//logerror ("146181 WR=%d DS=%d AS=%d CE=%d \n",m_MC146818_WR,m_MC146818_DS,m_MC146818_AS,m_MC146818_CE);

	// if chip enabled
	if (m_MC146818_CE)
	{
		// if data select is set then access the data in the 146818
		if (m_MC146818_DS)
		{
			if (m_MC146818_WR)
			{
				m_via_system_porta = m_rtc->read(space, 1);
				//logerror("read 146818 data %d \n",m_via_system_porta);
			}
			else
			{
				m_rtc->write(space, 1, m_via_system_porta);
				//logerror("write 146818 data %d \n",m_via_system_porta);
			}
		}

		// if address select is set then set the address in the 146818
		if (m_MC146818_AS)
		{
			m_rtc->write(space, 0, m_via_system_porta);
			//logerror("write 146818 address %d \n",m_via_system_porta);
		}
	}
}


WRITE8_MEMBER(bbc_state::bbcb_via_system_write_porta)
{
	//logerror("SYSTEM write porta %d\n",data);

	m_via_system_porta = data;
	if (m_b0_sound == 0)
	{
		//logerror("Doing an unsafe write to the sound chip %d \n",data);
		if (m_sn) m_sn->write(m_via_system_porta);
	}
	if (m_b1_speech_read == 0)
	{
		if (m_tms) m_via_system_porta = m_tms->status_r();
		//logerror("Doing an unsafe read to the speech chip %d \n",m_via_system_porta);
	}
	if (m_b2_speech_write == 0)
	{
		//logerror("Doing an unsafe write to the speech chip %d \n",data);
		if (m_tms) m_tms->data_w(m_via_system_porta);
	}
	if (m_b3_keyboard == 0)
	{
		//logerror("Doing an unsafe write to the keyboard %d \n",data);
		m_via_system_porta = bbc_keyboard(space, m_via_system_porta);
	}
	if (m_rtc) MC146818_set(space);
}


WRITE8_MEMBER(bbc_state::bbcb_via_system_write_portb)
{
	int bit, value;
	bit = data & 0x07;
	value = BIT(data, 3);

	//logerror("SYSTEM write portb %d %d %d\n",data,bit,value);

	if (value)
	{
		switch (bit)
		{
		case 0:
			if (m_b0_sound == 0)
			{
				m_b0_sound = 1;
			}
			break;
		case 1:
			if (m_machinetype == MASTER && m_MC146818_WR == 0)
			{
				/* BBC Master has NVRAM Here */
				m_MC146818_WR = 1;
				MC146818_set(space);
			}
			else
			{
				if (m_b1_speech_read == 0)
				{
					/* VSP TMS 5220 */
					m_b1_speech_read = 1;
					//logerror("Speech read select true\n");
					if (m_tms) m_tms->rsq_w(true);
				}
			}
			break;
		case 2:
			if (m_machinetype == MASTER && m_MC146818_DS == 0)
			{
				/* BBC Master has NVRAM Here */
				m_MC146818_DS = 1;
				MC146818_set(space);
			}
			else
			{
				if (m_b2_speech_write == 0)
				{
					/* VSP TMS 5220 */
					m_b2_speech_write = 1;
					//logerror("Speech write select true\n");
					if (m_tms) m_tms->wsq_w(true);
				}
			}
			break;
		case 3:
			if (m_b3_keyboard == 0)
			{
				m_b3_keyboard = 1;
			}
			break;
		case 4:
			if (m_b4_video0 == 0)
			{
				m_b4_video0 = 1;
			}
			break;
		case 5:
			if (m_b5_video1 == 0)
			{
				m_b5_video1 = 1;
			}
			break;
		case 6:
			if (m_b6_caps_lock_led == 0)
			{
				m_b6_caps_lock_led = 1;
				/* call caps lock led update */
				output().set_value("capslock_led", m_b6_caps_lock_led);
			}
			break;
		case 7:
			if (m_b7_shift_lock_led == 0)
			{
				m_b7_shift_lock_led = 1;
				/* call shift lock led update */
				output().set_value("shiftlock_led", m_b7_shift_lock_led);
			}
			break;
		}
	}
	else
	{
		switch (bit)
		{
		case 0:
			if (m_b0_sound == 1)
			{
				m_b0_sound = 0;
				if (m_sn) m_sn->write(m_via_system_porta);
			}
			break;
		case 1:
			if (m_machinetype == MASTER && m_MC146818_WR == 1)
			{
				/* BBC Master has NVRAM Here */
				m_MC146818_WR = 0;
				MC146818_set(space);
			}
			else
			{
				if (m_b1_speech_read == 1)
				{
					/* VSP TMS 5220 */
					m_b1_speech_read = 0;
					//logerror("Speech read select false\n");
					if (m_tms) m_tms->rsq_w(false);
				}
			}
			break;
		case 2:
			if (m_machinetype == MASTER && m_MC146818_DS == 1)
			{
				/* BBC Master has NVRAM Here */
				m_MC146818_DS = 0;
				MC146818_set(space);
			}
			else
			{
				if (m_b2_speech_write == 1)
				{
					/* VSP TMS 5220 */
					m_b2_speech_write = 0;
					//logerror("Speech write select false\n");
					if (m_tms) m_tms->wsq_w(false);
				}
			}
			break;
		case 3:
			if (m_b3_keyboard == 1)
			{
				m_b3_keyboard = 0;
				/* *** call keyboard enabled *** */
				m_via_system_porta = bbc_keyboard(space, m_via_system_porta);
			}
			break;
		case 4:
			if (m_b4_video0 == 1)
			{
				m_b4_video0 = 0;
			}
			break;
		case 5:
			if (m_b5_video1 == 1)
			{
				m_b5_video1 = 0;
			}
			break;
		case 6:
			if (m_b6_caps_lock_led == 1)
			{
				m_b6_caps_lock_led = 0;
				/* call caps lock led update */
				output().set_value("capslock_led", m_b6_caps_lock_led);
			}
			break;
		case 7:
			if (m_b7_shift_lock_led == 1)
			{
				m_b7_shift_lock_led = 0;
				/* call shift lock led update */
				output().set_value("shiftlock_led", m_b7_shift_lock_led);
			}
			break;
		}
	}


	if (m_machinetype == MASTER)
	{
		//set the Address Select
		if (m_MC146818_AS != BIT(data,7))
		{
			m_MC146818_AS = BIT(data,7);
			MC146818_set(space);
		}

		//if CE changes
		if (m_MC146818_CE != BIT(data,6))
		{
			m_MC146818_CE = BIT(data,6);
			MC146818_set(space);
		}
	}
}


READ8_MEMBER(bbc_state::bbcb_via_system_read_porta)
{
	//logerror("SYSTEM read porta %d\n",m_via_system_porta);
	return m_via_system_porta;
}


READ8_MEMBER(bbc_state::bbcb_via_system_read_portb)
{
	// D4 of portb is joystick fire button 1
	// D5 of portb is joystick fire button 2
	// D6 VSPINT
	// D7 VSPRDY
	int vspint = m_tms ? m_tms->intq_r() : 0;
	int vsprdy = m_tms ? m_tms->readyq_r() : 0;
	//logerror("TMSint %d\n",TMSint);
	//logerror("TMSrdy %d\n",TMSrdy);
	return ((m_analog ? m_analog->pb_r() : 0x30) | (!vsprdy << 7) | (!vspint << 6));
}


/**************************************
  BBC Joystick Support
**************************************/

UPD7002_GET_ANALOGUE(bbc_state::BBC_get_analogue_input)
{
	if (m_analog)
		return ((0xff - m_analog->ch_r(channel_number)) << 8);
	else
		return 0xff;
}

UPD7002_EOC(bbc_state::BBC_uPD7002_EOC)
{
	m_via6522_0->write_cb1(data);
}

/***************************************
  BBC 2C199 Serial Interface Cassette
****************************************/


void bbc_state::MC6850_Receive_Clock(int new_clock)
{
	m_rxd_cass = new_clock;
	update_acia_rxd();

	//
	// Somehow the "serial processor" generates 16 clock signals towards
	// the 6850. Exact details are unknown, faking it with the following
	// loop.
	//
	for (int i = 0; i < 16; i++ )
	{
		m_acia->write_rxc(1);
		m_acia->write_rxc(0);
	}
}

TIMER_CALLBACK_MEMBER(bbc_state::bbc_tape_timer_cb)
{
	if ( m_cass_out_enabled )
	{
		// 0 = 18-18 18-17-1
		// 1 = 9-9-9-9 9-9-9-8-1

		switch ( m_cass_out_samples_to_go )
		{
			case 0:
				if ( m_cass_out_phase == 0 )
				{
					// get bit value
					m_cass_out_bit = m_txd;
					if ( m_cass_out_bit )
					{
						m_cass_out_phase = 3;
						m_cass_out_samples_to_go = 9;
					}
					else
					{
						m_cass_out_phase = 1;
						m_cass_out_samples_to_go = 18;
					}
					m_cassette->output( +1.0 );
				}
				else
				{
					// switch phase
					m_cass_out_phase--;
					m_cass_out_samples_to_go = m_cass_out_bit ? 9 : 18;
					m_cassette->output( ( m_cass_out_phase & 1 ) ? +1.0 : -1.0 );
				}
				break;
			case 1:
				if ( m_cass_out_phase == 0 )
				{
					m_cassette->output( 0.0 );
				}
				break;
		}

		m_cass_out_samples_to_go--;
	}
	else
	{
		double dev_val = m_cassette->input();

		// look for edges on the cassette wave
		if (((dev_val>=0.0) && (m_last_dev_val<0.0)) || ((dev_val<0.0) && (m_last_dev_val>=0.0)))
		{
			if (m_wav_len>(9*3))
			{
				//this is too long to receive anything so reset the serial IC. This is a hack, this should be done as a timer in the MC6850 code.
				logerror ("Cassette length %d\n",m_wav_len);
				m_nr_high_tones = 0;
				m_dcd_cass = 0;
				update_acia_dcd();
				m_len0=0;
				m_len1=0;
				m_len2=0;
				m_len3=0;
				m_wav_len=0;
			}

			m_len3=m_len2;
			m_len2=m_len1;
			m_len1=m_len0;
			m_len0=m_wav_len;

			m_wav_len=0;
			logerror ("cassette  %d  %d  %d  %d\n",m_len3,m_len2,m_len1,m_len0);

			if ((m_len0+m_len1)>=(18+18-5))
			{
				/* Clock a 0 onto the serial line */
				logerror("Serial value 0\n");
				m_nr_high_tones = 0;
				m_dcd_cass = 0;
				update_acia_dcd();
				MC6850_Receive_Clock(0);
				m_len0=0;
				m_len1=0;
				m_len2=0;
				m_len3=0;
			}

			if (((m_len0+m_len1+m_len2+m_len3)<=41) && (m_len3!=0))
			{
				/* Clock a 1 onto the serial line */
				logerror("Serial value 1\n");
				m_nr_high_tones++;
				if ( m_nr_high_tones > 100 )
				{
					m_dcd_cass = 1;
					update_acia_dcd();
				}
				MC6850_Receive_Clock(1);
				m_len0=0;
				m_len1=0;
				m_len2=0;
				m_len3=0;
			}
		}

		m_wav_len++;
		m_last_dev_val=dev_val;
	}
}

WRITE_LINE_MEMBER( bbc_state::write_rxd_serial )
{
	m_rxd_serial = state;
	update_acia_rxd();
}

void bbc_state::update_acia_rxd()
{
	m_acia->write_rxd(( m_serproc_data & 0x40 ) ? m_rxd_serial : m_rxd_cass);
}


WRITE_LINE_MEMBER( bbc_state::write_dcd_serial )
{
	m_dcd_serial = state;
	update_acia_dcd();
}

void bbc_state::update_acia_dcd()
{
	m_acia->write_dcd(( m_serproc_data & 0x40 ) ? m_dcd_serial : m_dcd_cass);
}


WRITE_LINE_MEMBER( bbc_state::write_cts_serial )
{
	m_cts_serial = state;
	update_acia_cts();
}

void bbc_state::update_acia_cts()
{
	m_acia->write_cts(( m_serproc_data & 0x40 ) ? m_cts_serial : 0);
}


WRITE_LINE_MEMBER( bbc_state::bbc_rts_w )
{
	if ( m_serproc_data & 0x40 )
	{
		m_rs232->write_rts(state);
		m_cass_out_enabled = 0;
	}
	else
	{
		m_cass_out_enabled = state ? 0 : 1;
	}
}


WRITE_LINE_MEMBER( bbc_state::bbc_txd_w )
{
	if ( m_serproc_data & 0x40 )
	{
		m_rs232->write_txd(state);
	}
	else
	{
		m_txd = state;
	}
}


void bbc_state::cassette_motor(bool motor_state)
{
	const bool prev_state = ((m_cassette->get_state() & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_ENABLED) ? true : false;

	/* cassette relay sound */
	if (prev_state != motor_state)
		m_samples->start(0, motor_state ? 1 : 0);

	if (motor_state)
	{
		m_cassette->change_state(CASSETTE_MOTOR_ENABLED, CASSETTE_MASK_MOTOR);
		m_tape_timer->adjust(attotime::zero, 0, attotime::from_hz(44100));
	}
	else
	{
		m_cassette->change_state(CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
		m_tape_timer->reset();
		m_len0 = 0;
		m_len1 = 0;
		m_len2 = 0;
		m_len3 = 0;
		m_wav_len = 0;
		m_cass_out_phase = 0;
		m_cass_out_samples_to_go = 4;
	}
	output().set_value("motor_led", !motor_state);
}


//
// Serial processor control
// x--- ---- - Motor OFF(0)/ON(1)
// -x-- ---- - Cassette(0)/RS243 input(1)
// --xx x--- - Receive baud rate generator control
// ---- -xxx - Transmit baud rate generator control
//             These possible settings apply to both the receive
//             and transmit baud generator control bits:
//             000 - 16MHz / 13 /   1 - 19200 baud
//             001 - 16MHz / 13 /  16 -  1200 baud
//             010 - 16MHz / 13 /   4 -  4800 baud
//             011 - 16MHz / 13 / 128 -   150 baud
//             100 - 16MHz / 13 /   2 -  9600 baud
//             101 - 16MHz / 13 /  64 -   300 baud
//             110 - 16MHz / 13 /   8 -  2400 baud
//             110 - 16MHz / 13 / 256 -    75 baud
//
WRITE8_MEMBER(bbc_state::bbc_SerialULA_w)
{
	static const int serial_clocks[8] =
	{
		1,    // 000
		16,   // 001
		4,    // 010
		128,  // 011
		2,    // 100
		64,   // 101
		8,    // 110
		256   // 111
	};

	m_serproc_data = data;
	update_acia_rxd();
	update_acia_dcd();
	update_acia_cts();
	if (m_cassette) cassette_motor(BIT(m_serproc_data, 7));

	// Set transmit clock rate
	m_acia_clock->set_clock_scale( (double) 1 / serial_clocks[ data & 0x07 ] );
}

WRITE_LINE_MEMBER(bbc_state::write_acia_clock)
{
	m_acia->write_txc(state);

	if (m_serproc_data & 0x40)
		m_acia->write_rxc(state);
}


/**************************************
   1MHz Bus interrupts
***************************************/


WRITE_LINE_MEMBER(bbc_state::bus_nmi_w)
{
	m_bus_nmi = state;
	bbc_update_nmi();
}


/**************************************
   i8271 disc control function
***************************************/


WRITE_LINE_MEMBER(bbc_state::motor_w)
{
	for (int i=0; i != 2; i++) {
		char devname[8];
		sprintf(devname, "%d", i);
		floppy_connector *con = m_i8271->subdevice<floppy_connector>(devname);
		if (con) {
			con->get_device()->mon_w(!state);
		}
	}
}

WRITE_LINE_MEMBER(bbc_state::side_w)
{
	for (int i=0; i != 2; i++) {
		char devname[8];
		sprintf(devname, "%d", i);
		floppy_connector *con = m_i8271->subdevice<floppy_connector>(devname);
		if (con) {
			con->get_device()->ss_w(state);
		}
	}
}


/**************************************
   WD1770 disc control function
***************************************/


/* wd177x_IRQ_SET and latch bit 4 (nmi_enable) are NAND'ED together
   wd177x_DRQ_SET and latch bit 4 (nmi_enable) are NAND'ED together
   the output of the above two NAND gates are then OR'ED together and sent to the 6502 NMI line.
    DRQ and IRQ are active low outputs from wd177x. We use wd177x_DRQ_SET for DRQ = 0,
    and wd177x_DRQ_CLR for DRQ = 1. Similarly wd177x_IRQ_SET for IRQ = 0 and wd177x_IRQ_CLR
    for IRQ = 1.

  The above means that if IRQ or DRQ are set, a interrupt should be generated.
  The nmi_enable decides if interrupts are actually triggered.
  The nmi is edge triggered, and triggers on a +ve edge.
*/


void bbc_state::bbc_update_nmi()
{
	if (m_fdc_irq || m_fdc_drq || m_adlc_irq || m_bus_nmi)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	}
	else
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
	}
}

WRITE_LINE_MEMBER(bbc_state::fdc_intrq_w)
{
	m_fdc_irq = state;
	bbc_update_nmi();
}

WRITE_LINE_MEMBER(bbc_state::fdc_drq_w)
{
	m_fdc_drq = state;
	bbc_update_nmi();
}

/*
   B/ B+ drive control:

        Bit       Meaning
        -----------------
        7,6       Not used.
         5        Reset drive controller chip. (0 = reset controller, 1 = no reset)
         4        Interrupt Enable (0 = enable int, 1 = disable int)
         3        Double density select (0 = double, 1 = single).
         2        Side select (0 = side 0, 1 = side 1).
         1        Drive select 1.
         0        Drive select 0.
*/

WRITE8_MEMBER(bbc_state::bbc_wd1770_status_w)
{
	floppy_image_device *floppy = nullptr;

	m_drive_control = data;

	// bit 0, 1: drive select
	if (BIT(data, 0)) floppy = m_wd1770->subdevice<floppy_connector>("0")->get_device();
	if (BIT(data, 1)) floppy = m_wd1770->subdevice<floppy_connector>("1")->get_device();
	m_wd1770->set_floppy(floppy);

	// bit 2: side select
	if (floppy)
		floppy->ss_w(BIT(data, 2));

	// bit 3: density
	m_wd1770->dden_w(BIT(data, 3));

	// bit 4: interrupt enable

	// bit 5: reset
	if (!BIT(data, 5)) m_wd1770->soft_reset();
}

/*
   Master drive control:

        Bit       Meaning
        -----------------
        7,6       Not used.
         5        Double density select (0 = double, 1 = single).
         4        Side select (0 = side 0, 1 = side 1).
         3        Drive select 2.
         2        Reset drive controller chip. (0 = reset controller, 1 = no reset)
         1        Drive select 1.
         0        Drive select 0.
*/

READ8_MEMBER(bbc_state::bbcm_wd177xl_read)
{
	return m_drive_control;
}

WRITE8_MEMBER(bbc_state::bbcm_wd1770l_write)
{
	floppy_image_device *floppy = nullptr;

	m_drive_control = data;

	// bit 0, 1, 3: drive select
	if (BIT(data, 0)) floppy = m_wd1770->subdevice<floppy_connector>("0")->get_device();
	if (BIT(data, 1)) floppy = m_wd1770->subdevice<floppy_connector>("1")->get_device();
	if (BIT(data, 3)) floppy = m_wd1770->subdevice<floppy_connector>("2")->get_device();
	m_wd1770->set_floppy(floppy);

	// bit 4: side select
	if (floppy)
		floppy->ss_w(BIT(data, 4));

	// bit 5: density
	m_wd1770->dden_w(BIT(data, 5));

	// bit 2: reset
	if (!BIT(data, 2)) m_wd1770->soft_reset();
}

WRITE8_MEMBER(bbc_state::bbcm_wd1772l_write)
{
	floppy_image_device *floppy = nullptr;

	m_drive_control = data;

	// bit 0, 1, 3: drive select
	if (BIT(data, 0)) floppy = m_wd1772->subdevice<floppy_connector>("0")->get_device();
	if (BIT(data, 1)) floppy = m_wd1772->subdevice<floppy_connector>("1")->get_device();
	if (BIT(data, 3)) floppy = m_wd1772->subdevice<floppy_connector>("2")->get_device();
	m_wd1772->set_floppy(floppy);

	// bit 4: side select
	if (floppy)
		floppy->ss_w(BIT(data, 4));

	// bit 5: density
	m_wd1772->dden_w(BIT(data, 5));

	// bit 2: reset
	if (!BIT(data, 2)) m_wd1772->soft_reset();
}

/**************************************
   BBC B Rom loading functions
***************************************/

image_init_result bbc_state::bbc_load_rom(device_image_interface &image, generic_slot_device *slot)
{
	uint32_t size = slot->common_get_size("rom");

	// socket accepts 8K and 16K ROM only
	if (size != 0x2000 && size != 0x4000)
	{
		image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported ROM size");
		return image_init_result::FAIL;
	}

	slot->rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	slot->common_load_rom(slot->get_rom_base(), size, "rom");

	return image_init_result::PASS;
}

/**************************************
   BBC Master Rom loading functions
***************************************/

image_init_result bbc_state::bbcm_load_cart(device_image_interface &image, generic_slot_device *slot)
{
	if (!image.loaded_through_softlist())
	{
		uint32_t filesize = image.length();

		if (filesize > 0x8000)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
			return image_init_result::FAIL;
		}

		slot->rom_alloc(filesize, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
		image.fread(slot->get_rom_base(), filesize);
		return image_init_result::PASS;
	}
	else
	{
		uint32_t size_lo = image.get_software_region_length("lorom");
		uint32_t size_hi = image.get_software_region_length("uprom");

		if (size_lo + size_hi > 0x8000)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Unsupported cartridge size");
			return image_init_result::FAIL;
		}

		slot->rom_alloc(size_lo + size_hi, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
		memcpy(slot->get_rom_base() + 0,       image.get_software_region("uprom"), size_hi);
		memcpy(slot->get_rom_base() + size_hi, image.get_software_region("lorom"), size_lo);
	}

	return image_init_result::PASS;
}


/**************************************
   Machine Initialisation functions
***************************************/

void bbc_state::init_bbc()
{
	m_os01 = false;

	m_rxd_cass = 0;
	m_nr_high_tones = 0;
	m_serproc_data = 0;
	m_cass_out_enabled = 0;
	m_tape_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bbc_state::bbc_tape_timer_cb),this));

	/* vertical sync pulse from video circuit */
	m_via6522_0->write_ca1(1);

	/* light pen strobe detect (not emulated) */
	m_via6522_0->write_cb2(1);
}


// setup pointers for optional EPROMs
void bbc_state::bbc_setup_banks(memory_bank *membank, int banks, uint32_t shift, uint32_t size)
{
	std::string region_tag;
	memory_region *tmp_reg;
	uint8_t *eprom[4];
	if (m_exp1 && (tmp_reg = memregion(region_tag.assign(m_exp1->tag()).append(GENERIC_ROM_REGION_TAG).c_str())))
		eprom[0] = tmp_reg->base() + shift;
	else
		eprom[0] = m_region_opt->base() + 0x0000 + shift;
	if (m_exp2 && (tmp_reg = memregion(region_tag.assign(m_exp2->tag()).append(GENERIC_ROM_REGION_TAG).c_str())))
		eprom[1] = tmp_reg->base() + shift;
	else
		eprom[1] = m_region_opt->base() + 0x4000 + shift;
	if (m_exp3 && (tmp_reg = memregion(region_tag.assign(m_exp3->tag()).append(GENERIC_ROM_REGION_TAG).c_str())))
		eprom[2] = tmp_reg->base() + shift;
	else
		eprom[2] = m_region_opt->base() + 0x8000 + shift;
	if (m_exp4 && (tmp_reg = memregion(region_tag.assign(m_exp4->tag()).append(GENERIC_ROM_REGION_TAG).c_str())))
		eprom[3] = tmp_reg->base() + shift;
	else
		eprom[3] = m_region_opt->base() + 0xc000 + shift;

	membank->configure_entries(0, 1, eprom[0], size);
	membank->configure_entries(1, 1, eprom[1], size);
	membank->configure_entries(2, 1, eprom[2], size);
	membank->configure_entries(3, 1, eprom[3], size);

	if (banks > 4)
	{
		for (int i = 0; i < banks - 4; i++)
			membank->configure_entries(i + 4, 1,  m_region_opt->base() + 0x10000 + shift + i * 0x4000, size);
	}
}

void bbc_state::bbcm_setup_banks(memory_bank *membank, int banks, uint32_t shift, uint32_t size)
{
	std::string region_tag;
	memory_region *tmp_reg;
	uint8_t *eprom[2];
	if (m_exp1 && (tmp_reg = memregion(region_tag.assign(m_exp1->tag()).append(GENERIC_ROM_REGION_TAG).c_str())))
		eprom[0] = tmp_reg->base() + shift;
	else
		eprom[0] = m_region_opt->base() + 0x0000 + shift;
	if (m_exp2 && (tmp_reg = memregion(region_tag.assign(m_exp2->tag()).append(GENERIC_ROM_REGION_TAG).c_str())))
		eprom[1] = tmp_reg->base() + shift;
	else
		eprom[1] = m_region_opt->base() + 0x8000 + shift;

	membank->configure_entries(0, 1, eprom[0], size);
	membank->configure_entries(1, 1, eprom[0] + 0x4000, size);
	membank->configure_entries(2, 1, eprom[1], size);
	membank->configure_entries(3, 1, eprom[1] + 0x4000, size);

	if (banks > 4)
	{
		for (int i = 0; i < banks - 4; i++)
			membank->configure_entries(i + 4, 1,  m_region_opt->base() + 0x10000 + shift + i * 0x4000, size);
	}
}

MACHINE_START_MEMBER(bbc_state, bbca)
{
	m_machinetype = MODELA;

	bbc_setup_banks(m_bank4, 4, 0, 0x4000);
}

MACHINE_RESET_MEMBER(bbc_state, bbca)
{
	m_monitortype = m_bbcconfig.read_safe(0) & 0x03;
	m_swramtype = (m_bbcconfig.read_safe(0) & 0x0c) >> 2;

	m_bank1->set_base(m_region_maincpu->base());
	if (m_ram->size() == 32*1024)
	{
		/* 32K Model A */
		m_bank3->set_base(m_region_maincpu->base() + 0x4000);
	}
	else
	{
		/* 16K just repeat the lower 16K*/
		m_bank3->set_base(m_region_maincpu->base());
	}

	m_bank4->set_entry(0);
	m_bank7->set_base(m_region_os->base());  /* bank 7 points at the OS rom  from c000 to ffff */

	bbcb_IC32_initialise(this);
}

MACHINE_START_MEMBER(bbc_state, bbcb)
{
	m_machinetype = MODELB;
	m_mc6850_clock = 0;
	bbc_setup_banks(m_bank4, 16, 0, 0x4000);
}

MACHINE_RESET_MEMBER(bbc_state, bbcb)
{
	m_monitortype = m_bbcconfig.read_safe(0) & 0x03;
	m_swramtype = (m_bbcconfig.read_safe(0) & 0x0c) >> 2;

	m_bank1->set_base(m_region_maincpu->base());
	m_bank3->set_base(m_region_maincpu->base() + 0x4000);

	m_bank4->set_entry(0);
	m_bank7->set_base(m_region_os->base());  /* bank 7 points at the OS rom  from c000 to ffff */

	bbcb_IC32_initialise(this);
}


MACHINE_RESET_MEMBER(bbc_state, torch)
{
	MACHINE_RESET_CALL_MEMBER(bbcb);

	m_monitortype = monitor_type_t::COLOUR;
	m_swramtype = 0;
}


MACHINE_START_MEMBER(bbc_state, bbcbp)
{
	m_machinetype = BPLUS;
	m_mc6850_clock = 0;

	bbc_setup_banks(m_bank4, 16, 0, 0x3000);
	m_bank4->configure_entries(16, 1, m_region_maincpu->base() + 0x8000, 0x3000);   // additional bank for paged ram
	bbc_setup_banks(m_bank6, 16, 0x3000, 0x1000);
}

MACHINE_RESET_MEMBER(bbc_state, bbcbp)
{
	m_monitortype = m_bbcconfig.read_safe(0) & 0x03;
	m_swramtype = 0;

	m_bank1->set_base(m_region_maincpu->base());
	m_bank2->set_base(m_region_maincpu->base() + 0x3000);  /* bank 2 screen/shadow ram     from 3000 to 7fff */
	m_bank4->set_entry(0);
	m_bank6->set_entry(0);
	m_bank7->set_base(m_region_os->base());                /* bank 7 points at the OS rom  from c000 to ffff */

	bbcb_IC32_initialise(this);
}


MACHINE_START_MEMBER(bbc_state, bbcm)
{
	m_machinetype = MASTER;
	m_mc6850_clock = 0;

	m_lk18_ic41_paged_rom = false; /* Link set for RAM in slots 6 and 7 */
	m_lk19_ic37_paged_rom = false; /* Link set for RAM in slots 4 and 5 */

	bbcm_setup_banks(m_bank4, 16, 0, 0x1000);
	m_bank4->configure_entries(16, 1, m_region_maincpu->base() + 0x8000, 0x1000);   // additional bank for paged ram
	bbcm_setup_banks(m_bank5, 16, 0x1000, 0x3000);

	/* Set ROM/IO bank to point to rom */
	m_bank8->set_base(m_region_os->base() + 0x3c00);
	m_maincpu->space(AS_PROGRAM).install_read_bank(0xfc00, 0xfeff, "bank8");

	output().set_value("power_led", 0);
}

MACHINE_RESET_MEMBER(bbc_state, bbcm)
{
	m_monitortype = m_bbcconfig.read_safe(0) & 0x03;
	m_swramtype = 0;

	m_bank1->set_base(m_region_maincpu->base());           /* bank 1 regular lower RAM     from 0000 to 2fff */
	m_bank2->set_base(m_region_maincpu->base() + 0x3000);  /* bank 2 screen/shadow RAM     from 3000 to 7fff */
	m_bank4->set_entry(0);
	m_bank5->set_entry(0);
	m_bank7->set_base(m_region_os->base());                /* bank 6 OS rom of RAM         from c000 to dfff */

	bbcb_IC32_initialise(this);
}


MACHINE_START_MEMBER(bbc_state, bbcmc)
{
	MACHINE_START_CALL_MEMBER(bbcm);

	m_machinetype = COMPACT;
}

MACHINE_RESET_MEMBER(bbc_state, bbcmc)
{
	MACHINE_RESET_CALL_MEMBER(bbcm);
}


MACHINE_RESET_MEMBER(bbc_state, ltmpbp)
{
	MACHINE_RESET_CALL_MEMBER(bbcbp);

	m_monitortype = monitor_type_t::GREEN;
	m_swramtype = 0;
}

MACHINE_RESET_MEMBER(bbc_state, ltmpm)
{
	MACHINE_RESET_CALL_MEMBER(bbcm);

	m_monitortype = monitor_type_t::GREEN;
	m_swramtype = 0;
}


MACHINE_START_MEMBER(bbc_state, cfa3000)
{
	MACHINE_START_CALL_MEMBER(bbcm);

	m_lk18_ic41_paged_rom = true; /* Link set for ROM in slots 6 and 7 */
	m_lk19_ic37_paged_rom = true; /* Link set for ROM in slots 4 and 5 */
}
