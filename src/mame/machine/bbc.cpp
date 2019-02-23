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


TIMER_CALLBACK_MEMBER(bbc_state::reset_timer_cb)
{
	m_maincpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);
}

/****************************************
  BBC Model B memory handling functions
****************************************/

WRITE8_MEMBER(bbc_state::bbc_romsel_w)
{
	if (m_swramtype == 0)
	{
		/* no sideways expansion board fitted so address only the 4 on board ROM sockets */
		m_swrbank = (data & 0x03) | 0x0c;
	}
	else
	{
		/* expansion board fitted so address all 16 ROM sockets */
		m_swrbank = data & 0x0f;
	}
}

READ8_MEMBER(bbc_state::bbc_paged_r)
{
	uint8_t data;
	std::string region_tag;

	if (m_rom[m_swrbank] && memregion(region_tag.assign(m_rom[m_swrbank]->tag()).append(BBC_ROM_REGION_TAG).c_str()))
	{
		data = m_rom[m_swrbank]->read(space, offset);
	}
	else
	{
		data = m_region_swr->base()[offset + (m_swrbank << 14)];
	}

	return data;
}

WRITE8_MEMBER(bbc_state::bbc_paged_w)
{
	static const unsigned short swramtype[4][16] = {
		// TODO: move sideways ROM/RAM boards to slot devices
		{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }, // 0: none
		{ 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1 }, // 1: 128K (bank 8 to 15) Solidisk sideways ram userport bank latch (not implemented)
		{ 0,0,0,0,1,1,1,1,0,0,0,0,0,0,0,0 }, // 2: 64K (banks 4 to 7) for Acorn sideways ram FE30 bank latch
		{ 0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1 }, // 3: 128K (banks 8 to 15) for Acorn sideways ram FE30 bank latch
	};
	std::string region_tag;

	if (m_rom[m_swrbank] && memregion(region_tag.assign(m_rom[m_swrbank]->tag()).append(BBC_ROM_REGION_TAG).c_str()))
	{
		m_rom[m_swrbank]->write(space, offset, data);
	}
	else if (swramtype[m_swramtype][m_swrbank])
	{
		m_region_swr->base()[offset + (m_swrbank << 14)] = data;
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
			m_bank2->set_base(m_ram->pointer() + 0xb000);
		}
		else
		{
			m_bank2->set_base(m_ram->pointer() + 0x3000);
		}
		break;

	case 0xc000:
	case 0xd000:
		/* Access shadow RAM if VDU drivers and shadow RAM selected */
		if (m_vdusel)
		{
			m_bank2->set_base(m_ram->pointer() + 0xb000);
		}
		else
		{
			m_bank2->set_base(m_ram->pointer() + 0x3000);
		}
		break;

	default:
		m_bank2->set_base(m_ram->pointer() + 0x3000);
		break;
	}
	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

WRITE8_MEMBER(bbc_state::bbcbp_romsel_w)
{
	/* the BBC Model B+ addresses all 16 ROM sockets and extra 12K of RAM at 0x8000 and 20K of shadow RAM at 0x3000 */
	switch (offset & 0x07)
	{
	case 0x00:
		m_paged_ram = BIT(data, 7);

		m_swrbank = data & 0x0f;
		break;

	case 0x04:
		/* the video display should now use this flag to display the shadow RAM memory */
		m_vdusel = BIT(data, 7);
		setvideoshadow(m_vdusel);
		break;
	}
}

READ8_MEMBER(bbc_state::bbcbp_paged_r)
{
	uint8_t data;
	std::string region_tag;

	if (m_paged_ram && offset < 0x3000)
	{
		data = m_ram->pointer()[offset + 0x8000];
	}
	else
	{
		if (m_rom[m_swrbank] && memregion(region_tag.assign(m_rom[m_swrbank]->tag()).append(BBC_ROM_REGION_TAG).c_str()))
		{
			data = m_rom[m_swrbank]->read(space, offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_swrbank << 14)];
		}
	}

	return data;
}

WRITE8_MEMBER(bbc_state::bbcbp_paged_w)
{
	/* the BBC Model B+ 128K has extra RAM mapped in replacing the ROM banks 0,1,c and d. */
	static const unsigned short swram_banks[16] = { 1,1,0,0,0,0,0,0,0,0,0,0,1,1,0,0 };
	std::string region_tag;

	if (m_paged_ram && offset < 0x3000)
	{
		m_ram->pointer()[offset + 0x8000] = data;
	}
	else
	{
		if (m_rom[m_swrbank] && memregion(region_tag.assign(m_rom[m_swrbank]->tag()).append(BBC_ROM_REGION_TAG).c_str()))
		{
			m_rom[m_swrbank]->write(space, offset, data);
		}
		else if (m_ram->size() == 128 * 1024 && swram_banks[m_swrbank])
		{
			m_region_swr->base()[offset + (m_swrbank << 14)] = data;
		}
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
		m_bank2->set_base(m_ram->pointer() + 0xb000);
	}
	else
	{
		m_bank2->set_base(m_ram->pointer() + 0x3000);
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

	/* Bit D causes the CRT controller to display the contents of LYNNE. */
	setvideoshadow(m_acccon_d);

	/* Bit X causes all accesses to 0x3000-0x7fff to be re-directed to LYNNE.*/
	if (m_acccon_x)
	{
		m_bank2->set_base(m_ram->pointer() + 0xb000);
	}
	else
	{
		m_bank2->set_base(m_ram->pointer() + 0x3000);
	}

	/* Bit TST controls paging of ROM reads in the 0xfc00-0xfeff region */
	/* if 0 the I/O is paged for both reads and writes */
	/* if 1 the ROM is paged in for reads but writes still go to I/O */
	m_bankdev->set_bank(m_acccon_tst);
}

WRITE8_MEMBER(bbc_state::bbcm_romsel_w)
{
	m_paged_ram = (data & 0x80) >> 7;
	m_swrbank = data & 0x0f;
}

READ8_MEMBER(bbc_state::bbcm_paged_r)
{
	uint8_t data;
	std::string region_tag;

	if (m_paged_ram && offset < 0x1000)
	{
		data = m_ram->pointer()[offset + 0x8000];
	}
	else
	{
		if (m_rom[m_swrbank] && memregion(region_tag.assign(m_rom[m_swrbank]->tag()).append(BBC_ROM_REGION_TAG).c_str()))
		{
			data = m_rom[m_swrbank]->read(space, offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_swrbank << 14)];
		}
	}

	return data;
}

WRITE8_MEMBER(bbc_state::bbcm_paged_w)
{
	std::string region_tag;

	if (m_paged_ram && offset < 0x1000)
	{
		m_ram->pointer()[offset + 0x8000] = data;
	}
	else
	{
		if (m_rom[m_swrbank] && memregion(region_tag.assign(m_rom[m_swrbank]->tag()).append(BBC_ROM_REGION_TAG).c_str()))
		{
			m_rom[m_swrbank]->write(space, offset, data);
		}
		else if ((!m_lk19_ic37_paged_rom && (m_swrbank == 4 || m_swrbank == 5)) || (!m_lk18_ic41_paged_rom && (m_swrbank == 6 || m_swrbank == 7)))
		{
			m_region_swr->base()[offset + (m_swrbank << 14)] = data;
		}
	}
}

READ8_MEMBER(bbc_state::bbcm_hazel_r)
{
	uint8_t data;

	if (m_acccon_y)
	{
		data = m_ram->pointer()[offset + 0x9000];
	}
	else
	{
		data = m_region_mos->base()[offset];
	}

	return data;
}

WRITE8_MEMBER(bbc_state::bbcm_hazel_w)
{
	if (m_acccon_y)
	{
		m_ram->pointer()[offset + 0x9000] = data;
	}
}

READ8_MEMBER(bbc_state::bbcm_tube_r)
{
	uint8_t data = 0xfe;

	if (m_acccon_itu)
	{
		/* internal Tube */
		if (m_intube) data = m_intube->host_r(space, offset);
	}
	else
	{
		/* external Tube */
		if (m_extube) data = m_extube->host_r(space, offset);
	}

	return data;
}

WRITE8_MEMBER(bbc_state::bbcm_tube_w)
{
	if (m_acccon_itu)
	{
		/* internal Tube */
		if (m_intube) m_intube->host_w(space, offset, data);
	}
	else
	{
		/* external Tube */
		if (m_extube) m_extube->host_w(space, offset, data);
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
	if (m_latch->q3_r())
	{
		/* KBD IC1 4 bit addressable counter */
		/* KBD IC3 4 to 10 line decoder */
		/* keyboard not enabled so increment counter */
		m_column = (m_column + 1) % 16;

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


int bbc_state::bbc_keyboard(int data)
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


WRITE_LINE_MEMBER(bbc_state::snd_enable_w)
{
	if (!state && m_sn)
	{
		m_sn->write(m_via_system_porta);
	}
}

WRITE_LINE_MEMBER(bbc_state::speech_rsq_w)
{
	if (m_tms)
	{
		m_tms->rsq_w(state);
		if (!m_latch->q1_r() && m_latch->q2_r())
		{
			m_via_system_porta = m_tms->status_r();
		}
	}
}

WRITE_LINE_MEMBER(bbc_state::speech_wsq_w)
{
	/* Write select on the speech processor */
	if (m_tms)
	{
		m_tms->wsq_w(state);
		if (m_latch->q1_r() && !m_latch->q2_r())
		{
			m_tms->data_w(m_via_system_porta);
		}
	}
}

WRITE_LINE_MEMBER(bbc_state::kbd_enable_w)
{
	if (!state)
	{
		m_via_system_porta = bbc_keyboard(m_via_system_porta);
	}
}

WRITE_LINE_MEMBER(bbc_state::capslock_led_w)
{
	output().set_value("capslock_led", state);
}

WRITE_LINE_MEMBER(bbc_state::shiftlock_led_w)
{
	output().set_value("shiftlock_led", state);
}


void bbc_state::mc146818_set(address_space &space)
{
	/* if chip enabled */
	if (m_mc146818_ce)
	{
		/* if data select is set then access the data in the 146818 */
		if (m_latch->q2_r()) // DS
		{
			if (m_latch->q1_r()) // WR
			{
				m_via_system_porta = m_rtc->read(space, 1);
			}
			else
			{
				m_rtc->write(space, 1, m_via_system_porta);
			}
		}

		/* if address select is set then set the address in the 146818 */
		if (m_mc146818_as)
		{
			m_rtc->write(space, 0, m_via_system_porta);
		}
	}
}


READ8_MEMBER(bbc_state::via_system_porta_r)
{
	return m_via_system_porta;
}

WRITE8_MEMBER(bbc_state::via_system_porta_w)
{
	m_via_system_porta = data;

	/* Write enable to the sound generator */
	//if (!m_latch->q0_r() && m_sn)
	//{
	//  m_sn->write(m_via_system_porta);
	//}
	/* Keyboard write enable */
	if (!m_latch->q3_r())
	{
		m_via_system_porta = bbc_keyboard(m_via_system_porta);
	}

	if (m_rtc)
	{
		mc146818_set(space);
	}
}


READ8_MEMBER(bbc_state::via_system_portb_r)
{
	uint8_t data = 0xff;

	/* B/B+/Master only */
	if (m_analog)
	{
		data &= ~0x30;
		data |= m_analog->pb_r();
	}

	/* Master Compact only */
	if (m_i2cmem)
	{
		data &= ~0x30;
		data |= m_i2cmem->read_sda() << 4;
	}

	/* B/B+ only */
	if (m_tms)
	{
		// TODO: Fix speech, it's never worked
		//data &= ~0xc0;
		//data |= m_tms->intq_r() << 6;
		//data |= m_tms->readyq_r() << 7;
	}

	return data;
}

WRITE8_MEMBER(bbc_state::via_system_portb_w)
{
	m_latch->write_nibble_d3(space, 0, data);

	/* Master only */
	if (m_rtc)
	{
		/* set Chip Enable and Address Strobe lines */
		m_mc146818_ce = BIT(data, 6);
		m_mc146818_as = BIT(data, 7);
		mc146818_set(space);
	}

	/* Master Compact only */
	if (m_i2cmem)
	{
		m_i2cmem->write_sda(BIT(data, 4));
		m_i2cmem->write_scl(BIT(data, 5));
	}
}


WRITE_LINE_MEMBER(bbc_state::lpstb_w)
{
	m_via6522_0->write_cb2(state);
	if (!state)
		m_hd6845->assert_light_pen_input();
}

/**************************************
   BBC Joystick Support
**************************************/

int bbc_state::get_analogue_input(int channel_number)
{
	if (m_analog)
		return ((0xff - m_analog->ch_r(channel_number)) << 8);
	else
		return 0xff;
}

void bbc_state::upd7002_eoc(int data)
{
	m_via6522_0->write_cb1(data);
}

/***************************************
   BBC 2C199 Serial Interface Cassette
****************************************/


void bbc_state::mc6850_receive_clock(int new_clock)
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

TIMER_CALLBACK_MEMBER(bbc_state::tape_timer_cb)
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
				mc6850_receive_clock(0);
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
				mc6850_receive_clock(1);
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

WRITE_LINE_MEMBER(bbc_state::write_rxd)
{
	m_rxd_serial = state;
	update_acia_rxd();
}

void bbc_state::update_acia_rxd()
{
	m_acia->write_rxd(BIT(m_serproc_data, 6) ? m_rxd_serial : m_rxd_cass);
}


WRITE_LINE_MEMBER(bbc_state::write_dcd)
{
	m_dcd_serial = state;
	update_acia_dcd();
}

void bbc_state::update_acia_dcd()
{
	m_acia->write_dcd(BIT(m_serproc_data, 6) ? m_dcd_serial : m_dcd_cass);
}


WRITE_LINE_MEMBER(bbc_state::write_cts)
{
	m_cts_serial = state;
	update_acia_cts();
}

void bbc_state::update_acia_cts()
{
	m_acia->write_cts(BIT(m_serproc_data, 6) ? m_cts_serial : 0);
}


WRITE_LINE_MEMBER(bbc_state::write_rts)
{
	if (BIT(m_serproc_data, 6))
	{
		m_rs232->write_rts(state);
		m_cass_out_enabled = 0;
	}
	else
	{
		m_cass_out_enabled = state ? 0 : 1;
	}
}


WRITE_LINE_MEMBER(bbc_state::write_txd)
{
	if (BIT(m_serproc_data, 6))
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


/*
   Serial processor control
   x--- ---- - Motor OFF(0)/ON(1)
   -x-- ---- - Cassette(0)/RS243 input(1)
   --xx x--- - Receive baud rate generator control
   ---- -xxx - Transmit baud rate generator control
               These possible settings apply to both the receive
               and transmit baud generator control bits:
               000 - 16MHz / 13 /   1 - 19200 baud
               001 - 16MHz / 13 /  16 -  1200 baud
               010 - 16MHz / 13 /   4 -  4800 baud
               011 - 16MHz / 13 / 128 -   150 baud
               100 - 16MHz / 13 /   2 -  9600 baud
               101 - 16MHz / 13 /  64 -   300 baud
               110 - 16MHz / 13 /   8 -  2400 baud
               110 - 16MHz / 13 / 256 -    75 baud
*/

WRITE8_MEMBER(bbc_state::serial_ula_w)
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

	if (BIT(m_serproc_data, 6))
		m_acia->write_rxc(state);
}


/**************************************
   1MHz Bus interrupts
***************************************/


WRITE_LINE_MEMBER(bbc_state::bus_nmi_w)
{
	m_bus_nmi = state;
	update_nmi();
}


/**************************************
   i8271 disc control function
***************************************/


WRITE_LINE_MEMBER(bbc_state::motor_w)
{
	floppy_image_device *floppy0 = m_i8271->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = m_i8271->subdevice<floppy_connector>("1")->get_device();

	if (floppy0) floppy0->mon_w(!state);
	if (floppy1) floppy1->mon_w(!state);

	m_i8271->ready_w(!state);
}

WRITE_LINE_MEMBER(bbc_state::side_w)
{
	floppy_image_device *floppy0 = m_i8271->subdevice<floppy_connector>("0")->get_device();
	floppy_image_device *floppy1 = m_i8271->subdevice<floppy_connector>("1")->get_device();

	if (floppy0) floppy0->ss_w(state);
	if (floppy1) floppy1->ss_w(state);
}


/**************************************
   WD1770 disc control function
***************************************/


void bbc_state::update_nmi()
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
	update_nmi();
}

WRITE_LINE_MEMBER(bbc_state::fdc_drq_w)
{
	m_fdc_drq = state;
	update_nmi();
}

/*
   B+ drive control:

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

WRITE8_MEMBER(bbc_state::bbcbp_drive_control_w)
{
	floppy_image_device *floppy = nullptr;

	// bit 0, 1: drive select
	if (BIT(data, 0)) floppy = m_wd1770->subdevice<floppy_connector>("0")->get_device();
	if (BIT(data, 1)) floppy = m_wd1770->subdevice<floppy_connector>("1")->get_device();
	m_wd1770->set_floppy(floppy);

	// bit 2: side select
	if (floppy)
		floppy->ss_w(BIT(data, 2));

	// bit 3: density
	m_wd1770->dden_w(BIT(data, 3));

	// bit 4: interrupt enable (S5 wire link not fitted)

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

WRITE8_MEMBER(bbc_state::bbcm_drive_control_w)
{
	floppy_image_device *floppy = nullptr;

	// bit 0, 1, 3: drive select
	if (BIT(data, 0)) floppy = m_wd1770->subdevice<floppy_connector>("0")->get_device();
	if (BIT(data, 1)) floppy = m_wd1770->subdevice<floppy_connector>("1")->get_device();
	m_wd1770->set_floppy(floppy);

	// bit 4: side select
	if (floppy)
		floppy->ss_w(BIT(data, 4));

	// bit 5: density
	m_wd1770->dden_w(BIT(data, 5));

	// bit 2: reset
	if (!BIT(data, 2)) m_wd1770->soft_reset();
}

WRITE8_MEMBER(bbc_state::bbcmc_drive_control_w)
{
	floppy_image_device *floppy = nullptr;

	// bit 0, 1, 3: drive select
	if (BIT(data, 0)) floppy = m_wd1772->subdevice<floppy_connector>("0")->get_device();
	if (BIT(data, 1)) floppy = m_wd1772->subdevice<floppy_connector>("1")->get_device();
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
   BBC cartslot loading functions
***************************************/

image_init_result bbc_state::load_cart(device_image_interface &image, generic_slot_device *slot)
{
	if (!image.loaded_through_softlist())
	{
		uint32_t filesize = image.length();

		if (filesize > 0x8000)
		{
			image.seterror(IMAGE_ERROR_UNSPECIFIED, "Cartridge socket accepts 16K/32K only");
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

		slot->rom_alloc(0x8000, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
		memcpy(slot->get_rom_base() + 0x0000, image.get_software_region("lorom"), size_lo);
		memcpy(slot->get_rom_base() + 0x4000, image.get_software_region("uprom"), size_hi);
	}

	return image_init_result::PASS;
}


/**************************************
   Machine Initialisation functions
***************************************/

void bbc_state::init_bbc()
{
	m_rxd_cass = 0;
	m_nr_high_tones = 0;
	m_serproc_data = 0;
	m_cass_out_enabled = 0;
	m_tape_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bbc_state::tape_timer_cb), this));
	m_reset_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(bbc_state::reset_timer_cb), this));

	/* vertical sync pulse from video circuit */
	m_via6522_0->write_ca1(1);

	/* light pen strobe detect (not emulated) */
	m_via6522_0->write_cb2(1);

	m_monitortype = monitor_type_t::COLOUR;
	m_swramtype = 0;
}

void bbc_state::init_ltmp()
{
	init_bbc();

	/* LTM machines used a 9" Hantarex MT3000 green monitor */
	m_monitortype = monitor_type_t::GREEN;
}

void bbc_state::init_bbcm()
{
	std::string region_tag;

	init_bbc();

	/* set links if ROM present, disabling RAM */
	if (m_rom[4] && memregion(region_tag.assign(m_rom[4]->tag()).append(BBC_ROM_REGION_TAG).c_str()))
	{
		/* link for ROM in slots 4 and 5 */
		m_lk19_ic37_paged_rom = true;
	}
	else
	{
		m_lk18_ic41_paged_rom = false;
	}
	if (m_rom[6] && memregion(region_tag.assign(m_rom[6]->tag()).append(BBC_ROM_REGION_TAG).c_str()))
	{
		/* link for ROM in slots 6 and 7 */
		m_lk18_ic41_paged_rom = true;
	}
	else
	{
		m_lk18_ic41_paged_rom = false;
	}
}

void bbc_state::init_cfa()
{
	init_bbc();

	/* link for ROM in slots 4 and 5 */
	m_lk19_ic37_paged_rom = true;
	/* link for ROM in slots 6 and 7 */
	m_lk18_ic41_paged_rom = true;

	m_monitortype = monitor_type_t::GREEN;
}


/**************************************
   Helpers for ROM management
***************************************/

std::string bbc_state::get_rom_name(uint8_t* header)
{
	std::string title = "";
	/* check for valid copyright */
	uint8_t offset = header[7];
	if (header[offset + 0] == 0 && header[offset + 1] == '(' && header[offset + 2] == 'C' && header[offset + 3] == ')')
	{
		/* extract ROM title from header*/
		for (int pos = 9; pos < header[7]; pos++)
			title.append(1, header[pos] == 0x00 ? 0x20 : header[pos]);
	}
	return title;
}

void bbc_state::insert_device_rom(memory_region *rom)
{
	std::string region_tag;

	if (rom == nullptr) return;

	/* check whether ROM already present */
	for (int bank = 15; bank >= 0; bank--)
	{
		/* compare first 1K of bank with what we want to insert */
		if (!memcmp(rom->base(), m_region_swr->base() + (bank * 0x4000), 0x400))
		{
			osd_printf_verbose("Found '%s' in romslot%d\n", get_rom_name(rom->base()).c_str(), bank);
			return;
		}
	}

	/* iterate over romslots until an empty socket is found */
	for (int bank = 15; bank >= 0; bank--)
	{
		/* if bank has socket and is empty */
		if (m_rom[bank] && !memregion(region_tag.assign(m_rom[bank]->tag()).append(BBC_ROM_REGION_TAG).c_str()))
		{
			uint8_t *swr = m_region_swr->base() + (bank * 0x4000);
			switch (rom->bytes())
			{
			case 0x8000:
				/* 32K (or 2x16K) ROM, check whether ROM exists in this and next bank */
				if (swr[0x0006] == 0xff && swr[0x4006] == 0xff)
				{
					memcpy(m_region_swr->base() + (bank * 0x4000), rom->base(), rom->bytes());
					osd_printf_verbose("Inserting '%s' into romslot%d\n", get_rom_name(rom->base() + 0x4000).c_str(), bank + 1);
					osd_printf_verbose("Inserting '%s' into romslot%d\n", get_rom_name(rom->base()).c_str(), bank);
					return;
				}
				break;
			case 0x4000:
			case 0x2000:
				/* 16/8K ROM, check whether ROM exists in this bank */
				if (swr[0x0006] == 0xff)
				{
					memcpy(m_region_swr->base() + (bank * 0x4000), rom->base(), rom->bytes());
					osd_printf_verbose("Inserting '%s' into romslot%d\n", get_rom_name(rom->base()).c_str(), bank);
					return;
				}
				break;
			}
		}
	}

	/* unable to insert ROM */
	fatalerror("Unable to insert '%s'. Add a sideways ROM board for more sockets.\n", get_rom_name(rom->base()).c_str());
}

void bbc_state::setup_device_roms()
{
	std::string region_tag;
	memory_region *rom_region;
	device_t* exp_device;
	device_t* ext_device;

	/* insert ROM for FDC devices (BBC Model B only), always place into romslot 12 */
	if (m_fdc && (exp_device = dynamic_cast<device_t*>(m_fdc->get_card_device())))
	{
		if (exp_device->memregion("dfs_rom"))
		{
			memcpy(m_region_swr->base() + 0x30000, exp_device->memregion("dfs_rom")->base(), exp_device->memregion("dfs_rom")->bytes());
		}
	}

	/* configure romslots */
	for (int i = 0; i < 16; i++)
	{
		if (m_rom[i] && (rom_region = memregion(region_tag.assign(m_rom[i]->tag()).append(BBC_ROM_REGION_TAG).c_str())))
		{
			if (m_rom[i]->get_rom_size())
				memcpy(m_region_swr->base() + (i * 0x4000), rom_region->base(), std::min((int32_t)m_rom[i]->get_slot_size(), (int32_t)m_rom[i]->get_rom_size()));
			else
				memset(m_region_swr->base() + (i * 0x4000), 0, 0x4000);
		}
	}

	/* configure cartslots */
	for (int i = 0; i < 2; i++)
	{
		if (m_cart[i] && (rom_region = memregion(region_tag.assign(m_cart[i]->tag()).append(GENERIC_ROM_REGION_TAG).c_str())))
		{
			memcpy(m_region_swr->base() + (i * 0x8000), rom_region->base(), rom_region->bytes());
		}
	}

	/* insert ROM(s) for Expansion port devices (Compact only), with additional Mertec slots */
	if (m_exp && (ext_device = dynamic_cast<device_t*>(m_exp->get_card_device())))
	{
		/* only the Mertec device has ext_rom region and should be placed in romslots 0,1 */
		if (ext_device->memregion("ext_rom"))
		{
			memcpy(m_region_swr->base(), ext_device->memregion("ext_rom")->base(), ext_device->memregion("ext_rom")->bytes());
		}

		bbc_analogue_slot_device* analogue_port = ext_device->subdevice<bbc_analogue_slot_device>("analogue");
		if (analogue_port && (exp_device = dynamic_cast<device_t*>(analogue_port->get_card_device())))
		{
			insert_device_rom(exp_device->memregion("exp_rom"));
		}

		bbc_userport_slot_device* user_port = ext_device->subdevice<bbc_userport_slot_device>("userport");
		if (user_port && (exp_device = dynamic_cast<device_t*>(user_port->get_card_device())))
		{
			insert_device_rom(exp_device->memregion("exp_rom"));
		}

		bbc_1mhzbus_slot_device* exp_port = ext_device->subdevice<bbc_1mhzbus_slot_device>("2mhzbus");
		while (exp_port != nullptr)
		{
			if ((exp_device = dynamic_cast<device_t*>(exp_port->get_card_device())))
			{
				insert_device_rom(exp_device->memregion("exp_rom"));
				exp_port = exp_device->subdevice<bbc_1mhzbus_slot_device>("1mhzbus");
			}
			else
			{
				exp_port = nullptr;
			}
		}
	}

	/* insert ROM(s) for 1MHz bus devices, with pass-through */
	if (m_1mhzbus)
	{
		bbc_1mhzbus_slot_device* exp_port = m_1mhzbus;
		while (exp_port != nullptr)
		{
			if ((exp_device = dynamic_cast<device_t*>(exp_port->get_card_device())))
			{
				insert_device_rom(exp_device->memregion("exp_rom"));
				exp_port = exp_device->subdevice<bbc_1mhzbus_slot_device>("1mhzbus");
			}
			else
			{
				exp_port = nullptr;
			}
		}
	}

	/* insert ROM(s) for Tube devices */
	if (m_tube && (exp_device = dynamic_cast<device_t*>(m_tube->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}
	if (m_intube && (exp_device = dynamic_cast<device_t*>(m_intube->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}
	if (m_extube && (exp_device = dynamic_cast<device_t*>(m_extube->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}

	/* insert ROM(s) for Userport devices */
	if (m_userport && (exp_device = dynamic_cast<device_t*>(m_userport->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}

	/* insert ROM(s) for Analogue port devices */
	if (m_analog && (exp_device = dynamic_cast<device_t*>(m_analog->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}

	/* list all inserted ROMs */
	for (int i = 15; i >= 0; i--)
	{
		osd_printf_info("ROM %X : %s\n", i, get_rom_name(m_region_swr->base() + (i * 0x4000)).c_str());
	}
}


/**************************************
   Machine start/reset
***************************************/

void bbc_state::machine_start()
{
	setup_device_roms();
}

void bbc_state::machine_reset()
{
	m_swramtype = (m_bbcconfig.read_safe(0) & 0x38) >> 3;
	/* bank 1 regular lower RAM from 0000 to 3fff */
	m_bank1->set_base(m_ram->pointer());
	/* bank 3 regular higher RAM from 4000 to 7fff */
	if (m_ram->size() == 16 * 1024)
	{
		/* 16K just repeat the lower 16K*/
		m_bank3->set_base(m_ram->pointer());
	}
	else
	{
		/* 32K */
		m_bank3->set_base(m_ram->pointer() + 0x4000);
	}

	/* install econet hardware */
	if (m_bbcconfig.read_safe(0) & 0x04)
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xfea0, 0xfebf, read8sm_delegate(FUNC(mc6854_device::read), m_adlc.target()), write8sm_delegate(FUNC(mc6854_device::write), m_adlc.target()));
	else
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xfea0, 0xfebf, read8_delegate(FUNC(bbc_state::bbc_fe_r), this));

	/* power-on reset timer, should produce "boo...beep" startup sound before sn76496 is initialised */
	//m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	//m_reset_timer->adjust(attotime::from_msec(138.6));
}


void bbcbp_state::machine_start()
{
	setup_device_roms();
}

void bbcbp_state::machine_reset()
{
	/* bank 1 regular lower RAM from 0000 to 2fff */
	m_bank1->set_base(m_ram->pointer());
	/* bank 2 screen/shadow RAM from 3000 to 7fff */
	m_bank2->set_base(m_ram->pointer() + 0x3000);
}


void bbcm_state::machine_start()
{
	setup_device_roms();

	output().set_value("power_led", 0);
}

void bbcm_state::machine_reset()
{
	/* bank 1 regular lower RAM from 0000 to 2fff */
	m_bank1->set_base(m_ram->pointer());
	/* bank 2 screen/shadow RAM from 3000 to 7fff */
	m_bank2->set_base(m_ram->pointer() + 0x3000);

	m_bankdev->set_bank(0);
}
