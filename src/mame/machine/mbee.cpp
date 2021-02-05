// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    microbee.cpp

    machine driver
    Originally written by Juergen Buchmueller, Jan 2000

    Rewritten by Robbbert (see notes in driver file).

****************************************************************************/

#include "emu.h"
#include "includes/mbee.h"
#include "machine/z80bin.h"


/***********************************************************

    PIO

************************************************************/

WRITE_LINE_MEMBER( mbee_state::pio_ardy )
{
	m_centronics->write_strobe((state) ? 0 : 1);
}

void mbee_state::pio_port_b_w(uint8_t data)
{
/*  PIO port B - d5..d2 not emulated
    d7 interrupt from network or rtc or vsync or not used (see config switch)
    d6 speaker
    d5 rs232 output (1=mark)
    d4 rs232 input (0=mark)
    d3 rs232 CTS (0=clear to send)
    d2 rs232 clock or DTR
    d1 cass out and (on new keyboard) keyboard irq
    d0 cass in */

	m_cassette->output(BIT(data, 1) ? -1.0 : +1.0);
	m_speaker->level_w(BIT(data, 6));
}

uint8_t mbee_state::pio_port_b_r()
{
	uint8_t data = 0;

	if (m_cassette->input() > 0.03)
		data |= 1;

	data |= 8; // CTS held high via resistor. If low, the disk-based models think a mouse is plugged in.

	switch (m_io_config->read() & 0xc0)
	{
		case 0x00:
			data |= (uint8_t)m_b7_vs << 7;
			break;
		case 0x40:
			data |= (uint8_t)m_b7_rtc << 7;
			break;
		case 0x80:
			data |= 0x80;
			break;
	}
	data |= (uint8_t)m_b2 << 1; // key pressed on new keyboard

	return data;
}

/*************************************************************************************

    Floppy Disk

    The callback is quite simple, no interrupts are used.
    If either IRQ or DRQ activate, they set bit 7 of inport 0x48.

*************************************************************************************/

WRITE_LINE_MEMBER( mbee_state::fdc_intrq_w )
{
	m_fdc_rq = (m_fdc_rq & 2) | state;
}

WRITE_LINE_MEMBER( mbee_state::fdc_drq_w )
{
	m_fdc_rq = (m_fdc_rq & 1) | (state << 1);
}

uint8_t mbee_state::fdc_status_r()
{
/*  d7 indicate if IRQ or DRQ is occurring (1=happening)
    d6..d0 not used */

	return m_fdc_rq ? 0xff : 0x7f;
}

void mbee_state::fdc_motor_w(uint8_t data)
{
/*  d7..d4 not used
    d3 density (1=MFM)
    d2 side (1=side 1)
    d1..d0 drive select (0 to 3) */

	floppy_image_device *floppy = nullptr;
	floppy = m_floppy0->get_device();
	floppy->mon_w(0); // motor on always
	floppy = m_floppy1->get_device();
	floppy->mon_w(0); // motor on always

	if ((data&3)==0)
		floppy = m_floppy0->get_device();
	else
	if ((data&3)==1)
		floppy = m_floppy1->get_device();

	m_fdc->set_floppy(floppy);
	m_fdc->dden_w(!BIT(data, 3)); // /Q output of ic29

	if (floppy)
	{
		floppy->ss_w(BIT(data, 2)); // inverted on the board
	}
}

/***********************************************************

    256TC Keyboard

************************************************************/


TIMER_DEVICE_CALLBACK_MEMBER( mbee_state::newkb_timer )
{
	/* Keyboard scanner is a Mostek M3870 chip. Its speed of operation is determined by a 15k resistor on
	pin 2 (XTL2) and is therefore 2MHz. If a key change is detected (up or down), the /strobe
	line activates, sending a high to bit 1 of port 2 (one of the pio input lines). The next read of
	port 18 will clear this line, and read the key scancode. It will also signal the 3870 that the key
	data has been read, on pin 38 (/extint). The 3870 can cache up to 9 keys. With no rom dump
	available, the following is a guess.

	The 3870 (MK3870) 8-bit microcontroller is a single chip implementation of Fairchild F8 (Mostek 3850).
	It includes up to 4 KB of mask-programmable ROM, 64 bytes of scratchpad RAM and up to 64 bytes
	of executable RAM. The MCU also integrates 32-bit I/O and a programmable timer. */

	if (!BIT(m_features, 2))
		return;

	uint8_t i, j, pressed;

	// find what has changed
	for (i = 0; i < 15; i++)
	{
		pressed = m_io_newkb[i]->read();
		if (pressed != m_newkb_was_pressed[i])
		{
			// get scankey value
			for (j = 0; j < 8; j++)
			{
				if (BIT(pressed^m_newkb_was_pressed[i], j))
				{
					// put it in the queue
					uint8_t code = (i << 3) | j | (BIT(pressed, j) ? 0x80 : 0);
					m_newkb_q[m_newkb_q_pos] = code;
					if (m_newkb_q_pos < 19) m_newkb_q_pos++;
				}
			}
			m_newkb_was_pressed[i] = pressed;
		}
	}

	// if anything queued, cause an interrupt
	if (m_newkb_q_pos)
		m_b2 = 1; // set irq

	if (m_b2)
		m_pio->port_b_write(pio_port_b_r());
}

uint8_t mbee_state::port18_r()
{
	uint8_t i, data = m_newkb_q[0]; // get oldest key

	if (m_newkb_q_pos)
	{
		m_newkb_q_pos--;
		for (i = 0; i < m_newkb_q_pos; i++) m_newkb_q[i] = m_newkb_q[i+1]; // ripple queue
	}

	m_b2 = 0; // clear irq
	return data;
}


/***********************************************************

    256TC Change CPU speed

************************************************************/

uint8_t mbee_state::speed_low_r()
{
	m_maincpu->set_unscaled_clock(3375000);
	return 0xff;
}

uint8_t mbee_state::speed_high_r()
{
	m_maincpu->set_unscaled_clock(6750000);
	return 0xff;
}



/***********************************************************

    256TC Real Time Clock

************************************************************/

void mbee_state::port04_w(uint8_t data)  // address
{
	m_rtc->write(0, data);
}

void mbee_state::port06_w(uint8_t data)  // write
{
	m_rtc->write(1, data);
}

uint8_t mbee_state::port07_r()   // read
{
	return m_rtc->read(1);
}

// See it work: Run mbeett, choose RTC in the config switches, run the F3 test, press Esc.
WRITE_LINE_MEMBER( mbee_state::rtc_irq_w )
{
	m_b7_rtc = state; // inverted by IC15 (pins 8,9,10) then again by mame

	if ((m_io_config->read() & 0xc0) == 0x40) // RTC selected in config menu
		m_pio->port_b_write(pio_port_b_r());
}


/***********************************************************

    256TC Memory Banking

    Selection of ROM, RAM, and video access by the CPU is controlled by U39,
    a PAL14L8. When read as an ordinary rom it is 16k in size. The dumper has
    arranged the pins as (bit0,1,..) (input = 1,23,2,3,4,5,6,7,8,9,10,11,14,13)
    and (output = 22,21,20,19,18,17,16,15). The prom is also used to control
    the refresh required by the dynamic rams, however we ignore this function.

    b_mask = total dynamic ram (1=64k; 3=128k; 7=256k or more)

    Certain software (such as the PJB system) constantly switch banks around,
    causing slowness. Therefore this function only changes the banks that need
    changing, leaving the others as is.

************************************************************/

void mbee_state::setup_banks(uint8_t data, bool first_time, uint8_t b_mask)
{
	b_mask &= 7;
	u32 dbank = m_ramsize / 0x1000;
	u8 extra_bits = data & 0xc0;
	data &= 0x3f; // (bits 0-5 are referred to as S0-S5)
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	uint8_t *prom = memregion("pals")->base();
	uint8_t b_data = bitswap<8>(data, 7,5,3,2,4,6,1,0) & 0x3b; // arrange data bits to S0,S1,-,S4,S2,S3
	uint8_t b_bank, b_byte, b_byte_t, b_addr, p_bank = 1;
	uint16_t b_vid;

	if (first_time || (b_data != m_bank_array[0])) // if same data as last time, leave now
	{
		m_bank_array[0] = b_data;

		for (b_bank = 0; b_bank < 16; b_bank++)
		{
			b_vid = b_bank << 12;
			b_addr = bitswap<8>(b_bank, 7,4,5,3,1,2,6,0) & 0x1f; // arrange address bits to A12,-,A14,A13,A15

			// Calculate read-bank
			b_byte_t = prom[b_addr | (b_data << 8) | 0x82]; // read-bank (RDS and MREQ are low, RFSH is high)
			b_byte = bitswap<8>(b_byte_t, 7,5,0,3,6,2,1,4); // rearrange so that bits 0-2 are rambank, bit 3 = rom select, bit 4 = video select, others not used

			if (first_time || (b_byte != m_bank_array[p_bank]))
			{
				m_bank_array[p_bank] = b_byte;

				if (!BIT(data, 5))
					b_byte &= 0xfb;  // U42/1 - S17 only valid if S5 is on

				if (!BIT(b_byte, 4))
				{
					// select video
					mem.install_read_handler (b_vid, b_vid + 0x7ff, read8sm_delegate(*this, FUNC(mbee_state::video_low_r)));
					mem.install_read_handler (b_vid + 0x800, b_vid + 0xfff, read8sm_delegate(*this, FUNC(mbee_state::video_high_r)));
				}
				else
				{
					mem.install_read_bank( b_vid, b_vid+0xfff, m_bankr[b_bank] );

					if (!BIT(b_byte, 3))
						m_bankr[b_bank]->set_entry(dbank + (b_bank & 3)); // read from rom
					else
						m_bankr[b_bank]->set_entry(extra_bits + (b_bank & 7) + ((b_byte & b_mask) << 3)); // ram
				}
			}
			p_bank++;

			// Calculate write-bank
			b_byte_t = prom[b_addr | (b_data << 8) | 0xc0]; // write-bank (XWR and MREQ are low, RFSH is high)
			b_byte = bitswap<8>(b_byte_t, 7,5,0,3,6,2,1,4); // rearrange so that bits 0-2 are rambank, bit 3 = rom select, bit 4 = video select, others not used

			if (first_time || (b_byte != m_bank_array[p_bank]))
			{
				m_bank_array[p_bank] = b_byte;

				if (!BIT(data, 5))
					b_byte &= 0xfb;  // U42/1 - S17 only valid if S5 is on

				if (!BIT(b_byte, 4))
				{
					// select video
					mem.install_write_handler (b_vid, b_vid + 0x7ff, write8sm_delegate(*this, FUNC(mbee_state::video_low_w)));
					mem.install_write_handler (b_vid + 0x800, b_vid + 0xfff, write8sm_delegate(*this, FUNC(mbee_state::video_high_w)));
				}
				else
				{
					mem.install_write_bank( b_vid, b_vid+0xfff, m_bankw[b_bank] );

					//if (!BIT(b_byte, 3))
						//m_bankw[b_bank]->set_entry(dbank); // write to rom dummy area
					//else
						m_bankw[b_bank]->set_entry(extra_bits + (b_bank & 7) + ((b_byte & b_mask) << 3)); // ram
				}
			}
			p_bank++;
		}
	}
}

void mbee_state::port50_w(u8 data)
{
	u8 mask = ((m_ramsize / 0x8000) - 1) & 7;
	setup_banks(data, 0, mask);
}

/***********************************************************

    128k Memory Banking

    The only difference to the 256TC is that bit 5 switches
    between rom2 and rom3. Since neither of these is dumped,
    this bit is not emulated. If it was, this scheme is used:

    Low - rom2 occupies C000-FFFF
    High - ram = C000-DFFF, rom3 = E000-FFFF.

************************************************************/


/***********************************************************

    ROM Banking on older models

    Set A to 0 or 1 then read the port to switch between the
    halves of the Telcom ROM.

    Output the PAK number to choose an optional PAK ROM.

    The bios will support 256 PAKs, although normally only
    8 are available in hardware. Each PAK is normally a 4K
    ROM. If 8K ROMs are used, the 2nd half becomes PAK+8,
    thus 16 PAKs in total. This is used in the PC85 models.

************************************************************/

void mbee_state::port0a_w(uint8_t data)
{
	m_0a = data;

	if (m_pak)
		m_pak->set_entry(data & 15);
}

uint8_t mbee_state::telcom_low_r()
{
/* Read of port 0A - set Telcom rom to first half */
	if (m_telcom)
		m_telcom->set_entry(0);

	return m_0a;
}

uint8_t mbee_state::telcom_high_r()
{
/* Read of port 10A - set Telcom rom to 2nd half */
	if (m_telcom)
		m_telcom->set_entry(1);

	return m_0a;
}


/***********************************************************

    Machine

************************************************************/

/*
  On reset or power on, a circuit forces rom 8000-8FFF to appear at 0000-0FFF, while ram is disabled.
  It gets set back to normal on the first attempt to write to memory. (/WR line goes active).
*/

void mbee_state::machine_start()
{
	save_item(NAME(m_ramsize));
	save_item(NAME(m_features));
	save_item(NAME(m_size));
	save_item(NAME(m_b7_rtc));
	save_item(NAME(m_b7_vs));
	save_item(NAME(m_b2));
	save_item(NAME(m_framecnt)); // not important
	save_item(NAME(m_08));
	save_item(NAME(m_0a));
	save_item(NAME(m_0b));
	save_item(NAME(m_1c));
	save_item(NAME(m_newkb_was_pressed));
	save_item(NAME(m_newkb_q));
	save_item(NAME(m_newkb_q_pos));
	save_item(NAME(m_sy6545_reg));
	save_item(NAME(m_sy6545_ind));
	save_item(NAME(m_fdc_rq));
	save_item(NAME(m_bank_array));

	// banking of the BASIC roms
	if (m_basic)
	{
		u8 *b = memregion("basicrom")->base();
		m_basic->configure_entries(0, 2, b, 0x2000);
	}

	// banking of the TELCOM rom
	if (m_telcom)
	{
		u8 *t = memregion("telcomrom")->base();
		m_telcom->configure_entries(0, 2, t, 0x1000);
	}

	// PAKs fitted
	if (m_pak)
	{
		u8 *p = memregion("pakrom")->base();
		m_pak->configure_entries(0, 16, p, 0x2000);
	}

	// videoram
	m_vram = make_unique_clear<u8[]>(0x0800);
	save_pointer(NAME(m_vram), 0x0800);

	// colour
	if (BIT(m_features, 0))
	{
		m_cram = make_unique_clear<u8[]>(0x0800);
		save_pointer(NAME(m_cram), 0x0800);
	}

	// minimal main ram
	if (BIT(m_features, 1))
		m_size = 0xE000;
	else
		m_size = 0x8000;

	// premium
	if (BIT(m_features, 3))
	{
		m_aram = make_unique_clear<u8[]>(0x0800);
		save_pointer(NAME(m_aram), 0x0800);
		m_pram = make_unique_clear<u8[]>(0x8800);
		save_pointer(NAME(m_pram), 0x8800);
	}
	else
	{
		m_pram = make_unique_clear<u8[]>(0x1000);
		save_pointer(NAME(m_pram), 0x1000);
	}

	// Banked systems
	u8 b = BIT(m_features, 4, 2);
	if (b)
	{
		m_ramsize = 0x20000;  // 128k
		if (b == 2)
			m_ramsize = 0x40000;  // 256k
		else
		if (b == 3)
			m_ramsize = 0x100000;  // 1MB for PP

		m_ram = make_unique_clear<u8[]>(m_ramsize);
		save_pointer(NAME(m_ram), m_ramsize);
		m_dummy = std::make_unique<u8[]>(0x1000);  // don't save this

		u8 *r = m_ram.get();
		u8 *d = m_dummy.get();
		u8 *m = memregion("maincpu")->base();

		u32 banks = m_ramsize / 0x1000;

		for (u8 b_bank = 0; b_bank < 16; b_bank++)
		{
			m_bankr[b_bank]->configure_entries(0, banks, r, 0x1000); // RAM banks
			m_bankr[b_bank]->configure_entries(banks, 4, m, 0x1000); // rom
			m_bankw[b_bank]->configure_entries(0, banks, r, 0x1000); // RAM banks
			m_bankw[b_bank]->configure_entry(banks, d); // dummy rom
		}
	}
}

void mbee_state::machine_reset()
{
	m_fdc_rq = 0;
	m_08 = 0;
	m_0a = 0;
	m_0b = 0;
	m_1c = 0;

	// set default chars
	memcpy(m_pram.get(), memregion("chargen")->base(), 0x800);

	if (m_basic)
		m_basic->set_entry(0);

	if (m_telcom)
		m_telcom->set_entry(0);

	if (m_pak)
		m_pak->set_entry(5);

	m_maincpu->set_pc(m_size);

	// init new kbd
	if (BIT(m_features, 2))
		m_newkb_q_pos = 0;

	// set banks to default
	if (BIT(m_features, 4, 2) == 1)
		setup_banks(0, 1, 3);
	else
	if (BIT(m_features, 4, 2) == 2)
		setup_banks(0, 1, 7);
	else
	if (BIT(m_features, 4, 2) == 3)
		setup_banks(0, 1, 31);
}


/* m_features:
bit 0 : (colour fitted) 1 = yes
bit 1 : (initial pc value / size of main ram) 1=0xE000, 0=0x8000
bit 2 : (keyboard type) 1 = new keyboard
bit 3 : (core type) 1 = premium
bit 4,5 : (banking main ram) 0x10 = 128k; 0x20 = 256k
*/

/***********************************************************

    Quickload

    These load the standard BIN format, as well
    as BEE, COM and MWB files.

************************************************************/

QUICKLOAD_LOAD_MEMBER(mbee_state::quickload_bee)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint16_t i, j;
	uint8_t data, sw = m_io_config->read() & 1;   /* reading the config switch: 1 = autorun */

	size_t quickload_size = image.length();
	if (image.is_filetype("mwb"))
	{
		/* mwb files - standard basic files */
		for (i = 0; i < quickload_size; i++)
		{
			j = 0x8c0 + i;

			if (image.fread(&data, 1) != 1)
			{
				image.message("Unexpected EOF");
				return image_init_result::FAIL;
			}

			if ((j < m_size) || (j > 0xefff))
				space.write_byte(j, data);
			else
			{
				image.message("Not enough memory in this microbee");
				return image_init_result::FAIL;
			}
		}

		if (sw)
		{
			space.write_word(0xa2,0x801e);  /* fix warm-start vector to get around some copy-protections */
			m_maincpu->set_pc(0x801e);
		}
		else
			space.write_word(0xa2,0x8517);
	}
	else if (image.is_filetype("com"))
	{
		/* com files - most com files are just machine-language games with a wrapper and don't need cp/m to be present */
		for (i = 0; i < quickload_size; i++)
		{
			j = 0x100 + i;

			if (image.fread(&data, 1) != 1)
			{
				image.message("Unexpected EOF");
				return image_init_result::FAIL;
			}

			if ((j < m_size) || (j > 0xefff))
				space.write_byte(j, data);
			else
			{
				image.message("Not enough memory in this microbee");
				return image_init_result::FAIL;
			}
		}

		if (sw) m_maincpu->set_pc(0x100);
	}
	else if (image.is_filetype("bee"))
	{
		/* bee files - machine-language games that start at 0900 */
		for (i = 0; i < quickload_size; i++)
		{
			j = 0x900 + i;

			if (image.fread(&data, 1) != 1)
			{
				image.message("Unexpected EOF");
				return image_init_result::FAIL;
			}

			if ((j < m_size) || (j > 0xefff))
				space.write_byte(j, data);
			else
			{
				image.message("Not enough memory in this microbee");
				return image_init_result::FAIL;
			}
		}

		if (sw) m_maincpu->set_pc(0x900);
	}

	return image_init_result::PASS;
}


/*-------------------------------------------------
    QUICKLOAD_LOAD_MEMBER( mbee_state::quickload_bin )
-------------------------------------------------*/

QUICKLOAD_LOAD_MEMBER(mbee_state::quickload_bin)
{
	uint16_t execute_address, start_addr, end_addr;
	int autorun;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	/* load the binary into memory */
	if (z80bin_load_file(image, space, execute_address, start_addr, end_addr) != image_init_result::PASS)
		return image_init_result::FAIL;

	/* is this file executable? */
	if (execute_address != 0xffff)
	{
		/* check to see if autorun is on */
		autorun = m_io_config->read() & 1;

		space.write_word(0xa6, execute_address);            /* fix the EXEC command */

		if (autorun)
		{
			space.write_word(0xa2, execute_address);        /* fix warm-start vector to get around some copy-protections */
			m_maincpu->set_pc(execute_address);
		}
		else
		{
			space.write_word(0xa2, 0x8517);
		}
	}

	return image_init_result::PASS;
}
