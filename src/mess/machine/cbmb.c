/***************************************************************************
    commodore b series computer

    peter.trauner@jk.uni-linz.ac.at
***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6509.h"
#include "sound/sid6581.h"
#include "machine/6525tpi.h"
#include "machine/6526cia.h"
#include "machine/ieee488.h"

#include "includes/cbmb.h"
#include "crsshair.h"

#include "imagedev/cartslot.h"

/* 2008-05 FP: Were these added as a reminder to add configs of
drivers 8 & 9 as in pet.c ? */
#define IEEE8ON 0
#define IEEE9ON 0

#define VERBOSE_LEVEL 0
#define DBG_LOG( MACHINE, N, M, A ) \
	do { \
		if(VERBOSE_LEVEL >= N) \
		{ \
			if( M ) \
				logerror("%11.6f: %-24s", MACHINE.time().as_double(), (char*) M ); \
			logerror A; \
		} \
	} while (0)

static TIMER_CALLBACK( cbmb_frame_interrupt );
/* keyboard lines */


/* tpi at 0xfde00
 in interrupt mode
 irq to m6509 irq
 pa7 ieee nrfd
 pa6 ieee ndac
 pa5 ieee eoi
 pa4 ieee dav
 pa3 ieee atn
 pa2 ieee ren
 pa1 ieee io chips te
 pa0 ieee io chip dc
 pb1 ieee seq
 pb0 ieee ifc
 cb ?
 ca chargen rom address line 12
 i4 acia irq
 i3 ?
 i2 cia6526 irq
 i1 self pb0
 i0 tod (50 or 60 hertz frequency)
 */
READ8_DEVICE_HANDLER( cbmb_tpi0_port_a_r )
{
	cbmb_state *state = device->machine().driver_data<cbmb_state>();
	UINT8 data = 0;

	if (state->m_ieee->nrfd_r())
		data |= 0x80;

	if (state->m_ieee->ndac_r())
		data |= 0x40;

	if (state->m_ieee->eoi_r())
		data |= 0x20;

	if (state->m_ieee->dav_r())
		data |= 0x10;

	if (state->m_ieee->atn_r())
		data |= 0x08;

	if (state->m_ieee->ren_r())
        data |= 0x04;

	return data;
}

WRITE8_DEVICE_HANDLER( cbmb_tpi0_port_a_w )
{
	cbmb_state *state = device->machine().driver_data<cbmb_state>();

	state->m_ieee->nrfd_w(BIT(data, 7));
	state->m_ieee->ndac_w(BIT(data, 6));
	state->m_ieee->eoi_w(BIT(data, 5));
	state->m_ieee->dav_w(BIT(data, 4));
	state->m_ieee->atn_w(BIT(data, 3));
	state->m_ieee->ren_w(BIT(data, 2));
}

READ8_DEVICE_HANDLER( cbmb_tpi0_port_b_r )
{
	cbmb_state *state = device->machine().driver_data<cbmb_state>();
	UINT8 data = 0;

	if (state->m_ieee->srq_r())
		data |= 0x02;

	if (state->m_ieee->ifc_r())
		data |= 0x01;

	return data;
}

WRITE8_DEVICE_HANDLER( cbmb_tpi0_port_b_w )
{
	cbmb_state *state = device->machine().driver_data<cbmb_state>();

	state->m_ieee->srq_w(BIT(data, 1));
	state->m_ieee->ifc_w(BIT(data, 0));
}

/* tpi at 0xfdf00
  cbm 500
   port c7 video address lines?
   port c6 ?
  cbm 600
   port c7 low
   port c6 ntsc 1, 0 pal
  cbm 700
   port c7 high
   port c6 ?
  port c5 .. c0 keyboard line select
  port a7..a0 b7..b0 keyboard input */
WRITE8_DEVICE_HANDLER( cbmb_keyboard_line_select_a )
{
	cbmb_state *state = device->machine().driver_data<cbmb_state>();
	state->m_keyline_a = data;
}

WRITE8_DEVICE_HANDLER( cbmb_keyboard_line_select_b )
{
	cbmb_state *state = device->machine().driver_data<cbmb_state>();
	state->m_keyline_b = data;
}

WRITE8_DEVICE_HANDLER( cbmb_keyboard_line_select_c )
{
	cbmb_state *state = device->machine().driver_data<cbmb_state>();
	state->m_keyline_c = data;
}

READ8_DEVICE_HANDLER( cbmb_keyboard_line_a )
{
	cbmb_state *state = device->machine().driver_data<cbmb_state>();
	int data = 0;
	if (!(state->m_keyline_c & 0x01))
		data |= device->machine().root_device().ioport("ROW0")->read();

	if (!(state->m_keyline_c & 0x02))
		data |= device->machine().root_device().ioport("ROW2")->read();

	if (!(state->m_keyline_c & 0x04))
		data |= device->machine().root_device().ioport("ROW4")->read();

	if (!(state->m_keyline_c & 0x08))
		data |= device->machine().root_device().ioport("ROW6")->read();

	if (!(state->m_keyline_c & 0x10))
		data |= device->machine().root_device().ioport("ROW8")->read();

	if (!(state->m_keyline_c & 0x20))
		data |= state->ioport("ROW10")->read();

	return data ^0xff;
}

READ8_DEVICE_HANDLER( cbmb_keyboard_line_b )
{
	cbmb_state *state = device->machine().driver_data<cbmb_state>();
	int data = 0;
	if (!(state->m_keyline_c & 0x01))
		data |= device->machine().root_device().ioport("ROW1")->read();

	if (!(state->m_keyline_c & 0x02))
		data |= device->machine().root_device().ioport("ROW3")->read();

	if (!(state->m_keyline_c & 0x04))
		data |= device->machine().root_device().ioport("ROW5")->read();

	if (!(state->m_keyline_c & 0x08))
		data |= device->machine().root_device().ioport("ROW7")->read();

	if (!(state->m_keyline_c & 0x10))
		data |= device->machine().root_device().ioport("ROW9")->read() | ((device->machine().root_device().ioport("SPECIAL")->read() & 0x04) ? 1 : 0 );

	if (!(state->m_keyline_c & 0x20))
		data |= state->ioport("ROW11")->read();

	return data ^0xff;
}

READ8_DEVICE_HANDLER( cbmb_keyboard_line_c )
{
	int data = 0;
	cbmb_state *state = device->machine().driver_data<cbmb_state>();
	if ((device->machine().root_device().ioport("ROW0")->read() & ~state->m_keyline_a) ||
				(device->machine().root_device().ioport("ROW1")->read() & ~state->m_keyline_b))
		 data |= 0x01;

	if ((device->machine().root_device().ioport("ROW2")->read() & ~state->m_keyline_a) ||
				(device->machine().root_device().ioport("ROW3")->read() & ~state->m_keyline_b))
		 data |= 0x02;

	if ((device->machine().root_device().ioport("ROW4")->read() & ~state->m_keyline_a) ||
				(device->machine().root_device().ioport("ROW5")->read() & ~state->m_keyline_b))
		 data |= 0x04;

	if ((device->machine().root_device().ioport("ROW6")->read() & ~state->m_keyline_a) ||
				(device->machine().root_device().ioport("ROW7")->read() & ~state->m_keyline_b))
		 data |= 0x08;

	if ((device->machine().root_device().ioport("ROW8")->read() & ~state->m_keyline_a) ||
				((device->machine().root_device().ioport("ROW9")->read() | ((device->machine().root_device().ioport("SPECIAL")->read() & 0x04) ? 1 : 0)) & ~state->m_keyline_b))
		 data |= 0x10;

	if ((device->machine().root_device().ioport("ROW10")->read() & ~state->m_keyline_a) ||
				(state->ioport("ROW11")->read() & ~state->m_keyline_b))
		 data |= 0x20;

	if (!state->m_p500)
	{
		if (!state->m_cbm_ntsc)
			data |= 0x40;

		if (!state->m_cbm700)
			data |= 0x80;
	}
	return data ^0xff;
}

WRITE_LINE_DEVICE_HANDLER( cbmb_irq )
{
	cbmb_state *driver_state = device->machine().driver_data<cbmb_state>();
	if (state != driver_state->m_old_level)
	{
		DBG_LOG(device->machine(), 3, "mos6509", ("irq %s\n", state ? "start" : "end"));
		device->machine().device("maincpu")->execute().set_input_line(M6502_IRQ_LINE, state);
		driver_state->m_old_level = state;
	}
}

/*
  port a ieee in/out
  pa7 trigger gameport 2
  pa6 trigger gameport 1
  pa1 ?
  pa0 ?
  pb7 .. 4 gameport 2
  pb3 .. 0 gameport 1
 */

const mos6526_interface cbmb_cia =
{
	DEVCB_DEVICE_LINE("tpi6525_0", tpi6525_i2_w),
	DEVCB_NULL,	/* pc_func */
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DEVICE_MEMBER(IEEE488_TAG, ieee488_device, dio_r),
	DEVCB_DEVICE_MEMBER(IEEE488_TAG, ieee488_device, dio_w),
	DEVCB_NULL,
	DEVCB_NULL
};

WRITE8_MEMBER(cbmb_state::cbmb_colorram_w)
{
	m_colorram[offset] = data | 0xf0;
}

WRITE_LINE_DEVICE_HANDLER( cbmb_change_font )
{
	cbmb_vh_set_font(device->machine(), state);
}

static void cbmb_common_driver_init( running_machine &machine )
{
	cbmb_state *state = machine.driver_data<cbmb_state>();
	state->m_chargen = state->memregion("maincpu")->base() + 0x100000;
	/*    memset(c64_memory, 0, 0xfd00); */

	machine.scheduler().timer_pulse(attotime::from_msec(10), FUNC(cbmb_frame_interrupt));

	state->m_p500 = 0;
	state->m_cbm700 = 0;
}

DRIVER_INIT_MEMBER(cbmb_state,cbm600)
{
	cbmb_common_driver_init(machine());
	m_cbm_ntsc = 1;
	cbm600_vh_init(machine());
}

DRIVER_INIT_MEMBER(cbmb_state,cbm600pal)
{
	cbmb_common_driver_init(machine());
	m_cbm_ntsc = 0;
	cbm600_vh_init(machine());
}

DRIVER_INIT_MEMBER(cbmb_state,cbm600hu)
{
	cbmb_common_driver_init(machine());
	m_cbm_ntsc = 0;
}

DRIVER_INIT_MEMBER(cbmb_state,cbm700)
{
	cbmb_common_driver_init(machine());
	m_cbm700 = 1;
	m_cbm_ntsc = 0;
	cbm700_vh_init(machine());
}

DRIVER_INIT_MEMBER(cbmb_state,p500)
{
	cbmb_common_driver_init(machine());
	m_p500 = 1;
	m_cbm_ntsc = 1;
}

MACHINE_RESET_MEMBER(cbmb_state,cbmb)
{
//removed   cbm_drive_0_config (IEEE8ON ? IEEE : 0, 8);
//removed   cbm_drive_1_config (IEEE9ON ? IEEE : 0, 9);
}


static TIMER_CALLBACK( p500_lightpen_tick )
{
	if ((machine.root_device().ioport("CTRLSEL")->read_safe(0x00) & 0x07) == 0x04)
	{
		/* enable lightpen crosshair */
		crosshair_set_screen(machine, 0, CROSSHAIR_SCREEN_ALL);
	}
	else
	{
		/* disable lightpen crosshair */
		crosshair_set_screen(machine, 0, CROSSHAIR_SCREEN_NONE);
	}
}

static TIMER_CALLBACK(cbmb_frame_interrupt)
{
	cbmb_state *state = machine.driver_data<cbmb_state>();
	device_t *tpi_0 = machine.device("tpi6525_0");

#if 0
	int controller1 = machine.root_device().ioport("CTRLSEL")->read() & 0x07;
	int controller2 = machine.root_device().ioport("CTRLSEL")->read() & 0x70;
#endif

	tpi6525_i0_w(tpi_0, state->m_irq_level);
	state->m_irq_level = !state->m_irq_level;
	if (state->m_irq_level) return ;

#if 0
	value = 0xff;
	switch (controller1)
	{
		case 0x00:
			value &= ~machine.root_device().ioport("JOY1_1B")->read();			/* Joy1 Directions + Button 1 */
			break;

		case 0x01:
			if (machine.root_device().ioport("OTHER")->read() & 0x40)			/* Paddle2 Button */
				value &= ~0x08;
			if (machine.root_device().ioport("OTHER")->read() & 0x80)			/* Paddle1 Button */
				value &= ~0x04;
			break;

		case 0x02:
			if (machine.root_device().ioport("OTHER")->read() & 0x02)			/* Mouse Button Left */
				value &= ~0x10;
			if (machine.root_device().ioport("OTHER")->read() & 0x01)			/* Mouse Button Right */
				value &= ~0x01;
			break;

		case 0x03:
			value &= ~(machine.root_device().ioport("JOY1_2B")->read() & 0x1f);	/* Joy1 Directions + Button 1 */
			break;

		case 0x04:
/* was there any input on the lightpen? where is it mapped? */
//          if (machine.root_device().ioport("OTHER")->read() & 0x04)           /* Lightpen Signal */
//              value &= ?? ;
			break;

		case 0x07:
			break;

		default:
			logerror("Invalid Controller 1 Setting %d\n", controller1);
			break;
	}

	c64_keyline[8] = value;


	value = 0xff;
	switch (controller2)
	{
		case 0x00:
			value &= ~machine.root_device().ioport("JOY2_1B")->read();			/* Joy2 Directions + Button 1 */
			break;

		case 0x10:
			if (machine.root_device().ioport("OTHER")->read() & 0x10)			/* Paddle4 Button */
				value &= ~0x08;
			if (machine.root_device().ioport("OTHER")->read() & 0x20)			/* Paddle3 Button */
				value &= ~0x04;
			break;

		case 0x20:
			if (machine.root_device().ioport("OTHER")->read() & 0x02)			/* Mouse Button Left */
				value &= ~0x10;
			if (machine.root_device().ioport("OTHER")->read() & 0x01)			/* Mouse Button Right */
				value &= ~0x01;
			break;

		case 0x30:
			value &= ~(machine.root_device().ioport("JOY2_2B")->read() & 0x1f);	/* Joy2 Directions + Button 1 */
			break;

		case 0x40:
/* was there any input on the lightpen? where is it mapped? */
//          if (machine.root_device().ioport("OTHER")->read() & 0x04)           /* Lightpen Signal */
//              value &= ?? ;
			break;

		case 0x70:
			break;

		default:
			logerror("Invalid Controller 2 Setting %d\n", controller2);
			break;
	}

	c64_keyline[9] = value;
#endif

// 128u4 FIXME
//  vic2_frame_interrupt (device);

	/* for p500, check if lightpen has been chosen as input: if so, enable crosshair (but c64-like inputs for p500 are not working atm) */
	machine.scheduler().timer_set(attotime::zero, FUNC(p500_lightpen_tick));

	set_led_status(machine, 1, machine.root_device().ioport("SPECIAL")->read() & 0x04 ? 1 : 0);		/* Shift Lock */
}


/***********************************************

    CBM Business Computers Cartridges

***********************************************/

static DEVICE_IMAGE_LOAD(cbmb_cart)
{
	UINT32 size = image.length();
	const char *filetype = image.filetype();
	UINT8 *cbmb_memory = image.device().machine().root_device().memregion("maincpu")->base();
	int address = 0;

	/* Assign loading address according to extension */
	if (!mame_stricmp(filetype, "10"))
		address = 0x1000;

	else if (!mame_stricmp(filetype, "20"))
		address = 0x2000;

	else if (!mame_stricmp(filetype, "40"))
		address = 0x4000;

	else if (!mame_stricmp(filetype, "60"))
		address = 0x6000;

	logerror("Loading cart %s at %.4x size:%.4x\n", image.filename(), address, size);

	image.fread(cbmb_memory + 0xf0000 + address, size);

	return IMAGE_INIT_PASS;
}


MACHINE_CONFIG_FRAGMENT(cbmb_cartslot)
	MCFG_CARTSLOT_ADD("cart1")
	MCFG_CARTSLOT_EXTENSION_LIST("10,20,40,60")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(cbmb_cart)

	MCFG_CARTSLOT_ADD("cart2")
	MCFG_CARTSLOT_EXTENSION_LIST("10,20,40,60")
	MCFG_CARTSLOT_NOT_MANDATORY
	MCFG_CARTSLOT_LOAD(cbmb_cart)
MACHINE_CONFIG_END
