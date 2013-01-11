/***************************************************************************

        Irisha machine driver by Miodrag Milanovic

        27/03/2008 Preliminary driver.

****************************************************************************/


#include "includes/irisha.h"


/* Driver initialization */
DRIVER_INIT_MEMBER(irisha_state,irisha)
{
	m_keyboard_mask = 0;
}



TIMER_CALLBACK_MEMBER(irisha_state::irisha_key)
{
	m_keypressed = 1;
	m_keyboard_cnt = 0;
}

void irisha_state::machine_start()
{
	machine().scheduler().timer_pulse(attotime::from_msec(30), timer_expired_delegate(FUNC(irisha_state::irisha_key),this));
}

void irisha_state::machine_reset()
{
	m_keypressed = 0;
}

void irisha_state::update_speaker()
{
	int level = ((m_ppi_portc & 0x20) || (m_ppi_porta & 0x10) || !m_sg1_line) ? 1 : 0;

	speaker_level_w(m_speaker, level);
}

static const char *const keynames[] = {
							"LINE0", "LINE1", "LINE2", "LINE3",
							"LINE4", "LINE5", "LINE6", "LINE7",
							"LINE8", "LINE9"
};

READ8_MEMBER(irisha_state::irisha_8255_portb_r)
{
	if (m_keypressed==1) {
		m_keypressed = 0;
		return 0x80;
	}

	return 0x00;
}

READ8_MEMBER(irisha_state::irisha_8255_portc_r)
{
	logerror("irisha_8255_portc_r\n");
	return 0;
}

READ8_MEMBER(irisha_state::irisha_keyboard_r)
{
	UINT8 keycode;
	if (m_keyboard_cnt!=0 && m_keyboard_cnt<11) {
		keycode = ioport(keynames[m_keyboard_cnt-1])->read() ^ 0xff;
	} else {
		keycode = 0xff;
	}
	m_keyboard_cnt++;
	return keycode;
}

WRITE8_MEMBER(irisha_state::irisha_8255_porta_w)
{
	logerror("irisha_8255_porta_w %02x\n",data);

	m_ppi_porta = data;

	update_speaker();
}

WRITE8_MEMBER(irisha_state::irisha_8255_portb_w)
{
	logerror("irisha_8255_portb_w %02x\n",data);
}

WRITE8_MEMBER(irisha_state::irisha_8255_portc_w)
{
	//logerror("irisha_8255_portc_w %02x\n",data);

	if (data & 0x40)
		pit8253_gate2_w(m_pit, (BIT(m_ppi_porta,5) && !BIT(data,5)) ? 1 : 0);

	m_ppi_portc = data;

	update_speaker();
}

WRITE_LINE_MEMBER(irisha_state::speaker_w)
{
	m_sg1_line = state;
	update_speaker();
}

I8255A_INTERFACE( irisha_ppi8255_interface )
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(irisha_state, irisha_8255_porta_w),
	DEVCB_DRIVER_MEMBER(irisha_state, irisha_8255_portb_r),
	DEVCB_DRIVER_MEMBER(irisha_state, irisha_8255_portb_w),
	DEVCB_DRIVER_MEMBER(irisha_state, irisha_8255_portc_r),
	DEVCB_DRIVER_MEMBER(irisha_state, irisha_8255_portc_w),
};

WRITE_LINE_MEMBER(irisha_state::irisha_pic_set_int_line)
{
	machine().device("maincpu")->execute().set_input_line(0, state ? HOLD_LINE : CLEAR_LINE);
}

const struct pic8259_interface irisha_pic8259_config =
{
	DEVCB_DRIVER_LINE_MEMBER(irisha_state,irisha_pic_set_int_line),
	DEVCB_LINE_VCC,
	DEVCB_NULL
};

const struct pit8253_config irisha_pit8253_intf =
{
	{
		{
			XTAL_16MHz / 9,
			DEVCB_LINE_VCC,
			DEVCB_DEVICE_LINE("pic8259", pic8259_ir0_w)
		},
		{
			XTAL_16MHz / 9 / 8 / 8,
			DEVCB_LINE_VCC,
			DEVCB_NULL          // UART transmit/receive clock
		},
		{
			XTAL_16MHz / 9,
			DEVCB_NULL,
			DEVCB_DRIVER_LINE_MEMBER(irisha_state, speaker_w)
		}
	}
};
