// license:GPL-2.0+
// copyright-holders:Lee Hammerton, Dirk Best
/***************************************************************************

    Miles Gordon Technology SAM Coupe

    Notes:
    ------

    Beware - the early ROMs are very buggy, and tend to go mad once BASIC
    starts paging.  ROM 10 (version 1.0) requires the CALL after F9 or BOOT
    because the ROM loads the bootstrap but fails to execute it. The address
    depends on the RAM size:

    On a 256K SAM: CALL 229385
    Or on a 512K (or larger) machine: CALL 491529


    Todo:
    -----

    - Attribute read
    - Better timing
    - Harddisk interfaces

***************************************************************************/

/* core includes */
#include "emu.h"
#include "includes/samcoupe.h"
#
/* components */
#include "cpu/z80/z80.h"
#include "sound/saa1099.h"

/* devices */
#include "formats/tzx_cas.h"
#include "formats/coupedsk.h"

/***************************************************************************
    CONSTANTS
***************************************************************************/

#define SAMCOUPE_XTAL_X1  XTAL_24MHz
#define SAMCOUPE_XTAL_X2  XTAL_4_433619MHz


/***************************************************************************
    TIMERS
***************************************************************************/

void samcoupe_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_IRQ_OFF:
		irq_off(ptr, param);
		break;
	case TIMER_MOUSE_RESET:
		samcoupe_mouse_reset(ptr, param);
		break;
	case TIMER_VIDEO_UPDATE:
		sam_video_update_callback(ptr, param);
		break;
	default:
		assert_always(FALSE, "Unknown id in samcoupe_state::device_timer");
	}
}

/***************************************************************************
    I/O PORTS
***************************************************************************/

READ8_MEMBER(samcoupe_state::samcoupe_disk_r)
{
	/* drive and side is encoded into bit 5 and 3 */
	floppy_connector *con = (BIT(offset, 4) ? m_wd1772_1 : m_wd1772_0);
	floppy_image_device *floppy = con ? con->get_device() : 0;

	if(floppy)
		floppy->ss_w(BIT(offset, 2));
	m_fdc->set_floppy(floppy);

	/* bit 1 and 2 select the controller register */
	switch (offset & 0x03)
	{
	case 0: return m_fdc->status_r();
	case 1: return m_fdc->track_r();
	case 2: return m_fdc->sector_r();
	case 3: return m_fdc->data_r();
	}

	return 0xff;
}

WRITE8_MEMBER(samcoupe_state::samcoupe_disk_w)
{
	/* drive and side is encoded into bit 5 and 3 */
	floppy_connector *con = (BIT(offset, 4) ? m_wd1772_1 : m_wd1772_0);
	floppy_image_device *floppy = con ? con->get_device() : 0;

	if(floppy)
		floppy->ss_w(BIT(offset, 2));
	m_fdc->set_floppy(floppy);

	/* bit 1 and 2 select the controller register */
	switch (offset & 0x03)
	{
	case 0: m_fdc->cmd_w(data);     break;
	case 1: m_fdc->track_w(data);   break;
	case 2: m_fdc->sector_w(data);  break;
	case 3: m_fdc->data_w(data);    break;
	}
}

READ8_MEMBER(samcoupe_state::samcoupe_pen_r)
{
	screen_device *scr = machine().first_screen();
	UINT8 data;

	if (offset & 0x100)
	{
		int vpos = scr->vpos();

		/* return the current screen line or 192 for the border area */
		if (vpos < SAM_BORDER_TOP || vpos >= SAM_BORDER_TOP + SAM_SCREEN_HEIGHT)
			data = 192;
		else
			data = vpos - SAM_BORDER_TOP;
	}
	else
	{
		/* horizontal position is encoded into bits 3 to 8 */
		data = scr->hpos() & 0xfc;
	}

	return data;
}

WRITE8_MEMBER(samcoupe_state::samcoupe_clut_w)
{
	m_clut[(offset >> 8) & 0x0f] = data & 0x7f;
}

READ8_MEMBER(samcoupe_state::samcoupe_status_r)
{
	UINT8 data = 0xe0;

	/* bit 5-7, keyboard input */
	if (!BIT(offset,  8)) data &= m_keyboard_row_fe->read() & 0xe0;
	if (!BIT(offset,  9)) data &= m_keyboard_row_fd->read() & 0xe0;
	if (!BIT(offset, 10)) data &= m_keyboard_row_fb->read() & 0xe0;
	if (!BIT(offset, 11)) data &= m_keyboard_row_f7->read() & 0xe0;
	if (!BIT(offset, 12)) data &= m_keyboard_row_ef->read() & 0xe0;
	if (!BIT(offset, 13)) data &= m_keyboard_row_df->read() & 0xe0;
	if (!BIT(offset, 14)) data &= m_keyboard_row_bf->read() & 0xe0;
	if (!BIT(offset, 15)) data &= m_keyboard_row_7f->read() & 0xe0;

	/* bit 0-4, interrupt source */
	data |= m_status;

	return data;
}

WRITE8_MEMBER(samcoupe_state::samcoupe_line_int_w)
{
	m_line_int = data;
}

READ8_MEMBER(samcoupe_state::samcoupe_lmpr_r)
{
	return m_lmpr;
}

WRITE8_MEMBER(samcoupe_state::samcoupe_lmpr_w)
{
	address_space &space_program = m_maincpu->space(AS_PROGRAM);

	m_lmpr = data;
	samcoupe_update_memory(space_program);
}

READ8_MEMBER(samcoupe_state::samcoupe_hmpr_r)
{
	return m_hmpr;
}

WRITE8_MEMBER(samcoupe_state::samcoupe_hmpr_w)
{
	address_space &space_program = m_maincpu->space(AS_PROGRAM);

	m_hmpr = data;
	samcoupe_update_memory(space_program);
}

READ8_MEMBER(samcoupe_state::samcoupe_vmpr_r)
{
	return m_vmpr;
}

WRITE8_MEMBER(samcoupe_state::samcoupe_vmpr_w)
{
	address_space &space_program = m_maincpu->space(AS_PROGRAM);

	m_vmpr = data;
	samcoupe_update_memory(space_program);
}

READ8_MEMBER(samcoupe_state::samcoupe_midi_r)
{
	logerror("%s: read from midi port\n", machine().describe_context());
	return 0xff;
}

WRITE8_MEMBER(samcoupe_state::samcoupe_midi_w)
{
	logerror("%s: write to midi port: 0x%02x\n", machine().describe_context(), data);
}

READ8_MEMBER(samcoupe_state::samcoupe_keyboard_r)
{
	UINT8 data = 0x1f;

	/* bit 0-4, keyboard input */
	if (!BIT(offset,  8)) data &= m_keyboard_row_fe->read() & 0x1f;
	if (!BIT(offset,  9)) data &= m_keyboard_row_fd->read() & 0x1f;
	if (!BIT(offset, 10)) data &= m_keyboard_row_fb->read() & 0x1f;
	if (!BIT(offset, 11)) data &= m_keyboard_row_f7->read() & 0x1f;
	if (!BIT(offset, 12)) data &= m_keyboard_row_ef->read() & 0x1f;
	if (!BIT(offset, 13)) data &= m_keyboard_row_df->read() & 0x1f;
	if (!BIT(offset, 14)) data &= m_keyboard_row_bf->read() & 0x1f;
	if (!BIT(offset, 15)) data &= m_keyboard_row_7f->read() & 0x1f;

	if (offset == 0xff00)
	{
		data &= m_keyboard_row_ff->read() & 0x1f;

		/* if no key has been pressed, return the mouse state */
		if (data == 0x1f)
			data = samcoupe_mouse_r();
	}

	/* bit 5, lightpen strobe */
	data |= 1 << 5;

	/* bit 6, cassette input */
	data |= (m_cassette->input() > 0 ? 1 : 0) << 6;

	/* bit 7, external memory */
	data |= 1 << 7;

	return data;
}

WRITE8_MEMBER(samcoupe_state::samcoupe_border_w)
{
	m_border = data;

	/* bit 3, cassette output */
	m_cassette->output( BIT(data, 3) ? -1.0 : +1.0);

	/* bit 4, beep */
	m_speaker->level_w(BIT(data, 4));
}

READ8_MEMBER(samcoupe_state::samcoupe_attributes_r)
{
	return m_attribute;
}

WRITE_LINE_MEMBER(samcoupe_state::write_lpt1_busy)
{
	m_lpt1_busy = state;
}

READ8_MEMBER(samcoupe_state::samcoupe_lpt1_busy_r)
{
	return m_lpt1_busy;
}

WRITE8_MEMBER(samcoupe_state::samcoupe_lpt1_strobe_w)
{
	m_lpt1->write_strobe(data & 1);
}

DECLARE_WRITE_LINE_MEMBER(samcoupe_state::write_lpt2_busy)
{
	m_lpt2_busy = state;
}

READ8_MEMBER(samcoupe_state::samcoupe_lpt2_busy_r)
{
	return m_lpt2_busy;
}

WRITE8_MEMBER(samcoupe_state::samcoupe_lpt2_strobe_w)
{
	m_lpt2->write_strobe(data & 1);
}

/***************************************************************************
    ADDRESS MAPS
***************************************************************************/

static ADDRESS_MAP_START( samcoupe_mem, AS_PROGRAM, 8, samcoupe_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_READWRITE(sam_bank1_r, sam_bank1_w) // AM_RAMBANK("bank1")
	AM_RANGE(0x4000, 0x7fff) AM_RAM AM_READWRITE(sam_bank2_r, sam_bank2_w) // AM_RAMBANK("bank2")
	AM_RANGE(0x8000, 0xbfff) AM_RAM AM_READWRITE(sam_bank3_r, sam_bank3_w) // AM_RAMBANK("bank3")
	AM_RANGE(0xc000, 0xffff) AM_RAM AM_READWRITE(sam_bank4_r, sam_bank4_w) // AM_RAMBANK("bank4")
ADDRESS_MAP_END

static ADDRESS_MAP_START( samcoupe_io, AS_IO, 8, samcoupe_state )
	AM_RANGE(0x0080, 0x0081) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_WRITE(samcoupe_ext_mem_w)
	AM_RANGE(0x00e0, 0x00e7) AM_MIRROR(0xff10) AM_MASK(0xffff) AM_READWRITE(samcoupe_disk_r, samcoupe_disk_w)
	AM_RANGE(0x00e8, 0x00e8) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_DEVWRITE("lpt1_data_out", output_latch_device, write)
	AM_RANGE(0x00e9, 0x00e9) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_READWRITE(samcoupe_lpt1_busy_r, samcoupe_lpt1_strobe_w)
	AM_RANGE(0x00ea, 0x00ea) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_DEVWRITE("lpt2_data_out", output_latch_device, write)
	AM_RANGE(0x00eb, 0x00eb) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_READWRITE(samcoupe_lpt2_busy_r, samcoupe_lpt2_strobe_w)
	AM_RANGE(0x00f8, 0x00f8) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_READWRITE(samcoupe_pen_r, samcoupe_clut_w)
	AM_RANGE(0x00f9, 0x00f9) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_READWRITE(samcoupe_status_r, samcoupe_line_int_w)
	AM_RANGE(0x00fa, 0x00fa) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_READWRITE(samcoupe_lmpr_r, samcoupe_lmpr_w)
	AM_RANGE(0x00fb, 0x00fb) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_READWRITE(samcoupe_hmpr_r, samcoupe_hmpr_w)
	AM_RANGE(0x00fc, 0x00fc) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_READWRITE(samcoupe_vmpr_r, samcoupe_vmpr_w)
	AM_RANGE(0x00fd, 0x00fd) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_READWRITE(samcoupe_midi_r, samcoupe_midi_w)
	AM_RANGE(0x00fe, 0x00fe) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_READWRITE(samcoupe_keyboard_r, samcoupe_border_w)
	AM_RANGE(0x00ff, 0x00ff) AM_MIRROR(0xff00) AM_MASK(0xffff) AM_READ(samcoupe_attributes_r)
	AM_RANGE(0x00ff, 0x00ff) AM_MIRROR(0xfe00) AM_MASK(0xffff) AM_DEVWRITE("saa1099", saa1099_device, data_w)
	AM_RANGE(0x01ff, 0x01ff) AM_MIRROR(0xfe00) AM_MASK(0xffff) AM_DEVWRITE("saa1099", saa1099_device, control_w)
ADDRESS_MAP_END


/***************************************************************************
    INTERRUPTS
***************************************************************************/

TIMER_CALLBACK_MEMBER(samcoupe_state::irq_off)
{
	/* adjust STATUS register */
	m_status |= param;

	/* clear interrupt */
	if ((m_status & 0x1f) == 0x1f)
		m_maincpu->set_input_line(0, CLEAR_LINE);

}

void samcoupe_state::samcoupe_irq(UINT8 src)
{
	/* assert irq and a timer to set it off again */
	m_maincpu->set_input_line(0, ASSERT_LINE);
	timer_set(attotime::from_usec(20), TIMER_IRQ_OFF, src);

	/* adjust STATUS register */
	m_status &= ~src;
}

INTERRUPT_GEN_MEMBER(samcoupe_state::samcoupe_frame_interrupt)
{
	/* signal frame interrupt */
	samcoupe_irq(SAM_FRAME_INT);
}


/***************************************************************************
    INPUT PORTS
***************************************************************************/

static INPUT_PORTS_START( samcoupe )
	PORT_START("keyboard_row_fe")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)     PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)     PORT_CHAR('x') PORT_CHAR('X') PORT_CHAR('?')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)     PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)     PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD) PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("keyboard_row_fd")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)     PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)     PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)     PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)     PORT_CHAR('f') PORT_CHAR('F') PORT_CHAR('{')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)     PORT_CHAR('g') PORT_CHAR('G') PORT_CHAR('}')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD) PORT_CHAR(UCHAR_MAMEKEY(F6))

	PORT_START("keyboard_row_fb")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)     PORT_CHAR('q') PORT_CHAR('Q') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)     PORT_CHAR('w') PORT_CHAR('W') PORT_CHAR('>')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)     PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)     PORT_CHAR('r') PORT_CHAR('R') PORT_CHAR('[')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)     PORT_CHAR('t') PORT_CHAR('T') PORT_CHAR(']')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD) PORT_CHAR(UCHAR_MAMEKEY(F9))

	PORT_START("keyboard_row_f7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)        PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)        PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)        PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)        PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)        PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)    PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)      PORT_CHAR('\t')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK) PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("keyboard_row_ef")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)         PORT_CHAR('0') PORT_CHAR('~')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)         PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)         PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)         PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)         PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)     PORT_CHAR('-') PORT_CHAR('/')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)    PORT_CHAR('+') PORT_CHAR('*')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("DELETE") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)

	PORT_START("keyboard_row_df")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('=') PORT_CHAR('_')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR('"') PORT_CHAR('\xa9')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("F0") PORT_CODE(KEYCODE_0_PAD) PORT_CHAR(UCHAR_MAMEKEY(F10))

	PORT_START("keyboard_row_bf")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)     PORT_CHAR('l') PORT_CHAR('L') PORT_CHAR('\xa3')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)     PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)     PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)     PORT_CHAR('h') PORT_CHAR('H') PORT_CHAR('^')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR(':')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("EDIT") PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(INSERT))

	PORT_START("keyboard_row_7f")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("SYMBOL") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_SHIFT_2)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)     PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)     PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)     PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',')
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)  PORT_CHAR('.')
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("INV") PORT_CODE(KEYCODE_SLASH) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CHAR('\\')

	PORT_START("keyboard_row_ff")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("CTRL") PORT_CODE(KEYCODE_LALT) PORT_CHAR(UCHAR_MAMEKEY(LALT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)     PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)   PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)   PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)  PORT_CHAR(UCHAR_MAMEKEY(RIGHT))

	PORT_START("mouse_buttons")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("Mouse Button 1")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_CODE(MOUSECODE_BUTTON3) PORT_NAME("Mouse Button 3")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("Mouse Button 2")
	PORT_BIT(0xf8, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("mouse_x")
	PORT_BIT(0xfff, 0x000, IPT_MOUSE_X) PORT_SENSITIVITY(50) PORT_KEYDELTA(0) PORT_REVERSE

	PORT_START("mouse_y")
	PORT_BIT(0xfff, 0x000, IPT_MOUSE_Y) PORT_SENSITIVITY(50) PORT_KEYDELTA(0)

	PORT_START("config")
	PORT_CONFNAME(0x01, 0x00, "Real Time Clock")
	PORT_CONFSETTING(   0x00, DEF_STR(None))
	PORT_CONFSETTING(   0x01, "SAMBUS")
INPUT_PORTS_END


/***************************************************************************
    PALETTE
***************************************************************************/

/*
    Decode colours for the palette as follows:

    bit     7       6       5       4       3       2       1       0
         nothing   G+4     R+4     B+4    ALL+1    G+2     R+2     B+2

*/
PALETTE_INIT_MEMBER(samcoupe_state, samcoupe)
{
	for (int i = 0; i < 128; i++)
	{
		UINT8 b = BIT(i, 0) * 2 + BIT(i, 4) * 4 + BIT(i, 3);
		UINT8 r = BIT(i, 1) * 2 + BIT(i, 5) * 4 + BIT(i, 3);
		UINT8 g = BIT(i, 2) * 2 + BIT(i, 6) * 4 + BIT(i, 3);

		r <<= 5;
		g <<= 5;
		b <<= 5;

		palette.set_pen_color(i, rgb_t(r, g, b));
	}

	palette.palette()->normalize_range(0, 127);
}


/***************************************************************************
    MACHINE DRIVERS
***************************************************************************/

FLOPPY_FORMATS_MEMBER( samcoupe_state::floppy_formats )
	FLOPPY_MGT_FORMAT
FLOPPY_FORMATS_END

static SLOT_INTERFACE_START( samcoupe_floppies )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END


static MACHINE_CONFIG_START( samcoupe, samcoupe_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, SAMCOUPE_XTAL_X1 / 4) /* 6 MHz */
	MCFG_CPU_PROGRAM_MAP(samcoupe_mem)
	MCFG_CPU_IO_MAP(samcoupe_io)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", samcoupe_state,  samcoupe_frame_interrupt)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(SAMCOUPE_XTAL_X1/2, SAM_TOTAL_WIDTH, 0, SAM_BORDER_LEFT + SAM_SCREEN_WIDTH + SAM_BORDER_RIGHT, SAM_TOTAL_HEIGHT, 0, SAM_BORDER_TOP + SAM_SCREEN_HEIGHT + SAM_BORDER_BOTTOM)
	MCFG_SCREEN_UPDATE_DRIVER(samcoupe_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 128)
	MCFG_PALETTE_INIT_OWNER(samcoupe_state, samcoupe)

	/* devices */
	MCFG_CENTRONICS_ADD("lpt1", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(samcoupe_state, write_lpt1_busy))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("lpt1_data_out", "lpt1")

	MCFG_CENTRONICS_ADD("lpt2", centronics_devices, "printer")
	MCFG_CENTRONICS_BUSY_HANDLER(WRITELINE(samcoupe_state, write_lpt2_busy))

	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("lpt2_data_out", "lpt2")

	MCFG_DEVICE_ADD("sambus_clock", MSM6242, XTAL_32_768kHz)
	MCFG_CASSETTE_ADD("cassette")
	MCFG_CASSETTE_FORMATS(tzx_cassette_formats)
	MCFG_CASSETTE_DEFAULT_STATE(CASSETTE_STOPPED | CASSETTE_SPEAKER_ENABLED | CASSETTE_MOTOR_ENABLED)
	MCFG_CASSETTE_INTERFACE("samcoupe_cass")

	MCFG_SOFTWARE_LIST_ADD("cass_list","samcoupe_cass")

	MCFG_WD1772_ADD("wd1772", SAMCOUPE_XTAL_X1/3)
	MCFG_FLOPPY_DRIVE_ADD("wd1772:0", samcoupe_floppies, "35dd", samcoupe_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("wd1772:1", samcoupe_floppies, "35dd", samcoupe_state::floppy_formats)
	MCFG_SOFTWARE_LIST_ADD("flop_list","samcoupe_flop")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
	MCFG_SAA1099_ADD("saa1099", SAMCOUPE_XTAL_X1/3) /* 8 MHz */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)


	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
	MCFG_RAM_EXTRA_OPTIONS("256K,1280K,1536K,2304K,2560K,3328K,3584K,4352K,4608K")
MACHINE_CONFIG_END


/***************************************************************************
    ROM DEFINITIONS
***************************************************************************/

/*
    The bios is actually 32K. This is the combined version of the two old 16K MESS roms.
    It does match the 3.0 one the most, but the first half differs in one byte
    and in the second half, the case of the "plc" in the company string differs.
*/
ROM_START( samcoupe )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0,  "31",  "v3.1" )
	ROMX_LOAD( "rom31.z5",  0x0000, 0x8000, CRC(0b7e3585) SHA1(c86601633fb61a8c517f7657aad9af4e6870f2ee), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1,  "30",  "v3.0" )
	ROMX_LOAD( "rom30.z5",  0x0000, 0x8000, CRC(e535c25d) SHA1(d390f0be420dfb12b1e54a4f528b5055d7d97e2a), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2,  "25",  "v2.5" )
	ROMX_LOAD( "rom25.z5",  0x0000, 0x8000, CRC(ddadd358) SHA1(a25ed85a0f1134ac3a481a3225f24a8bd3a790cf), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 3,  "24",  "v2.4" )
	ROMX_LOAD( "rom24.z5",  0x0000, 0x8000, CRC(bb23fee4) SHA1(10cd911ba237dd2cf0c2637be1ad6745b7cc89b9), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 4,  "21",  "v2.1" )
	ROMX_LOAD( "rom21.z5",  0x0000, 0x8000, CRC(f6804b46) SHA1(11dcac5fdea782cdac03b4d0d7ac25d88547eefe), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 5,  "20",  "v2.0" )
	ROMX_LOAD( "rom20.z5",  0x0000, 0x8000, CRC(eaf32054) SHA1(41736323f0236649f2d5fe111f900def8db93a13), ROM_BIOS(6) )
	ROM_SYSTEM_BIOS( 6,  "181", "v1.81" )
	ROMX_LOAD( "rom181.z5", 0x0000, 0x8000, CRC(d25e1de1) SHA1(cb0fa79e4d5f7df0b57ede08ea7ecc9ae152f534), ROM_BIOS(7) )
	ROM_SYSTEM_BIOS( 7,  "18",  "v1.8" )
	ROMX_LOAD( "rom18.z5",  0x0000, 0x8000, CRC(f626063f) SHA1(485e7d9e9a4f8a70c0f93cd6e69ff12269438829), ROM_BIOS(8) )
	ROM_SYSTEM_BIOS( 8,  "14",  "v1.4" )
	ROMX_LOAD( "rom14.z5",  0x0000, 0x8000, CRC(08799596) SHA1(b4e596051f2748dee9481ea4af7d15ccddc1e1b5), ROM_BIOS(9) )
	ROM_SYSTEM_BIOS( 9,  "13",  "v1.3" )
	ROMX_LOAD( "rom13.z5",  0x0000, 0x8000, CRC(2093768c) SHA1(af8d348fd080b18a4cbe9ed69d254be7be330146), ROM_BIOS(10) )
	ROM_SYSTEM_BIOS( 10, "12",  "v1.2" )
	ROMX_LOAD( "rom12.z5",  0x0000, 0x8000, CRC(7fe37dd8) SHA1(9339a0c1f72e8512c6f32dec15ab7d6c3bb04151), ROM_BIOS(11) )
	ROM_SYSTEM_BIOS( 11, "10",  "v1.0" )
	ROMX_LOAD( "rom10.z5",  0x0000, 0x8000, CRC(3659d31f) SHA1(d3de7bb74e04d5b4dc7477f70de54d540b1bcc07), ROM_BIOS(12) )
	ROM_SYSTEM_BIOS( 12, "04",  "v0.4" )
	ROMX_LOAD( "rom04.z5",  0x0000, 0x8000, CRC(f439e84e) SHA1(8bc457a5c764b0bb0aa7008c57f28c30248fc6a4), ROM_BIOS(13) )
	ROM_SYSTEM_BIOS( 13, "01",  "v0.1" )
	ROMX_LOAD( "rom01.z5",  0x0000, 0x8000, CRC(c04acfdf) SHA1(8976ed005c14905eec1215f0a5c28aa686a7dda2), ROM_BIOS(14) )
ROM_END

/***************************************************************************
    GAME DRIVERS
***************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     INIT  COMPANY                        FULLNAME     FLAGS */
COMP( 1989, samcoupe, 0,      0,      samcoupe, samcoupe, driver_device, 0,    "Miles Gordon Technology plc", "SAM Coupe", 0 )
