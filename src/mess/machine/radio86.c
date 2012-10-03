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
READ8_MEMBER(radio86_state::radio86_8255_portb_r2)
{
	UINT8 key = 0xff;
	if ((m_keyboard_mask & 0x01)!=0) { key &= ioport("LINE0")->read(); }
	if ((m_keyboard_mask & 0x02)!=0) { key &= machine().root_device().ioport("LINE1")->read(); }
	if ((m_keyboard_mask & 0x04)!=0) { key &= machine().root_device().ioport("LINE2")->read(); }
	if ((m_keyboard_mask & 0x08)!=0) { key &= machine().root_device().ioport("LINE3")->read(); }
	if ((m_keyboard_mask & 0x10)!=0) { key &= machine().root_device().ioport("LINE4")->read(); }
	if ((m_keyboard_mask & 0x20)!=0) { key &= machine().root_device().ioport("LINE5")->read(); }
	if ((m_keyboard_mask & 0x40)!=0) { key &= machine().root_device().ioport("LINE6")->read(); }
	if ((m_keyboard_mask & 0x80)!=0) { key &= machine().root_device().ioport("LINE7")->read(); }
	return key;
}

READ8_MEMBER(radio86_state::radio86_8255_portc_r2)
{
	double level = (machine().device<cassette_image_device>(CASSETTE_TAG)->input());
	UINT8 dat = ioport("LINE8")->read();
	if (level <  0) {
		dat ^= m_tape_value;
	}
	return dat;
}

WRITE8_MEMBER(radio86_state::radio86_8255_porta_w2)
{
	m_keyboard_mask = data ^ 0xff;
}

WRITE8_MEMBER(radio86_state::radio86_8255_portc_w2)
{
	machine().device<cassette_image_device>(CASSETTE_TAG)->output(data & 0x01 ? 1 : -1);
}


I8255A_INTERFACE( radio86_ppi8255_interface_1 )
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_8255_porta_w2),
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_8255_portb_r2),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_8255_portc_r2),
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_8255_portc_w2),
};

I8255A_INTERFACE( mikrosha_ppi8255_interface_1 )
{
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_8255_portb_r2),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_8255_porta_w2),
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_8255_portc_r2),
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_8255_portc_w2),
};



READ8_MEMBER(radio86_state::rk7007_8255_portc_r)
{
	double level = (machine().device<cassette_image_device>(CASSETTE_TAG)->input());
	UINT8 key = 0xff;
	if ((m_keyboard_mask & 0x01)!=0) { key &= ioport("CLINE0")->read(); }
	if ((m_keyboard_mask & 0x02)!=0) { key &= machine().root_device().ioport("CLINE1")->read(); }
	if ((m_keyboard_mask & 0x04)!=0) { key &= machine().root_device().ioport("CLINE2")->read(); }
	if ((m_keyboard_mask & 0x08)!=0) { key &= machine().root_device().ioport("CLINE3")->read(); }
	if ((m_keyboard_mask & 0x10)!=0) { key &= machine().root_device().ioport("CLINE4")->read(); }
	if ((m_keyboard_mask & 0x20)!=0) { key &= machine().root_device().ioport("CLINE5")->read(); }
	if ((m_keyboard_mask & 0x40)!=0) { key &= machine().root_device().ioport("CLINE6")->read(); }
	if ((m_keyboard_mask & 0x80)!=0) { key &= machine().root_device().ioport("CLINE7")->read(); }
	key &= 0xe0;
	if (level <  0) {
		key ^= m_tape_value;
	}
	return key;
}

I8255A_INTERFACE( rk7007_ppi8255_interface )
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_8255_porta_w2),
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_8255_portb_r2),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(radio86_state,rk7007_8255_portc_r),
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_8255_portc_w2),
};

WRITE_LINE_MEMBER(radio86_state::hrq_w)
{
	/* HACK - this should be connected to the BUSREQ line of Z80 */
	machine().device("maincpu")->execute().set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the BUSACK line of Z80 */
	i8257_hlda_w(machine().device("dma8257"), state);
}

static UINT8 memory_read_byte(address_space &space, offs_t address, UINT8 mem_mask) { return space.read_byte(address); }
static void memory_write_byte(address_space &space, offs_t address, UINT8 data, UINT8 mem_mask) { space.write_byte(address, data); }

I8257_INTERFACE( radio86_dma )
{
	DEVCB_DRIVER_LINE_MEMBER(radio86_state,hrq_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, memory_read_byte),
	DEVCB_MEMORY_HANDLER("maincpu", PROGRAM, memory_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_DEVICE_HANDLER("i8275", i8275_dack_w), DEVCB_NULL }
};

TIMER_CALLBACK_MEMBER(radio86_state::radio86_reset)
{
	membank("bank1")->set_entry(0);
}


READ8_MEMBER(radio86_state::radio_cpu_state_r)
{
	return space.device().state().state_int(I8085_STATUS);
}

READ8_MEMBER(radio86_state::radio_io_r)
{
	return machine().device("maincpu")->memory().space(AS_PROGRAM).read_byte((offset << 8) + offset);
}

WRITE8_MEMBER(radio86_state::radio_io_w)
{
	machine().device("maincpu")->memory().space(AS_PROGRAM).write_byte((offset << 8) + offset,data);
}

MACHINE_RESET_MEMBER(radio86_state,radio86)
{
	machine().scheduler().timer_set(attotime::from_usec(10), timer_expired_delegate(FUNC(radio86_state::radio86_reset),this));
	membank("bank1")->set_entry(1);

	m_keyboard_mask = 0;
	m_disk_sel = 0;
}


WRITE8_MEMBER(radio86_state::radio86_pagesel)
{
	m_disk_sel = data;
}

READ8_MEMBER(radio86_state::radio86_romdisk_porta_r)
{
	UINT8 *romdisk = memregion("maincpu")->base() + 0x10000;
	if ((m_disk_sel & 0x0f) ==0) {
		return romdisk[m_romdisk_msb*256+m_romdisk_lsb];
	} else {
		if (m_disk_sel==0xdf) {
			return m_radio_ram_disk[m_romdisk_msb*256+m_romdisk_lsb + 0x10000];
		} else {
			return m_radio_ram_disk[m_romdisk_msb*256+m_romdisk_lsb];
		}
	}
}

WRITE8_MEMBER(radio86_state::radio86_romdisk_portb_w)
{
	m_romdisk_lsb = data;
}

WRITE8_MEMBER(radio86_state::radio86_romdisk_portc_w)
{
	m_romdisk_msb = data;
}

I8255A_INTERFACE( radio86_ppi8255_interface_2 )
{
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_romdisk_porta_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_romdisk_portb_w),
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(radio86_state,radio86_romdisk_portc_w)
};

WRITE8_MEMBER(radio86_state::mikrosha_8255_font_page_w)
{
	m_mikrosha_font_page = (data  > 7) & 1;
}

I8255A_INTERFACE( mikrosha_ppi8255_interface_2 )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(radio86_state,mikrosha_8255_font_page_w),
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
