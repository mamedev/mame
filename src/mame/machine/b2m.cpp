// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Bashkiria-2M machine driver by Miodrag Milanovic

        28/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "includes/b2m.h"

uint8_t b2m_state::keyboard_r(offs_t offset)
{
	uint8_t key = 0x00;
	if (offset < 0x100) {
		if ((offset & 0x01)!=0) { key |= ioport("LINE0")->read(); }
		if ((offset & 0x02)!=0) { key |= ioport("LINE1")->read(); }
		if ((offset & 0x04)!=0) { key |= ioport("LINE2")->read(); }
		if ((offset & 0x08)!=0) { key |= ioport("LINE3")->read(); }
		if ((offset & 0x10)!=0) { key |= ioport("LINE4")->read(); }
		if ((offset & 0x20)!=0) { key |= ioport("LINE5")->read(); }
		if ((offset & 0x40)!=0) { key |= ioport("LINE6")->read(); }
		if ((offset & 0x80)!=0) { key |= ioport("LINE7")->read(); }
	} else {
		if ((offset & 0x01)!=0) { key |= ioport("LINE8")->read(); }
		if ((offset & 0x02)!=0) { key |= ioport("LINE9")->read(); }
		if ((offset & 0x04)!=0) { key |= ioport("LINE10")->read(); }
	}
	return key;
}


void b2m_state::set_bank(int bank)
{
	uint8_t *rom;
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();

	space.install_write_bank(0x0000, 0x27ff, "bank1");
	space.install_write_bank(0x2800, 0x2fff, "bank2");
	space.install_write_bank(0x3000, 0x6fff, "bank3");
	space.install_write_bank(0x7000, 0xdfff, "bank4");
	space.install_write_bank(0xe000, 0xffff, "bank5");

	rom = memregion("maincpu")->base();
	switch(bank) {
		case 0 :
		case 1 :
						space.unmap_write(0xe000, 0xffff);

						membank("bank1")->set_base(ram);
						membank("bank2")->set_base(ram + 0x2800);
						membank("bank3")->set_base(ram + 0x3000);
						membank("bank4")->set_base(ram + 0x7000);
						membank("bank5")->set_base(rom);
						break;
#if 0
		case 1 :
						space.unmap_write(0x3000, 0x6fff);
						space.unmap_write(0xe000, 0xffff);

						membank("bank1")->set_base(ram);
						membank("bank2")->set_base(ram + 0x2800);
						membank("bank3")->set_base(rom + 0x2000);
						membank("bank4")->set_base(rom + 0x6000);
						membank("bank5")->set_base(rom);
						break;
#endif
		case 2 :
						space.unmap_write(0x2800, 0x2fff);
						space.unmap_write(0xe000, 0xffff);

						membank("bank1")->set_base(ram);
						space.install_read_handler(0x2800, 0x2fff, read8sm_delegate(*this, FUNC(b2m_state::keyboard_r)));
						membank("bank3")->set_base(ram + 0x10000);
						membank("bank4")->set_base(ram + 0x7000);
						membank("bank5")->set_base(rom);
						break;
		case 3 :
						space.unmap_write(0x2800, 0x2fff);
						space.unmap_write(0xe000, 0xffff);

						membank("bank1")->set_base(ram);
						space.install_read_handler(0x2800, 0x2fff, read8sm_delegate(*this, FUNC(b2m_state::keyboard_r)));
						membank("bank3")->set_base(ram + 0x14000);
						membank("bank4")->set_base(ram + 0x7000);
						membank("bank5")->set_base(rom);
						break;
		case 4 :
						space.unmap_write(0x2800, 0x2fff);
						space.unmap_write(0xe000, 0xffff);

						membank("bank1")->set_base(ram);
						space.install_read_handler(0x2800, 0x2fff, read8sm_delegate(*this, FUNC(b2m_state::keyboard_r)));
						membank("bank3")->set_base(ram + 0x18000);
						membank("bank4")->set_base(ram + 0x7000);
						membank("bank5")->set_base(rom);

						break;
		case 5 :
						space.unmap_write(0x2800, 0x2fff);
						space.unmap_write(0xe000, 0xffff);

						membank("bank1")->set_base(ram);
						space.install_read_handler(0x2800, 0x2fff, read8sm_delegate(*this, FUNC(b2m_state::keyboard_r)));
						membank("bank3")->set_base(ram + 0x1c000);
						membank("bank4")->set_base(ram + 0x7000);
						membank("bank5")->set_base(rom);

						break;
		case 6 :
						membank("bank1")->set_base(ram);
						membank("bank2")->set_base(ram + 0x2800);
						membank("bank3")->set_base(ram + 0x3000);
						membank("bank4")->set_base(ram + 0x7000);
						membank("bank5")->set_base(ram + 0xe000);
						break;
		case 7 :
						space.unmap_write(0x0000, 0x27ff);
						space.unmap_write(0x2800, 0x2fff);
						space.unmap_write(0x3000, 0x6fff);
						space.unmap_write(0x7000, 0xdfff);
						space.unmap_write(0xe000, 0xffff);

						membank("bank1")->set_base(rom);
						membank("bank2")->set_base(rom);
						membank("bank3")->set_base(rom);
						membank("bank4")->set_base(rom);
						membank("bank5")->set_base(rom);
						break;
	}
}


WRITE_LINE_MEMBER(b2m_state::pit_out1)
{
	m_speaker->level_w(state);
}

void b2m_state::ppi1_porta_w(uint8_t data)
{
	m_porta = data;
}

void b2m_state::ppi1_portb_w(uint8_t data)
{
	m_video_scroll = data;
}

void b2m_state::ppi1_portc_w(uint8_t data)
{
	m_portc = data;
	set_bank(m_portc & 7);
	m_video_page = BIT(m_portc, 7);
}

uint8_t b2m_state::ppi1_portb_r()
{
	return m_video_scroll;
}

WRITE_LINE_MEMBER( b2m_state::fdc_drq )
{
	/* Clears HALT state of CPU when data is ready to read */
	if (state)
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}


void b2m_state::ppi2_portc_w(uint8_t data)
{
	uint8_t drive = BIT(~data, 1);
	uint8_t side  = BIT(~data, 0);

	floppy_image_device *floppy = nullptr;
	if (m_fd[drive].found())
		floppy = m_fd[drive]->get_device();

	if (floppy)
		floppy->mon_w(0);
	m_fdc->set_floppy(floppy);

	if (floppy)
		floppy->ss_w(side);

	/*
	    When bit 5 is set CPU is in HALT state and stay there until
	    DRQ is triggered from floppy side
	*/

	if ((data & 0xf0)==0x20) {
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}

uint8_t b2m_state::romdisk_porta_r()
{
	uint8_t *romdisk = memregion("maincpu")->base() + 0x2000;
	return romdisk[m_romdisk_msb*256+m_romdisk_lsb];
}

void b2m_state::romdisk_portb_w(uint8_t data)
{
	m_romdisk_lsb = data;
}

void b2m_state::romdisk_portc_w(uint8_t data)
{
	m_romdisk_msb = data & 0x7f;
}

void b2m_state::palette_w(offs_t offset, uint8_t data)
{
	uint8_t b = BIT(~data, 6, 2) * 0x55;
	uint8_t g = BIT(~data, 4, 2) * 0x55;
	uint8_t r = BIT(~data, 2, 2) * 0x55;

	uint8_t bw = BIT(~data, 0, 2) * 0x55;

	m_color[offset & 3] = data;

	if (ioport("MONITOR")->read()==1)
		m_palette->set_pen_color(offset, r, g, b);
	else
		m_palette->set_pen_color(offset, bw, bw, bw);
}

uint8_t b2m_state::palette_r(offs_t offset)
{
	return m_color[offset];
}

void b2m_state::localmachine_w(uint8_t data)
{
	m_localmachine = data;
}

uint8_t b2m_state::localmachine_r()
{
	return m_localmachine;
}

void b2m_state::postload()
{
	set_bank(m_portc & 7);
}

void b2m_state::machine_start()
{
	/* register for state saving */
	save_item(NAME(m_porta));
	save_item(NAME(m_video_scroll));
	save_item(NAME(m_portc));
	save_item(NAME(m_video_page));
	save_item(NAME(m_romdisk_lsb));
	save_item(NAME(m_romdisk_msb));
	save_pointer(NAME(m_color), 4);
	save_item(NAME(m_localmachine));
	save_item(NAME(m_vblank_state));

	machine().save().register_postload(save_prepost_delegate(FUNC(b2m_state::postload), this));
}

INTERRUPT_GEN_MEMBER(b2m_state::vblank_interrupt)
{
	m_vblank_state++;
	if (m_vblank_state>1) m_vblank_state=0;
	m_pic->ir0_w(m_vblank_state);
}

void b2m_state::machine_reset()
{
	m_vblank_state = 0;
	set_bank(7);
}

uint32_t b2m_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t code1;
	uint8_t code2;
	uint8_t col;
	int y, x, b;
	uint8_t *ram = m_ram->pointer();

	for (x = 0; x < 48; x++)
	{
		for (y = 0; y < 256; y++)
		{
			u16 t = x*256 + ((y + m_video_scroll) & 0xff);
			if (m_video_page==0)
			{
				code1 = ram[0x11000 + t];
				code2 = ram[0x15000 + t];
			}
			else
			{
				code1 = ram[0x19000 + t];
				code2 = ram[0x1d000 + t];
			}
			for (b = 7; b >= 0; b--)
			{
				col = (BIT(code2, b)<<1) + BIT(code1, b);
				bitmap.pix16(y, x*8+b) =  col;
			}
		}
	}

	return 0;
}

void b2m_state::b2m_palette(palette_device &palette) const
{
	static constexpr rgb_t b2m_pens[4] = {
		{ 0x00, 0x00, 0x00 }, // 0
		{ 0x00, 0x00, 0x00 }, // 1
		{ 0x00, 0x00, 0x00 }, // 2
		{ 0x00, 0x00, 0x00 }, // 3
	};

	palette.set_pen_colors(0, b2m_pens);
}
