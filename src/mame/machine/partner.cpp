// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Partner driver by Miodrag Milanovic

        09/06/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "includes/radio86.h"
#include "includes/partner.h"

/* Driver initialization */
void partner_state::init_partner()
{
	m_tape_value = 0x80;
}

void partner_state::partner_window_1(uint8_t bank_num, uint16_t offset,uint8_t *rom)
{
	switch(m_win_mem_page) {
		case 2 : // FDD BIOS
				m_bank[bank_num]->set_base(rom + 0x16000 + offset);
				break;
		case 4 : // MCPG BIOS
				m_bank[bank_num]->set_base(rom + 0x14000 + offset);
				break;
		default : // BIOS
				m_bank[bank_num]->set_base(rom + 0x10000 + offset);
				break;
	}
}

void partner_state::partner_window_2(uint8_t bank_num, uint16_t offset,uint8_t *rom)
{
	switch(m_win_mem_page) {
		case 4 : // MCPG FONT
				m_bank[bank_num]->set_base(rom + 0x18000 + offset);
				break;
		default : // BIOS
				m_bank[bank_num]->set_base(rom + 0x10000 + offset);
				break;
	}
}

uint8_t partner_state::partner_floppy_r(offs_t offset)
{
	if (offset<0x100) {
		switch(offset & 3) {
			case 0x00 : return m_fdc->status_r();
			case 0x01 : return m_fdc->track_r();
			case 0x02 : return m_fdc->sector_r();
			default   :
						return m_fdc->data_r();
		}
	} else {
		return 0;
	}
}

void partner_state::partner_floppy_w(offs_t offset, uint8_t data)
{
	if (offset<0x100) {
		switch(offset & 3) {
			case 0x00 : m_fdc->cmd_w(data); break;
			case 0x01 : m_fdc->track_w(data);break;
			case 0x02 : m_fdc->sector_w(data);break;
			default   : m_fdc->data_w(data);break;
		}
	} else {
		floppy_image_device *floppy0 = m_fdc->subdevice<floppy_connector>("0")->get_device();
		floppy_image_device *floppy1 = m_fdc->subdevice<floppy_connector>("1")->get_device();

		if (((data >> 6) & 1)==1) {
			m_fdc->set_floppy(floppy0);
			floppy0->mon_w(0);
			floppy0->ss_w(data >> 7);
		}
		else
			floppy0->mon_w(1);

		if (((data >> 3) & 1)==1) {
			m_fdc->set_floppy(floppy1);
			floppy1->mon_w(0);
			floppy1->ss_w(data >> 7);
		}
		else
			floppy1->mon_w(1);
	}
}

void partner_state::partner_iomap_bank(uint8_t *rom)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	switch(m_win_mem_page) {
		case 2 :
				// FDD
				space.install_write_handler(0xdc00, 0xddff, write8sm_delegate(FUNC(partner_state::partner_floppy_w),this));
				space.install_read_handler (0xdc00, 0xddff, read8sm_delegate(FUNC(partner_state::partner_floppy_r),this));
				break;
		case 4 :
				// Timer
				break;
		default : // BIOS
				m_bank[10]->set_base(rom + 0x10000);
				break;
	}
}
void partner_state::partner_bank_switch()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint8_t *rom = memregion("maincpu")->base();
	uint8_t *ram = m_ram->pointer();

	space.install_write_bank(0x0000, 0x07ff, "bank1");
	space.install_write_bank(0x0800, 0x3fff, "bank2");
	space.install_write_bank(0x4000, 0x5fff, "bank3");
	space.install_write_bank(0x6000, 0x7fff, "bank4");
	space.install_write_bank(0x8000, 0x9fff, "bank5");
	space.install_write_bank(0xa000, 0xb7ff, "bank6");
	space.install_write_bank(0xb800, 0xbfff, "bank7");
	space.install_write_bank(0xc000, 0xc7ff, "bank8");
	space.install_write_bank(0xc800, 0xcfff, "bank9");
	space.install_write_bank(0xd000, 0xd7ff, "bank10");
	space.unmap_write(0xdc00, 0xddff);
	space.install_read_bank (0xdc00, 0xddff, "bank11");
	space.unmap_write(0xe000, 0xe7ff);
	space.unmap_write(0xe800, 0xffff);

	// BANK 1 (0x0000 - 0x07ff)
	if (m_mem_page==0) {
		space.unmap_write(0x0000, 0x07ff);
		m_bank[0]->set_base(rom + 0x10000);
	} else {
		if (m_mem_page==7) {
			m_bank[0]->set_base(ram + 0x8000);
		} else {
			m_bank[0]->set_base(ram + 0x0000);
		}
	}

	// BANK 2 (0x0800 - 0x3fff)
	if (m_mem_page==7) {
		m_bank[1]->set_base(ram + 0x8800);
	} else {
		m_bank[1]->set_base(ram + 0x0800);
	}

	// BANK 3 (0x4000 - 0x5fff)
	if (m_mem_page==7) {
		m_bank[2]->set_base(ram + 0xC000);
	} else {
		if (m_mem_page==10) {
			//window 1
			space.unmap_write(0x4000, 0x5fff);
			partner_window_1(2, 0, rom);
		} else {
			m_bank[2]->set_base(ram + 0x4000);
		}
	}

	// BANK 4 (0x6000 - 0x7fff)
	if (m_mem_page==7) {
		m_bank[3]->set_base(ram + 0xe000);
	} else {
		m_bank[3]->set_base(ram + 0x6000);
	}

	// BANK 5 (0x8000 - 0x9fff)
	switch (m_mem_page) {
		case 5:
		case 10:
				//window 2
				space.unmap_write(0x8000, 0x9fff);
				partner_window_2(4, 0, rom);
				break;
		case 8:
		case 9:
				//window 1
				space.unmap_write(0x8000, 0x9fff);
				partner_window_1(4, 0, rom);
				break;
		case 7:
				m_bank[4]->set_base(ram + 0x0000);
				break;
		default:
				m_bank[4]->set_base(ram + 0x8000);
				break;
	}

	// BANK 6 (0xa000 - 0xb7ff)
	switch (m_mem_page) {
		case 5:
		case 10:
				//window 2
				space.unmap_write(0xa000, 0xb7ff);
				partner_window_2(5, 0, rom);
				break;
		case 6:
		case 8:
				//BASIC
				space.unmap_write(0xa000, 0xb7ff);
				m_bank[5]->set_base(rom + 0x12000); // BASIC
				break;
		case 7:
				m_bank[5]->set_base(ram + 0x2000);
				break;
		default:
				m_bank[5]->set_base(ram + 0xa000);
				break;
	}

	// BANK 7 (0xb800 - 0xbfff)
	switch (m_mem_page) {
		case 4:
		case 5:
		case 10:
				//window 2
				space.unmap_write(0xb800, 0xbfff);
				partner_window_2(6, 0x1800, rom);
				break;
		case 6:
		case 8:
				//BASIC
				space.unmap_write(0xb800, 0xbfff);
				m_bank[6]->set_base(rom + 0x13800); // BASIC
				break;
		case 7:
				m_bank[6]->set_base(ram + 0x3800);
				break;
		default:
				m_bank[6]->set_base(ram + 0xb800);
				break;
	}

	// BANK 8 (0xc000 - 0xc7ff)
	switch (m_mem_page) {
		case 7:
				m_bank[7]->set_base(ram + 0x4000);
				break;
		case 8:
		case 10:
				space.unmap_write(0xc000, 0xc7ff);
				m_bank[7]->set_base(rom + 0x10000);
				break;
		default:
				m_bank[7]->set_base(ram + 0xc000);
				break;
	}

	// BANK 9 (0xc800 - 0xcfff)
	switch (m_mem_page) {
		case 7:
				m_bank[8]->set_base(ram + 0x4800);
				break;
		case 8:
		case 9:
				// window 2
				space.unmap_write(0xc800, 0xcfff);
				partner_window_2(8, 0, rom);
				break;
		case 10:
				space.unmap_write(0xc800, 0xcfff);
				m_bank[8]->set_base(rom + 0x10800);
				break;
		default:
				m_bank[8]->set_base(ram + 0xc800);
				break;
	}

	// BANK 10 (0xd000 - 0xd7ff)
	switch (m_mem_page) {
		case 7:
				m_bank[9]->set_base(ram + 0x5000);
				break;
		case 8:
		case 9:
				// window 2
				space.unmap_write(0xd000, 0xd7ff);
				partner_window_2(9, 0x0800, rom);
				break;
		default:
				m_bank[9]->set_base(ram + 0xd000);
				break;
	}

	// BANK 11 (0xdc00 - 0xddff)
	partner_iomap_bank(rom);

	// BANK 12 (0xe000 - 0xe7ff)
	if (m_mem_page==1) {
		m_bank[11]->set_base(rom + 0x10000);
	} else {
		//window 1
		partner_window_1(12, 0, rom);
	}

	// BANK 13 (0xe800 - 0xffff)
	switch (m_mem_page) {
		case 3:
		case 4:
		case 5:
				// window 1
				partner_window_1(12, 0x800, rom);
				break;
		default:
				// BIOS
				m_bank[12]->set_base(rom + 0x10800);
				break;
	}
}

void partner_state::partner_win_memory_page_w(uint8_t data)
{
	m_win_mem_page = ~data;
	partner_bank_switch();
}

void partner_state::partner_mem_page_w(uint8_t data)
{
	m_mem_page = (data >> 4) & 0x0f;
	partner_bank_switch();
}

I8275_DRAW_CHARACTER_MEMBER(partner_state::display_pixels)
{
	int i;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	const uint8_t *charmap = m_charmap + 0x400 * (gpa * 2 + hlgt);
	uint8_t pixels = charmap[(linecount & 7) + (charcode << 3)] ^ 0xff;
	if (vsp) {
		pixels = 0;
	}
	if (lten) {
		pixels = 0xff;
	}
	if (rvv) {
		pixels ^= 0xff;
	}
	for(i=0;i<6;i++) {
		bitmap.pix32(y, x + i) = palette[(pixels >> (5-i)) & 1];
	}
}

void partner_state::machine_reset()
{
	m_mem_page = 0;
	m_win_mem_page = 0;
	partner_bank_switch();
}
