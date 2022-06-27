// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, MetalliC
/***************************************************************************

        Vector06c driver by Miodrag Milanovic

        10/07/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "vector06.h"
#include "screen.h"


uint8_t vector06_state::ppi1_portb_r()
{
	uint8_t key = 0xff;
	for (u8 i = 0; i < 8; i++)
		if (BIT(m_keyboard_mask, i))
			key &= m_io_keyboard[i]->read();

	return key;
}

uint8_t vector06_state::ppi1_portc_r()
{
	uint8_t ret = m_io_keyboard[8]->read();

	if (m_cassette->input() > 0)
		ret |= 0x10;

	return ret;
}

void vector06_state::ppi1_porta_w(uint8_t data)
{
	m_keyboard_mask = data ^ 0xff;
}

void vector06_state::ppi1_portb_w(uint8_t data)
{
	m_color_index = data & 0x0f;
	if (BIT(data, 4) != m_video_mode)
	{
		m_video_mode = BIT(data, 4);
		u16 width = m_video_mode ? 512 : 256;
		rectangle visarea(0, width+64-1, 0, 256+64-1);
		m_screen->configure(width+64, 256+64, visarea, m_screen->frame_period().attoseconds());
	}
}

void vector06_state::color_set(uint8_t data)
{
	uint8_t r = (data & 7) << 5;
	uint8_t g = ((data >> 3) & 7) << 5;
	uint8_t b = ((data >>6) & 3) << 6;
	m_palette->set_pen_color( m_color_index, rgb_t(r,g,b) );
}


uint8_t vector06_state::ppi2_portb_r()
{
	uint16_t addr = ((m_romdisk_msb & 0x7f) << 8) | m_romdisk_lsb;
	if ((m_romdisk_msb & 0x80) && m_cart->exists() && addr < m_cart->get_rom_size())
		return m_cart->read_rom(addr);
	else
		return m_ay->data_r();
}

void vector06_state::ppi2_portb_w(uint8_t data)
{
	m_aylatch = data;
}

void vector06_state::ppi2_porta_w(uint8_t data)
{
	m_romdisk_lsb = data;
}

void vector06_state::ppi2_portc_w (uint8_t data)
{
	if (data & 4)
		m_ay->address_data_w((data >> 1) & 1, m_aylatch);
	m_romdisk_msb = data;
}

IRQ_CALLBACK_MEMBER(vector06_state::irq_callback)
{
	// Interrupt is RST 7
	return 0xff;
}

INPUT_CHANGED_MEMBER(vector06_state::f11_button)
{
	if (newval)
	{
		m_romen = true;
		update_mem();
		m_maincpu->reset();
	}
}

INPUT_CHANGED_MEMBER(vector06_state::f12_button)
{
	if (newval)
	{
		m_romen = false;
		update_mem();
		m_maincpu->reset();
	}
}

void vector06_state::disc_w(uint8_t data)
{
	floppy_image_device *floppy = nullptr;

	switch (data & 0x01)
	{
	case 0: floppy = m_floppy0->get_device(); break;
	case 1: floppy = m_floppy1->get_device(); break;
	}

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		// something here needs to turn the motor on
		floppy->mon_w(0);
		floppy->ss_w(!BIT(data, 2));
	}
}

void vector06_state::update_mem()
{
	if (BIT(m_rambank, 4) && m_stack_state)
	{
		u8 sentry = ((m_rambank >> 2) & 3) + 1;
		m_bank1->set_entry(sentry);
		m_bank3->set_entry(sentry);
		m_bank2->set_entry(sentry + 1u);
	}
	else
	{
		m_bank1->set_entry(0);
		u8 ventry = 0;
		if (BIT(m_rambank, 5))
			ventry = (m_rambank & 3) + 1;
		m_bank3->set_entry(ventry);
		if (m_romen)
			m_bank2->set_entry(0);
		else
			m_bank2->set_entry(1);
	}
}

void vector06_state::ramdisk_w(uint8_t data)
{
	const uint8_t oldbank = m_rambank;
	m_rambank = data;
	if (oldbank != m_rambank)
		update_mem();
}

void vector06_state::status_callback(uint8_t data)
{
	const bool oldstate = m_stack_state;
	m_stack_state = bool(data & i8080_cpu_device::STATUS_STACK);
	if ((oldstate != m_stack_state) && BIT(m_rambank, 4))
		update_mem();
}

WRITE_LINE_MEMBER(vector06_state::speaker_w)
{
	m_speaker->level_w(state);
}

void vector06_state::machine_start()
{
	u8 *r = m_ram->pointer();

	m_bank1->configure_entries(0, 5, r, 0x10000);
	m_bank2->configure_entry(0, m_rom);
	m_bank2->configure_entries(1, 5, r, 0x10000);
	m_bank3->configure_entries(0, 5, r + 0xa000, 0x10000);

	save_item(NAME(m_keyboard_mask));
	save_item(NAME(m_color_index));
	save_item(NAME(m_romdisk_msb));
	save_item(NAME(m_romdisk_lsb));
	save_item(NAME(m_vblank_state));
	save_item(NAME(m_rambank));
	save_item(NAME(m_aylatch));
	save_item(NAME(m_video_mode));
	save_item(NAME(m_stack_state));
	save_item(NAME(m_romen));
}

void vector06_state::machine_reset()
{
	m_stack_state = false;
	m_rambank = 0;
	m_romen = true;

	update_mem();

	m_keyboard_mask = 0;
	m_color_index = 0;
	m_video_mode = 0;
	m_bank1->set_entry(0);
	m_bank2->set_entry(0);
	m_bank3->set_entry(0);
}
