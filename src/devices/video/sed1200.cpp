// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/***************************************************************************

    SED1200

    A LCD controller similar to HD44780/SED1278 that drives a 20-character
    display. Data input is 4 bits at a time. (SED1210 has a 40-character
    display and 8-bit data input.)

    The D/F variants have a packaging difference (QFP80 vs. bare chip).

    The A/B variants have an internal CGROM difference (jis
    vs. european characters)

***************************************************************************/

#include "emu.h"
#include "sed1200.h"

DEFINE_DEVICE_TYPE(SED1200D0A, sed1200d0a_device, "sed1200da", "Epson SED1200D-0A LCD Controller")
DEFINE_DEVICE_TYPE(SED1200F0A, sed1200f0a_device, "sed1200fa", "Epson SED1200F-0A LCD Controller")
DEFINE_DEVICE_TYPE(SED1200D0B, sed1200d0b_device, "sed1200db", "Epson SED1200D-0B LCD Controller")
DEFINE_DEVICE_TYPE(SED1200F0B, sed1200f0b_device, "sed1200fb", "Epson SED1200F-0B LCD Controller")

ROM_START( sed1200x0a )
	ROM_REGION( 0x800, "cgrom", 0 )
	ROM_LOAD( "sed1200-a.bin", 0x000, 0x800, CRC(e8c28054) SHA1(086406eb74e9ed97b309d2a4bdedc567626e9a98))
ROM_END

ROM_START( sed1200x0b )
	ROM_REGION( 0x800, "cgrom", 0 )
	ROM_LOAD( "sed1200-b.bin", 0x000, 0x800, CRC(d0741f51) SHA1(c8c856f1357286a2c8c806af81724a828345357e))
ROM_END

sed1200_device::sed1200_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, cursor_direction(false)
	, cursor_blinking(false)
	, cursor_full(false)
	, cursor_on(false)
	, display_on(false)
	, two_lines(false)
	, cursor_address(0)
	, cgram_address(0)
	, cgrom(nullptr)
	, chip_select(false)
	, first_input(false)
	, first_data(0)
{
}

sed1200d0a_device::sed1200d0a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sed1200_device(mconfig, SED1200D0A, tag, owner, clock)
{
}

sed1200f0a_device::sed1200f0a_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sed1200_device(mconfig, SED1200F0A, tag, owner, clock)
{
}

sed1200d0b_device::sed1200d0b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sed1200_device(mconfig, SED1200D0B, tag, owner, clock)
{
}

sed1200f0b_device::sed1200f0b_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: sed1200_device(mconfig, SED1200F0B, tag, owner, clock)
{
}

const tiny_rom_entry *sed1200d0a_device::device_rom_region() const
{
	return ROM_NAME(sed1200x0a);
}

const tiny_rom_entry *sed1200f0a_device::device_rom_region() const
{
	return ROM_NAME(sed1200x0a);
}

const tiny_rom_entry *sed1200d0b_device::device_rom_region() const
{
	return ROM_NAME(sed1200x0b);
}

const tiny_rom_entry *sed1200f0b_device::device_rom_region() const
{
	return ROM_NAME(sed1200x0b);
}

void sed1200_device::device_start()
{
	memset(cgram, 0, sizeof(cgram));
	memset(ddram, 0, sizeof(ddram));
	if(memregion("cgrom"))
		cgrom = memregion("cgrom")->base();
	else
		cgrom = nullptr;

	chip_select = false;

	save_item(NAME(cgram));
	save_item(NAME(ddram));
	save_item(NAME(cursor_direction));
	save_item(NAME(cursor_blinking));
	save_item(NAME(cursor_full));
	save_item(NAME(cursor_on));
	save_item(NAME(display_on));
	save_item(NAME(two_lines));
	save_item(NAME(cursor_address));
	save_item(NAME(cgram_address));
	save_item(NAME(chip_select));
	save_item(NAME(first_input));
	save_item(NAME(first_data));

	soft_reset();
}

void sed1200_device::cs_w(int state)
{
	if (chip_select != !state) {
		chip_select = !state;
		first_input = true;
	}
}

void sed1200_device::soft_reset()
{
	cursor_direction = false;
	cursor_blinking = false;
	cursor_full = false;
	cursor_on = false;
	display_on = false;
	two_lines = false;
	cursor_address = 0x00;
	cgram_address = 0x00;
}

void sed1200_device::control_w(uint8_t data)
{
	if(chip_select) {
		if(first_input)
			first_data = data & 0x0f;
		else
			control_write(first_data << 4 | (data & 0x0f));
		first_input = !first_input;
	}
}

void sed1200_device::control_write(uint8_t data)
{
	switch(data) {
	case 0x04: case 0x05:
		cursor_direction = data & 0x01;
		break;
	case 0x06: case 0x07:
		cursor_step();
		break;
	case 0x08: case 0x09:
		cursor_full = data & 0x01;
		break;
	case 0x0a: case 0x0b:
		cursor_blinking = data & 0x01;
		break;
	case 0x0c: case 0x0d:
		display_on = data & 0x01;
		break;
	case 0x0e: case 0x0f:
		cursor_on = data & 0x01;
		break;
	case 0x10:
		soft_reset();
		break;
	case 0x12: case 0x13:
		two_lines = data & 0x01;
		break;
	default:
		if((data & 0xf0) == 0x20)
			cgram_address = (data & 3)*8;
		else if((data & 0xe0) == 0x40) {
			cgram[cgram_address++] = data;
			if(cgram_address == 4*8)
				cgram_address = 0;
		} else if(data & 0x80) {
			if (two_lines) {
				cursor_address = data & 0x40 ? 10 : 0;
				cursor_address += (data & 0x3f) >= 10 ? 9 : data & 0x3f;
			} else
				cursor_address = (data & 0x3f) >= 20 ? 19 : data & 0x3f;
		}
		break;
	}
}

uint8_t sed1200_device::busy_r()
{
	// TODO: bit 3 = busy flag
	return 0x00;
}

void sed1200_device::data_w(uint8_t data)
{
	if(chip_select) {
		if(first_input)
			first_data = data & 0x0f;
		else
			data_write(first_data << 4 | (data & 0x0f));
		first_input = !first_input;
	}
}

void sed1200_device::data_write(uint8_t data)
{
	ddram[cursor_address] = data;
	cursor_step();
}

void sed1200_device::cursor_step()
{
	if(cursor_direction) {
		if(cursor_address != 0 && (!two_lines || cursor_address != 10))
			cursor_address --;
	} else {
		if((!two_lines || cursor_address != 9) && cursor_address != 19)
			cursor_address ++;
	}
}

const uint8_t *sed1200_device::render()
{
	memset(render_buf, 0, 20*8);
	if(!display_on)
		return render_buf;

	for(int i=0; i<20; i++) {
		uint8_t c = ddram[i];
		if(c < 4)
			memcpy(render_buf + 8*i, cgram + 8*c, 8);
		else if(cgrom)
			memcpy(render_buf + 8*i, cgrom + 8*c, 8);
	}

	if(cursor_on && (!cursor_blinking || (machine().time().as_ticks(2) & 1))) {
		if(cursor_full)
			for(int i=0; i<8; i++)
				render_buf[cursor_address*8+i] ^= 0x1f;
		else
			render_buf[cursor_address*8+7] ^= 0x1f;
	}

	return render_buf;
}
