// license:BSD-3-Clause
// copyright-holders:Nathan Woods
/***************************************************************************

    dragon.cpp

    Dragon Family

Dragon Alpha code added 21-Oct-2004,
            Phill Harvey-Smith (afra@aurigae.demon.co.uk)

            Added AY-8912 and FDC code 30-Oct-2004.

Fixed Dragon Alpha NMI enable/disable, following circuit traces on a real machine.
    P.Harvey-Smith, 11-Aug-2005.

Re-implemented Alpha NMI enable/disable, using direct PIA reads, rather than
keeping track of it in a variable in the driver.
    P.Harvey-Smith, 25-Sep-2006.

Radically re-wrote memory emulation code for CoCo 1/2 & Dragon machines, the
new code emulates the memory mapping of the SAM, dependent on what size of
RAM chips it is programed to use, including proper mirroring of the RAM.

Replaced the kludged emulation of the cart line, with a timer based trigger
this is set to toggle at 1Hz, this seems to be good enough to trigger the
cartline, but is so slow in real terms that it should have very little
impact on the emulation speed.

Re-factored the code common to all machines, and separated the code different,
into callbacks/functions unique to the machines, in preparation for splitting
the code for individual machine types into separate files, I have preposed, that
the CoCo 1/2 should stay in coco.c, and that the coco3 and dragon specific code
should go into coco3.c and dragon.c which should (hopefully) make the code
easier to manage.
    P.Harvey-Smith, Dec 2006-Feb 2007

***************************************************************************/

#include "emu.h"
#include "dragon.h"


/***************************************************************************
  DRAGON32
***************************************************************************/

//-------------------------------------------------
//  pia1_pa_changed - called when PIA1 PA changes
//-------------------------------------------------

void dragon_state::pia1_pa_changed(uint8_t data)
{
	/* call inherited function */
	coco12_state::pia1_pa_changed(data);

	/* if strobe bit is high send data from pia0 port b to dragon parallel printer */
	if (data & 0x02)
	{
		uint8_t output = pia_0().b_output();
		m_printer->output(output);
	}
}


/***************************************************************************
  DRAGON64
***************************************************************************/

//-------------------------------------------------
//  device_start
//-------------------------------------------------

void dragon64_state::device_start()
{
	dragon_state::device_start();

	uint8_t *rom = memregion("maincpu")->base();
	m_rombank[0]->configure_entries(0, 2, &rom[0x0000], 0x8000);
	m_rombank[1]->configure_entries(0, 2, &rom[0x2000], 0x8000);
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void dragon64_state::device_reset()
{
	dragon_state::device_reset();

	m_rombank[0]->set_entry(0);
	m_rombank[1]->set_entry(0);
}


//-------------------------------------------------
//  pia1_pb_changed
//-------------------------------------------------

void dragon64_state::pia1_pb_changed(uint8_t data)
{
	dragon_state::pia1_pb_changed(data);

	uint8_t ddr = ~pia_1().port_b_z_mask();

	/* If bit 2 of the pia1 ddrb is 1 then this pin is an output so use it */
	/* to control the paging of the 32k and 64k basic roms */
	/* Otherwise it set as an input, with an EXTERNAL pull-up so it should */
	/* always be high (enabling 32k basic rom) */
	if (ddr & 0x04)
	{
		page_rom(data & 0x04 ? true : false);
	}
}


//-------------------------------------------------
//  page_rom - Controls rom paging in Dragon 64,
//  and Dragon Alpha.
//
//  On 64, switches between the two versions of the
//  basic rom mapped in at 0x8000
//
//  On the alpha switches between the
//  Boot/Diagnostic rom and the basic rom
//-------------------------------------------------

void dragon64_state::page_rom(bool romswitch)
{
	int bank = romswitch
		? 0    // This is the 32k mode basic(64)/boot rom(alpha)
		: 1;   // This is the 64k mode basic(64)/basic rom(alpha)
	m_rombank[0]->set_entry(bank);      // 0x8000-0x9FFF
	m_rombank[1]->set_entry(bank);      // 0xA000-0xBFFF
}


/***************************************************************************
  DRAGON200-E
***************************************************************************/

uint8_t dragon200e_state::sam_read(offs_t offset)
{
	uint8_t data = sam().display_read(offset);
	m_vdg->as_w(data & 0x80 ? ASSERT_LINE : CLEAR_LINE);
	m_vdg->intext_w(data & 0x80 ? CLEAR_LINE : ASSERT_LINE);
	m_vdg->inv_w(m_lk1->read() ? ASSERT_LINE : CLEAR_LINE);
	return data;
}

MC6847_GET_CHARROM_MEMBER(dragon200e_state::char_rom_r)
{
	uint16_t addr = (line << 8) | (BIT(pia_1().b_output(), 4) << 7) | ch;
	return m_char_rom->base()[addr & 0xfff];
}


/***************************************************************************
  DRAGON64 PLUS
***************************************************************************/

//-------------------------------------------------
//  d64plus_6845_disp_r
//
//  The status of the 6845 display is determined by bit 0 of the $FFE2
//  register as follows:-
//    1  Video display busy, ie. not blanking
//    0  Video display available, ie. blanking
//  The display is blanking during a horizontal or vertical retrace period.
//-------------------------------------------------

uint8_t d64plus_state::d64plus_6845_disp_r()
{
	return m_crtc->de_r() ? 0xff : 0xfe;
}


//-------------------------------------------------
//  d64plus_bank_w
//-------------------------------------------------

void d64plus_state::d64plus_bank_w(uint8_t data)
{
	switch (data & 0x06)
	{
	case 0:  // Standard Dragon 32 Dynamic bank
		m_pram_bank->set_entry(0);
		m_vram_bank->set_entry(0);
		break;
	case 2:  // First extra 32K bank (A)
		m_pram_bank->set_entry(1);
		m_vram_bank->set_entry(1);
		break;
	case 6:  // Second extra 32K bank (B)
		m_pram_bank->set_entry(2);
		m_vram_bank->set_entry(2);
		break;
	default:
		logerror("unknown bank register $FFE2 = %02x\n", data);
		break;
	}
	if (data & 0x01)
	{
		m_vram_bank->set_entry(3);  // Video RAM bank (C)
	}
}


MC6845_UPDATE_ROW(d64plus_state::crtc_update_row)
{
	for (int column = 0; column < x_count; column++)
	{
		uint8_t code = m_video_ram[((ma + column) & 0x7ff)];
		uint16_t addr = (code << 4) | (ra & 0x0f);
		uint8_t data = m_char_rom->base()[addr & 0xfff];

		if (column == cursor_x)
		{
			data = 0xff;
		}

		for (int bit = 0; bit < 8; bit++)
		{
			int x = (column * 8) + bit;
			bitmap.pix(y, x) = m_palette->pen(BIT(data, 7) && de);
			data <<= 1;
		}
	}
}


//-------------------------------------------------
//  device_start
//-------------------------------------------------

void d64plus_state::device_start()
{
	dragon64_state::device_start();

	m_sam->space(0).install_readwrite_bank(0x0000, 0x7fff, m_pram_bank);
	m_sam->space(0).install_readwrite_bank(0x0000, 0x07ff, m_vram_bank);

	m_pram_bank->configure_entry(0, m_ram->pointer());
	m_pram_bank->configure_entries(1, 2, m_plus_ram, 0x8000);

	m_vram_bank->configure_entry(0, m_ram->pointer());
	m_vram_bank->configure_entries(1, 2, m_plus_ram, 0x8000);
	m_vram_bank->configure_entry(3, m_video_ram);

	address_space &space = m_maincpu->space(AS_PROGRAM);
	space.install_readwrite_handler(0xffe0, 0xffe0, read8smo_delegate(*m_crtc, FUNC(mc6845_device::status_r)), write8smo_delegate(*m_crtc, FUNC(mc6845_device::address_w)));
	space.install_readwrite_handler(0xffe1, 0xffe1, read8smo_delegate(*m_crtc, FUNC(mc6845_device::register_r)), write8smo_delegate(*m_crtc, FUNC(mc6845_device::register_w)));
	space.install_readwrite_handler(0xffe2, 0xffe2, read8smo_delegate(*this, FUNC(d64plus_state::d64plus_6845_disp_r)), write8smo_delegate(*this, FUNC(d64plus_state::d64plus_bank_w)));
}


//-------------------------------------------------
//  device_reset
//-------------------------------------------------

void d64plus_state::device_reset()
{
	dragon64_state::device_reset();

	m_pram_bank->set_entry(0);
	m_vram_bank->set_entry(0);
}
