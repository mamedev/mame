// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, MetalliC
/***************************************************************************

        Vector06c driver by Miodrag Milanovic

        10/07/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "includes/vector06.h"
#include "screen.h"


READ8_MEMBER( vector06_state::vector06_8255_portb_r )
{
	uint8_t key = 0xff;
	if (BIT(m_keyboard_mask, 0)) key &= m_line[0]->read();
	if (BIT(m_keyboard_mask, 1)) key &= m_line[1]->read();
	if (BIT(m_keyboard_mask, 2)) key &= m_line[2]->read();
	if (BIT(m_keyboard_mask, 3)) key &= m_line[3]->read();
	if (BIT(m_keyboard_mask, 4)) key &= m_line[4]->read();
	if (BIT(m_keyboard_mask, 5)) key &= m_line[5]->read();
	if (BIT(m_keyboard_mask, 6)) key &= m_line[6]->read();
	if (BIT(m_keyboard_mask, 7)) key &= m_line[7]->read();
	return key;
}

READ8_MEMBER( vector06_state::vector06_8255_portc_r )
{
	uint8_t ret = m_line[8]->read();

	if (m_cassette->input() > 0)
		ret |= 0x10;

	return ret;
}

WRITE8_MEMBER( vector06_state::vector06_8255_porta_w )
{
	m_keyboard_mask = data ^ 0xff;
}

void vector06_state::vector06_set_video_mode(int width)
{
	rectangle visarea(0, width+64-1, 0, 256+64-1);
	m_screen->configure(width+64, 256+64, visarea, m_screen->frame_period().attoseconds());
}

WRITE8_MEMBER( vector06_state::vector06_8255_portb_w )
{
	m_color_index = data & 0x0f;
	if ((data & 0x10) != m_video_mode)
	{
		m_video_mode = data & 0x10;
		vector06_set_video_mode((m_video_mode==0x10) ? 512 : 256);
	}
}

WRITE8_MEMBER( vector06_state::vector06_color_set )
{
	uint8_t r = (data & 7) << 5;
	uint8_t g = ((data >> 3) & 7) << 5;
	uint8_t b = ((data >>6) & 3) << 6;
	m_palette->set_pen_color( m_color_index, rgb_t(r,g,b) );
}


READ8_MEMBER( vector06_state::vector06_romdisk_portb_r )
{
	uint16_t addr = ((m_romdisk_msb & 0x7f) << 8) | m_romdisk_lsb;
	if ((m_romdisk_msb & 0x80) && m_cart->exists() && addr < m_cart->get_rom_size())
		return m_cart->read_rom(addr);
	else
		return m_ay->data_r();
}

WRITE8_MEMBER(vector06_state::vector06_romdisk_portb_w)
{
	m_aylatch = data;
}

WRITE8_MEMBER( vector06_state::vector06_romdisk_porta_w )
{
	m_romdisk_lsb = data;
}

WRITE8_MEMBER( vector06_state::vector06_romdisk_portc_w )
{
	if (data & 4)
		m_ay->address_data_w((data >> 1) & 1, m_aylatch);
	m_romdisk_msb = data;
}

INTERRUPT_GEN_MEMBER(vector06_state::vector06_interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE);
}

IRQ_CALLBACK_MEMBER(vector06_state::vector06_irq_callback)
{
	// Interrupt is RST 7
	return 0xff;
}

TIMER_CALLBACK_MEMBER(vector06_state::reset_check_callback)
{
	uint8_t val = m_reset->read();

	if (BIT(val, 0))
	{
		m_romen = true;
		update_mem();
		m_maincpu->reset();
	}

	if (BIT(val, 1))
	{
		m_romen = false;
		update_mem();
		m_maincpu->reset();
	}
}

WRITE8_MEMBER( vector06_state::vector06_disc_w )
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
	if ((m_rambank & 0x10) && m_stack_state) {
		int sentry = ((m_rambank >> 2) & 3) + 1;
		m_bank1->set_entry(sentry);
		m_bank3->set_entry(sentry);
		m_bank2->set_base(m_ram->pointer() + sentry * 0x10000);
	}
	else {
		m_bank1->set_entry(0);
		int ventry = 0;
		if (m_rambank & 0x20)
			ventry = (m_rambank & 3) + 1;
		m_bank3->set_entry(ventry);
		if (m_romen)
			m_bank2->set_base(m_region_maincpu->base() + 0x10000);
		else
			m_bank2->set_base(m_ram->pointer());
	}
}

WRITE8_MEMBER(vector06_state::vector06_ramdisk_w)
{
	const uint8_t oldbank = m_rambank;
	m_rambank = data;
	if (oldbank != m_rambank)
		update_mem();
}

WRITE8_MEMBER(vector06_state::vector06_status_callback)
{
	const bool oldstate = m_stack_state;
	m_stack_state = bool(data & i8080_cpu_device::STATUS_STACK);
	if (oldstate != m_stack_state && (m_rambank & 0x10))
		update_mem();
}

WRITE_LINE_MEMBER(vector06_state::speaker_w)
{
	m_speaker->level_w(state);
}

void vector06_state::machine_start()
{
	m_reset_check_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(vector06_state::reset_check_callback), this));
	m_reset_check_timer->adjust(attotime::from_hz(50), 0, attotime::from_hz(50));
}

void vector06_state::machine_reset()
{
	m_stack_state = false;
	m_rambank = 0;
	m_romen = true;

	m_bank1->configure_entries(0, 5, m_ram->pointer(), 0x10000);
	for (int i = 0; i < 5; i++)
		m_bank3->configure_entry(i, m_ram->pointer() + 0x10000 * i + 0xa000);

	update_mem();

	m_keyboard_mask = 0;
	m_color_index = 0;
	m_video_mode = 0;
}
