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

WRITE_LINE_MEMBER(partner_state::partner_wd17xx_drq_w)
{
	i8257_device *device = machine().device<i8257_device>("dma8257");
	if (state)
		device->i8257_drq0_w(1);
}

const wd17xx_interface partner_wd17xx_interface =
{
	DEVCB_LINE_GND,
	DEVCB_NULL,
	DEVCB_DRIVER_LINE_MEMBER(partner_state,partner_wd17xx_drq_w),
	{FLOPPY_0, FLOPPY_1, NULL, NULL}
};

MACHINE_START_MEMBER(partner_state,partner)
{
	m_fdc->set_pause_time(10);
}

void partner_state::partner_window_1(UINT8 bank_num, UINT16 offset,UINT8 *rom)
{
	char bank[10];
	sprintf(bank,"bank%d",bank_num);
	switch(m_win_mem_page) {
		case 2 : // FDD BIOS
				membank(bank)->set_base(rom + 0x16000 + offset);
				break;
		case 4 : // MCPG BIOS
				membank(bank)->set_base(rom + 0x14000 + offset);
				break;
		default : // BIOS
				membank(bank)->set_base(rom + 0x10000 + offset);
				break;
	}
}

void partner_state::partner_window_2(UINT8 bank_num, UINT16 offset,UINT8 *rom)
{
	char bank[10];
	sprintf(bank,"bank%d",bank_num);
	switch(m_win_mem_page) {
		case 4 : // MCPG FONT
				membank(bank)->set_base(rom + 0x18000 + offset);
				break;
		default : // BIOS
				membank(bank)->set_base(rom + 0x10000 + offset);
				break;
	}
}

READ8_MEMBER(partner_state::partner_floppy_r){
	if (offset<0x100) {
		switch(offset & 3) {
			case 0x00 : return m_fdc->status_r(space, 0);
			case 0x01 : return m_fdc->track_r(space, 0);
			case 0x02 : return m_fdc->sector_r(space, 0);
			default   :
						return m_fdc->data_r(space, 0);
		}
	} else {
		return 0;
	}
}

WRITE8_MEMBER(partner_state::partner_floppy_w){
	if (offset<0x100) {
		switch(offset & 3) {
			case 0x00 : m_fdc->command_w(space, 0,data); break;
			case 0x01 : m_fdc->track_w(space, 0,data);break;
			case 0x02 : m_fdc->sector_w(space, 0,data);break;
			default   : m_fdc->data_w(space, 0,data);break;
		}
	} else {
		floppy_get_device(machine(), 0)->floppy_mon_w(1);
		floppy_get_device(machine(), 1)->floppy_mon_w(1);
		if (((data >> 6) & 1)==1) {
			m_fdc->set_drive(0);
			floppy_get_device(machine(), 0)->floppy_mon_w(0);
			floppy_get_device(machine(), 0)->floppy_drive_set_ready_state(1, 1);
		}
		if (((data >> 3) & 1)==1) {
			m_fdc->set_drive(1);
			floppy_get_device(machine(), 1)->floppy_mon_w(0);
			floppy_get_device(machine(), 1)->floppy_drive_set_ready_state(1, 1);
		}
		m_fdc->set_side(data >> 7);
	}
}

void partner_state::partner_iomap_bank(UINT8 *rom)
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	switch(m_win_mem_page) {
		case 2 :
				// FDD
				space.install_write_handler(0xdc00, 0xddff, write8_delegate(FUNC(partner_state::partner_floppy_w),this));
				space.install_read_handler (0xdc00, 0xddff, read8_delegate(FUNC(partner_state::partner_floppy_r),this));
				break;
		case 4 :
				// Timer
				break;
		default : // BIOS
				membank("bank11")->set_base(rom + 0x10000);
				break;
	}
}
void partner_state::partner_bank_switch()
{
	address_space &space = m_maincpu->space(AS_PROGRAM);
	UINT8 *rom = memregion("maincpu")->base();
	UINT8 *ram = m_ram->pointer();

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
	if (m_mem_page==0) {
		space.unmap_write(0x0000, 0x07ff);
		membank("bank1")->set_base(rom + 0x10000);
	} else {
		if (m_mem_page==7) {
			membank("bank1")->set_base(ram + 0x8000);
		} else {
			membank("bank1")->set_base(ram + 0x0000);
		}
	}

	// BANK 2 (0x0800 - 0x3fff)
	if (m_mem_page==7) {
		membank("bank2")->set_base(ram + 0x8800);
	} else {
		membank("bank2")->set_base(ram + 0x0800);
	}

	// BANK 3 (0x4000 - 0x5fff)
	if (m_mem_page==7) {
		membank("bank3")->set_base(ram + 0xC000);
	} else {
		if (m_mem_page==10) {
			//window 1
			space.unmap_write(0x4000, 0x5fff);
			partner_window_1(3, 0, rom);
		} else {
			membank("bank3")->set_base(ram + 0x4000);
		}
	}

	// BANK 4 (0x6000 - 0x7fff)
	if (m_mem_page==7) {
		membank("bank4")->set_base(ram + 0xe000);
	} else {
		membank("bank4")->set_base(ram + 0x6000);
	}

	// BANK 5 (0x8000 - 0x9fff)
	switch (m_mem_page) {
		case 5:
		case 10:
				//window 2
				space.unmap_write(0x8000, 0x9fff);
				partner_window_2(5, 0, rom);
				break;
		case 8:
		case 9:
				//window 1
				space.unmap_write(0x8000, 0x9fff);
				partner_window_1(5, 0, rom);
				break;
		case 7:
				membank("bank5")->set_base(ram + 0x0000);
				break;
		default:
				membank("bank5")->set_base(ram + 0x8000);
				break;
	}

	// BANK 6 (0xa000 - 0xb7ff)
	switch (m_mem_page) {
		case 5:
		case 10:
				//window 2
				space.unmap_write(0xa000, 0xb7ff);
				partner_window_2(6, 0, rom);
				break;
		case 6:
		case 8:
				//BASIC
				space.unmap_write(0xa000, 0xb7ff);
				membank("bank6")->set_base(rom + 0x12000); // BASIC
				break;
		case 7:
				membank("bank6")->set_base(ram + 0x2000);
				break;
		default:
				membank("bank6")->set_base(ram + 0xa000);
				break;
	}

	// BANK 7 (0xb800 - 0xbfff)
	switch (m_mem_page) {
		case 4:
		case 5:
		case 10:
				//window 2
				space.unmap_write(0xb800, 0xbfff);
				partner_window_2(7, 0x1800, rom);
				break;
		case 6:
		case 8:
				//BASIC
				space.unmap_write(0xb800, 0xbfff);
				membank("bank7")->set_base(rom + 0x13800); // BASIC
				break;
		case 7:
				membank("bank7")->set_base(ram + 0x3800);
				break;
		default:
				membank("bank7")->set_base(ram + 0xb800);
				break;
	}

	// BANK 8 (0xc000 - 0xc7ff)
	switch (m_mem_page) {
		case 7:
				membank("bank8")->set_base(ram + 0x4000);
				break;
		case 8:
		case 10:
				space.unmap_write(0xc000, 0xc7ff);
				membank("bank8")->set_base(rom + 0x10000);
				break;
		default:
				membank("bank8")->set_base(ram + 0xc000);
				break;
	}

	// BANK 9 (0xc800 - 0xcfff)
	switch (m_mem_page) {
		case 7:
				membank("bank9")->set_base(ram + 0x4800);
				break;
		case 8:
		case 9:
				// window 2
				space.unmap_write(0xc800, 0xcfff);
				partner_window_2(9, 0, rom);
				break;
		case 10:
				space.unmap_write(0xc800, 0xcfff);
				membank("bank9")->set_base(rom + 0x10800);
				break;
		default:
				membank("bank9")->set_base(ram + 0xc800);
				break;
	}

	// BANK 10 (0xd000 - 0xd7ff)
	switch (m_mem_page) {
		case 7:
				membank("bank10")->set_base(ram + 0x5000);
				break;
		case 8:
		case 9:
				// window 2
				space.unmap_write(0xd000, 0xd7ff);
				partner_window_2(10, 0x0800, rom);
				break;
		default:
				membank("bank10")->set_base(ram + 0xd000);
				break;
	}

	// BANK 11 (0xdc00 - 0xddff)
	partner_iomap_bank(rom);

	// BANK 12 (0xe000 - 0xe7ff)
	if (m_mem_page==1) {
		membank("bank12")->set_base(rom + 0x10000);
	} else {
		//window 1
		partner_window_1(12, 0, rom);
	}

	// BANK 13 (0xe800 - 0xffff)
	switch (m_mem_page) {
		case 3:
		case 4:
		case 5:
				// window 1
				partner_window_1(13, 0x800, rom);
				break;
		default:
				// BIOS
				membank("bank13")->set_base(rom + 0x10800);
				break;
	}
}

WRITE8_MEMBER(partner_state::partner_win_memory_page_w)
{
	m_win_mem_page = ~data;
	partner_bank_switch();
}

WRITE8_MEMBER(partner_state::partner_mem_page_w)
{
	m_mem_page = (data >> 4) & 0x0f;
	partner_bank_switch();
}

WRITE_LINE_MEMBER(partner_state::hrq_w)
{
	/* HACK - this should be connected to the BUSREQ line of Z80 */
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the BUSACK line of Z80 */
	machine().device<i8257_device>("dma8257")->i8257_hlda_w(state);
}

READ8_MEMBER(partner_state::partner_fdc_r)
{
	return m_fdc->data_r(space,offset);
}
WRITE8_MEMBER(partner_state::partner_fdc_w)
{
	m_fdc->data_w(space,offset,data);
}

I8257_INTERFACE( partner_dma )
{
	DEVCB_DRIVER_LINE_MEMBER(partner_state,hrq_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(radio86_state, memory_read_byte),
	DEVCB_DRIVER_MEMBER(radio86_state, memory_write_byte),
	{ DEVCB_DRIVER_MEMBER(partner_state, partner_fdc_r), DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_DRIVER_MEMBER(partner_state, partner_fdc_w), DEVCB_NULL, DEVCB_DEVICE_MEMBER("i8275", i8275_device, dack_w), DEVCB_NULL }
};


MACHINE_RESET_MEMBER(partner_state,partner)
{
	m_mem_page = 0;
	m_win_mem_page = 0;
	partner_bank_switch();
}
