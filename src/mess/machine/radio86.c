/***************************************************************************

        Radio-86RK machine driver by Miodrag Milanovic

        06/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "imagedev/cassette.h"
#include "machine/i8255.h"
#include "machine/8257dma.h"
#include "video/i8275.h"
#include "includes/radio86.h"



void radio86_init_keyboard(running_machine &machine)
{
	radio86_state *state = machine.driver_data<radio86_state>();
	state->m_keyboard_mask = 0;
	state->m_tape_value = 0x10;
}

/* Driver initialization */
DRIVER_INIT_MEMBER(radio86_state,radio86)
{
	/* set initialy ROM to be visible on first bank */
	UINT8 *RAM = memregion("maincpu")->base();
	memset(RAM,0x0000,0x1000); // make frist page empty by default
	membank("bank1")->configure_entries(1, 2, RAM, 0x0000);
	membank("bank1")->configure_entries(0, 2, RAM, 0xf800);
	radio86_init_keyboard(machine());
}

DRIVER_INIT_MEMBER(radio86_state,radioram)
{
	DRIVER_INIT_CALL(radio86);
	m_radio_ram_disk = auto_alloc_array(machine(), UINT8, 0x20000);
	memset(m_radio_ram_disk,0,0x20000);
}
static READ8_DEVICE_HANDLER (radio86_8255_portb_r2 )
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	UINT8 key = 0xff;
	if ((state->m_keyboard_mask & 0x01)!=0) { key &= state->ioport("LINE0")->read(); }
	if ((state->m_keyboard_mask & 0x02)!=0) { key &= device->machine().root_device().ioport("LINE1")->read(); }
	if ((state->m_keyboard_mask & 0x04)!=0) { key &= device->machine().root_device().ioport("LINE2")->read(); }
	if ((state->m_keyboard_mask & 0x08)!=0) { key &= device->machine().root_device().ioport("LINE3")->read(); }
	if ((state->m_keyboard_mask & 0x10)!=0) { key &= device->machine().root_device().ioport("LINE4")->read(); }
	if ((state->m_keyboard_mask & 0x20)!=0) { key &= device->machine().root_device().ioport("LINE5")->read(); }
	if ((state->m_keyboard_mask & 0x40)!=0) { key &= device->machine().root_device().ioport("LINE6")->read(); }
	if ((state->m_keyboard_mask & 0x80)!=0) { key &= device->machine().root_device().ioport("LINE7")->read(); }
	return key;
}

static READ8_DEVICE_HANDLER (radio86_8255_portc_r2 )
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	double level = (device->machine().device<cassette_image_device>(CASSETTE_TAG)->input());
	UINT8 dat = state->ioport("LINE8")->read();
	if (level <  0) {
		dat ^= state->m_tape_value;
	}
	return dat;
}

static WRITE8_DEVICE_HANDLER (radio86_8255_porta_w2 )
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	state->m_keyboard_mask = data ^ 0xff;
}

static WRITE8_DEVICE_HANDLER (radio86_8255_portc_w2 )
{
	device->machine().device<cassette_image_device>(CASSETTE_TAG)->output(data & 0x01 ? 1 : -1);
}


I8255A_INTERFACE( radio86_ppi8255_interface_1 )
{
	DEVCB_NULL,
	DEVCB_HANDLER(radio86_8255_porta_w2),
	DEVCB_HANDLER(radio86_8255_portb_r2),
	DEVCB_NULL,
	DEVCB_HANDLER(radio86_8255_portc_r2),
	DEVCB_HANDLER(radio86_8255_portc_w2),
};

I8255A_INTERFACE( mikrosha_ppi8255_interface_1 )
{
	DEVCB_HANDLER(radio86_8255_portb_r2),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(radio86_8255_porta_w2),
	DEVCB_HANDLER(radio86_8255_portc_r2),
	DEVCB_HANDLER(radio86_8255_portc_w2),
};



static READ8_DEVICE_HANDLER (rk7007_8255_portc_r )
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	double level = (device->machine().device<cassette_image_device>(CASSETTE_TAG)->input());
	UINT8 key = 0xff;
	if ((state->m_keyboard_mask & 0x01)!=0) { key &= state->ioport("CLINE0")->read(); }
	if ((state->m_keyboard_mask & 0x02)!=0) { key &= device->machine().root_device().ioport("CLINE1")->read(); }
	if ((state->m_keyboard_mask & 0x04)!=0) { key &= device->machine().root_device().ioport("CLINE2")->read(); }
	if ((state->m_keyboard_mask & 0x08)!=0) { key &= device->machine().root_device().ioport("CLINE3")->read(); }
	if ((state->m_keyboard_mask & 0x10)!=0) { key &= device->machine().root_device().ioport("CLINE4")->read(); }
	if ((state->m_keyboard_mask & 0x20)!=0) { key &= device->machine().root_device().ioport("CLINE5")->read(); }
	if ((state->m_keyboard_mask & 0x40)!=0) { key &= device->machine().root_device().ioport("CLINE6")->read(); }
	if ((state->m_keyboard_mask & 0x80)!=0) { key &= device->machine().root_device().ioport("CLINE7")->read(); }
	key &= 0xe0;
	if (level <  0) {
		key ^= state->m_tape_value;
	}
	return key;
}

I8255A_INTERFACE( rk7007_ppi8255_interface )
{
	DEVCB_NULL,
	DEVCB_HANDLER(radio86_8255_porta_w2),
	DEVCB_HANDLER(radio86_8255_portb_r2),
	DEVCB_NULL,
	DEVCB_HANDLER(rk7007_8255_portc_r),
	DEVCB_HANDLER(radio86_8255_portc_w2),
};

static WRITE_LINE_DEVICE_HANDLER( hrq_w )
{
	/* HACK - this should be connected to the BUSREQ line of Z80 */
	device->machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the BUSACK line of Z80 */
	i8257_hlda_w(device, state);
}

static UINT8 memory_read_byte(address_space *space, offs_t address) { return space->read_byte(address); }
static void memory_write_byte(address_space *space, offs_t address, UINT8 data) { space->write_byte(address, data); }

I8257_INTERFACE( radio86_dma )
{
	DEVCB_LINE(hrq_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, memory_read_byte),
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, memory_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_DEVICE_HANDLER("i8275", i8275_dack_w), DEVCB_NULL }
};

static TIMER_CALLBACK( radio86_reset )
{
	radio86_state *state = machine.driver_data<radio86_state>();
	state->membank("bank1")->set_entry(0);
}


READ8_MEMBER(radio86_state::radio_cpu_state_r)
{
	return space.device().state().state_int(I8085_STATUS);
}

READ8_MEMBER(radio86_state::radio_io_r)
{
	return machine().device("maincpu")->memory().space(AS_PROGRAM)->read_byte((offset << 8) + offset);
}

WRITE8_MEMBER(radio86_state::radio_io_w)
{
	machine().device("maincpu")->memory().space(AS_PROGRAM)->write_byte((offset << 8) + offset,data);
}

MACHINE_RESET_MEMBER(radio86_state,radio86)
{
	machine().scheduler().timer_set(attotime::from_usec(10), FUNC(radio86_reset));
	membank("bank1")->set_entry(1);

	m_keyboard_mask = 0;
	m_disk_sel = 0;
}


WRITE8_MEMBER(radio86_state::radio86_pagesel)
{
	m_disk_sel = data;
}

static READ8_DEVICE_HANDLER (radio86_romdisk_porta_r )
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	UINT8 *romdisk = state->memregion("maincpu")->base() + 0x10000;
	if ((state->m_disk_sel & 0x0f) ==0) {
		return romdisk[state->m_romdisk_msb*256+state->m_romdisk_lsb];
	} else {
		if (state->m_disk_sel==0xdf) {
			return state->m_radio_ram_disk[state->m_romdisk_msb*256+state->m_romdisk_lsb + 0x10000];
		} else {
			return state->m_radio_ram_disk[state->m_romdisk_msb*256+state->m_romdisk_lsb];
		}
	}
}

static WRITE8_DEVICE_HANDLER (radio86_romdisk_portb_w )
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	state->m_romdisk_lsb = data;
}

static WRITE8_DEVICE_HANDLER (radio86_romdisk_portc_w )
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	state->m_romdisk_msb = data;
}

I8255A_INTERFACE( radio86_ppi8255_interface_2 )
{
	DEVCB_HANDLER(radio86_romdisk_porta_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(radio86_romdisk_portb_w),
	DEVCB_NULL,
	DEVCB_HANDLER(radio86_romdisk_portc_w)
};

static WRITE8_DEVICE_HANDLER (mikrosha_8255_font_page_w )
{
	radio86_state *state = device->machine().driver_data<radio86_state>();
	state->m_mikrosha_font_page = (data  > 7) & 1;
}

I8255A_INTERFACE( mikrosha_ppi8255_interface_2 )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_HANDLER(mikrosha_8255_font_page_w),
	DEVCB_NULL,
	DEVCB_NULL
};

const i8275_interface radio86_i8275_interface = {
	"screen",
	6,
	0,
	DEVCB_DEVICE_LINE("dma8257", i8257_drq2_w),
	DEVCB_NULL,
	radio86_display_pixels
};

const i8275_interface mikrosha_i8275_interface = {
	"screen",
	6,
	0,
	DEVCB_DEVICE_LINE("dma8257", i8257_drq2_w),
	DEVCB_NULL,
	mikrosha_display_pixels
};

const i8275_interface apogee_i8275_interface = {
	"screen",
	6,
	0,
	DEVCB_DEVICE_LINE("dma8257", i8257_drq2_w),
	DEVCB_NULL,
	apogee_display_pixels
};

const i8275_interface partner_i8275_interface = {
	"screen",
	6,
	1,
	DEVCB_DEVICE_LINE("dma8257", i8257_drq2_w),
	DEVCB_NULL,
	partner_display_pixels
};
