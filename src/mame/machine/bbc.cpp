// license:BSD-3-Clause
// copyright-holders:Gordon Jefferyes, Nigel Barnes
/******************************************************************************

    BBC Model B

******************************************************************************/

#include <cctype>
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

uint8_t bbc_state::bbc_ram_r(offs_t offset)
{
	if (m_internal && m_internal->overrides_ram())
		return m_internal->ram_r(offset);
	else
		return m_ram->pointer()[offset & m_ram->mask()];
}

void bbc_state::bbc_ram_w(offs_t offset, uint8_t data)
{
	if (m_internal && m_internal->overrides_ram())
		m_internal->ram_w(offset, data);
	else
		m_ram->pointer()[offset & m_ram->mask()] = data;
}

uint8_t bbc_state::bbc_romsel_r(offs_t offset)
{
	if (m_internal && m_internal->overrides_rom())
		return m_internal->romsel_r(offset);
	else
		return 0xfe;
}

void bbc_state::bbc_romsel_w(offs_t offset, uint8_t data)
{
	/* no sideways expansion board fitted so address only the 4 on board ROM sockets */
	m_romsel = data & 0x03;

	/* pass romsel to internal expansion board */
	if (m_internal && m_internal->overrides_rom())
		m_internal->romsel_w(offset, data);
}

uint8_t bbc_state::bbc_paged_r(offs_t offset)
{
	uint8_t data;

	if (m_internal && m_internal->overrides_rom())
	{
		data = m_internal->paged_r(offset);
	}
	else
	{
		if (m_rom[m_romsel] && m_rom[m_romsel]->present())
		{
			data = m_rom[m_romsel]->read(offset);
		}
		else
		{
			data = m_region_swr->base()[offset + (m_romsel << 14)];
		}
	}

	return data;
}

void bbc_state::bbc_paged_w(offs_t offset, uint8_t data)
{
	if (m_internal && m_internal->overrides_rom())
	{
		m_internal->paged_w(offset, data);
	}
	else
	{
		if (m_rom[m_romsel])
		{
			m_rom[m_romsel]->write(offset, data);
		}
	}
}

uint8_t bbc_state::bbc_mos_r(offs_t offset)
{
	if (m_internal && m_internal->overrides_mos())
		return m_internal->mos_r(offset);
	else
		return m_region_mos->base()[offset];
}

void bbc_state::bbc_mos_w(offs_t offset, uint8_t data)
{
	if (m_internal && m_internal->overrides_mos())
		m_internal->mos_w(offset, data);
}

uint8_t bbc_state::bbc_fred_r(offs_t offset)
{
	uint8_t data = 0xff;

	data &= m_1mhzbus->fred_r(offset);

	if (m_cart[0])
		data &= m_cart[0]->read(offset, 1, 0, m_romsel & 0x01, 0, 0);
	if (m_cart[1])
		data &= m_cart[1]->read(offset, 1, 0, m_romsel & 0x01, 0, 0);

	return data;
}

void bbc_state::bbc_fred_w(offs_t offset, uint8_t data)
{
	m_1mhzbus->fred_w(offset, data);

	if (m_cart[0])
		m_cart[0]->write(offset, data, 1, 0, m_romsel & 0x01, 0, 0);
	if (m_cart[1])
		m_cart[1]->write(offset, data, 1, 0, m_romsel & 0x01, 0, 0);
}

uint8_t bbc_state::bbc_jim_r(offs_t offset)
{
	uint8_t data = 0xff;

	data &= m_1mhzbus->jim_r(offset);

	if(m_cart[0])
		data &= m_cart[0]->read(offset, 0, 1, m_romsel & 0x01, 0, 0);
	if (m_cart[1])
		data &= m_cart[1]->read(offset, 0, 1, m_romsel & 0x01, 0, 0);

	return data;
}

void bbc_state::bbc_jim_w(offs_t offset, uint8_t data)
{
	m_1mhzbus->jim_w(offset, data);

	if (m_cart[0])
		m_cart[0]->write(offset, data, 0, 1, m_romsel & 0x01, 0, 0);
	if (m_cart[1])
		m_cart[1]->write(offset, data, 0, 1, m_romsel & 0x01, 0, 0);
}


/****************************************
  BBC Model B+ memory handling functions
****************************************/

uint8_t bbc_state::bbcbp_fetch_r(offs_t offset)
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

void bbc_state::bbcbp_romsel_w(offs_t offset, uint8_t data)
{
	/* the BBC Model B+ addresses all 16 ROM sockets and extra 12K of RAM at 0x8000 and 20K of shadow RAM at 0x3000 */
	switch (offset & 0x07)
	{
	case 0x00:
		m_paged_ram = BIT(data, 7);

		m_romsel = data & 0x0f;
		break;

	case 0x04:
		/* the video display should now use this flag to display the shadow RAM memory */
		m_vdusel = BIT(data, 7);
		setvideoshadow(m_vdusel);
		break;
	}

	/* pass romsel to internal expansion board */
	if (m_internal && m_internal->overrides_rom())
		m_internal->romsel_w(offset, data);
}

uint8_t bbc_state::bbcbp_paged_r(offs_t offset)
{
	uint8_t data;
	std::string region_tag;

	if (m_paged_ram && offset < 0x3000)
	{
		data = m_ram->pointer()[offset + 0x8000];
	}
	else
	{
		if (m_internal && m_internal->overrides_rom())
		{
			data = m_internal->paged_r(offset);
		}
		else
		{
			/* 32K sockets */
			if (m_rom[m_romsel & 0x0e] && m_rom[m_romsel & 0x0e]->present())
			{
				data = m_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
			}
			else
			{
				data = m_region_swr->base()[offset + (m_romsel << 14)];
			}
		}
	}

	return data;
}

void bbc_state::bbcbp_paged_w(offs_t offset, uint8_t data)
{
	if (m_paged_ram && offset < 0x3000)
	{
		m_ram->pointer()[offset + 0x8000] = data;
	}
	else
	{
		if (m_internal && m_internal->overrides_rom())
		{
			m_internal->paged_w(offset, data);
		}
		else
		{
			/* 32K sockets */
			if (m_rom[m_romsel & 0x0e])
			{
				m_rom[m_romsel & 0x0e]->write(offset | (m_romsel & 0x01) << 14, data);
			}
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

uint8_t bbc_state::bbcm_fetch_r(offs_t offset)
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

uint8_t bbc_state::bbcm_acccon_r()
{
	return m_acccon;
}

void bbc_state::bbcm_acccon_w(uint8_t data)
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

void bbc_state::bbcm_romsel_w(offs_t offset, uint8_t data)
{
	m_paged_ram = BIT(data, 7);
	m_romsel = data & 0x0f;

	/* pass romsel to internal expansion board */
	if (m_internal && m_internal->overrides_rom())
		m_internal->romsel_w(offset, data);
}

uint8_t bbc_state::bbcm_paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_paged_ram && offset < 0x1000)
	{
		data = m_ram->pointer()[offset + 0x8000];
	}
	else
	{
		switch (m_romsel)
		{
		case 0: case 1:
			if (m_cart[0] && m_cart[0]->present())
			{
				data = m_cart[0]->read(offset, 0, 0, m_romsel & 0x01, 1, 0);
			}
			else
			{
				data = bus_video_data();
			}
			break;
		case 2: case 3:
			if (m_cart[1] && m_cart[1]->present())
			{
				data = m_cart[1]->read(offset, 0, 0, m_romsel & 0x01, 1, 0);
			}
			else
			{
				data = bus_video_data();
			}
			break;
		default:
			if (m_internal && m_internal->overrides_rom())
			{
				data = m_internal->paged_r(offset);
			}
			else
			{
				switch (m_romsel)
				{
				case 4: case 5: case 6: case 7:
					/* 32K sockets */
					if (m_rom[m_romsel & 0x0e] && m_rom[m_romsel & 0x0e]->present())
					{
						data = m_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
					}
					else
					{
						data = m_region_swr->base()[offset + (m_romsel << 14)];
					}
					break;
				default:
					/* 16K sockets */
					if (m_rom[m_romsel] && m_rom[m_romsel]->present())
					{
						data = m_rom[m_romsel]->read(offset);
					}
					else
					{
						data = m_region_swr->base()[offset + (m_romsel << 14)];
					}
					break;
				}
			}
			break;
		}
	}

	return data;
}

void bbc_state::bbcm_paged_w(offs_t offset, uint8_t data)
{
	if (m_paged_ram && offset < 0x1000)
	{
		m_ram->pointer()[offset + 0x8000] = data;
	}
	else
	{
		switch (m_romsel)
		{
		case 0: case 1:
			if (m_cart[0])
			{
				m_cart[0]->write(offset, data, 0, 0, m_romsel & 0x01, 1, 0);
			}
			break;
		case 2: case 3:
			if (m_cart[1])
			{
				m_cart[1]->write(offset, data, 0, 0, m_romsel & 0x01, 1, 0);
			}
			break;
		default:
			if (m_internal && m_internal->overrides_rom())
			{
				m_internal->paged_w(offset, data);
			}
			else
			{
				switch (m_romsel)
				{
				case 4: case 5: case 6: case 7:
					/* 32K sockets */
					if (m_rom[m_romsel & 0x0e])
					{
						m_rom[m_romsel & 0x0e]->write(offset | (m_romsel & 0x01) << 14, data);
					}
					break;
				default:
					/* 16K sockets */
					if (m_rom[m_romsel])
					{
						m_rom[m_romsel]->write(offset, data);
					}
					break;
				}
			}
			break;
		}
	}
}

uint8_t bbc_state::bbcmc_paged_r(offs_t offset)
{
	uint8_t data = 0xff;

	if (m_paged_ram && offset < 0x1000)
	{
		data = m_ram->pointer()[offset + 0x8000];
	}
	else
	{
		if (m_internal && m_internal->overrides_rom())
		{
			data = m_internal->paged_r(offset);
		}
		else
		{
			switch (m_romsel)
			{
			case 0: case 1: case 4: case 5: case 6: case 7:
				/* 32K sockets */
				if (m_rom[m_romsel & 0x0e] && m_rom[m_romsel & 0x0e]->present())
				{
					data = m_rom[m_romsel & 0x0e]->read(offset | (m_romsel & 0x01) << 14);
				}
				else
				{
					data = m_region_swr->base()[offset + (m_romsel << 14)];
				}
				break;
			default:
				/* 16K sockets */
				if (m_rom[m_romsel] && m_rom[m_romsel]->present())
				{
					data = m_rom[m_romsel]->read(offset);
				}
				else
				{
					data = m_region_swr->base()[offset + (m_romsel << 14)];
				}
				break;
			}
		}
	}

	return data;
}

void bbc_state::bbcmc_paged_w(offs_t offset, uint8_t data)
{
	if (m_paged_ram && offset < 0x1000)
	{
		m_ram->pointer()[offset + 0x8000] = data;
	}
	else
	{
		if (m_internal && m_internal->overrides_rom())
		{
			m_internal->paged_w(offset, data);
		}
		else
		{
			switch (m_romsel)
			{
			case 0: case 1: case 4: case 5: case 6: case 7:
				/* 32K sockets */
				if (m_rom[m_romsel & 0x0e])
				{
					m_rom[m_romsel & 0x0e]->write(offset | (m_romsel & 0x01) << 14, data);
				}
				break;
			default:
				/* 16K sockets */
				if (m_rom[m_romsel])
				{
					m_rom[m_romsel]->write(offset, data);
				}
				break;
			}
		}
	}
}

uint8_t bbc_state::bbcm_hazel_r(offs_t offset)
{
	uint8_t data;

	if (m_acccon_y)
	{
		data = m_ram->pointer()[offset + 0x9000];
	}
	else
	{
		if (m_internal && m_internal->overrides_mos())
			data = m_internal->mos_r(offset);
		else
			data = m_region_mos->base()[offset];
	}

	return data;
}

void bbc_state::bbcm_hazel_w(offs_t offset, uint8_t data)
{
	if (m_acccon_y)
	{
		m_ram->pointer()[offset + 0x9000] = data;
	}
}

uint8_t bbc_state::bbcm_tube_r(offs_t offset)
{
	uint8_t data = 0xfe;

	if (m_acccon_itu)
	{
		/* internal Tube */
		if (m_intube) data = m_intube->host_r(offset);
	}
	else
	{
		/* external Tube */
		if (m_extube) data = m_extube->host_r(offset);
	}

	return data;
}

void bbc_state::bbcm_tube_w(offs_t offset, uint8_t data)
{
	if (m_acccon_itu)
	{
		/* internal Tube */
		if (m_intube) m_intube->host_w(offset, data);
	}
	else
	{
		/* external Tube */
		if (m_extube) m_extube->host_w(offset, data);
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
}


int bbc_state::bbc_keyboard(int data)
{
	int bit;
	int row;
	int res;

	m_column = data & 0x0f;
	row = (data>>4) & 0x07;

	bit = 0;

	res = m_keyboard[m_column]->read();

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


void bbc_state::mc146818_set()
{
	/* if chip enabled */
	if (m_mc146818_ce)
	{
		/* if data select is set then access the data in the 146818 */
		if (m_latch->q2_r()) // DS
		{
			if (m_latch->q1_r()) // WR
			{
				m_via_system_porta = m_rtc->read(1);
			}
			else
			{
				m_rtc->write(1, m_via_system_porta);
			}
		}

		/* if address select is set then set the address in the 146818 */
		if (m_mc146818_as)
		{
			m_rtc->write(0, m_via_system_porta);
		}
	}
}


uint8_t bbc_state::via_system_porta_r()
{
	return m_via_system_porta;
}

void bbc_state::via_system_porta_w(uint8_t data)
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
		mc146818_set();
	}
}


uint8_t bbc_state::via_system_portb_r()
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

void bbc_state::via_system_portb_w(uint8_t data)
{
	m_latch->write_nibble_d3(data);

	/* Master only */
	if (m_rtc)
	{
		/* set Chip Enable and Address Strobe lines */
		m_mc146818_ce = BIT(data, 6);
		m_mc146818_as = BIT(data, 7);
		mc146818_set();
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
	m_motor_led = !motor_state;
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

void bbc_state::serial_ula_w(uint8_t data)
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

void bbc_state::bbcbp_drive_control_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	// bit 0, 1: drive select
	if (BIT(data, 0)) floppy = m_wd_fdc->subdevice<floppy_connector>("0")->get_device();
	if (BIT(data, 1)) floppy = m_wd_fdc->subdevice<floppy_connector>("1")->get_device();
	m_wd_fdc->set_floppy(floppy);

	// bit 2: side select
	if (floppy)
		floppy->ss_w(BIT(data, 2));

	// bit 3: density
	m_wd_fdc->dden_w(BIT(data, 3));

	// bit 4: interrupt enable (S5 wire link not fitted)

	// bit 5: reset
	m_wd_fdc->mr_w(BIT(data, 5));
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

void bbc_state::bbcm_drive_control_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	// bit 0, 1, 3: drive select
	if (BIT(data, 0)) floppy = m_wd_fdc->subdevice<floppy_connector>("0")->get_device();
	if (BIT(data, 1)) floppy = m_wd_fdc->subdevice<floppy_connector>("1")->get_device();
	m_wd_fdc->set_floppy(floppy);

	// bit 4: side select
	if (floppy)
		floppy->ss_w(BIT(data, 4));

	// bit 5: density
	m_wd_fdc->dden_w(BIT(data, 5));

	// bit 2: reset
	m_wd_fdc->mr_w(BIT(data, 2));
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
	m_tape_timer = timer_alloc(FUNC(bbc_state::tape_timer_cb), this);
	m_reset_timer = timer_alloc(FUNC(bbc_state::reset_timer_cb), this);

	/* vertical sync pulse from video circuit */
	m_via6522_0->write_ca1(1);

	/* light pen strobe detect (not emulated) */
	m_via6522_0->write_cb2(1);

	update_palette(monitor_type::COLOUR);
}

void bbc_state::init_ltmp()
{
	init_bbc();

	/* LTM machines used a 9" Hantarex MT3000 green monitor */
	update_palette(monitor_type::GREEN);
}

void bbc_state::init_cfa()
{
	init_bbc();

	update_palette(monitor_type::GREEN);
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
			osd_printf_verbose("Found '%s' in romslot%d\n", get_rom_name(rom->base()), bank);
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
					osd_printf_verbose("Inserting '%s' into romslot%d\n", get_rom_name(rom->base() + 0x4000), bank + 1);
					osd_printf_verbose("Inserting '%s' into romslot%d\n", get_rom_name(rom->base()), bank);
					return;
				}
				break;
			case 0x4000:
			case 0x2000:
				/* 16/8K ROM, check whether ROM exists in this bank */
				if (swr[0x0006] == 0xff)
				{
					memcpy(m_region_swr->base() + (bank * 0x4000), rom->base(), rom->bytes());
					osd_printf_verbose("Inserting '%s' into romslot%d\n", get_rom_name(rom->base()), bank);
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

	/* insert ROM(s) for internal expansion boards */
	if (m_internal && (exp_device = dynamic_cast<device_t*>(m_internal->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}

	/* insert ROM for FDC devices (BBC Model B only), always place into romslot 0 */
	if (m_fdc && m_fdc->insert_rom() && (exp_device = dynamic_cast<device_t*>(m_fdc->get_card_device())))
	{
		if (exp_device->memregion("dfs_rom"))
		{
			memcpy(m_region_swr->base(), exp_device->memregion("dfs_rom")->base(), exp_device->memregion("dfs_rom")->bytes());
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
		if (m_cart[i] && (rom_region = memregion(region_tag.assign(m_cart[i]->tag()).append(ELECTRON_CART_ROM_REGION_TAG).c_str())))
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
	if (m_tube && m_tube->insert_rom() && (exp_device = dynamic_cast<device_t*>(m_tube->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}
	if (m_intube && m_intube->insert_rom() && (exp_device = dynamic_cast<device_t*>(m_intube->get_card_device())))
	{
		insert_device_rom(exp_device->memregion("exp_rom"));
	}
	if (m_extube && m_extube->insert_rom() && (exp_device = dynamic_cast<device_t*>(m_extube->get_card_device())))
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
		osd_printf_verbose("ROM %X : %s\n", i, get_rom_name(m_region_swr->base() + (i * 0x4000)));
	}
}


/**************************************
   Machine start/reset
***************************************/

void bbc_state::machine_start()
{
	setup_device_roms();

	m_motor_led.resolve();

	m_romsel = 0;
	m_adlc_irq = 0;
	m_bus_nmi = 0;
	m_fdc_irq = 0;
	m_fdc_drq = 0;
}

void bbc_state::machine_reset()
{
	/* install econet hardware */
	if (m_bbcconfig.read_safe(0) & 0x04)
		m_maincpu->space(AS_PROGRAM).install_readwrite_handler(0xfea0, 0xfebf, read8sm_delegate(*m_adlc, FUNC(mc6854_device::read)), write8sm_delegate(*m_adlc, FUNC(mc6854_device::write)));
	else
		m_maincpu->space(AS_PROGRAM).install_read_handler(0xfea0, 0xfebf, read8smo_delegate(*this, FUNC(bbc_state::bbc_fe_r)));

	/* power-on reset timer, should produce "boo...beep" startup sound before sn76496 is initialised */
	//m_maincpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	//m_reset_timer->adjust(attotime::from_msec(138.6));
}


void bbcbp_state::machine_start()
{
	bbc_state::machine_start();
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
	bbc_state::machine_start();

	m_power_led.resolve();

	m_power_led = 0;
}

void bbcm_state::machine_reset()
{
	/* bank 1 regular lower RAM from 0000 to 2fff */
	m_bank1->set_base(m_ram->pointer());
	/* bank 2 screen/shadow RAM from 3000 to 7fff */
	m_bank2->set_base(m_ram->pointer() + 0x3000);

	m_bankdev->set_bank(0);
}
