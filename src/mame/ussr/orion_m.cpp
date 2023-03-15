// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        Orion machine driver by Miodrag Milanovic

        22/04/2008 Orion Pro added
        02/04/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "orion.h"

#include "screen.h"


#define SCREEN_WIDTH_384 48
#define SCREEN_WIDTH_480 60
#define SCREEN_WIDTH_512 64



uint8_t orion_state::orion_romdisk_porta_r()
{
	uint16_t addr = (m_romdisk_msb << 8) | m_romdisk_lsb;
	if (m_cart->exists() && addr < m_cart->get_rom_size())
		return m_cart->read_rom(addr);
	else
		return 0xff;
}

void orion_state::orion_romdisk_portb_w(uint8_t data)
{
	m_romdisk_lsb = data;
}

void orion_state::orion_romdisk_portc_w(uint8_t data)
{
	m_romdisk_msb = data;
}

uint8_t orion_state::orion128_system_r(offs_t offset)
{
	return m_ppi2->read(offset & 3);
}

void orion_state::orion128_system_w(offs_t offset, uint8_t data)
{
	m_ppi2->write(offset & 3, data);
}

uint8_t orion_state::orion128_romdisk_r(offs_t offset)
{
	return m_ppi1->read(offset & 3);
}

void orion_state::orion128_romdisk_w(offs_t offset, uint8_t data)
{
	m_ppi1->write(offset & 3, data);
}

void orion_state::orion_set_video_mode(int width)
{
	rectangle visarea(0, width-1, 0, 255);
	m_screen->configure(width, 256, visarea, m_screen->frame_period().attoseconds());
}

void orion_state::orion128_video_mode_w(uint8_t data)
{
	if (BIT(data, 7)!=BIT(m_orion128_video_mode, 7))
	{
		if (BIT(data, 7))
		{
			if (m_video_mode_mask == 31)
			{
				m_orion128_video_width = SCREEN_WIDTH_512;
				orion_set_video_mode(512);
			}
			else
			{
				m_orion128_video_width = SCREEN_WIDTH_480;
				orion_set_video_mode(480);
			}
		}
		else
		{
			m_orion128_video_width = SCREEN_WIDTH_384;
			orion_set_video_mode(384);
		}
	}

	m_orion128_video_mode = data;
}

void orion_state::orion128_video_page_w(uint8_t data)
{
	if (m_orion128_video_page != data)
	{
		if (BIT(data, 7)!=BIT(m_orion128_video_page, 7))
		{
			if (BIT(data, 7))
			{
				if (m_video_mode_mask == 31)
				{
					m_orion128_video_width = SCREEN_WIDTH_512;
					orion_set_video_mode(512);
				}
				else
				{
					m_orion128_video_width = SCREEN_WIDTH_480;
					orion_set_video_mode(480);
				}
			}
			else
			{
				m_orion128_video_width = SCREEN_WIDTH_384;
				orion_set_video_mode(384);
			}
		}
	}
	m_orion128_video_page = data;
}


void orion_state::orion128_memory_page_w(uint8_t data)
{
	if (data!=m_orion128_memory_page )
	{
		m_bank1->set_base(m_ram->pointer() + (data & 3) * 0x10000);
		m_orion128_memory_page = (data & 3);
	}
}

void orion_state::machine_reset()
{
	m_video_mode_mask = 7;
	m_orion128_video_page = 0;
	m_orion128_video_mode = 0;
	m_orion128_memory_page = -1;
	m_bank1->set_base(m_rom + 0xf800);
	m_bank2->set_base(m_ram->pointer() + 0xf000);
	m_orion128_video_width = SCREEN_WIDTH_384;
	orion_set_video_mode(384);
	radio86_init_keyboard();
}

void orion_state::machine_start()
{
	save_item(NAME(m_tape_value));
	save_item(NAME(m_orion128_video_mode));
	save_item(NAME(m_orion128_video_page));
	save_item(NAME(m_orion128_video_width));
	save_item(NAME(m_video_mode_mask));
	save_item(NAME(m_orionpro_pseudo_color));
	save_item(NAME(m_romdisk_lsb));
	save_item(NAME(m_romdisk_msb));
	save_item(NAME(m_orion128_memory_page));
	save_item(NAME(m_orionz80_memory_page));
	save_item(NAME(m_orionz80_dispatcher));
	save_item(NAME(m_speaker_data));
	save_item(NAME(m_orionpro_ram0_segment));
	save_item(NAME(m_orionpro_ram1_segment));
	save_item(NAME(m_orionpro_ram2_segment));
	save_item(NAME(m_orionpro_page));
	save_item(NAME(m_orionpro_128_page));
	save_item(NAME(m_orionpro_rom2_segment));
	save_item(NAME(m_orionpro_dispatcher));
}

void orion_state::orion_disk_control_w(uint8_t data)
{
	floppy_connector *names[] = { m_fd0, m_fd1, m_fd2, m_fd3};
	floppy_image_device *floppy = names[data & 3]->get_device();

	m_fdc->set_floppy(floppy);
	floppy->mon_w(0);
	floppy->ss_w(((data & 0x10) >> 4) ^ 1);
}

uint8_t orion_state::orion128_floppy_r(offs_t offset)
{
	switch(offset)
	{
		case 0x0  :
		case 0x10 : return m_fdc->status_r();
		case 0x1  :
		case 0x11 : return m_fdc->track_r();
		case 0x2  :
		case 0x12 : return m_fdc->sector_r();
		case 0x3  :
		case 0x13 : return m_fdc->data_r();
	}
	return 0xff;
}

void orion_state::orion128_floppy_w(offs_t offset, uint8_t data)
{
	switch(offset)
	{
		case 0x0  :
		case 0x10 : m_fdc->cmd_w(data); break;
		case 0x1  :
		case 0x11 : m_fdc->track_w(data);break;
		case 0x2  :
		case 0x12 : m_fdc->sector_w(data);break;
		case 0x3  :
		case 0x13 : m_fdc->data_w(data);break;
		case 0x4  :
		case 0x14 :
		case 0x20 : orion_disk_control_w(data);break;
	}
}
uint8_t orion_state::orionz80_floppy_rtc_r(offs_t offset)
{
	if ((offset >= 0x60) && (offset <= 0x6f))
		return m_rtc->read(offset-0x60);
	else
		return orion128_floppy_r(offset);
}

void orion_state::orionz80_floppy_rtc_w(offs_t offset, uint8_t data)
{
	if ((offset >= 0x60) && (offset <= 0x6f))
		m_rtc->write(offset-0x60,data);
	else
		orion128_floppy_w(offset,data);
}

void orion_state::orionz80_sound_w(uint8_t data)
{
	if (m_speaker_data == 0)
		m_speaker_data = data;
	else
		m_speaker_data = 0;

	m_speaker->level_w(m_speaker_data);

}

void orion_state::orionz80_sound_fe_w(uint8_t data)
{
	m_speaker->level_w(BIT(data, 4));
}

void orion_state::orionz80_switch_bank()
{
	uint8_t bank_select;
	uint8_t segment_select;
	address_space &space = m_maincpu->space(AS_PROGRAM);

	bank_select = (m_orionz80_dispatcher & 0x0c) >> 2;
	segment_select = m_orionz80_dispatcher & 0x03;

	space.install_write_bank(0x0000, 0x3fff, m_bank1);
	if ((m_orionz80_dispatcher & 0x80)==0)
	{ // dispatcher on
		m_bank1->set_base(m_ram->pointer() + 0x10000 * bank_select + segment_select * 0x4000 );
	}
	else
	{ // dispatcher off
		m_bank1->set_base(m_ram->pointer() + 0x10000 * m_orionz80_memory_page);
	}

	m_bank2->set_base(m_ram->pointer() + 0x4000 + 0x10000 * m_orionz80_memory_page);

	if ((m_orionz80_dispatcher & 0x20) == 0)
	{
		space.install_write_handler(0xf400, 0xf4ff, write8sm_delegate(*this, FUNC(orion_state::orion128_system_w)));
		space.install_write_handler(0xf500, 0xf5ff, write8sm_delegate(*this, FUNC(orion_state::orion128_romdisk_w)));
		space.install_write_handler(0xf700, 0xf7ff, write8sm_delegate(*this, FUNC(orion_state::orionz80_floppy_rtc_w)));
		space.install_read_handler(0xf400, 0xf4ff, read8sm_delegate(*this, FUNC(orion_state::orion128_system_r)));
		space.install_read_handler(0xf500, 0xf5ff, read8sm_delegate(*this, FUNC(orion_state::orion128_romdisk_r)));
		space.install_read_handler(0xf700, 0xf7ff, read8sm_delegate(*this, FUNC(orion_state::orionz80_floppy_rtc_r)));

		space.install_write_handler(0xf800, 0xf8ff, write8smo_delegate(*this, FUNC(orion_state::orion128_video_mode_w)));
		space.install_write_handler(0xf900, 0xf9ff, write8smo_delegate(*this, FUNC(orion_state::orionz80_memory_page_w)));
		space.install_write_handler(0xfa00, 0xfaff, write8smo_delegate(*this, FUNC(orion_state::orion128_video_page_w)));
		space.install_write_handler(0xfb00, 0xfbff, write8smo_delegate(*this, FUNC(orion_state::orionz80_dispatcher_w)));
		space.unmap_write(0xfc00, 0xfeff);
		space.install_write_handler(0xff00, 0xffff, write8smo_delegate(*this, FUNC(orion_state::orionz80_sound_w)));

		m_bank3->set_base(m_ram->pointer() + 0xf000);
		m_bank5->set_base(m_rom + 0xf800);

	}
	else
	{
		/* if it is full memory access */
		m_bank3->set_base(m_ram->pointer() + 0xf000 + 0x10000 * m_orionz80_memory_page);
		m_bank4->set_base(m_ram->pointer() + 0xf400 + 0x10000 * m_orionz80_memory_page);
		m_bank5->set_base(m_ram->pointer() + 0xf800 + 0x10000 * m_orionz80_memory_page);
	}
}

void orion_state::orionz80_memory_page_w(uint8_t data)
{
	m_orionz80_memory_page = data & 7;
	orionz80_switch_bank();
}

void orion_state::orionz80_dispatcher_w(uint8_t data)
{
	m_orionz80_dispatcher = data;
	orionz80_switch_bank();
}

void orion_z80_state::machine_reset()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);

	space.unmap_write(0x0000, 0x3fff);
	space.install_write_bank(0x4000, 0xefff, m_bank2);
	space.install_write_bank(0xf000, 0xf3ff, m_bank3);

	space.install_write_handler(0xf400, 0xf4ff, write8sm_delegate(*this, FUNC(orion_z80_state::orion128_system_w)));
	space.install_write_handler(0xf500, 0xf5ff, write8sm_delegate(*this, FUNC(orion_z80_state::orion128_romdisk_w)));
	space.install_write_handler(0xf700, 0xf7ff, write8sm_delegate(*this, FUNC(orion_z80_state::orionz80_floppy_rtc_w)));
	space.install_read_handler(0xf400, 0xf4ff, read8sm_delegate(*this, FUNC(orion_z80_state::orion128_system_r)));
	space.install_read_handler(0xf500, 0xf5ff, read8sm_delegate(*this, FUNC(orion_z80_state::orion128_romdisk_r)));
	space.install_read_handler(0xf700, 0xf7ff, read8sm_delegate(*this, FUNC(orion_z80_state::orionz80_floppy_rtc_r)));

	space.install_write_handler(0xf800, 0xf8ff, write8smo_delegate(*this, FUNC(orion_z80_state::orion128_video_mode_w)));
	space.install_write_handler(0xf900, 0xf9ff, write8smo_delegate(*this, FUNC(orion_z80_state::orionz80_memory_page_w)));
	space.install_write_handler(0xfa00, 0xfaff, write8smo_delegate(*this, FUNC(orion_z80_state::orion128_video_page_w)));
	space.install_write_handler(0xfb00, 0xfbff, write8smo_delegate(*this, FUNC(orion_z80_state::orionz80_dispatcher_w)));
	space.unmap_write(0xfc00, 0xfeff);
	space.install_write_handler(0xff00, 0xffff, write8smo_delegate(*this, FUNC(orion_z80_state::orionz80_sound_w)));


	m_bank1->set_base(m_rom + 0xf800);
	m_bank2->set_base(m_ram->pointer() + 0x4000);
	m_bank3->set_base(m_ram->pointer() + 0xf000);
	m_bank5->set_base(m_rom + 0xf800);

	m_video_mode_mask = 7;
	m_orion128_video_page = 0;
	m_orion128_video_mode = 0;
	m_orionz80_memory_page = 0;
	m_orionz80_dispatcher = 0;
	m_speaker_data = 0;
	m_orion128_video_width = SCREEN_WIDTH_384;
	orion_set_video_mode(384);
	radio86_init_keyboard();
}

INTERRUPT_GEN_MEMBER(orion_state::orionz80_interrupt)
{
	if ((m_orionz80_dispatcher & 0x40)==0x40)
		device.execute().set_input_line(0, HOLD_LINE);
}

uint8_t orion_state::orionz80_io_r(offs_t offset)
{
	if (offset == 0xFFFD)
		return m_ay8912->data_r();

	return 0xff;
}

void orion_state::orionz80_io_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xff)
	{
		case 0xf8 : orion128_video_mode_w(data);break;
		case 0xf9 : orionz80_memory_page_w(data);break;
		case 0xfa : orion128_video_page_w(data);break;
		case 0xfb : orionz80_dispatcher_w(data);break;
		case 0xfe : orionz80_sound_fe_w(data);break;
		case 0xff : orionz80_sound_w(data);break;
	}
	switch(offset)
	{
		case 0xfffd : m_ay8912->address_w(data);
						break;
		case 0xbffd :
		case 0xbefd : m_ay8912->data_w(data);
						break;
	}
}

void orion_state::orionpro_bank_switch()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	int page = m_orionpro_page & 7; // we have only 8 pages
	int is128 = (m_orionpro_dispatcher & 0x80) ? 1 : 0;
	uint8_t *ram = m_ram->pointer();

	if (is128==1)
		page = m_orionpro_128_page & 7;

	space.install_write_bank(0x0000, 0x1fff, m_bank1);
	space.install_write_bank(0x2000, 0x3fff, m_bank2);
	space.install_write_bank(0x4000, 0x7fff, m_bank3);
	space.install_write_bank(0x8000, 0xbfff, m_bank4);
	space.install_write_bank(0xc000, 0xefff, m_bank5);
	space.install_write_bank(0xf000, 0xf3ff, m_bank6);
	space.install_write_bank(0xf400, 0xf7ff, m_bank7);
	space.install_write_bank(0xf800, 0xffff, m_bank8);

	if ((m_orionpro_dispatcher & 0x01)==0x00)
	{   // RAM0 segment disabled
		m_bank1->set_base(ram + 0x10000 * page);
		m_bank2->set_base(ram + 0x10000 * page + 0x2000);
	}
	else
	{
		m_bank1->set_base(ram + (m_orionpro_ram0_segment & 31) * 0x4000);
		m_bank2->set_base(ram + (m_orionpro_ram0_segment & 31) * 0x4000 + 0x2000);
	}
	if ((m_orionpro_dispatcher & 0x10)==0x10)
	{   // ROM1 enabled
		space.unmap_write(0x0000, 0x1fff);
		m_bank1->set_base(m_rom + 0x20000);
	}
	if ((m_orionpro_dispatcher & 0x08)==0x08)
	{   // ROM2 enabled
		space.unmap_write(0x2000, 0x3fff);
		m_bank2->set_base(m_rom + 0x22000 + (m_orionpro_rom2_segment & 7) * 0x2000);
	}

	if ((m_orionpro_dispatcher & 0x02)==0x00)
	{   // RAM1 segment disabled
		m_bank3->set_base(ram + 0x10000 * page + 0x4000);
	}
	else
	{
		m_bank3->set_base(ram + (m_orionpro_ram1_segment & 31) * 0x4000);
	}

	if ((m_orionpro_dispatcher & 0x04)==0x00)
	{   // RAM2 segment disabled
		m_bank4->set_base(ram + 0x10000 * page + 0x8000);
	}
	else
	{
		m_bank4->set_base(ram + (m_orionpro_ram2_segment & 31) * 0x4000);
	}

	m_bank5->set_base(ram + 0x10000 * page + 0xc000);

	if (is128)
	{
		m_bank6->set_base(ram + 0x10000 * 0 + 0xf000);

		space.install_write_handler(0xf400, 0xf4ff, write8sm_delegate(*this, FUNC(orion_state::orion128_system_w)));
		space.install_write_handler(0xf500, 0xf5ff, write8sm_delegate(*this, FUNC(orion_state::orion128_romdisk_w)));
		space.unmap_write(0xf600, 0xf6ff);
		space.install_write_handler(0xf700, 0xf7ff, write8sm_delegate(*this, FUNC(orion_state::orion128_floppy_w)));
		space.install_read_handler(0xf400, 0xf4ff, read8sm_delegate(*this, FUNC(orion_state::orion128_system_r)));
		space.install_read_handler(0xf500, 0xf5ff, read8sm_delegate(*this, FUNC(orion_state::orion128_romdisk_r)));
		space.unmap_read(0xf600, 0xf6ff);
		space.install_read_handler(0xf700, 0xf7ff, read8sm_delegate(*this, FUNC(orion_state::orion128_floppy_r)));

		space.install_write_handler(0xf800, 0xf8ff, write8smo_delegate(*this, FUNC(orion_state::orion128_video_mode_w)));
		space.install_write_handler(0xf900, 0xf9ff, write8smo_delegate(*this, FUNC(orion_state::orionpro_memory_page_w)));
		space.install_write_handler(0xfa00, 0xfaff, write8smo_delegate(*this, FUNC(orion_state::orion128_video_page_w)));
		space.unmap_write(0xfb00, 0xfeff);
		space.install_write_handler(0xff00, 0xffff, write8smo_delegate(*this, FUNC(orion_state::orionz80_sound_w)));


		m_bank8->set_base(ram + 0x10000 * 0 + 0xf800);
	}
	else
	{
		if ((m_orionpro_dispatcher & 0x40)==0x40)
		{   // FIX F000 enabled
			m_bank6->set_base(ram + 0x10000 * 0 + 0xf000);
			m_bank7->set_base(ram + 0x10000 * 0 + 0xf400);
			m_bank8->set_base(ram + 0x10000 * 0 + 0xf800);
		}
		else
		{
			m_bank6->set_base(ram + 0x10000 * page + 0xf000);
			m_bank7->set_base(ram + 0x10000 * page + 0xf400);
			m_bank8->set_base(ram + 0x10000 * page + 0xf800);
		}
	}
}

void orion_state::orionpro_memory_page_w(uint8_t data)
{
	m_orionpro_128_page = data;
	orionpro_bank_switch();
}

void orion_pro_state::machine_reset()
{
	radio86_init_keyboard();

	m_orion128_video_page = 0;
	m_orion128_video_mode = 0;
	m_orionpro_ram0_segment = 0;
	m_orionpro_ram1_segment = 0;
	m_orionpro_ram2_segment = 0;

	m_orionpro_page = 0;
	m_orionpro_128_page = 0;
	m_orionpro_rom2_segment = 0;

	m_orionpro_dispatcher = 0x50;
	orionpro_bank_switch();

	m_speaker_data = 0;
	m_orion128_video_width = SCREEN_WIDTH_384;
	orion_set_video_mode(384);

	m_video_mode_mask = 31;
	m_orionpro_pseudo_color = 0;
}

uint8_t orion_state::orionpro_io_r(offs_t offset)
{
	switch (offset & 0xff)
	{
		case 0x00 : return 0x56;
		case 0x04 : return m_orionpro_ram0_segment;
		case 0x05 : return m_orionpro_ram1_segment;
		case 0x06 : return m_orionpro_ram2_segment;
		case 0x08 : return m_orionpro_page;
		case 0x09 : return m_orionpro_rom2_segment;
		case 0x0a : return m_orionpro_dispatcher;
		case 0x10 : return m_fdc->status_r();
		case 0x11 : return m_fdc->track_r();
		case 0x12 : return m_fdc->sector_r();
		case 0x13 : return m_fdc->data_r();
		case 0x18 :
		case 0x19 :
		case 0x1a :
		case 0x1b :
					return orion128_system_r((offset & 0xff)-0x18);
		case 0x28 : return orion128_romdisk_r(0);
		case 0x29 : return orion128_romdisk_r(1);
		case 0x2a : return orion128_romdisk_r(2);
		case 0x2b : return orion128_romdisk_r(3);
	}
	if (offset == 0xFFFD)
	{
		return m_ay8912->data_r();
	}
	return 0xff;
}

void orion_state::orionpro_io_w(offs_t offset, uint8_t data)
{
	switch (offset & 0xff)
	{
		case 0x04 : m_orionpro_ram0_segment = data; orionpro_bank_switch(); break;
		case 0x05 : m_orionpro_ram1_segment = data; orionpro_bank_switch(); break;
		case 0x06 : m_orionpro_ram2_segment = data; orionpro_bank_switch(); break;
		case 0x08 : m_orionpro_page = data;       orionpro_bank_switch(); break;
		case 0x09 : m_orionpro_rom2_segment = data; orionpro_bank_switch(); break;
		case 0x0a : m_orionpro_dispatcher = data;   orionpro_bank_switch(); break;
		case 0x10 : m_fdc->cmd_w(data); break;
		case 0x11 : m_fdc->track_w(data);break;
		case 0x12 : m_fdc->sector_w(data);break;
		case 0x13 : m_fdc->data_w(data);break;
		case 0x14 : orion_disk_control_w(data);break;
		case 0x18 :
		case 0x19 :
		case 0x1a :
		case 0x1b :
					orion128_system_w((offset & 0xff)-0x18,data); break;
		case 0x28 : orion128_romdisk_w(0,data); break;
		case 0x29 : orion128_romdisk_w(1,data); break;
		case 0x2a : orion128_romdisk_w(2,data); break;
		case 0x2b : orion128_romdisk_w(3,data); break;
		case 0xf8 : orion128_video_mode_w(data);break;
		case 0xf9 : m_orionpro_128_page = data;   orionpro_bank_switch(); break;
		case 0xfa : orion128_video_page_w(data);break;
		case 0xfc : m_orionpro_pseudo_color = data;break;
		case 0xfe : orionz80_sound_fe_w(data);break;
		case 0xff : orionz80_sound_w(data);break;
	}
	switch(offset)
	{
		case 0xfffd : m_ay8912->address_w(data);
						break;
		case 0xbffd :
		case 0xbefd : m_ay8912->data_w(data);
						break;
	}
}
