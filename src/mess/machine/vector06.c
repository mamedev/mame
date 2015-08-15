// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Vector06c driver by Miodrag Milanovic

        10/07/2008 Preliminary driver.

****************************************************************************/


#include "includes/vector06.h"


READ8_MEMBER( vector06_state::vector06_8255_portb_r )
{
	UINT8 key = 0xff;
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
	UINT8 ret = m_line[8]->read();

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
	machine().first_screen()->configure(width+64, 256+64, visarea, machine().first_screen()->frame_period().attoseconds());
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
	UINT8 r = (data & 7) << 5;
	UINT8 g = ((data >> 3) & 7) << 5;
	UINT8 b = ((data >>6) & 3) << 6;
	m_palette->set_pen_color( m_color_index, rgb_t(r,g,b) );
}


READ8_MEMBER( vector06_state::vector06_romdisk_portb_r )
{
	UINT16 addr = (m_romdisk_msb << 8) | m_romdisk_lsb;
	if (m_cart->exists() && addr < m_cart->get_rom_size())
		return m_cart->read_rom(space, addr);
	else
		return 0xff;
}

WRITE8_MEMBER( vector06_state::vector06_romdisk_porta_w )
{
	m_romdisk_lsb = data;
}

WRITE8_MEMBER( vector06_state::vector06_romdisk_portc_w )
{
	m_romdisk_msb = data;
}

READ8_MEMBER( vector06_state::vector06_8255_1_r )
{
	return m_ppi->read(space, offset^3);
}

WRITE8_MEMBER( vector06_state::vector06_8255_1_w )
{
	m_ppi->write(space, offset^3, data);
}

READ8_MEMBER( vector06_state::vector06_8255_2_r )
{
	return m_ppi2->read(space, offset^3);
}

WRITE8_MEMBER( vector06_state::vector06_8255_2_w )
{
	m_ppi2->write(space, offset^3, data);
}

INTERRUPT_GEN_MEMBER(vector06_state::vector06_interrupt)
{
	m_vblank_state++;
	if (m_vblank_state>1) m_vblank_state=0;
	device.execute().set_input_line(0,m_vblank_state ? HOLD_LINE : CLEAR_LINE);

}

IRQ_CALLBACK_MEMBER(vector06_state::vector06_irq_callback)
{
	// Interupt is RST 7
	return 0xff;
}

TIMER_CALLBACK_MEMBER(vector06_state::reset_check_callback)
{
	UINT8 val = m_reset->read();

	if (BIT(val, 0))
	{
		m_bank1->set_base(m_region_maincpu->base() + 0x10000);
		m_maincpu->reset();
	}

	if (BIT(val, 1))
	{
		m_bank1->set_base(m_ram->pointer() + 0x0000);
		m_maincpu->reset();
	}
}

WRITE8_MEMBER( vector06_state::vector06_disc_w )
{
	floppy_image_device *floppy = NULL;

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

void vector06_state::machine_start()
{
	machine().scheduler().timer_pulse(attotime::from_hz(50), timer_expired_delegate(FUNC(vector06_state::reset_check_callback),this));
}

void vector06_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.install_read_bank (0x0000, 0x7fff, m_bank1);
	space.install_write_bank(0x0000, 0x7fff, m_bank2);
	space.install_read_bank (0x8000, 0xffff, m_bank3);
	space.install_write_bank(0x8000, 0xffff, m_bank4);

	m_bank1->set_base(m_region_maincpu->base() + 0x10000);
	m_bank2->set_base(m_ram->pointer() + 0x0000);
	m_bank3->set_base(m_ram->pointer() + 0x8000);
	m_bank4->set_base(m_ram->pointer() + 0x8000);

	m_keyboard_mask = 0;
	m_color_index = 0;
	m_video_mode = 0;
}
