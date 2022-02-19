// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Radio-86RK machine driver by Miodrag Milanovic

        06/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/radio86.h"

#include "cpu/i8085/i8085.h"


void radio86_state::radio86_init_keyboard()
{
	m_keyboard_mask = 0;
	m_tape_value = 0x10;
}

/* Driver initialization */
void radio86_state::init_radio86()
{
	radio86_init_keyboard();
}

void radio86_state::init_radioram()
{
	init_radio86();
	m_radio_ram_disk = make_unique_clear<u8[]>(0x20000);
	save_pointer(NAME(m_radio_ram_disk), 0x20000);
}

u8 radio86_state::radio86_8255_portb_r2()
{
	u8 key = 0xff;
	for (u8 i = 0; i < 8; i++)
		if (BIT(m_keyboard_mask, i))
			key &= m_io_line[i]->read();

	return key;
}

uint8_t radio86_state::kr03_8255_portb_r2()
{
	uint8_t key = 0xff;
	uint16_t data;
	for (int i = 0; i < 8; i++)
	{
		if (BIT(m_keyboard_mask, i))
		{
			data = m_io_line[i]->read();
			if (!BIT(data, 8)) data &= ~3;
			if (!BIT(data, 9)) data &= ~5;
			if (!BIT(data, 10)) data &= ~7;
			key &= data;
		}
	}
	return key;
}

u8 radio86_state::radio86_8255_portc_r2()
{
	double level = m_cassette->input();
	u8 dat = m_io_line[8]->read();
	if (level < 0)
		dat ^= m_tape_value;

	return dat;
}

void radio86_state::radio86_8255_porta_w2(u8 data)
{
	m_keyboard_mask = data ^ 0xff;
}

void radio86_state::radio86_8255_portc_w2(u8 data)
{
	m_cassette->output(data & 0x01 ? 1 : -1);
}


u8 radio86_state::rk7007_8255_portc_r()
{
	double level = m_cassette->input();
	u8 key = 0xff;
	for (u8 i = 0; i < 8; i++)
		if ((m_keyboard_mask & (1 << i))!=0)
			key &= m_io_cline[i]->read();

	key &= 0xe0;
	if (level < 0)
		key ^= m_tape_value;

	return key;
}

void radio86_state::hrq_w(int state)
{
	/* HACK - this should be connected to the BUSREQ line of Z80 */
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the BUSACK line of Z80 */
	m_dma->hlda_w(state);
}

u8 radio86_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void radio86_state::memory_write_byte(offs_t offset, u8 data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

u8 radio86_state::radio_cpu_state_r()
{
	// FIXME: the driver should handle the status callback rather than accessing this through the state interface
	return m_maincpu->state_int(i8080_cpu_device::I8085_STATUS);
}

u8 radio86_state::radio_io_r(offs_t offset)
{
	return m_maincpu->space(AS_PROGRAM).read_byte((offset << 8) + offset);
}

void radio86_state::radio_io_w(offs_t offset, u8 data)
{
	m_maincpu->space(AS_PROGRAM).write_byte((offset << 8) + offset,data);
}

void radio86_state::machine_reset()
{
	m_keyboard_mask = 0;
	m_disk_sel = 0;

	address_space &program = m_maincpu->space(AS_PROGRAM);
	program.install_rom(0x0000, 0x0fff, m_rom);   // do it here for F3
	m_rom_shadow_tap.remove();
	m_rom_shadow_tap = program.install_read_tap(
			0xf000, 0xffff,
			"rom_shadow_r",
			[this] (offs_t offset, u8 &data, u8 mem_mask)
			{
				if (!machine().side_effects_disabled())
				{
					// delete this tap
					m_rom_shadow_tap.remove();

					// reinstall RAM over the ROM shadow
					m_maincpu->space(AS_PROGRAM).install_ram(0x0000, 0x0fff, m_ram);
				}
			},
			&m_rom_shadow_tap);
}

void radio86_state::machine_start()
{
	save_item(NAME(m_tape_value));
	save_item(NAME(m_keyboard_mask));
	save_item(NAME(m_romdisk_lsb));
	save_item(NAME(m_romdisk_msb));
	save_item(NAME(m_disk_sel));
}

void radio86_state::radio86_pagesel(u8 data)
{
	m_disk_sel = data;
}

u8 radio86_state::radio86rom_romdisk_porta_r()
{
	u16 addr = (m_romdisk_msb << 8) | m_romdisk_lsb;
	if (m_cart->exists() && addr < m_cart->get_rom_size())
		return m_cart->read_rom(addr);
	else
		return 0xff;
}

u8 radio86_state::radio86ram_romdisk_porta_r()
{
	u8 *romdisk = m_rom + 0x10000;
	if ((m_disk_sel & 0x0f) ==0)
		return romdisk[m_romdisk_msb*256+m_romdisk_lsb];
	else if (m_disk_sel==0xdf)
		return m_radio_ram_disk[m_romdisk_msb*256+m_romdisk_lsb + 0x10000];
	else
		return m_radio_ram_disk[m_romdisk_msb*256+m_romdisk_lsb];
}

void radio86_state::radio86_romdisk_portb_w(u8 data)
{
	m_romdisk_lsb = data;
}

void radio86_state::radio86_romdisk_portc_w(u8 data)
{
	m_romdisk_msb = data;
}

I8275_DRAW_CHARACTER_MEMBER(radio86_state::display_pixels)
{
	rgb_t const *const palette = m_palette->palette()->entry_list_raw();
	u8 pixels = m_chargen[(linecount & 7) + (charcode << 3)] ^ 0xff;
	if (vsp)
		pixels = 0;

	if (lten)
		pixels = 0xff;

	if (rvv)
		pixels ^= 0xff;

	for (u8 i = 0; i < 6; i++)
		bitmap.pix(y, x + i) = palette[(pixels >> (5-i)) & 1 ? (hlgt ? 2 : 1) : 0];
}

static constexpr rgb_t radio86_pens[3] = {
	{ 0x00, 0x00, 0x00 }, // black
	{ 0xa0, 0xa0, 0xa0 }, // white
	{ 0xff, 0xff, 0xff }  // highlight
};

void radio86_state::radio86_palette(palette_device &palette) const
{
	palette.set_pen_colors(0, radio86_pens);
}
