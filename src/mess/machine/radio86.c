/***************************************************************************

        Radio-86RK machine driver by Miodrag Milanovic

        06/03/2008 Preliminary driver.

****************************************************************************/


#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/i8255.h"
#include "machine/8257dma.h"
#include "video/i8275.h"
#include "includes/radio86.h"



void radio86_state::radio86_init_keyboard()
{
	m_keyboard_mask = 0;
	m_tape_value = 0x10;
}

/* Driver initialization */
DRIVER_INIT_MEMBER(radio86_state,radio86)
{
	/* set initialy ROM to be visible on first bank */
	UINT8 *RAM = m_region_maincpu->base();
	memset(RAM,0x0000,0x1000); // make frist page empty by default
	m_bank1->configure_entries(1, 2, RAM, 0x0000);
	m_bank1->configure_entries(0, 2, RAM, 0xf800);
	radio86_init_keyboard();
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
	if ((m_keyboard_mask & 0x01)!=0) { key &= m_io_line0->read(); }
	if ((m_keyboard_mask & 0x02)!=0) { key &= m_io_line1->read(); }
	if ((m_keyboard_mask & 0x04)!=0) { key &= m_io_line2->read(); }
	if ((m_keyboard_mask & 0x08)!=0) { key &= m_io_line3->read(); }
	if ((m_keyboard_mask & 0x10)!=0) { key &= m_io_line4->read(); }
	if ((m_keyboard_mask & 0x20)!=0) { key &= m_io_line5->read(); }
	if ((m_keyboard_mask & 0x40)!=0) { key &= m_io_line6->read(); }
	if ((m_keyboard_mask & 0x80)!=0) { key &= m_io_line7->read(); }
	return key;
}

READ8_MEMBER(radio86_state::radio86_8255_portc_r2)
{
	double level = m_cassette->input();
	UINT8 dat = m_io_line8->read();
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
	m_cassette->output(data & 0x01 ? 1 : -1);
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
	double level = m_cassette->input();
	UINT8 key = 0xff;
	if ((m_keyboard_mask & 0x01)!=0) { key &= m_io_cline0->read(); }
	if ((m_keyboard_mask & 0x02)!=0) { key &= m_io_cline1->read(); }
	if ((m_keyboard_mask & 0x04)!=0) { key &= m_io_cline2->read(); }
	if ((m_keyboard_mask & 0x08)!=0) { key &= m_io_cline3->read(); }
	if ((m_keyboard_mask & 0x10)!=0) { key &= m_io_cline4->read(); }
	if ((m_keyboard_mask & 0x20)!=0) { key &= m_io_cline5->read(); }
	if ((m_keyboard_mask & 0x40)!=0) { key &= m_io_cline6->read(); }
	if ((m_keyboard_mask & 0x80)!=0) { key &= m_io_cline7->read(); }
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
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the BUSACK line of Z80 */
	m_dma8257->i8257_hlda_w(state);
}

READ8_MEMBER(radio86_state::memory_read_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

WRITE8_MEMBER(radio86_state::memory_write_byte)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

I8257_INTERFACE( radio86_dma )
{
	DEVCB_DRIVER_LINE_MEMBER(radio86_state,hrq_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(radio86_state, memory_read_byte),
	DEVCB_DRIVER_MEMBER(radio86_state, memory_write_byte),
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, DEVCB_NULL },
	{ DEVCB_NULL, DEVCB_NULL, DEVCB_DEVICE_MEMBER("i8275", i8275_device, dack_w), DEVCB_NULL }
};

void radio86_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_RESET:
		m_bank1->set_entry(0);
		break;
	default:
		assert_always(FALSE, "Unknown id in radio86_state::device_timer");
	}
}


READ8_MEMBER(radio86_state::radio_cpu_state_r)
{
	return space.device().state().state_int(I8085_STATUS);
}

READ8_MEMBER(radio86_state::radio_io_r)
{
	return m_maincpu->space(AS_PROGRAM).read_byte((offset << 8) + offset);
}

WRITE8_MEMBER(radio86_state::radio_io_w)
{
	m_maincpu->space(AS_PROGRAM).write_byte((offset << 8) + offset,data);
}

MACHINE_RESET_MEMBER(radio86_state,radio86)
{
	timer_set(attotime::from_usec(10), TIMER_RESET);
	m_bank1->set_entry(1);

	m_keyboard_mask = 0;
	m_disk_sel = 0;
}


WRITE8_MEMBER(radio86_state::radio86_pagesel)
{
	m_disk_sel = data;
}

READ8_MEMBER(radio86_state::radio86_romdisk_porta_r)
{
	UINT8 *romdisk = m_region_maincpu->base() + 0x10000;
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
	6,
	0,
	DEVCB_DEVICE_LINE_MEMBER("dma8257", i8257_device, i8257_drq2_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	radio86_display_pixels
};

const i8275_interface mikrosha_i8275_interface = {
	6,
	0,
	DEVCB_DEVICE_LINE_MEMBER("dma8257", i8257_device, i8257_drq2_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	mikrosha_display_pixels
};

const i8275_interface apogee_i8275_interface = {
	6,
	0,
	DEVCB_DEVICE_LINE_MEMBER("dma8257", i8257_device, i8257_drq2_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	apogee_display_pixels
};

const i8275_interface partner_i8275_interface = {
	6,
	1,
	DEVCB_DEVICE_LINE_MEMBER("dma8257", i8257_device, i8257_drq2_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	partner_display_pixels
};
