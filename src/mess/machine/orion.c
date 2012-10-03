/***************************************************************************

        Orion machine driver by Miodrag Milanovic

        22/04/2008 Orion Pro added
        02/04/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/mc146818.h"
#include "machine/wd17xx.h"
#include "sound/speaker.h"
#include "sound/ay8910.h"
#include "includes/orion.h"
#include "includes/radio86.h"
#include "machine/ram.h"

#define SCREEN_WIDTH_384 48
#define SCREEN_WIDTH_480 60
#define SCREEN_WIDTH_512 64





READ8_MEMBER(orion_state::orion_romdisk_porta_r)
{
	UINT8 *romdisk = memregion("maincpu")->base() + 0x10000;
	return romdisk[m_romdisk_msb*256+m_romdisk_lsb];
}

WRITE8_MEMBER(orion_state::orion_romdisk_portb_w)
{
	m_romdisk_lsb = data;
}

WRITE8_MEMBER(orion_state::orion_romdisk_portc_w)
{
	m_romdisk_msb = data;
}

I8255A_INTERFACE( orion128_ppi8255_interface_1)
{
	DEVCB_DRIVER_MEMBER(orion_state,orion_romdisk_porta_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(orion_state,orion_romdisk_portb_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(orion_state,orion_romdisk_portc_w)
};


MACHINE_START_MEMBER(orion_state,orion128)
{
	m_video_mode_mask = 7;
}

READ8_MEMBER(orion_state::orion128_system_r)
{
	return machine().device<i8255_device>("ppi8255_2")->read(space, offset & 3);
}

WRITE8_MEMBER(orion_state::orion128_system_w)
{
	machine().device<i8255_device>("ppi8255_2")->write(space, offset & 3, data);
}

READ8_MEMBER(orion_state::orion128_romdisk_r)
{
	return machine().device<i8255_device>("ppi8255_1")->read(space, offset & 3);
}

WRITE8_MEMBER(orion_state::orion128_romdisk_w)
{
	machine().device<i8255_device>("ppi8255_1")->write(space, offset & 3, data);
}

static void orion_set_video_mode(running_machine &machine, int width)
{
		rectangle visarea(0, width-1, 0, 255);
		machine.primary_screen->configure(width, 256, visarea, machine.primary_screen->frame_period().attoseconds);
}

WRITE8_MEMBER(orion_state::orion128_video_mode_w)
{
	if ((data & 0x80)!=(m_orion128_video_mode & 0x80))
	{
		if ((data & 0x80)==0x80)
		{
			if (m_video_mode_mask == 31)
			{
				m_orion128_video_width = SCREEN_WIDTH_512;
				orion_set_video_mode(machine(),512);
			}
			else
			{
				m_orion128_video_width = SCREEN_WIDTH_480;
				orion_set_video_mode(machine(),480);
			}
		}
		else
		{
			m_orion128_video_width = SCREEN_WIDTH_384;
			orion_set_video_mode(machine(),384);
		}
	}

	m_orion128_video_mode = data;
}

WRITE8_MEMBER(orion_state::orion128_video_page_w)
{
	if (m_orion128_video_page != data)
	{
		if ((data & 0x80)!=(m_orion128_video_page & 0x80))
		{
			if ((data & 0x80)==0x80)
			{
				if (m_video_mode_mask == 31)
				{
					m_orion128_video_width = SCREEN_WIDTH_512;
					orion_set_video_mode(machine(),512);
				}
				else
				{
					m_orion128_video_width = SCREEN_WIDTH_480;
					orion_set_video_mode(machine(),480);
				}
			}
			else
			{
				m_orion128_video_width = SCREEN_WIDTH_384;
				orion_set_video_mode(machine(),384);
			}
		}
	}
	m_orion128_video_page = data;
}


WRITE8_MEMBER(orion_state::orion128_memory_page_w)
{
	if (data!=m_orion128_memory_page )
	{
		membank("bank1")->set_base(machine().device<ram_device>(RAM_TAG)->pointer() + (data & 3) * 0x10000);
		m_orion128_memory_page = (data & 3);
	}
}

MACHINE_RESET_MEMBER(orion_state,orion128)
{
	m_orion128_video_page = 0;
	m_orion128_video_mode = 0;
	m_orion128_memory_page = -1;
	membank("bank1")->set_base(memregion("maincpu")->base() + 0xf800);
	membank("bank2")->set_base(machine().device<ram_device>(RAM_TAG)->pointer() + 0xf000);
	m_orion128_video_width = SCREEN_WIDTH_384;
	orion_set_video_mode(machine(),384);
	radio86_init_keyboard(machine());
}

WRITE8_MEMBER(orion_state::orion_disk_control_w)
{
	device_t *fdc = machine().device("wd1793");

	wd17xx_set_side(fdc,((data & 0x10) >> 4) ^ 1);
	wd17xx_set_drive(fdc,data & 3);
}

READ8_MEMBER(orion_state::orion128_floppy_r)
{
	device_t *fdc = machine().device("wd1793");

	switch(offset)
	{
		case 0x0	:
		case 0x10 : return wd17xx_status_r(fdc,space, 0);
		case 0x1	:
		case 0x11 : return wd17xx_track_r(fdc,space, 0);
		case 0x2  :
		case 0x12 : return wd17xx_sector_r(fdc,space, 0);
		case 0x3  :
		case 0x13 : return wd17xx_data_r(fdc,space, 0);
	}
	return 0xff;
}

WRITE8_MEMBER(orion_state::orion128_floppy_w)
{
	device_t *fdc = machine().device("wd1793");

	switch(offset)
	{
		case 0x0	:
		case 0x10 : wd17xx_command_w(fdc,space, 0,data); break;
		case 0x1	:
		case 0x11 : wd17xx_track_w(fdc,space, 0,data);break;
		case 0x2  :
		case 0x12 : wd17xx_sector_w(fdc,space, 0,data);break;
		case 0x3  :
		case 0x13 : wd17xx_data_w(fdc,space, 0,data);break;
		case 0x4  :
		case 0x14 :
		case 0x20 : orion_disk_control_w(space, offset, data);break;
	}
}
READ8_MEMBER(orion_state::orionz80_floppy_rtc_r)
{
	if ((offset >= 0x60) && (offset <= 0x6f))
	{
		return machine().device<mc146818_device>("rtc")->read(space,offset-0x60);
	}
	else
	{
		return orion128_floppy_r(space,offset);
	}
}

WRITE8_MEMBER(orion_state::orionz80_floppy_rtc_w)
{
	if ((offset >= 0x60) && (offset <= 0x6f))
	{
		machine().device<mc146818_device>("rtc")->write(space,offset-0x60,data);
	}
	else
	{
		orion128_floppy_w(space,offset,data);
	}
}


MACHINE_START_MEMBER(orion_state,orionz80)
{
	m_video_mode_mask = 7;
}

WRITE8_MEMBER(orion_state::orionz80_sound_w)
{
	device_t *speaker = machine().device(SPEAKER_TAG);
	if (m_speaker == 0)
	{
		m_speaker = data;
	}
	else
	{
		m_speaker = 0 ;
	}
	speaker_level_w(speaker,m_speaker);

}

WRITE8_MEMBER(orion_state::orionz80_sound_fe_w)
{
	device_t *speaker = machine().device(SPEAKER_TAG);
	speaker_level_w(speaker,(data>>4) & 0x01);
}


static void orionz80_switch_bank(running_machine &machine)
{
	orion_state *state = machine.driver_data<orion_state>();
	UINT8 bank_select;
	UINT8 segment_select;
	address_space &space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	bank_select = (state->m_orionz80_dispatcher & 0x0c) >> 2;
	segment_select = state->m_orionz80_dispatcher & 0x03;

	space.install_write_bank(0x0000, 0x3fff, "bank1");
	if ((state->m_orionz80_dispatcher & 0x80)==0)
	{ // dispatcher on
		state->membank("bank1")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0x10000 * bank_select + segment_select * 0x4000 );
	}
	else
	{ // dispatcher off
		state->membank("bank1")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0x10000 * state->m_orionz80_memory_page);
	}

	state->membank("bank2")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0x4000 + 0x10000 * state->m_orionz80_memory_page);

	if ((state->m_orionz80_dispatcher & 0x20) == 0)
	{
		space.install_write_handler(0xf400, 0xf4ff, write8_delegate(FUNC(orion_state::orion128_system_w),state));
		space.install_write_handler(0xf500, 0xf5ff, write8_delegate(FUNC(orion_state::orion128_romdisk_w),state));
		space.install_write_handler(0xf700, 0xf7ff, write8_delegate(FUNC(orion_state::orionz80_floppy_rtc_w),state));
		space.install_read_handler(0xf400, 0xf4ff, read8_delegate(FUNC(orion_state::orion128_system_r),state));
		space.install_read_handler(0xf500, 0xf5ff, read8_delegate(FUNC(orion_state::orion128_romdisk_r),state));
		space.install_read_handler(0xf700, 0xf7ff, read8_delegate(FUNC(orion_state::orionz80_floppy_rtc_r),state));

		space.install_write_handler(0xf800, 0xf8ff, write8_delegate(FUNC(orion_state::orion128_video_mode_w),state));
		space.install_write_handler(0xf900, 0xf9ff, write8_delegate(FUNC(orion_state::orionz80_memory_page_w),state));
		space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(orion_state::orion128_video_page_w),state));
		space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(orion_state::orionz80_dispatcher_w),state));
		space.unmap_write(0xfc00, 0xfeff);
		space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(orion_state::orionz80_sound_w),state));

		state->membank("bank3")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0xf000);
		state->membank("bank5")->set_base(machine.root_device().memregion("maincpu")->base() + 0xf800);

	}
	else
	{
		/* if it is full memory access */
		state->membank("bank3")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0xf000 + 0x10000 * state->m_orionz80_memory_page);
		state->membank("bank4")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0xf400 + 0x10000 * state->m_orionz80_memory_page);
		state->membank("bank5")->set_base(machine.device<ram_device>(RAM_TAG)->pointer() + 0xf800 + 0x10000 * state->m_orionz80_memory_page);
	}
}

WRITE8_MEMBER(orion_state::orionz80_memory_page_w)
{
	m_orionz80_memory_page = data & 7;
	orionz80_switch_bank(machine());
}

WRITE8_MEMBER(orion_state::orionz80_dispatcher_w)
{
	m_orionz80_dispatcher = data;
	orionz80_switch_bank(machine());
}

MACHINE_RESET_MEMBER(orion_state,orionz80)
{
	address_space &space = machine().device("maincpu")->memory().space(AS_PROGRAM);

	space.unmap_write(0x0000, 0x3fff);
	space.install_write_bank(0x4000, 0xefff, "bank2");
	space.install_write_bank(0xf000, 0xf3ff, "bank3");

	space.install_write_handler(0xf400, 0xf4ff, write8_delegate(FUNC(orion_state::orion128_system_w),this));
	space.install_write_handler(0xf500, 0xf5ff, write8_delegate(FUNC(orion_state::orion128_romdisk_w),this));
	space.install_write_handler(0xf700, 0xf7ff, write8_delegate(FUNC(orion_state::orionz80_floppy_rtc_w),this));
	space.install_read_handler(0xf400, 0xf4ff, read8_delegate(FUNC(orion_state::orion128_system_r),this));
	space.install_read_handler(0xf500, 0xf5ff, read8_delegate(FUNC(orion_state::orion128_romdisk_r),this));
	space.install_read_handler(0xf700, 0xf7ff, read8_delegate(FUNC(orion_state::orionz80_floppy_rtc_r),this));

	space.install_write_handler(0xf800, 0xf8ff, write8_delegate(FUNC(orion_state::orion128_video_mode_w),this));
	space.install_write_handler(0xf900, 0xf9ff, write8_delegate(FUNC(orion_state::orionz80_memory_page_w),this));
	space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(orion_state::orion128_video_page_w),this));
	space.install_write_handler(0xfb00, 0xfbff, write8_delegate(FUNC(orion_state::orionz80_dispatcher_w),this));
	space.unmap_write(0xfc00, 0xfeff);
	space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(orion_state::orionz80_sound_w),this));


	membank("bank1")->set_base(machine().root_device().memregion("maincpu")->base() + 0xf800);
	membank("bank2")->set_base(machine().device<ram_device>(RAM_TAG)->pointer() + 0x4000);
	membank("bank3")->set_base(machine().device<ram_device>(RAM_TAG)->pointer() + 0xf000);
	membank("bank5")->set_base(memregion("maincpu")->base() + 0xf800);


	m_orion128_video_page = 0;
	m_orion128_video_mode = 0;
	m_orionz80_memory_page = 0;
	m_orionz80_dispatcher = 0;
	m_speaker = 0;
	m_orion128_video_width = SCREEN_WIDTH_384;
	orion_set_video_mode(machine(),384);
	radio86_init_keyboard(machine());
}

INTERRUPT_GEN_MEMBER(orion_state::orionz80_interrupt)
{
	if ((m_orionz80_dispatcher & 0x40)==0x40)
	{
		device.execute().set_input_line(0, HOLD_LINE);
	}
}

READ8_MEMBER(orion_state::orionz80_io_r)
{
	if (offset == 0xFFFD)
	{
		return ay8910_r (machine().device("ay8912"), space, 0);
	}
	return 0xff;
}

WRITE8_MEMBER(orion_state::orionz80_io_w)
{
	switch (offset & 0xff)
	{
		case 0xf8 : orion128_video_mode_w(space,0,data);break;
		case 0xf9 : orionz80_memory_page_w(space,0,data);break;
		case 0xfa : orion128_video_page_w(space,0,data);break;
		case 0xfb : orionz80_dispatcher_w(space,0,data);break;
		case 0xfe : orionz80_sound_fe_w(space,0,data);break;
		case 0xff : orionz80_sound_w(space,0,data);break;
	}
	switch(offset)
	{
		case 0xfffd : ay8910_address_w(machine().device("ay8912"), space, 0, data);
					  break;
		case 0xbffd :
		case 0xbefd : ay8910_data_w(machine().device("ay8912"), space, 0, data);
					  break;
	}
}







static void orionpro_bank_switch(running_machine &machine)
{
	orion_state *state = machine.driver_data<orion_state>();
	address_space &space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	int page = state->m_orionpro_page & 7; // we have only 8 pages
	int is128 = (state->m_orionpro_dispatcher & 0x80) ? 1 : 0;
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	if (is128==1)
	{
		page = state->m_orionpro_128_page & 7;
	}
	space.install_write_bank(0x0000, 0x1fff, "bank1");
	space.install_write_bank(0x2000, 0x3fff, "bank2");
	space.install_write_bank(0x4000, 0x7fff, "bank3");
	space.install_write_bank(0x8000, 0xbfff, "bank4");
	space.install_write_bank(0xc000, 0xefff, "bank5");
	space.install_write_bank(0xf000, 0xf3ff, "bank6");
	space.install_write_bank(0xf400, 0xf7ff, "bank7");
	space.install_write_bank(0xf800, 0xffff, "bank8");


	if ((state->m_orionpro_dispatcher & 0x01)==0x00)
	{	// RAM0 segment disabled
		state->membank("bank1")->set_base(ram + 0x10000 * page);
		state->membank("bank2")->set_base(ram + 0x10000 * page + 0x2000);
	}
	else
	{
		state->membank("bank1")->set_base(ram + (state->m_orionpro_ram0_segment & 31) * 0x4000);
		state->membank("bank2")->set_base(ram + (state->m_orionpro_ram0_segment & 31) * 0x4000 + 0x2000);
	}
	if ((state->m_orionpro_dispatcher & 0x10)==0x10)
	{	// ROM1 enabled
		space.unmap_write(0x0000, 0x1fff);
		state->membank("bank1")->set_base(machine.root_device().memregion("maincpu")->base() + 0x20000);
	}
	if ((state->m_orionpro_dispatcher & 0x08)==0x08)
	{	// ROM2 enabled
		space.unmap_write(0x2000, 0x3fff);
		state->membank("bank2")->set_base(machine.root_device().memregion("maincpu")->base() + 0x22000 + (state->m_orionpro_rom2_segment & 7) * 0x2000);
	}

	if ((state->m_orionpro_dispatcher & 0x02)==0x00)
	{	// RAM1 segment disabled
		state->membank("bank3")->set_base(ram + 0x10000 * page + 0x4000);
	}
	else
	{
		state->membank("bank3")->set_base(ram + (state->m_orionpro_ram1_segment & 31) * 0x4000);
	}

	if ((state->m_orionpro_dispatcher & 0x04)==0x00)
	{	// RAM2 segment disabled
		state->membank("bank4")->set_base(ram + 0x10000 * page + 0x8000);
	}
	else
	{
		state->membank("bank4")->set_base(ram + (state->m_orionpro_ram2_segment & 31) * 0x4000);
	}

	state->membank("bank5")->set_base(ram + 0x10000 * page + 0xc000);

	if (is128)
	{
		state->membank("bank6")->set_base(ram + 0x10000 * 0 + 0xf000);

		space.install_write_handler(0xf400, 0xf4ff, write8_delegate(FUNC(orion_state::orion128_system_w),state));
		space.install_write_handler(0xf500, 0xf5ff, write8_delegate(FUNC(orion_state::orion128_romdisk_w),state));
		space.unmap_write(0xf600, 0xf6ff);
		space.install_write_handler(0xf700, 0xf7ff, write8_delegate(FUNC(orion_state::orion128_floppy_w),state));
		space.install_read_handler(0xf400, 0xf4ff, read8_delegate(FUNC(orion_state::orion128_system_r),state));
		space.install_read_handler(0xf500, 0xf5ff, read8_delegate(FUNC(orion_state::orion128_romdisk_r),state));
		space.unmap_read(0xf600, 0xf6ff);
		space.install_read_handler(0xf700, 0xf7ff, read8_delegate(FUNC(orion_state::orion128_floppy_r),state));

		space.install_write_handler(0xf800, 0xf8ff, write8_delegate(FUNC(orion_state::orion128_video_mode_w),state));
		space.install_write_handler(0xf900, 0xf9ff, write8_delegate(FUNC(orion_state::orionpro_memory_page_w),state));
		space.install_write_handler(0xfa00, 0xfaff, write8_delegate(FUNC(orion_state::orion128_video_page_w),state));
		space.unmap_write(0xfb00, 0xfeff);
		space.install_write_handler(0xff00, 0xffff, write8_delegate(FUNC(orion_state::orionz80_sound_w),state));


		state->membank("bank8")->set_base(ram + 0x10000 * 0 + 0xf800);
	}
	else
	{
		if ((state->m_orionpro_dispatcher & 0x40)==0x40)
		{	// FIX F000 enabled
			state->membank("bank6")->set_base(ram + 0x10000 * 0 + 0xf000);
			state->membank("bank7")->set_base(ram + 0x10000 * 0 + 0xf400);
			state->membank("bank8")->set_base(ram + 0x10000 * 0 + 0xf800);
		}
		else
		{
			state->membank("bank6")->set_base(ram + 0x10000 * page + 0xf000);
			state->membank("bank7")->set_base(ram + 0x10000 * page + 0xf400);
			state->membank("bank8")->set_base(ram + 0x10000 * page + 0xf800);
		}
	}
}

WRITE8_MEMBER(orion_state::orionpro_memory_page_w)
{
	m_orionpro_128_page = data;
	orionpro_bank_switch(machine());
}

MACHINE_RESET_MEMBER(orion_state,orionpro)
{
	radio86_init_keyboard(machine());

	m_orion128_video_page = 0;
	m_orion128_video_mode = 0;
	m_orionpro_ram0_segment = 0;
	m_orionpro_ram1_segment = 0;
	m_orionpro_ram2_segment = 0;

	m_orionpro_page = 0;
	m_orionpro_128_page = 0;
	m_orionpro_rom2_segment = 0;

	m_orionpro_dispatcher = 0x50;
	orionpro_bank_switch(machine());

	m_speaker = 0;
	m_orion128_video_width = SCREEN_WIDTH_384;
	orion_set_video_mode(machine(),384);

	m_video_mode_mask = 31;
	m_orionpro_pseudo_color = 0;
}

READ8_MEMBER(orion_state::orionpro_io_r)
{
	device_t *fdc = machine().device("wd1793");

	switch (offset & 0xff)
	{
		case 0x00 : return 0x56;
		case 0x04 : return m_orionpro_ram0_segment;
		case 0x05 : return m_orionpro_ram1_segment;
		case 0x06 : return m_orionpro_ram2_segment;
		case 0x08 : return m_orionpro_page;
		case 0x09 : return m_orionpro_rom2_segment;
		case 0x0a : return m_orionpro_dispatcher;
		case 0x10 : return wd17xx_status_r(fdc,space, 0);
		case 0x11 : return wd17xx_track_r(fdc,space, 0);
		case 0x12 : return wd17xx_sector_r(fdc,space, 0);
		case 0x13 : return wd17xx_data_r(fdc,space, 0);
		case 0x18 :
		case 0x19 :
		case 0x1a :
		case 0x1b :
					return orion128_system_r(space,(offset & 0xff)-0x18);
		case 0x28 : return orion128_romdisk_r(space,0);
		case 0x29 : return orion128_romdisk_r(space,1);
		case 0x2a : return orion128_romdisk_r(space,2);
		case 0x2b : return orion128_romdisk_r(space,3);
	}
	if (offset == 0xFFFD)
	{
		return ay8910_r (machine().device("ay8912"), space, 0);
	}
	return 0xff;
}

WRITE8_MEMBER(orion_state::orionpro_io_w)
{
	device_t *fdc = machine().device("wd1793");

	switch (offset & 0xff)
	{
		case 0x04 : m_orionpro_ram0_segment = data; orionpro_bank_switch(machine()); break;
		case 0x05 : m_orionpro_ram1_segment = data; orionpro_bank_switch(machine()); break;
		case 0x06 : m_orionpro_ram2_segment = data; orionpro_bank_switch(machine()); break;
		case 0x08 : m_orionpro_page = data;		  orionpro_bank_switch(machine()); break;
		case 0x09 : m_orionpro_rom2_segment = data; orionpro_bank_switch(machine()); break;
		case 0x0a : m_orionpro_dispatcher = data;   orionpro_bank_switch(machine()); break;
		case 0x10 : wd17xx_command_w(fdc,space, 0,data); break;
		case 0x11 : wd17xx_track_w(fdc,space, 0,data);break;
		case 0x12 : wd17xx_sector_w(fdc,space, 0,data);break;
		case 0x13 : wd17xx_data_w(fdc,space, 0,data);break;
		case 0x14 : orion_disk_control_w(space, 9, data);break;
		case 0x18 :
		case 0x19 :
		case 0x1a :
		case 0x1b :
					orion128_system_w(space,(offset & 0xff)-0x18,data); break;
		case 0x28 : orion128_romdisk_w(space,0,data); break;
		case 0x29 : orion128_romdisk_w(space,1,data); break;
		case 0x2a : orion128_romdisk_w(space,2,data); break;
		case 0x2b : orion128_romdisk_w(space,3,data); break;
		case 0xf8 : orion128_video_mode_w(space,0,data);break;
		case 0xf9 : m_orionpro_128_page = data;	  orionpro_bank_switch(machine()); break;
		case 0xfa : orion128_video_page_w(space,0,data);break;
		case 0xfc : m_orionpro_pseudo_color = data;break;
		case 0xfe : orionz80_sound_fe_w(space,0,data);break;
		case 0xff : orionz80_sound_w(space,0,data);break;
	}
	switch(offset)
	{
		case 0xfffd : ay8910_address_w(machine().device("ay8912"), space, 0, data);
					  break;
		case 0xbffd :
		case 0xbefd : ay8910_data_w(machine().device("ay8912"), space, 0, data);
					  break;
	}
}
