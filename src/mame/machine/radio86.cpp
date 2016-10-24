// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Radio-86RK machine driver by Miodrag Milanovic

        06/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "includes/radio86.h"



void radio86_state::radio86_init_keyboard()
{
	m_keyboard_mask = 0;
	m_tape_value = 0x10;
}

/* Driver initialization */
void radio86_state::init_radio86()
{
	/* set initialy ROM to be visible on first bank */
	uint8_t *RAM = m_region_maincpu->base();
	memset(RAM,0x0000,0x1000); // make frist page empty by default
	m_bank1->configure_entries(1, 2, RAM, 0x0000);
	m_bank1->configure_entries(0, 2, RAM, 0xf800);
	radio86_init_keyboard();
}

void radio86_state::init_radioram()
{
	init_radio86();
	m_radio_ram_disk = std::make_unique<uint8_t[]>(0x20000);
	memset(m_radio_ram_disk.get(),0,0x20000);
}
uint8_t radio86_state::radio86_8255_portb_r2(address_space &space, offs_t offset, uint8_t mem_mask)
{
	uint8_t key = 0xff;
	if ((m_keyboard_mask & 0x01)!=0) { key &= m_io_line0->read(); }
	if ((m_keyboard_mask & 0x02)!=0) { key &= m_io_line1->read(); }
	if ((m_keyboard_mask & 0x04)!=0) { key &= m_io_line2->read(); }
	if ((m_keyboard_mask & 0x08)!=0) { key &= m_io_line3->read(); }
	if ((m_keyboard_mask & 0x10)!=0) { key &= m_io_line4->read(); }
	if ((m_keyboard_mask & 0x20)!=0) { key &= m_io_line5->read(); }
	if ((m_keyboard_mask & 0x40)!=0) { key &= m_io_line6->read(); }
	if ((m_keyboard_mask & 0x80)!=0) { key &= m_io_line7->read(); }
	return key;
}

uint8_t radio86_state::radio86_8255_portc_r2(address_space &space, offs_t offset, uint8_t mem_mask)
{
	double level = m_cassette->input();
	uint8_t dat = m_io_line8->read();
	if (level <  0) {
		dat ^= m_tape_value;
	}
	return dat;
}

void radio86_state::radio86_8255_porta_w2(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_keyboard_mask = data ^ 0xff;
}

void radio86_state::radio86_8255_portc_w2(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_cassette->output(data & 0x01 ? 1 : -1);
}


uint8_t radio86_state::rk7007_8255_portc_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	double level = m_cassette->input();
	uint8_t key = 0xff;
	if ((m_keyboard_mask & 0x01)!=0) { key &= m_io_cline0->read(); }
	if ((m_keyboard_mask & 0x02)!=0) { key &= m_io_cline1->read(); }
	if ((m_keyboard_mask & 0x04)!=0) { key &= m_io_cline2->read(); }
	if ((m_keyboard_mask & 0x08)!=0) { key &= m_io_cline3->read(); }
	if ((m_keyboard_mask & 0x10)!=0) { key &= m_io_cline4->read(); }
	if ((m_keyboard_mask & 0x20)!=0) { key &= m_io_cline5->read(); }
	if ((m_keyboard_mask & 0x40)!=0) { key &= m_io_cline6->read(); }
	if ((m_keyboard_mask & 0x80)!=0) { key &= m_io_cline7->read(); }
	key &= 0xe0;
	if (level <  0) {
		key ^= m_tape_value;
	}
	return key;
}

void radio86_state::hrq_w(int state)
{
	/* HACK - this should be connected to the BUSREQ line of Z80 */
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the BUSACK line of Z80 */
	m_dma8257->hlda_w(state);
}

uint8_t radio86_state::memory_read_byte(address_space &space, offs_t offset, uint8_t mem_mask)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void radio86_state::memory_write_byte(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

void radio86_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RESET:
		m_bank1->set_entry(0);
		break;
	default:
		assert_always(false, "Unknown id in radio86_state::device_timer");
	}
}


uint8_t radio86_state::radio_cpu_state_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return space.device().state().state_int(I8085_STATUS);
}

uint8_t radio86_state::radio_io_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_maincpu->space(AS_PROGRAM).read_byte((offset << 8) + offset);
}

void radio86_state::radio_io_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_maincpu->space(AS_PROGRAM).write_byte((offset << 8) + offset,data);
}

void radio86_state::machine_reset_radio86()
{
	timer_set(attotime::from_usec(10), TIMER_RESET);
	m_bank1->set_entry(1);

	m_keyboard_mask = 0;
	m_disk_sel = 0;
}


void radio86_state::radio86_pagesel(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_disk_sel = data;
}

uint8_t radio86_state::radio86rom_romdisk_porta_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	uint16_t addr = (m_romdisk_msb << 8) | m_romdisk_lsb;
	if (m_cart->exists() && addr < m_cart->get_rom_size())
		return m_cart->read_rom(space, addr);
	else
		return 0xff;
}

uint8_t radio86_state::radio86ram_romdisk_porta_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	uint8_t *romdisk = m_region_maincpu->base() + 0x10000;
	if ((m_disk_sel & 0x0f) ==0) {
		return romdisk[m_romdisk_msb*256+m_romdisk_lsb];
	} else {
		if (m_disk_sel==0xdf) {
			return m_radio_ram_disk[m_romdisk_msb*256+m_romdisk_lsb + 0x10000];
		} else {
			return m_radio_ram_disk[m_romdisk_msb*256+m_romdisk_lsb];
		}
	}
}

void radio86_state::radio86_romdisk_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_romdisk_lsb = data;
}

void radio86_state::radio86_romdisk_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_romdisk_msb = data;
}

void radio86_state::mikrosha_8255_font_page_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_mikrosha_font_page = (data  > 7) & 1;
}

I8275_DRAW_CHARACTER_MEMBER(radio86_state::display_pixels)
{
	int i;
	const rgb_t *palette = m_palette->palette()->entry_list_raw();
	const uint8_t *charmap = m_charmap;
	uint8_t pixels = charmap[(linecount & 7) + (charcode << 3)] ^ 0xff;
	if(linecount == 8)
		pixels = 0;
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
		bitmap.pix32(y, x + i) = palette[(pixels >> (5-i)) & 1 ? (hlgt ? 2 : 1) : 0];
	}
}

static const rgb_t radio86_palette[3] = {
	rgb_t(0x00, 0x00, 0x00), // black
	rgb_t(0xa0, 0xa0, 0xa0), // white
	rgb_t(0xff, 0xff, 0xff)  // highlight
};

void radio86_state::palette_init_radio86(palette_device &palette)
{
	palette.set_pen_colors(0, radio86_palette, ARRAY_LENGTH(radio86_palette));
}

void radio86_state::video_start()
{
	m_charmap = memregion("gfx1")->base();
}
