// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/******************************************************************************
    Acorn Electron driver

    MESS Driver By:

    Wilbert Pol

******************************************************************************/

#include "emu.h"
#include "includes/electron.h"
#include "screen.h"


void electron_state::waitforramsync()
{
	int cycles = 0;

	if (!(m_ula.screen_mode & 4) && (m_screen->vpos() > m_screen->visible_area().top()) && (m_screen->vpos() < m_screen->visible_area().bottom()) && !m_screen->hblank())
	{
		cycles += (m_screen->visible_area().right() - m_screen->hpos()) / 16;
	}
	if (cycles & 1) cycles++;

	m_maincpu->adjust_icount(-cycles);
}


void electron_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_TAPE_HANDLER:
		electron_tape_timer_handler(ptr, param);
		break;
	case TIMER_SETUP_BEEP:
		setup_beep(ptr, param);
		break;
	case TIMER_SCANLINE_INTERRUPT:
		electron_scanline_interrupt(ptr, param);
		break;
	default:
		assert_always(false, "Unknown id in electron_state::device_timer");
	}
}


void electron_state::electron_tape_start()
{
	if (m_ula.tape_running )
	{
		return;
	}
	m_ula.tape_steps = 0;
	m_ula.tape_value = 0x80808080;
	m_ula.high_tone_set = 0;
	m_ula.bit_count = 0;
	m_ula.tape_running = 1;
	m_tape_timer->adjust(attotime::zero, 0, attotime::from_hz(4800));
}

void electron_state::electron_tape_stop()
{
	m_ula.tape_running = 0;
	m_tape_timer->reset();
}

#define TAPE_LOW    0x00;
#define TAPE_HIGH   0xFF;

TIMER_CALLBACK_MEMBER(electron_state::electron_tape_timer_handler)
{
	if ( m_ula.cassette_motor_mode )
	{
		double tap_val;
		tap_val = m_cassette->input();
		if ( tap_val < -0.5 )
		{
			m_ula.tape_value = ( m_ula.tape_value << 8 ) | TAPE_LOW;
			m_ula.tape_steps++;
		}
		else if ( tap_val > 0.5 )
		{
			m_ula.tape_value = ( m_ula.tape_value << 8 ) | TAPE_HIGH;
			m_ula.tape_steps++;
		}
		else
		{
			m_ula.tape_steps = 0;
			m_ula.bit_count = 0;
			m_ula.high_tone_set = 0;
			m_ula.tape_value = 0x80808080;
		}
		if ( m_ula.tape_steps > 2 && ( m_ula.tape_value == 0x0000FFFF || m_ula.tape_value == 0x00FF00FF ) )
		{
			m_ula.tape_steps = 0;
			switch( m_ula.bit_count )
			{
			case 0: /* start bit */
				m_ula.start_bit = ( ( m_ula.tape_value == 0x0000FFFF ) ? 0 : 1 );
				//logerror( "++ Read start bit: %d\n", m_ula.start_bit );
				if ( m_ula.start_bit )
				{
					if ( m_ula.high_tone_set )
					{
						m_ula.bit_count--;
					}
				}
				else
				{
					m_ula.high_tone_set = 0;
				}
				break;
			case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:
				//logerror( "++ Read regular bit: %d\n", m_ula.tape_value == 0x0000FFFF ? 0 : 1 );
				m_ula.tape_byte = ( m_ula.tape_byte >> 1 ) | ( m_ula.tape_value == 0x0000FFFF ? 0 : 0x80 );
				break;
			case 9: /* stop bit */
				m_ula.stop_bit = ( ( m_ula.tape_value == 0x0000FFFF ) ? 0 : 1 );
				//logerror( "++ Read stop bit: %d\n", m_ula.stop_bit );
				if ( m_ula.start_bit && m_ula.stop_bit && m_ula.tape_byte == 0xFF && ! m_ula.high_tone_set )
				{
					electron_interrupt_handler( INT_SET, INT_HIGH_TONE );
					m_ula.high_tone_set = 1;
				}
				else if ( ! m_ula.start_bit && m_ula.stop_bit )
				{
					//logerror( "-- Byte read from tape: %02x\n", m_ula.tape_byte );
					electron_interrupt_handler( INT_SET, INT_RECEIVE_FULL );
				}
				else
				{
					logerror( "Invalid start/stop bit combination detected: %d,%d\n", m_ula.start_bit, m_ula.stop_bit );
				}
				break;
			}
			m_ula.bit_count = ( m_ula.bit_count + 1 ) % 10;
		}
	}
}

READ8_MEMBER(electron_state::electron64_fetch_r)
{
	m_vdu_drivers = (offset & 0xe000) == 0xc000 ? true : false;

	return m_maincpu->space(AS_PROGRAM).read_byte(offset);
}

READ8_MEMBER(electron_state::electron_mem_r)
{
	switch (m_mrb.read_safe(0))
	{
	case 0x00: /* Normal */
		/* The processor will run at 1MHz during an access cycle to the RAM */
		m_maincpu->set_clock_scale(0.5f);
		waitforramsync();
		break;

	case 0x01: /* Turbo */
		if (m_mrb_mapped && offset < 0x3000) offset += 0x8000;
		break;

	case 0x02: /* Shadow */
		if (m_mrb_mapped && (offset < 0x3000 || !m_vdu_drivers)) offset += 0x8000;
		break;
	}
	return m_ram->read(offset);
}

WRITE8_MEMBER(electron_state::electron_mem_w)
{
	switch (m_mrb.read_safe(0))
	{
	case 0x00: /* Normal */
		/* The processor will run at 1MHz during an access cycle to the RAM */
		m_maincpu->set_clock_scale(0.5f);
		waitforramsync();
		break;

	case 0x01: /* Turbo */
		if (m_mrb_mapped && offset < 0x3000) offset += 0x8000;
		break;

	case 0x02: /* Shadow */
		if (m_mrb_mapped && (offset < 0x3000 || !m_vdu_drivers)) offset += 0x8000;
		break;
	}
	m_ram->write(offset, data);
}

READ8_MEMBER(electron_state::electron_paged_r)
{
	/*  0 Second external socket on the expansion module (SK2) */
	/*  1 Second external socket on the expansion module (SK2) */
	/*  2 First external socket on the expansion module (SK1)  */
	/*  3 First external socket on the expansion module (SK1)  */
	/*  4 Disc                                                 */
	/*  5 USER applications                                    */
	/*  6 USER applications                                    */
	/*  7 Modem interface ROM                                  */
	/*  8 Keyboard                                             */
	/*  9 Keyboard mirror                                      */
	/* 10 BASIC rom                                            */
	/* 11 BASIC rom mirror                                     */
	/* 12 Expansion module operating system                    */
	/* 13 High priority slot in expansion module               */
	/* 14 ECONET                                               */
	/* 15 Reserved                                             */

	uint8_t data = 0;

	/* The processor will run at 2MHz during an access cycle to the ROM */
	m_maincpu->set_clock_scale(1.0f);

	switch (m_ula.rompage)
	{
	case 8:
	case 9:
		/* Keyboard */
		for (int i = 0; i < 14; i++)
		{
			if (!(offset & 1))
				data |= m_keybd[i]->read() & 0x0f;

			offset = offset >> 1;
		}
		break;

	case 10:
	case 11:
		/* BASIC */
		data = m_region_basic->base()[offset & 0x3fff];
		break;

	default:
		/* ROM in extension devices */
		data = m_exp->expbus_r(0x8000 + offset);
		break;
	}
	return data;
}

WRITE8_MEMBER(electron_state::electron_paged_w)
{
	/* The processor will run at 2MHz during an access cycle to the ROM */
	m_maincpu->set_clock_scale(1.0f);

	m_exp->expbus_w(0x8000 + offset, data);
}

READ8_MEMBER(electron_state::electron_mos_r)
{
	/* The processor will run at 2MHz during an access cycle to the ROM */
	m_maincpu->set_clock_scale(1.0f);

	return m_region_mos->base()[offset & 0x3fff];
}

WRITE8_MEMBER(electron_state::electron_mos_w)
{
	/* The processor will run at 2MHz during an access cycle to the ROM */
	m_maincpu->set_clock_scale(1.0f);

	logerror("MOS: write %04x %02x\n", offset + 0xc000, data);
	m_exp->expbus_w(0xc000 + offset, data);
}

READ8_MEMBER(electron_state::electron_fred_r)
{
	/* The processor will run at 2MHz during an access cycle to the ROM */
	m_maincpu->set_clock_scale(1.0f);

	/* The Issue 4 ULA returns data from OS ROM, whereas Issue 6 ULA will return 0xff */
	//logerror("FRED: read fc%02x\n", offset);
	return m_exp->expbus_r(0xfc00 + offset);
}

WRITE8_MEMBER(electron_state::electron_fred_w)
{
	/* The processor will run at 2MHz during an access cycle to the ROM */
	m_maincpu->set_clock_scale(1.0f);

	/* Master RAM Board */
	if (offset == 0x7f) m_mrb_mapped = !(data & 0x80);

	//logerror("FRED: write fc%02x\n", offset);
	m_exp->expbus_w(0xfc00 + offset, data);
}

READ8_MEMBER(electron_state::electron_jim_r)
{
	/* The processor will run at 2MHz during an access cycle to the ROM */
	m_maincpu->set_clock_scale(1.0f);

	/* The Issue 4 ULA returns data from OS ROM, whereas Issue 6 ULA will return 0xff */
	//logerror("JIM: read fd%02x\n", offset);
	return m_exp->expbus_r(0xfd00 + offset);
}

WRITE8_MEMBER(electron_state::electron_jim_w)
{
	/* The processor will run at 2MHz during an access cycle to the ROM */
	m_maincpu->set_clock_scale(1.0f);

	//logerror("JIM: write fd%02x\n", offset);
	m_exp->expbus_w(0xfd00 + offset, data);
}

READ8_MEMBER(electron_state::electron_sheila_r)
{
	/* The processor will run at 2MHz during an access cycle to the ROM */
	m_maincpu->set_clock_scale(1.0f);

	/* The Issue 4 ULA returns data from OS ROM, whereas Issue 6 ULA will return 0xfe */
	uint8_t data = 0xff;

	switch ( offset & 0x0f )
	{
	case 0x00:  /* Interrupt status */
		data = m_ula.interrupt_status;
		m_ula.interrupt_status &= ~0x02;
		break;
	case 0x01:  /* Unknown */
		break;
	case 0x04:  /* Cassette data shift register */
		electron_interrupt_handler(INT_CLEAR, INT_RECEIVE_FULL );
		data = m_ula.tape_byte;
		break;
	}
	//logerror( "ULA: read fe%02x: %02x\n", offset, data );
	return data;
}

static const int electron_palette_offset[4] = { 0, 4, 5, 1 };
static const uint16_t electron_screen_base[8] = { 0x3000, 0x3000, 0x3000, 0x4000, 0x5800, 0x5800, 0x6000, 0x6000 };
static const int electron_mode_end[8] = { 255, 255, 255 ,249 ,255, 255, 249, 249 };

WRITE8_MEMBER(electron_state::electron_sheila_w)
{
	/* The processor will run at 2MHz during an access cycle to the ROM */
	m_maincpu->set_clock_scale(1.0f);

	m_exp->expbus_w(0xfe00 + offset, data);

	int i = electron_palette_offset[(( offset >> 1 ) & 0x03)];
	//logerror( "ULA: write fe%02x <- %02x\n", offset & 0x0f, data );
	switch( offset & 0x0f )
	{
	case 0x00:  /* Interrupt control */
		m_ula.interrupt_control = data;
		break;
	case 0x01:  /* Unknown */
		break;
	case 0x02:  /* Screen start address #1 */
		m_ula.screen_start = ( m_ula.screen_start & 0x7e00 ) | ( ( data & 0xe0 ) << 1 );
		logerror( "screen_start changed to %04x\n", m_ula.screen_start );
		break;
	case 0x03:  /* Screen start address #2 */
		m_ula.screen_start = ( m_ula.screen_start & 0x1ff ) | ( ( data & 0x3f ) << 9 );
		logerror( "screen_start changed to %04x\n", m_ula.screen_start );
		break;
	case 0x04:  /* Cassette data shift register */
		break;
	case 0x05:  /* Interrupt clear and paging */
		/* rom page requests are honoured when currently bank 0-7 or 12-15 is switched in,
		 * or when 8-11 is currently switched in only switching to bank 8-15 is allowed.
		 *
		 * Rompages 10 and 11 both select the Basic ROM.
		 * Rompages 8 and 9 both select the keyboard.
		 */
		if ( ( ( m_ula.rompage & 0x0C ) != 0x08 ) || ( data & 0x08 ) )
		{
			m_ula.rompage = data & 0x0f;
			if ( m_ula.rompage == 8 || m_ula.rompage == 9 )
			{
				m_ula.rompage = 8;
			}
		}
		if ( data & 0x10 )
		{
			electron_interrupt_handler( INT_CLEAR, INT_DISPLAY_END );
		}
		if ( data & 0x20 )
		{
			electron_interrupt_handler( INT_CLEAR, INT_RTC );
		}
		if ( data & 0x40 )
		{
			electron_interrupt_handler( INT_CLEAR, INT_HIGH_TONE );
		}
		if ( data & 0x80 )
		{
		}
		break;
	case 0x06:  /* Counter divider */
		if ( m_ula.communication_mode == 0x01)
		{
		/* GUESS
		 * the Advanced Users manual says this is the correct algorithm
		 * but the divider is wrong(?), says 16 but results in high pitch,
		 * 32 is more close
		 */
			m_beeper->set_clock( 1000000 / ( 32 * ( data + 1 ) ) );
		}
		break;
	case 0x07:  /* Misc. */
		m_ula.communication_mode = ( data >> 1 ) & 0x03;
		switch( m_ula.communication_mode )
		{
		case 0x00:  /* cassette input */
			m_beeper->set_state( 0 );
			electron_tape_start();
			break;
		case 0x01:  /* sound generation */
			m_beeper->set_state( 1 );
			electron_tape_stop();
			break;
		case 0x02:  /* cassette output */
			m_beeper->set_state( 0 );
			electron_tape_stop();
			break;
		case 0x03:  /* not used */
			m_beeper->set_state( 0 );
			electron_tape_stop();
			break;
		}
		m_ula.screen_mode = ( data >> 3 ) & 0x07;
		m_ula.screen_base = electron_screen_base[ m_ula.screen_mode ];
		m_ula.screen_size = 0x8000 - m_ula.screen_base;
		m_ula.screen_dispend = electron_mode_end[ m_ula.screen_mode ];
		logerror( "ULA: screen mode set to %d\n", m_ula.screen_mode );
		m_ula.cassette_motor_mode = ( data >> 6 ) & 0x01;
		m_cassette->change_state(m_ula.cassette_motor_mode ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MOTOR_DISABLED );
		m_ula.capslock_mode = ( data >> 7 ) & 0x01;
		output().set_value("capslock_led", m_ula.capslock_mode);
		break;
	case 0x08: case 0x0a: case 0x0c: case 0x0e:
		/* colour palette */
		m_ula.current_pal[i+10] = (m_ula.current_pal[i+10] & 0x01) | (((data & 0x80) >> 5) | ((data & 0x08) >> 2));
		m_ula.current_pal[i+8] = (m_ula.current_pal[i+8] & 0x01) | (((data & 0x40) >> 4) | ((data & 0x04) >> 1));
		m_ula.current_pal[i+2] = (m_ula.current_pal[i+2] & 0x03) | ((data & 0x20) >> 3);
		m_ula.current_pal[i] = (m_ula.current_pal[i] & 0x03) | ((data & 0x10) >> 2);
		break;
	case 0x09: case 0x0b: case 0x0d: case 0x0f:
		/* colour palette */
		m_ula.current_pal[i+10] = (m_ula.current_pal[i+10] & 0x06) | ((data & 0x08) >> 3);
		m_ula.current_pal[i+8] = (m_ula.current_pal[i+8] & 0x06) | ((data & 0x04) >> 2);
		m_ula.current_pal[i+2] = (m_ula.current_pal[i+2] & 0x04) | (((data & 0x20) >> 4) | ((data & 0x02) >> 1));
		m_ula.current_pal[i] = (m_ula.current_pal[i] & 0x04) | (((data & 0x10) >> 3) | ((data & 0x01)));
		break;
	}
}

void electron_state::electron_interrupt_handler(int mode, int interrupt)
{
	if ( mode == INT_SET )
	{
		m_ula.interrupt_status |= interrupt;
	}
	else
	{
		m_ula.interrupt_status &= ~interrupt;
	}
	if ( m_ula.interrupt_status & m_ula.interrupt_control & ~0x83 )
	{
		m_ula.interrupt_status |= 0x01;
		m_maincpu->set_input_line(0, ASSERT_LINE );
	}
	else
	{
		m_ula.interrupt_status &= ~0x01;
		m_maincpu->set_input_line(0, CLEAR_LINE );
	}
}

/**************************************
   Machine Initialisation functions
***************************************/

TIMER_CALLBACK_MEMBER(electron_state::setup_beep)
{
	m_beeper->set_state( 0 );
	m_beeper->set_clock( 300 );
}

void electron_state::machine_reset()
{
	m_ula.communication_mode = 0x04;
	m_ula.screen_mode = 0;
	m_ula.cassette_motor_mode = 0;
	m_ula.capslock_mode = 0;
	m_ula.screen_start = 0x3000;
	m_ula.screen_base = 0x3000;
	m_ula.screen_size = 0x8000 - 0x3000;
	m_ula.screen_addr = 0x3000;
	m_ula.tape_running = 0;

	m_mrb_mapped = true;
	m_vdu_drivers = false;
}

void electron_state::machine_start()
{
	m_ula.interrupt_status = 0x82;
	m_ula.interrupt_control = 0x00;
	timer_set(attotime::zero, TIMER_SETUP_BEEP);
	m_tape_timer = timer_alloc(TIMER_TAPE_HANDLER);
}
