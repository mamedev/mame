/***************************************************************************

        Partner driver by Miodrag Milanovic

        09/06/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/8257dma.h"
#include "machine/wd17xx.h"
#include "video/i8275.h"
#include "includes/radio86.h"
#include "includes/partner.h"
#include "imagedev/flopdrv.h"
#include "machine/ram.h"


/* Driver initialization */
DRIVER_INIT_MEMBER(partner_state,partner)
{
	m_tape_value = 0x80;
}

static WRITE_LINE_DEVICE_HANDLER( partner_wd17xx_drq_w )
{
	if (state)
		i8257_drq0_w(device, 1);
}

const wd17xx_interface partner_wd17xx_interface =
{
	DEVCB_LINE_GND,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE("dma8257", partner_wd17xx_drq_w),
	{FLOPPY_0, FLOPPY_1, NULL, NULL}
};

MACHINE_START_MEMBER(partner_state,partner)
{
	device_t *fdc = machine().device("wd1793");
	wd17xx_set_pause_time(fdc, 10);
}

static void partner_window_1(running_machine &machine, UINT8 bank_num, UINT16 offset,UINT8 *rom)
{
	partner_state *state = machine.driver_data<partner_state>();
	char bank[10];
	sprintf(bank,"bank%d",bank_num);
	switch(state->m_win_mem_page) {
		case 2 : // FDD BIOS
				state->membank(bank)->set_base(rom + 0x16000 + offset);
				break;
		case 4 : // MCPG BIOS
				state->membank(bank)->set_base(rom + 0x14000 + offset);
				break;
		default : // BIOS
				state->membank(bank)->set_base(rom + 0x10000 + offset);
				break;
	}
}

static void partner_window_2(running_machine &machine, UINT8 bank_num, UINT16 offset,UINT8 *rom)
{
	partner_state *state = machine.driver_data<partner_state>();
	char bank[10];
	sprintf(bank,"bank%d",bank_num);
	switch(state->m_win_mem_page) {
		case 4 : // MCPG FONT
				state->membank(bank)->set_base(rom + 0x18000 + offset);
				break;
		default : // BIOS
				state->membank(bank)->set_base(rom + 0x10000 + offset);
				break;
	}
}

READ8_MEMBER(partner_state::partner_floppy_r){
	device_t *fdc = machine().device("wd1793");

	if (offset<0x100) {
		switch(offset & 3) {
			case 0x00 : return wd17xx_status_r(fdc,space, 0);
			case 0x01 : return wd17xx_track_r(fdc,space, 0);
			case 0x02 : return wd17xx_sector_r(fdc,space, 0);
			default   :
						return wd17xx_data_r(fdc,space, 0);
		}
	} else {
		return 0;
	}
}

WRITE8_MEMBER(partner_state::partner_floppy_w){
	device_t *fdc = machine().device("wd1793");

	if (offset<0x100) {
		switch(offset & 3) {
			case 0x00 : wd17xx_command_w(fdc,space, 0,data); break;
			case 0x01 : wd17xx_track_w(fdc,space, 0,data);break;
			case 0x02 : wd17xx_sector_w(fdc,space, 0,data);break;
			default   : wd17xx_data_w(fdc,space, 0,data);break;
		}
	} else {
		floppy_mon_w(floppy_get_device(machine(), 0), 1);
		floppy_mon_w(floppy_get_device(machine(), 1), 1);
		if (((data >> 6) & 1)==1) {
			wd17xx_set_drive(fdc,0);
			floppy_mon_w(floppy_get_device(machine(), 0), 0);
			floppy_drive_set_ready_state(floppy_get_device(machine(), 0), 1, 1);
		}
		if (((data >> 3) & 1)==1) {
			wd17xx_set_drive(fdc,1);
			floppy_mon_w(floppy_get_device(machine(), 1), 0);
			floppy_drive_set_ready_state(floppy_get_device(machine(), 1), 1, 1);
		}
		wd17xx_set_side(fdc,data >> 7);
	}
}

static void partner_iomap_bank(running_machine &machine,UINT8 *rom)
{
	partner_state *state = machine.driver_data<partner_state>();
	address_space &space = *machine.device("maincpu")->memory().space(AS_PROGRAM);
	switch(state->m_win_mem_page) {
		case 2 :
				// FDD
				space.install_write_handler(0xdc00, 0xddff, write8_delegate(FUNC(partner_state::partner_floppy_w),state));
				space.install_read_handler (0xdc00, 0xddff, read8_delegate(FUNC(partner_state::partner_floppy_r),state));
				break;
		case 4 :
				// Timer
				break;
		default : // BIOS
				state->membank("bank11")->set_base(rom + 0x10000);
				break;
	}
}
static void partner_bank_switch(running_machine &machine)
{
	partner_state *state = machine.driver_data<partner_state>();
	address_space &space = *machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *rom = state->memregion("maincpu")->base();
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	space.install_write_bank(0x0000, 0x07ff, "bank1");
	space.install_write_bank(0x0800, 0x3fff, "bank2");
	space.install_write_bank(0x4000, 0x5fff, "bank3");
	space.install_write_bank(0x6000, 0x7fff, "bank4");
	space.install_write_bank(0x8000, 0x9fff, "bank5");
	space.install_write_bank(0xa000, 0xb7ff, "bank6");
	space.install_write_bank(0xb800, 0xbfff, "bank7");
	space.install_write_bank(0xc000, 0xc7ff, "bank8");
	space.install_write_bank(0xc800, 0xcfff, "bank9");
	space.install_write_bank(0xd000, 0xd7ff, "bank10");
	space.unmap_write(0xdc00, 0xddff);
	space.install_read_bank (0xdc00, 0xddff, "bank11");
	space.unmap_write(0xe000, 0xe7ff);
	space.unmap_write(0xe800, 0xffff);

	// BANK 1 (0x0000 - 0x07ff)
	if (state->m_mem_page==0) {
		space.unmap_write(0x0000, 0x07ff);
		state->membank("bank1")->set_base(rom + 0x10000);
	} else {
		if (state->m_mem_page==7) {
			state->membank("bank1")->set_base(ram + 0x8000);
		} else {
			state->membank("bank1")->set_base(ram + 0x0000);
		}
	}

	// BANK 2 (0x0800 - 0x3fff)
	if (state->m_mem_page==7) {
		state->membank("bank2")->set_base(ram + 0x8800);
	} else {
		state->membank("bank2")->set_base(ram + 0x0800);
	}

	// BANK 3 (0x4000 - 0x5fff)
	if (state->m_mem_page==7) {
		state->membank("bank3")->set_base(ram + 0xC000);
	} else {
		if (state->m_mem_page==10) {
			//window 1
			space.unmap_write(0x4000, 0x5fff);
			partner_window_1(machine, 3, 0, rom);
		} else {
			state->membank("bank3")->set_base(ram + 0x4000);
		}
	}

	// BANK 4 (0x6000 - 0x7fff)
	if (state->m_mem_page==7) {
		state->membank("bank4")->set_base(ram + 0xe000);
	} else {
		state->membank("bank4")->set_base(ram + 0x6000);
	}

	// BANK 5 (0x8000 - 0x9fff)
	switch (state->m_mem_page) {
		case 5:
		case 10:
				//window 2
				space.unmap_write(0x8000, 0x9fff);
				partner_window_2(machine, 5, 0, rom);
				break;
		case 8:
		case 9:
				//window 1
				space.unmap_write(0x8000, 0x9fff);
				partner_window_1(machine, 5, 0, rom);
				break;
		case 7:
				state->membank("bank5")->set_base(ram + 0x0000);
				break;
		default:
				state->membank("bank5")->set_base(ram + 0x8000);
				break;
	}

	// BANK 6 (0xa000 - 0xb7ff)
	switch (state->m_mem_page) {
		case 5:
		case 10:
				//window 2
				space.unmap_write(0xa000, 0xb7ff);
				partner_window_2(machine, 6, 0, rom);
				break;
		case 6:
		case 8:
				//BASIC
				space.unmap_write(0xa000, 0xb7ff);
				state->membank("bank6")->set_base(rom + 0x12000); // BASIC
				break;
		case 7:
				state->membank("bank6")->set_base(ram + 0x2000);
				break;
		default:
				state->membank("bank6")->set_base(ram + 0xa000);
				break;
	}

	// BANK 7 (0xb800 - 0xbfff)
	switch (state->m_mem_page) {
		case 4:
		case 5:
		case 10:
				//window 2
				space.unmap_write(0xb800, 0xbfff);
				partner_window_2(machine, 7, 0x1800, rom);
				break;
		case 6:
		case 8:
				//BASIC
				space.unmap_write(0xb800, 0xbfff);
				state->membank("bank7")->set_base(rom + 0x13800); // BASIC
				break;
		case 7:
				state->membank("bank7")->set_base(ram + 0x3800);
				break;
		default:
				state->membank("bank7")->set_base(ram + 0xb800);
				break;
	}

	// BANK 8 (0xc000 - 0xc7ff)
	switch (state->m_mem_page) {
		case 7:
				state->membank("bank8")->set_base(ram + 0x4000);
				break;
		case 8:
		case 10:
				space.unmap_write(0xc000, 0xc7ff);
				state->membank("bank8")->set_base(rom + 0x10000);
				break;
		default:
				state->membank("bank8")->set_base(ram + 0xc000);
				break;
	}

	// BANK 9 (0xc800 - 0xcfff)
	switch (state->m_mem_page) {
		case 7:
				state->membank("bank9")->set_base(ram + 0x4800);
				break;
		case 8:
		case 9:
				// window 2
				space.unmap_write(0xc800, 0xcfff);
				partner_window_2(machine, 9, 0, rom);
				break;
		case 10:
				space.unmap_write(0xc800, 0xcfff);
				state->membank("bank9")->set_base(rom + 0x10800);
				break;
		default:
				state->membank("bank9")->set_base(ram + 0xc800);
				break;
	}

	// BANK 10 (0xd000 - 0xd7ff)
	switch (state->m_mem_page) {
		case 7:
				state->membank("bank10")->set_base(ram + 0x5000);
				break;
		case 8:
		case 9:
				// window 2
				space.unmap_write(0xd000, 0xd7ff);
				partner_window_2(machine, 10, 0x0800, rom);
				break;
		default:
				state->membank("bank10")->set_base(ram + 0xd000);
				break;
	}

	// BANK 11 (0xdc00 - 0xddff)
	partner_iomap_bank(machine,rom);

	// BANK 12 (0xe000 - 0xe7ff)
	if (state->m_mem_page==1) {
		state->membank("bank12")->set_base(rom + 0x10000);
	} else {
		//window 1
		partner_window_1(machine, 12, 0, rom);
	}

	// BANK 13 (0xe800 - 0xffff)
	switch (state->m_mem_page) {
		case 3:
		case 4:
		case 5:
				// window 1
				partner_window_1(machine, 13, 0x800, rom);
				break;
		default:
				// BIOS
				state->membank("bank13")->set_base(rom + 0x10800);
				break;
	}
}

WRITE8_MEMBER(partner_state::partner_win_memory_page_w)
{
	m_win_mem_page = ~data;
	partner_bank_switch(machine());
}

WRITE8_MEMBER(partner_state::partner_mem_page_w)
{
	m_mem_page = (data >> 4) & 0x0f;
	partner_bank_switch(machine());
}

static WRITE_LINE_DEVICE_HANDLER( hrq_w )
{
	/* HACK - this should be connected to the BUSREQ line of Z80 */
	device->machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the BUSACK line of Z80 */
	i8257_hlda_w(device, state);
}

static UINT8 memory_read_byte(address_space &space, offs_t address) { return space.read_byte(address); }
static void memory_write_byte(address_space &space, offs_t address, UINT8 data) { space.write_byte(address, data); }

I8257_INTERFACE( partner_dma )
{
	DEVCB_LINE(hrq_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, memory_read_byte),
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, memory_write_byte),
	{ DEVCB_DEVICE_HANDLER("wd1793", wd17xx_data_r), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_DEVICE_HANDLER("wd1793", wd17xx_data_w), DEVCB_NULL, DEVCB_DEVICE_HANDLER("i8275", i8275_dack_w), DEVCB_NULL }
};


MACHINE_RESET_MEMBER(partner_state,partner)
{
	m_mem_page = 0;
	m_win_mem_page = 0;
	partner_bank_switch(machine());
}
