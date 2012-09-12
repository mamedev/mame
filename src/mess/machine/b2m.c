/***************************************************************************

        Bashkiria-2M machine driver by Miodrag Milanovic

        28/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/wd17xx.h"
#include "machine/pic8259.h"
#include "machine/i8251.h"
#include "includes/b2m.h"
#include "machine/ram.h"
#include "imagedev/flopdrv.h"

READ8_MEMBER(b2m_state::b2m_keyboard_r)
{
	UINT8 key = 0x00;
	if (offset < 0x100) {
		if ((offset & 0x01)!=0) { key |= ioport("LINE0")->read(); }
		if ((offset & 0x02)!=0) { key |= ioport("LINE1")->read(); }
		if ((offset & 0x04)!=0) { key |= ioport("LINE2")->read(); }
		if ((offset & 0x08)!=0) { key |= ioport("LINE3")->read(); }
		if ((offset & 0x10)!=0) { key |= ioport("LINE4")->read(); }
		if ((offset & 0x20)!=0) { key |= ioport("LINE5")->read(); }
		if ((offset & 0x40)!=0) { key |= ioport("LINE6")->read(); }
		if ((offset & 0x80)!=0) { key |= ioport("LINE7")->read(); }
	} else {
		if ((offset & 0x01)!=0) { key |= ioport("LINE8")->read(); }
		if ((offset & 0x02)!=0) { key |= ioport("LINE9")->read(); }
		if ((offset & 0x04)!=0) { key |= ioport("LINE10")->read(); }
	}
	return key;
}


static void b2m_set_bank(running_machine &machine,int bank)
{
	UINT8 *rom;
	b2m_state *state =  machine.driver_data<b2m_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	space->install_write_bank(0x0000, 0x27ff, "bank1");
	space->install_write_bank(0x2800, 0x2fff, "bank2");
	space->install_write_bank(0x3000, 0x6fff, "bank3");
	space->install_write_bank(0x7000, 0xdfff, "bank4");
	space->install_write_bank(0xe000, 0xffff, "bank5");

	rom = state->memregion("maincpu")->base();
	switch(bank) {
		case 0 :
		case 1 :
						space->unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(ram);
						state->membank("bank2")->set_base(ram + 0x2800);
						state->membank("bank3")->set_base(ram + 0x3000);
						state->membank("bank4")->set_base(ram + 0x7000);
						state->membank("bank5")->set_base(rom + 0x10000);
						break;
#if 0
		case 1 :
						space->unmap_write(0x3000, 0x6fff);
						space->unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(ram);
						state->membank("bank2")->set_base(ram + 0x2800);
						state->membank("bank3")->set_base(rom + 0x12000);
						state->membank("bank4")->set_base(rom + 0x16000);
						state->membank("bank5")->set_base(rom + 0x10000);
						break;
#endif
		case 2 :
						space->unmap_write(0x2800, 0x2fff);
						space->unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(ram);
						space->install_read_handler(0x2800, 0x2fff, read8_delegate(FUNC(b2m_state::b2m_keyboard_r),state));
						state->membank("bank3")->set_base(ram + 0x10000);
						state->membank("bank4")->set_base(ram + 0x7000);
						state->membank("bank5")->set_base(rom + 0x10000);
						break;
		case 3 :
						space->unmap_write(0x2800, 0x2fff);
						space->unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(ram);
						space->install_read_handler(0x2800, 0x2fff, read8_delegate(FUNC(b2m_state::b2m_keyboard_r),state));
						state->membank("bank3")->set_base(ram + 0x14000);
						state->membank("bank4")->set_base(ram + 0x7000);
						state->membank("bank5")->set_base(rom + 0x10000);
						break;
		case 4 :
						space->unmap_write(0x2800, 0x2fff);
						space->unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(ram);
						space->install_read_handler(0x2800, 0x2fff, read8_delegate(FUNC(b2m_state::b2m_keyboard_r),state));
						state->membank("bank3")->set_base(ram + 0x18000);
						state->membank("bank4")->set_base(ram + 0x7000);
						state->membank("bank5")->set_base(rom + 0x10000);

						break;
		case 5 :
						space->unmap_write(0x2800, 0x2fff);
						space->unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(ram);
						space->install_read_handler(0x2800, 0x2fff, read8_delegate(FUNC(b2m_state::b2m_keyboard_r),state));
						state->membank("bank3")->set_base(ram + 0x1c000);
						state->membank("bank4")->set_base(ram + 0x7000);
						state->membank("bank5")->set_base(rom + 0x10000);

						break;
		case 6 :
						state->membank("bank1")->set_base(ram);
						state->membank("bank2")->set_base(ram + 0x2800);
						state->membank("bank3")->set_base(ram + 0x3000);
						state->membank("bank4")->set_base(ram + 0x7000);
						state->membank("bank5")->set_base(ram + 0xe000);
						break;
		case 7 :
						space->unmap_write(0x0000, 0x27ff);
						space->unmap_write(0x2800, 0x2fff);
						space->unmap_write(0x3000, 0x6fff);
						space->unmap_write(0x7000, 0xdfff);
						space->unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(rom + 0x10000);
						state->membank("bank2")->set_base(rom + 0x10000);
						state->membank("bank3")->set_base(rom + 0x10000);
						state->membank("bank4")->set_base(rom + 0x10000);
						state->membank("bank5")->set_base(rom + 0x10000);
						break;
	}
}


static WRITE_LINE_DEVICE_HANDLER(bm2_pit_out1)
{
	b2m_state *st =  device->machine().driver_data<b2m_state>();
	speaker_level_w(st->m_speaker, state);
}

const struct pit8253_config b2m_pit8253_intf =
{
	{
		{
			0,
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pic8259", pic8259_ir1_w)
		},
		{
			2000000,
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pit8253", bm2_pit_out1)
		},
		{
			2000000,
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pit8253", pit8253_clk0_w)
		}
	}
};

static WRITE8_DEVICE_HANDLER (b2m_8255_porta_w )
{
	b2m_state *state = device->machine().driver_data<b2m_state>();
	state->m_b2m_8255_porta = data;
}
static WRITE8_DEVICE_HANDLER (b2m_8255_portb_w )
{
	b2m_state *state = device->machine().driver_data<b2m_state>();
	state->m_b2m_video_scroll = data;
}

static WRITE8_DEVICE_HANDLER (b2m_8255_portc_w )
{
	b2m_state *state = device->machine().driver_data<b2m_state>();

	state->m_b2m_8255_portc = data;
	b2m_set_bank(device->machine(), state->m_b2m_8255_portc & 7);
	state->m_b2m_video_page = (state->m_b2m_8255_portc >> 7) & 1;
}

static READ8_DEVICE_HANDLER (b2m_8255_portb_r )
{
	b2m_state *state = device->machine().driver_data<b2m_state>();
	return state->m_b2m_video_scroll;
}

I8255A_INTERFACE( b2m_ppi8255_interface_1 )
{
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("ppi8255_1", b2m_8255_porta_w),
	DEVCB_DEVICE_HANDLER("ppi8255_1", b2m_8255_portb_r),
	DEVCB_DEVICE_HANDLER("ppi8255_1", b2m_8255_portb_w),
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("ppi8255_1", b2m_8255_portc_w)
};



static WRITE8_DEVICE_HANDLER (b2m_ext_8255_portc_w )
{
	UINT8 drive = ((data >> 1) & 1) ^ 1;
	UINT8 side  = (data  & 1) ^ 1;
	b2m_state *state = device->machine().driver_data<b2m_state>();
	floppy_mon_w(floppy_get_device(device->machine(), 0), 1);
	floppy_mon_w(floppy_get_device(device->machine(), 1), 1);

	if (state->m_b2m_drive!=drive) {
		wd17xx_set_drive(state->m_fdc,drive);
		floppy_mon_w(floppy_get_device(device->machine(), 0), 0);
		floppy_drive_set_ready_state(floppy_get_device(device->machine(), 0), 1, 1);
		state->m_b2m_drive = drive;
	}
	if (state->m_b2m_side!=side) {
		wd17xx_set_side(state->m_fdc,side);
		floppy_mon_w(floppy_get_device(device->machine(), 1), 0);
		floppy_drive_set_ready_state(floppy_get_device(device->machine(), 1), 1, 1);
		state->m_b2m_side = side;
	}
	wd17xx_dden_w(state->m_fdc, 0);
}

I8255A_INTERFACE( b2m_ppi8255_interface_2 )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("ppi8255_2", b2m_ext_8255_portc_w)
};

static READ8_DEVICE_HANDLER (b2m_romdisk_porta_r )
{
	b2m_state *state = device->machine().driver_data<b2m_state>();

	UINT8 *romdisk = state->memregion("maincpu")->base() + 0x12000;
	return romdisk[state->m_b2m_romdisk_msb*256+state->m_b2m_romdisk_lsb];
}

static WRITE8_DEVICE_HANDLER (b2m_romdisk_portb_w )
{
	b2m_state *state = device->machine().driver_data<b2m_state>();
	state->m_b2m_romdisk_lsb = data;
}

static WRITE8_DEVICE_HANDLER (b2m_romdisk_portc_w )
{
	b2m_state *state = device->machine().driver_data<b2m_state>();
	state->m_b2m_romdisk_msb = data & 0x7f;
}

I8255A_INTERFACE( b2m_ppi8255_interface_3 )
{
	DEVCB_DEVICE_HANDLER("ppi8255_3", b2m_romdisk_porta_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("ppi8255_3", b2m_romdisk_portb_w),
	DEVCB_NULL,
	DEVCB_DEVICE_HANDLER("ppi8255_3", b2m_romdisk_portc_w)
};

static WRITE_LINE_DEVICE_HANDLER( b2m_pic_set_int_line )
{
	device->machine().device("maincpu")->execute().set_input_line(0, state ?  HOLD_LINE : CLEAR_LINE);
}

/* Driver initialization */
DRIVER_INIT_MEMBER(b2m_state,b2m)
{
	m_vblank_state = 0;
}

WRITE8_MEMBER(b2m_state::b2m_palette_w)
{

	UINT8 b = (3 - ((data >> 6) & 3)) * 0x55;
	UINT8 g = (3 - ((data >> 4) & 3)) * 0x55;
	UINT8 r = (3 - ((data >> 2) & 3)) * 0x55;

	UINT8 bw = (3 - (data & 3)) * 0x55;

	m_b2m_color[offset & 3] = data;

	if (ioport("MONITOR")->read()==1) {
		palette_set_color_rgb(machine(),offset, r, g, b);
	} else {
		palette_set_color_rgb(machine(),offset, bw, bw, bw);
	}
}

READ8_MEMBER(b2m_state::b2m_palette_r)
{
	return m_b2m_color[offset];
}

WRITE8_MEMBER(b2m_state::b2m_localmachine_w)
{
	m_b2m_localmachine = data;
}

READ8_MEMBER(b2m_state::b2m_localmachine_r)
{
	return m_b2m_localmachine;
}

static void b2m_postload(b2m_state *state)
{
	b2m_set_bank(state->machine(), state->m_b2m_8255_portc & 7);
}

MACHINE_START(b2m)
{
	b2m_state *state = machine.driver_data<b2m_state>();
	state->m_pic = machine.device("pic8259");
	state->m_fdc = machine.device("wd1793");
	state->m_speaker = machine.device(SPEAKER_TAG);

	wd17xx_set_pause_time(state->m_fdc,10);

	/* register for state saving */
	state->save_item(NAME(state->m_b2m_8255_porta));
	state->save_item(NAME(state->m_b2m_video_scroll));
	state->save_item(NAME(state->m_b2m_8255_portc));
	state->save_item(NAME(state->m_b2m_video_page));
	state->save_item(NAME(state->m_b2m_drive));
	state->save_item(NAME(state->m_b2m_side));
	state->save_item(NAME(state->m_b2m_romdisk_lsb));
	state->save_item(NAME(state->m_b2m_romdisk_msb));
	state->save_pointer(NAME(state->m_b2m_color), 4);
	state->save_item(NAME(state->m_b2m_localmachine));
	state->save_item(NAME(state->m_vblank_state));

	machine.save().register_postload(save_prepost_delegate(FUNC(b2m_postload), state));
}

static IRQ_CALLBACK(b2m_irq_callback)
{
	b2m_state *state = device->machine().driver_data<b2m_state>();
	return pic8259_acknowledge(state->m_pic);
}

const struct pic8259_interface b2m_pic8259_config =
{
	DEVCB_LINE(b2m_pic_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};

INTERRUPT_GEN( b2m_vblank_interrupt )
{
	b2m_state *state = device->machine().driver_data<b2m_state>();
	state->m_vblank_state++;
	if (state->m_vblank_state>1) state->m_vblank_state=0;
	pic8259_ir0_w(state->m_pic, state->m_vblank_state);
}

MACHINE_RESET(b2m)
{
	b2m_state *state = machine.driver_data<b2m_state>();
	state->m_b2m_side = 0;
	state->m_b2m_drive = 0;

	machine.device("maincpu")->execute().set_irq_acknowledge_callback(b2m_irq_callback);
	b2m_set_bank(machine, 7);
}
