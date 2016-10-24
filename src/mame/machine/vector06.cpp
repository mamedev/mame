// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, MetalliC
/***************************************************************************

        Vector06c driver by Miodrag Milanovic

        10/07/2008 Preliminary driver.

****************************************************************************/


#include "includes/vector06.h"


uint8_t vector06_state::vector06_8255_portb_r(address_space &space, offs_t offset, uint8_t mem_mask)
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

uint8_t vector06_state::vector06_8255_portc_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	uint8_t ret = m_line[8]->read();

	if (m_cassette->input() > 0)
		ret |= 0x10;

	return ret;
}

void vector06_state::vector06_8255_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_keyboard_mask = data ^ 0xff;
}

void vector06_state::vector06_set_video_mode(int width)
{
	rectangle visarea(0, width+64-1, 0, 256+64-1);
	machine().first_screen()->configure(width+64, 256+64, visarea, machine().first_screen()->frame_period().attoseconds());
}

void vector06_state::vector06_8255_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_color_index = data & 0x0f;
	if ((data & 0x10) != m_video_mode)
	{
		m_video_mode = data & 0x10;
		vector06_set_video_mode((m_video_mode==0x10) ? 512 : 256);
	}
}

void vector06_state::vector06_color_set(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	uint8_t r = (data & 7) << 5;
	uint8_t g = ((data >> 3) & 7) << 5;
	uint8_t b = ((data >>6) & 3) << 6;
	m_palette->set_pen_color( m_color_index, rgb_t(r,g,b) );
}


uint8_t vector06_state::vector06_romdisk_portb_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	uint16_t addr = ((m_romdisk_msb & 0x7f) << 8) | m_romdisk_lsb;
	if ((m_romdisk_msb & 0x80) && m_cart->exists() && addr < m_cart->get_rom_size())
		return m_cart->read_rom(space, addr);
	else
		return m_ay->ay8910_read_ym();
}

void vector06_state::vector06_romdisk_portb_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_aylatch = data;
}

void vector06_state::vector06_romdisk_porta_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_romdisk_lsb = data;
}

void vector06_state::vector06_romdisk_portc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	if (data & 4)
		m_ay->ay8910_write_ym((data >> 1) & 1, m_aylatch);
	m_romdisk_msb = data;
}

uint8_t vector06_state::vector06_8255_1_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_ppi->read(space, offset^3);
}

void vector06_state::vector06_8255_1_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_ppi->write(space, offset^3, data);
}

uint8_t vector06_state::vector06_8255_2_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_ppi2->read(space, offset^3);
}

void vector06_state::vector06_8255_2_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_ppi2->write(space, offset^3, data);
}

void vector06_state::vector06_interrupt(device_t &device)
{
	device.execute().set_input_line(0, HOLD_LINE);
}

int vector06_state::vector06_irq_callback(device_t &device, int irqline)
{
	// Interupt is RST 7
	return 0xff;
}

void vector06_state::reset_check_callback(void *ptr, int32_t param)
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

void vector06_state::vector06_disc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
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

void vector06_state::vector06_ramdisk_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	uint8_t oldbank = m_rambank;
	m_rambank = data;
	if (oldbank != m_rambank)
		update_mem();
}

void vector06_state::vector06_status_callback(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	bool oldstate = m_stack_state;
	m_stack_state = (data & I8085_STATUS_STACK) ? true : false;
	if (oldstate != m_stack_state && (m_rambank & 0x10))
		update_mem();
}

void vector06_state::speaker_w(int state)
{
	m_speaker->level_w(state);
}

void vector06_state::pit8253_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_pit8253->write(space, offset ^ 3, data);
}

uint8_t vector06_state::pit8253_r(address_space &space, offs_t offset, uint8_t mem_mask)
{
	return m_pit8253->read(space,  offset ^ 3);
}

void vector06_state::machine_start()
{
	machine().scheduler().timer_pulse(attotime::from_hz(50), timer_expired_delegate(FUNC(vector06_state::reset_check_callback),this));
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
