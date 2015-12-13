// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Specialist machine driver by Miodrag Milanovic

        20/03/2008 Cassette support
        15/03/2008 Preliminary driver.

****************************************************************************/



#include "includes/special.h"


/* Driver initialization */
DRIVER_INIT_MEMBER(special_state,special)
{
	/* set initialy ROM to be visible on first bank */
	UINT8 *RAM = m_region_maincpu->base();
	memset(RAM,0x0000,0x3000); // make first page empty by default
	m_bank1->configure_entries(1, 2, RAM, 0x0000);
	m_bank1->configure_entries(0, 2, RAM, 0xc000);
}

READ8_MEMBER( special_state::specialist_8255_porta_r )
{
	if (m_io_line0->read()!=0xff) return 0xfe;
	if (m_io_line1->read()!=0xff) return 0xfd;
	if (m_io_line2->read()!=0xff) return 0xfb;
	if (m_io_line3->read()!=0xff) return 0xf7;
	if (m_io_line4->read()!=0xff) return 0xef;
	if (m_io_line5->read()!=0xff) return 0xdf;
	if (m_io_line6->read()!=0xff) return 0xbf;
	if (m_io_line7->read()!=0xff) return 0x7f;
	return 0xff;
}

READ8_MEMBER( special_state::specialist_8255_portb_r )
{
	UINT8 dat = 0xff;

	if ((m_specialist_8255_porta & 0x01)==0) dat &= m_io_line0->read();
	if ((m_specialist_8255_porta & 0x02)==0) dat &= m_io_line1->read();
	if ((m_specialist_8255_porta & 0x04)==0) dat &= m_io_line2->read();
	if ((m_specialist_8255_porta & 0x08)==0) dat &= m_io_line3->read();
	if ((m_specialist_8255_porta & 0x10)==0) dat &= m_io_line4->read();
	if ((m_specialist_8255_porta & 0x20)==0) dat &= m_io_line5->read();
	if ((m_specialist_8255_porta & 0x40)==0) dat &= m_io_line6->read();
	if ((m_specialist_8255_porta & 0x80)==0) dat &= m_io_line7->read();
	if ((m_specialist_8255_portc & 0x01)==0) dat &= m_io_line8->read();
	if ((m_specialist_8255_portc & 0x02)==0) dat &= m_io_line9->read();
	if ((m_specialist_8255_portc & 0x04)==0) dat &= m_io_line10->read();
	if ((m_specialist_8255_portc & 0x08)==0) dat &= m_io_line11->read();

	// shift key
	if BIT(~m_io_line12->read(), 0)
		dat &= 0xfd;

	// cassette
	if (m_cassette->input() > 0.01)
		dat &= 0xfe;

	// strobe if a key is pressed
	if (dat < 0xfc) dat &= 0x7f;

	return dat;
}

READ8_MEMBER( special_state::specimx_8255_portb_r )
{
	UINT8 dat = 0xff;

	if ((m_specialist_8255_porta & 0x01)==0) dat &= m_io_line0->read();
	if ((m_specialist_8255_porta & 0x02)==0) dat &= m_io_line1->read();
	if ((m_specialist_8255_porta & 0x04)==0) dat &= m_io_line2->read();
	if ((m_specialist_8255_porta & 0x08)==0) dat &= m_io_line3->read();
	if ((m_specialist_8255_porta & 0x10)==0) dat &= m_io_line4->read();
	if ((m_specialist_8255_porta & 0x20)==0) dat &= m_io_line5->read();
	if ((m_specialist_8255_porta & 0x40)==0) dat &= m_io_line6->read();
	if ((m_specialist_8255_porta & 0x80)==0) dat &= m_io_line7->read();
	if ((m_specialist_8255_portc & 0x01)==0) dat &= m_io_line8->read();
	if ((m_specialist_8255_portc & 0x02)==0) dat &= m_io_line9->read();
	if ((m_specialist_8255_portc & 0x04)==0) dat &= m_io_line10->read();
	if ((m_specialist_8255_portc & 0x08)==0) dat &= m_io_line11->read();

	// shift key
	if BIT(~m_io_line12->read(), 0)
		dat &= 0xfd;

	// cassette
	if (m_cassette->input() > 0.01)
		dat &= 0xfe;

	return dat;
}

READ8_MEMBER( special_state::specialist_8255_portc_r )
{
	if (m_io_line8->read()!=0xff) return 0x0e;
	if (m_io_line9->read()!=0xff) return 0x0d;
	if (m_io_line10->read()!=0xff) return 0x0b;
	if (m_io_line11->read()!=0xff) return 0x07;
	return 0x0f;
}

WRITE8_MEMBER( special_state::specialist_8255_porta_w )
{
	m_specialist_8255_porta = data;
}

WRITE8_MEMBER( special_state::specialist_8255_portb_w )
{
	m_specialist_8255_portb = data;
}

WRITE8_MEMBER( special_state::specialist_8255_portc_w )
{
	m_specialist_8255_portc = data;

	m_cassette->output(BIT(data, 7) ? 1 : -1);

	m_dac->write_unsigned8(BIT(data, 5)); //beeper
}

void special_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RESET:
		m_bank1->set_entry(0);
		break;
	case TIMER_PIT8253_GATES:
	{
		m_pit->write_gate0(0);
		m_pit->write_gate1(0);
		m_pit->write_gate2(0);
		break;
	}
	default:
		assert_always(FALSE, "Unknown id in special_state::device_timer");
	}
}


MACHINE_RESET_MEMBER(special_state,special)
{
	timer_set(attotime::from_usec(10), TIMER_RESET);
	m_bank1->set_entry(1);
}


/*
     Specialist MX
*/
WRITE8_MEMBER( special_state::video_memory_w )
{
	m_ram->pointer()[0x9000 + offset] = data;
	m_specimx_colorram[offset] = m_specimx_color;
}

WRITE8_MEMBER( special_state::specimx_video_color_w )
{
	m_specimx_color = data;
}

READ8_MEMBER( special_state::specimx_video_color_r )
{
	return m_specimx_color;
}

void special_state::specimx_set_bank(offs_t i, UINT8 data)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 *ram = m_ram->pointer();

	space.install_write_bank(0xc000, 0xffbf, "bank3");
	space.install_write_bank(0xffc0, 0xffdf, "bank4");
	m_bank4->set_base(ram + 0xffc0);
	switch(i)
	{
		case 0 :
			space.install_write_bank(0x0000, 0x8fff, "bank1");
			space.install_write_handler(0x9000, 0xbfff, write8_delegate(FUNC(special_state::video_memory_w), this));

			m_bank1->set_base(ram);
			m_bank2->set_base(ram + 0x9000);
			m_bank3->set_base(ram + 0xc000);
			break;
		case 1 :
			space.install_write_bank(0x0000, 0x8fff, "bank1");
			space.install_write_bank(0x9000, 0xbfff, "bank2");

			m_bank1->set_base(ram + 0x10000);
			m_bank2->set_base(ram + 0x19000);
			m_bank3->set_base(ram + 0x1c000);
			break;
		case 2 :
			space.unmap_write(0x0000, 0x8fff);
			space.unmap_write(0x9000, 0xbfff);

			m_bank1->set_base(m_region_maincpu->base() + 0x10000);
			m_bank2->set_base(m_region_maincpu->base() + 0x19000);

			if (data & 0x80)
				m_bank3->set_base(ram + 0x1c000);
			else
				m_bank3->set_base(ram + 0xc000);

			break;
	}
}

WRITE8_MEMBER( special_state::specimx_select_bank )
{
	specimx_set_bank(offset, data);
}

MACHINE_START_MEMBER(special_state,specimx)
{
	m_drive = 0;
}

MACHINE_RESET_MEMBER(special_state,specimx)
{
	specimx_set_bank(2, 0); // Initiali load ROM disk
	timer_set(attotime::zero, TIMER_PIT8253_GATES);
}

READ8_MEMBER( special_state::specimx_disk_ctrl_r )
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

WRITE8_MEMBER( special_state::specimx_disk_ctrl_w )
{
	static const char *names[] = { "fd0", "fd1"};
	floppy_image_device *floppy = nullptr;
	floppy_connector *con = machine().device<floppy_connector>(names[m_drive & 1]);
	if(con)
		floppy = con->get_device();

	m_fdc->set_floppy(floppy);
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

/*
    Erik
*/

void special_state::erik_set_bank()
{
	UINT8 bank1 = m_RR_register & 3;
	UINT8 bank2 = (m_RR_register >> 2) & 3;
	UINT8 bank3 = (m_RR_register >> 4) & 3;
	UINT8 bank4 = (m_RR_register >> 6) & 3;
	UINT8 *mem = m_region_maincpu->base();
	UINT8 *ram = m_ram->pointer();
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.install_write_bank(0x0000, 0x3fff, "bank1");
	space.install_write_bank(0x4000, 0x8fff, "bank2");
	space.install_write_bank(0x9000, 0xbfff, "bank3");
	space.install_write_bank(0xc000, 0xefff, "bank4");
	space.install_write_bank(0xf000, 0xf7ff, "bank5");
	space.install_write_bank(0xf800, 0xffff, "bank6");

	switch(bank1)
	{
		case    1:
		case    2:
		case    3:
			m_bank1->set_base(ram + 0x10000*(bank1-1));
			break;
		case    0:
			space.unmap_write(0x0000, 0x3fff);
			m_bank1->set_base(mem + 0x10000);
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
			m_bank2->set_base(mem + 0x14000);
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
			m_bank3->set_base(mem + 0x19000);
			break;
	}
	switch(bank4)
	{
		case    1:
		case    2:
		case    3:
			m_bank4->set_base(ram + 0x10000*(bank4-1) + 0x0c000);
			m_bank5->set_base(ram + 0x10000*(bank4-1) + 0x0f000);
			m_bank6->set_base(ram + 0x10000*(bank4-1) + 0x0f800);
			break;
		case    0:
			space.unmap_write(0xc000, 0xefff);
			m_bank4->set_base(mem + 0x1c000);
			space.unmap_write(0xf000, 0xf7ff);
			space.nop_read(0xf000, 0xf7ff);
			space.install_readwrite_handler(0xf800, 0xf803, 0, 0x7fc, read8_delegate(FUNC(i8255_device::read), (i8255_device*)m_ppi), write8_delegate(FUNC(i8255_device::write), (i8255_device*)m_ppi));
			break;
	}
}

DRIVER_INIT_MEMBER(special_state,erik)
{
	m_erik_color_1 = 0;
	m_erik_color_2 = 0;
	m_erik_background = 0;
}

MACHINE_RESET_MEMBER(special_state,erik)
{
	m_RR_register = 0x00;
	m_RC_register = 0x00;
	erik_set_bank();
}

READ8_MEMBER( special_state::erik_rr_reg_r )
{
	return m_RR_register;
}

WRITE8_MEMBER( special_state::erik_rr_reg_w )
{
	m_RR_register = data;
	erik_set_bank();
}

READ8_MEMBER( special_state::erik_rc_reg_r )
{
	return m_RC_register;
}

WRITE8_MEMBER( special_state::erik_rc_reg_w )
{
	m_RC_register = data;
	m_erik_color_1 = m_RC_register & 7;
	m_erik_color_2 = (m_RC_register >> 3) & 7;
	m_erik_background = BIT(m_RC_register, 6) + BIT(m_RC_register, 7) * 4;
}

READ8_MEMBER( special_state::erik_disk_reg_r )
{
	return 0xff;
}

WRITE8_MEMBER( special_state::erik_disk_reg_w )
{
/*
    wd17xx_set_side (m_fdc,data & 1);
    wd17xx_set_drive(m_fdc,(data >> 1) & 1);
    wd17xx_dden_w(m_fdc, BIT(data, 2));
    floppy_mon_w(floppy_get_device(machine(), BIT(data, 1)), 0);
    floppy_mon_w(floppy_get_device(machine(), BIT(data, 1) ^ 1), 1);
    floppy_drive_set_ready_state(floppy_get_device(machine(), BIT(data, 1)), 1, 1);*/
}
