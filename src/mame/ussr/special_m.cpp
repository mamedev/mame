// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Specialist machine driver by Miodrag Milanovic

        20/03/2008 Cassette support
        15/03/2008 Preliminary driver.

****************************************************************************/



#include "emu.h"
#include "special.h"


/* Driver initialization */
void special_state::init_special()
{
	m_bank1->configure_entry(0, m_mram);
	m_bank1->configure_entry(1, m_rom);
}

uint8_t special_state::porta_r()
{
	for (int i = 0; i < 8; i++)
		if (m_io_keyboard[i]->read() < 0xfc)
			return (1 << i) ^ 0xff;

	return 0xff;
}

u8 special_state::kbd_r()
{
	u8 data = 0xfc;

	if (m_porta < 0xff)
		for (u8 i = 0; i < 8; i++)
			if (!BIT(m_porta, i))
				data &= m_io_keyboard[i]->read();

	if (BIT(m_portc, 0, 4) < 0x0f)
		for (u8 i = 0; i < 4; i++)
			if (!BIT(m_portc, i))
				data &= m_io_keyboard[8 + i]->read();

	// shift key
	if (m_io_keyboard[12]->read())
		data |= 2;

	return data;
}

uint8_t special_state::portb_r()
{
	u8 data = kbd_r();

	// strobe if a key is pressed
	// TODO: this conflicts with function keys - to check
	if ((data & 0xfc) < 0xfc)
		data &= 0x7f;

	// cassette
	if (m_cassette->input() > 0.01)
		data |= 0x81;

	return data;
}

uint8_t special_state::specimx_portb_r()
{
	u8 data = kbd_r();

	// cassette
	if (m_cassette->input() > 0.01)
		data |= 0x81;

	return data;
}

uint8_t special_state::portc_r()
{
	for (u8 i = 0; i < 4; i++)
		if (m_io_keyboard[8 + i]->read() < 0xfc)
			return (1 << i) ^ 0x0f;

	return 0x0f;
}

void special_state::porta_w(uint8_t data)
{
	m_porta = data;
}

void special_state::portb_w(uint8_t data)
{
	m_portb = data;
}

void special_state::portc_w(uint8_t data)
{
	specimx_portc_w(data);
	m_bank1->set_entry(BIT(data, 4));
}

void special_state::specimx_portc_w(uint8_t data)
{
	m_portc = data;

	m_cassette->output(BIT(data, 7) ? 1 : -1);

	m_dac->write(BIT(data, 5)); //beeper
}

TIMER_DEVICE_CALLBACK_MEMBER(special_state::pit_timer)
{
	m_pit->write_gate0(0);
	m_pit->write_gate1(0);
	m_pit->write_gate2(0);
}


/*
     Specialist MX
*/
void special_state::video_memory_w(offs_t offset, uint8_t data)
{
	m_ram->pointer()[0x9000 + offset] = data;
	m_specimx_colorram[offset] = m_specimx_color;
}

void special_state::specimx_video_color_w(uint8_t data)
{
	m_specimx_color = data;
}

uint8_t special_state::specimx_video_color_r()
{
	return m_specimx_color;
}

void special_state::specimx_set_bank(offs_t i, uint8_t data)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	uint8_t *ram = m_ram->pointer();

	space.install_write_bank(0xc000, 0xffbf, m_bank3);
	space.install_write_bank(0xffc0, 0xffdf, m_bank4);
	m_bank4->set_base(ram + 0xffc0);
	switch(i)
	{
		case 0 :
			space.install_write_bank(0x0000, 0x8fff, m_bank1);
			space.install_write_handler(0x9000, 0xbfff, write8sm_delegate(*this, FUNC(special_state::video_memory_w)));

			m_bank1->set_base(ram);
			m_bank2->set_base(ram + 0x9000);
			m_bank3->set_base(ram + 0xc000);
			break;
		case 1 :
			space.install_write_bank(0x0000, 0x8fff, m_bank1);
			space.install_write_bank(0x9000, 0xbfff, m_bank2);

			m_bank1->set_base(ram + 0x10000);
			m_bank2->set_base(ram + 0x19000);
			m_bank3->set_base(ram + 0x1c000);
			break;
		case 2 :
			space.unmap_write(0x0000, 0x8fff);
			space.unmap_write(0x9000, 0xbfff);

			m_bank1->set_base(m_rom);
			m_bank2->set_base(m_rom + 0x9000);
			m_bank3->set_base(ram + (BIT(data, 7) ? 0x1c000 : 0xc000));

			break;
	}
}

void special_state::specimx_select_bank(offs_t offset, uint8_t data)
{
	specimx_set_bank(offset, data);
}

uint8_t special_state::specimx_disk_ctrl_r()
{
	return 0xff;
}

WRITE_LINE_MEMBER( special_state::fdc_drq )
{
	/* Clears HALT state of CPU when data is ready to read */
	if(state) {
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
	}
}

void special_state::specimx_disk_ctrl_w(offs_t offset, uint8_t data)
{
	floppy_image_device *floppy = nullptr;
	floppy_connector *con = m_fdd[m_drive & 1].target();
	if(con)
		floppy = con->get_device();

	m_fdc->set_floppy(floppy);

	if (floppy)
	{
		floppy->mon_w(0);

		switch(offset)
		{
			case 0 :
				m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
				break;
			case 2 :
				floppy->ss_w(data & 1);
				break;
			case 3 :
				m_drive = data & 1;
				break;
		}
	}
}

/*
    Erik
*/

void special_state::erik_set_bank()
{
	uint8_t bank1 = m_RR_register & 3;
	uint8_t bank2 = BIT(m_RR_register, 2, 2);
	uint8_t bank3 = BIT(m_RR_register, 4, 2);
	uint8_t bank4 = BIT(m_RR_register, 6, 2);
	uint8_t *ram = m_ram->pointer();
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.install_write_bank(0x0000, 0x3fff, m_bank1);
	space.install_write_bank(0x4000, 0x8fff, m_bank2);
	space.install_write_bank(0x9000, 0xbfff, m_bank3);
	space.install_write_bank(0xc000, 0xefff, m_bank4);
	space.install_write_bank(0xf000, 0xf7ff, m_bank5);
	space.install_write_bank(0xf800, 0xffff, m_bank6);

	switch(bank1)
	{
		case    1:
		case    2:
		case    3:
			m_bank1->set_base(ram + 0x10000*(bank1-1));
			break;
		case    0:
			space.unmap_write(0x0000, 0x3fff);
			m_bank1->set_base(m_rom);
			break;
	}
	switch(bank2)
	{
		case    1:
		case    2:
		case    3:
			m_bank2->set_base(ram + 0x10000*(bank2-1) + 0x4000);
			break;
		case    0:
			space.unmap_write(0x4000, 0x8fff);
			m_bank2->set_base(m_rom + 0x4000);
			break;
	}
	switch(bank3)
	{
		case    1:
		case    2:
		case    3:
			m_bank3->set_base(ram + 0x10000*(bank3-1) + 0x9000);
			break;
		case    0:
			space.unmap_write(0x9000, 0xbfff);
			m_bank3->set_base(m_rom + 0x9000);
			break;
	}
	switch(bank4)
	{
		case    1:
		case    2:
		case    3:
			m_bank4->set_base(ram + 0x10000*(bank4-1) + 0xc000);
			m_bank5->set_base(ram + 0x10000*(bank4-1) + 0xf000);
			m_bank6->set_base(ram + 0x10000*(bank4-1) + 0xf800);
			break;
		case    0:
			space.unmap_write(0xc000, 0xefff);
			m_bank4->set_base(m_rom + 0xc000);
			space.unmap_write(0xf000, 0xf7ff);
			space.nop_read(0xf000, 0xf7ff);
			space.install_readwrite_handler(0xf800, 0xf803, 0, 0x7fc, 0, read8sm_delegate(*m_ppi, FUNC(i8255_device::read)), write8sm_delegate(*m_ppi, FUNC(i8255_device::write)));
			break;
	}
}

void special_state::init_erik()
{
	m_erik_color_1 = 0;
	m_erik_color_2 = 0;
	m_erik_background = 0;
}

void special_state::machine_reset()
{
	if (m_bank6) // erik
	{
		m_erik_color_1 = 0;
		m_erik_color_2 = 0;
		m_erik_background = 0;
		m_RR_register = 0x00;
		m_RC_register = 0x00;
		erik_set_bank();
	}
	else
	if (m_bank4) // specimx
	{
		m_specimx_color = 0xF0;  // default for -bios 1/2, since they don't have colour
		specimx_set_bank(2, 0); // Initial load ROM disk
		m_pit_timer->adjust(attotime::zero);
	}
}

void special_state::machine_start()
{
	if (m_bank6) // erik
	{
		save_item(NAME(m_erik_color_1));
		save_item(NAME(m_erik_color_2));
		save_item(NAME(m_erik_background));
		save_item(NAME(m_RR_register));
		save_item(NAME(m_RC_register));
	}
	else
	if (m_bank4) // specimx
	{
		m_specimx_colorram = std::make_unique<uint8_t[]>(0x3000);
		save_pointer(NAME(m_specimx_colorram), 0x3000);
		save_item(NAME(m_drive));
		save_item(NAME(m_specimx_color));
	}

	save_item(NAME(m_porta));
	save_item(NAME(m_portb));
	save_item(NAME(m_portc));
}

uint8_t special_state::erik_rr_reg_r()
{
	return m_RR_register;
}

void special_state::erik_rr_reg_w(uint8_t data)
{
	m_RR_register = data;
	erik_set_bank();
}

uint8_t special_state::erik_rc_reg_r()
{
	return m_RC_register;
}

void special_state::erik_rc_reg_w(uint8_t data)
{
	m_RC_register = data;
	m_erik_color_1 = m_RC_register & 7;
	m_erik_color_2 = BIT(m_RC_register, 3, 3);
	m_erik_background = BIT(m_RC_register, 6) + BIT(m_RC_register, 7) * 4;
}

uint8_t special_state::erik_disk_reg_r()
{
	return 0xff;
}

void special_state::erik_disk_reg_w(uint8_t data)
{
/*
    wd17xx_set_side (m_fdc,data & 1);
    wd17xx_set_drive(m_fdc,(data >> 1) & 1);
    wd17xx_dden_w(m_fdc, BIT(data, 2));
    floppy_mon_w(floppy_get_device(machine(), BIT(data, 1)), 0);
    floppy_mon_w(floppy_get_device(machine(), BIT(data, 1) ^ 1), 1);
    floppy_drive_set_ready_state(floppy_get_device(machine(), BIT(data, 1)), 1, 1);*/
}
