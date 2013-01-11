/***************************************************************************

        Bashkiria-2M machine driver by Miodrag Milanovic

        28/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/wd_fdc.h"
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
	address_space &space = machine.device("maincpu")->memory().space(AS_PROGRAM);
	UINT8 *ram = machine.device<ram_device>(RAM_TAG)->pointer();

	space.install_write_bank(0x0000, 0x27ff, "bank1");
	space.install_write_bank(0x2800, 0x2fff, "bank2");
	space.install_write_bank(0x3000, 0x6fff, "bank3");
	space.install_write_bank(0x7000, 0xdfff, "bank4");
	space.install_write_bank(0xe000, 0xffff, "bank5");

	rom = state->memregion("maincpu")->base();
	switch(bank) {
		case 0 :
		case 1 :
						space.unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(ram);
						state->membank("bank2")->set_base(ram + 0x2800);
						state->membank("bank3")->set_base(ram + 0x3000);
						state->membank("bank4")->set_base(ram + 0x7000);
						state->membank("bank5")->set_base(rom + 0x10000);
						break;
#if 0
		case 1 :
						space.unmap_write(0x3000, 0x6fff);
						space.unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(ram);
						state->membank("bank2")->set_base(ram + 0x2800);
						state->membank("bank3")->set_base(rom + 0x12000);
						state->membank("bank4")->set_base(rom + 0x16000);
						state->membank("bank5")->set_base(rom + 0x10000);
						break;
#endif
		case 2 :
						space.unmap_write(0x2800, 0x2fff);
						space.unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(ram);
						space.install_read_handler(0x2800, 0x2fff, read8_delegate(FUNC(b2m_state::b2m_keyboard_r),state));
						state->membank("bank3")->set_base(ram + 0x10000);
						state->membank("bank4")->set_base(ram + 0x7000);
						state->membank("bank5")->set_base(rom + 0x10000);
						break;
		case 3 :
						space.unmap_write(0x2800, 0x2fff);
						space.unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(ram);
						space.install_read_handler(0x2800, 0x2fff, read8_delegate(FUNC(b2m_state::b2m_keyboard_r),state));
						state->membank("bank3")->set_base(ram + 0x14000);
						state->membank("bank4")->set_base(ram + 0x7000);
						state->membank("bank5")->set_base(rom + 0x10000);
						break;
		case 4 :
						space.unmap_write(0x2800, 0x2fff);
						space.unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(ram);
						space.install_read_handler(0x2800, 0x2fff, read8_delegate(FUNC(b2m_state::b2m_keyboard_r),state));
						state->membank("bank3")->set_base(ram + 0x18000);
						state->membank("bank4")->set_base(ram + 0x7000);
						state->membank("bank5")->set_base(rom + 0x10000);

						break;
		case 5 :
						space.unmap_write(0x2800, 0x2fff);
						space.unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(ram);
						space.install_read_handler(0x2800, 0x2fff, read8_delegate(FUNC(b2m_state::b2m_keyboard_r),state));
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
						space.unmap_write(0x0000, 0x27ff);
						space.unmap_write(0x2800, 0x2fff);
						space.unmap_write(0x3000, 0x6fff);
						space.unmap_write(0x7000, 0xdfff);
						space.unmap_write(0xe000, 0xffff);

						state->membank("bank1")->set_base(rom + 0x10000);
						state->membank("bank2")->set_base(rom + 0x10000);
						state->membank("bank3")->set_base(rom + 0x10000);
						state->membank("bank4")->set_base(rom + 0x10000);
						state->membank("bank5")->set_base(rom + 0x10000);
						break;
	}
}


WRITE_LINE_MEMBER(b2m_state::bm2_pit_out1)
{
	speaker_level_w(m_speaker, state);
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
			DEVCB_DRIVER_LINE_MEMBER(b2m_state,bm2_pit_out1)
		},
		{
			2000000,
			DEVCB_NULL,
			DEVCB_DEVICE_LINE("pit8253", pit8253_clk0_w)
		}
	}
};

WRITE8_MEMBER(b2m_state::b2m_8255_porta_w)
{
	m_b2m_8255_porta = data;
}
WRITE8_MEMBER(b2m_state::b2m_8255_portb_w)
{
	m_b2m_video_scroll = data;
}

WRITE8_MEMBER(b2m_state::b2m_8255_portc_w)
{
	m_b2m_8255_portc = data;
	b2m_set_bank(machine(), m_b2m_8255_portc & 7);
	m_b2m_video_page = (m_b2m_8255_portc >> 7) & 1;
}

READ8_MEMBER(b2m_state::b2m_8255_portb_r)
{
	return m_b2m_video_scroll;
}

I8255A_INTERFACE( b2m_ppi8255_interface_1 )
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(b2m_state,b2m_8255_porta_w),
	DEVCB_DRIVER_MEMBER(b2m_state,b2m_8255_portb_r),
	DEVCB_DRIVER_MEMBER(b2m_state,b2m_8255_portb_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(b2m_state,b2m_8255_portc_w)
};

void b2m_state::b2m_fdc_drq(bool state)
{
	/* Clears HALT state of CPU when data is ready to read */
	if (state)
		m_maincpu->set_input_line(INPUT_LINE_HALT, CLEAR_LINE);
}


WRITE8_MEMBER(b2m_state::b2m_ext_8255_portc_w)
{
	UINT8 drive = ((data >> 1) & 1) ^ 1;
	UINT8 side  = (data  & 1) ^ 1;

	static const char *names[] = { "fd0", "fd1"};
	floppy_image_device *floppy = NULL;
	floppy_connector *con = machine().device<floppy_connector>(names[drive]);
	if(con)
		floppy = con->get_device();

	floppy->mon_w(0);
	m_fdc->set_floppy(floppy);
	if (m_b2m_drive!=drive) {
		m_b2m_drive = drive;
	}

	if (m_b2m_side!=side) {
		m_b2m_side = side;
		floppy->ss_w(side);
	}
	/*
	    When bit 5 is set CPU is in HALT state and stay there until
	    DRQ is triggered from floppy side
	*/

	if ((data & 0xf0)==0x20) {
		m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	}
}

I8255A_INTERFACE( b2m_ppi8255_interface_2 )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(b2m_state,b2m_ext_8255_portc_w)
};

READ8_MEMBER(b2m_state::b2m_romdisk_porta_r)
{
	UINT8 *romdisk = memregion("maincpu")->base() + 0x12000;
	return romdisk[m_b2m_romdisk_msb*256+m_b2m_romdisk_lsb];
}

WRITE8_MEMBER(b2m_state::b2m_romdisk_portb_w)
{
	m_b2m_romdisk_lsb = data;
}

WRITE8_MEMBER(b2m_state::b2m_romdisk_portc_w)
{
	m_b2m_romdisk_msb = data & 0x7f;
}

I8255A_INTERFACE( b2m_ppi8255_interface_3 )
{
	DEVCB_DRIVER_MEMBER(b2m_state,b2m_romdisk_porta_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(b2m_state,b2m_romdisk_portb_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(b2m_state,b2m_romdisk_portc_w)
};

WRITE_LINE_MEMBER(b2m_state::b2m_pic_set_int_line)
{
	machine().device("maincpu")->execute().set_input_line(0, state ?  HOLD_LINE : CLEAR_LINE);
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

void b2m_state::machine_start()
{
	m_pic = machine().device("pic8259");
	m_fdc = machine().device<fd1793_t>("fd1793");
	m_speaker = machine().device(SPEAKER_TAG);

	m_fdc->setup_drq_cb(fd1793_t::line_cb(FUNC(b2m_state::b2m_fdc_drq), this));

	/* register for state saving */
	save_item(NAME(m_b2m_8255_porta));
	save_item(NAME(m_b2m_video_scroll));
	save_item(NAME(m_b2m_8255_portc));
	save_item(NAME(m_b2m_video_page));
	save_item(NAME(m_b2m_drive));
	save_item(NAME(m_b2m_side));
	save_item(NAME(m_b2m_romdisk_lsb));
	save_item(NAME(m_b2m_romdisk_msb));
	save_pointer(NAME(m_b2m_color), 4);
	save_item(NAME(m_b2m_localmachine));
	save_item(NAME(m_vblank_state));

	machine().save().register_postload(save_prepost_delegate(FUNC(b2m_postload), this));
}

static IRQ_CALLBACK(b2m_irq_callback)
{
	b2m_state *state = device->machine().driver_data<b2m_state>();
	return pic8259_acknowledge(state->m_pic);
}

const struct pic8259_interface b2m_pic8259_config =
{
	DEVCB_DRIVER_LINE_MEMBER(b2m_state,b2m_pic_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};

INTERRUPT_GEN_MEMBER(b2m_state::b2m_vblank_interrupt)
{
	m_vblank_state++;
	if (m_vblank_state>1) m_vblank_state=0;
	pic8259_ir0_w(m_pic, m_vblank_state);
}

void b2m_state::machine_reset()
{
	m_b2m_side = 0;
	m_b2m_drive = 0;

	machine().device("maincpu")->execute().set_irq_acknowledge_callback(b2m_irq_callback);
	b2m_set_bank(machine(), 7);
}
