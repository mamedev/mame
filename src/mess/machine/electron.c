/******************************************************************************
    Acorn Electron driver

    MESS Driver By:

    Wilbert Pol

******************************************************************************/

#include "emu.h"
#include "includes/electron.h"
#include "sound/beep.h"
#include "imagedev/cassette.h"


static cassette_image_device *cassette_device_image( running_machine &machine )
{
	return machine.device<cassette_image_device>(CASSETTE_TAG);
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

#define TAPE_LOW	0x00;
#define TAPE_HIGH	0xFF;

static TIMER_CALLBACK(electron_tape_timer_handler)
{
	electron_state *state = machine.driver_data<electron_state>();
	if ( state->m_ula.cassette_motor_mode )
	{
		double tap_val;
		tap_val = cassette_device_image(machine)->input();
		if ( tap_val < -0.5 )
		{
			state->m_ula.tape_value = ( state->m_ula.tape_value << 8 ) | TAPE_LOW;
			state->m_ula.tape_steps++;
		}
		else if ( tap_val > 0.5 )
		{
			state->m_ula.tape_value = ( state->m_ula.tape_value << 8 ) | TAPE_HIGH;
			state->m_ula.tape_steps++;
		}
		else
		{
			state->m_ula.tape_steps = 0;
			state->m_ula.bit_count = 0;
			state->m_ula.high_tone_set = 0;
			state->m_ula.tape_value = 0x80808080;
		}
		if ( state->m_ula.tape_steps > 2 && ( state->m_ula.tape_value == 0x0000FFFF || state->m_ula.tape_value == 0x00FF00FF ) )
		{
			state->m_ula.tape_steps = 0;
			switch( state->m_ula.bit_count )
			{
			case 0:	/* start bit */
				state->m_ula.start_bit = ( ( state->m_ula.tape_value == 0x0000FFFF ) ? 0 : 1 );
				//logerror( "++ Read start bit: %d\n", state->m_ula.start_bit );
				if ( state->m_ula.start_bit )
				{
					if ( state->m_ula.high_tone_set )
					{
						state->m_ula.bit_count--;
					}
				}
				else
				{
					state->m_ula.high_tone_set = 0;
				}
				break;
			case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:
				//logerror( "++ Read regular bit: %d\n", state->m_ula.tape_value == 0x0000FFFF ? 0 : 1 );
				state->m_ula.tape_byte = ( state->m_ula.tape_byte >> 1 ) | ( state->m_ula.tape_value == 0x0000FFFF ? 0 : 0x80 );
				break;
			case 9: /* stop bit */
				state->m_ula.stop_bit = ( ( state->m_ula.tape_value == 0x0000FFFF ) ? 0 : 1 );
				//logerror( "++ Read stop bit: %d\n", state->m_ula.stop_bit );
				if ( state->m_ula.start_bit && state->m_ula.stop_bit && state->m_ula.tape_byte == 0xFF && ! state->m_ula.high_tone_set )
				{
					electron_interrupt_handler( machine, INT_SET, INT_HIGH_TONE );
					state->m_ula.high_tone_set = 1;
				}
				else if ( ! state->m_ula.start_bit && state->m_ula.stop_bit )
				{
					//logerror( "-- Byte read from tape: %02x\n", state->m_ula.tape_byte );
					electron_interrupt_handler( machine, INT_SET, INT_RECEIVE_FULL );
				}
				else
				{
					logerror( "Invalid start/stop bit combination detected: %d,%d\n", state->m_ula.start_bit, state->m_ula.stop_bit );
				}
				break;
			}
			state->m_ula.bit_count = ( state->m_ula.bit_count + 1 ) % 10;
		}
	}
}

READ8_MEMBER(electron_state::electron_read_keyboard)
{
	UINT8 data = 0;
	int i;
	static const char *const keynames[] = {
		"LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6",
		"LINE7", "LINE8", "LINE9", "LINE10", "LINE11", "LINE12", "LINE13"
	};

	//logerror( "PC=%04x: keyboard read from paged rom area, address: %04x", activecpu_get_pc(), offset );
	for( i = 0; i < 14; i++ )
	{
		if( !(offset & 1) )
			data |= ioport(keynames[i])->read() & 0x0f;

		offset = offset >> 1;
	}
	//logerror( ", data: %02x\n", data );
	return data;
}

READ8_MEMBER(electron_state::electron_jim_r)
{
	return 0xff;
}

WRITE8_MEMBER(electron_state::electron_jim_w)
{
}

READ8_MEMBER(electron_state::electron_1mhz_r)
{
	return 0xff;
}

WRITE8_MEMBER(electron_state::electron_1mhz_w)
{
}

READ8_MEMBER(electron_state::electron_ula_r)
{
	UINT8 data = ((UINT8 *)machine().root_device().memregion("user1")->base())[0x43E00 + offset];
	switch ( offset & 0x0f )
	{
	case 0x00:	/* Interrupt status */
		data = m_ula.interrupt_status;
		m_ula.interrupt_status &= ~0x02;
		break;
	case 0x01:	/* Unknown */
		break;
	case 0x04:	/* Casette data shift register */
		electron_interrupt_handler(machine(), INT_CLEAR, INT_RECEIVE_FULL );
		data = m_ula.tape_byte;
		break;
	}
	logerror( "ULA: read offset %02x: %02x\n", offset, data );
	return data;
}

static const int electron_palette_offset[4] = { 0, 4, 5, 1 };
static const UINT16 electron_screen_base[8] = { 0x3000, 0x3000, 0x3000, 0x4000, 0x5800, 0x5800, 0x6000, 0x5800 };

WRITE8_MEMBER(electron_state::electron_ula_w)
{
	device_t *speaker = machine().device(BEEPER_TAG);
	int i = electron_palette_offset[(( offset >> 1 ) & 0x03)];
	logerror( "ULA: write offset %02x <- %02x\n", offset & 0x0f, data );
	switch( offset & 0x0f )
	{
	case 0x00:	/* Interrupt control */
		m_ula.interrupt_control = data;
		break;
	case 0x01:	/* Unknown */
		break;
	case 0x02:	/* Screen start address #1 */
		m_ula.screen_start = ( m_ula.screen_start & 0x7e00 ) | ( ( data & 0xe0 ) << 1 );
		logerror( "screen_start changed to %04x\n", m_ula.screen_start );
		break;
	case 0x03:	/* Screen start addres #2 */
		m_ula.screen_start = ( m_ula.screen_start & 0x1c0 ) | ( ( data & 0x3f ) << 9 );
		logerror( "screen_start changed to %04x\n", m_ula.screen_start );
		break;
	case 0x04:	/* Cassette data shift register */
		break;
	case 0x05:	/* Interrupt clear and paging */
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
				space.install_read_handler( 0x8000, 0xbfff, read8_delegate(FUNC(electron_state::electron_read_keyboard),this));
			}
			else
			{
				space.install_read_bank( 0x8000, 0xbfff, "bank2");
			}
			membank("bank2")->set_entry(m_ula.rompage);
		}
		if ( data & 0x10 )
		{
			electron_interrupt_handler( machine(), INT_CLEAR, INT_DISPLAY_END );
		}
		if ( data & 0x20 )
		{
			electron_interrupt_handler( machine(), INT_CLEAR, INT_RTC );
		}
		if ( data & 0x40 )
		{
			electron_interrupt_handler( machine(), INT_CLEAR, INT_HIGH_TONE );
		}
		if ( data & 0x80 )
		{
		}
		break;
	case 0x06:	/* Counter divider */
		if ( m_ula.communication_mode == 0x01)
		{
			beep_set_frequency( speaker, 1000000 / ( 16 * ( data + 1 ) ) );
		}
		break;
	case 0x07:	/* Misc. */
		m_ula.communication_mode = ( data >> 1 ) & 0x03;
		switch( m_ula.communication_mode )
		{
		case 0x00:	/* cassette input */
			beep_set_state( speaker, 0 );
			electron_tape_start();
			break;
		case 0x01:	/* sound generation */
			beep_set_state( speaker, 1 );
			electron_tape_stop();
			break;
		case 0x02:	/* cassette output */
			beep_set_state( speaker, 0 );
			electron_tape_stop();
			break;
		case 0x03:	/* not used */
			beep_set_state( speaker, 0 );
			electron_tape_stop();
			break;
		}
		m_ula.screen_mode = ( data >> 3 ) & 0x07;
		m_ula.screen_base = electron_screen_base[ m_ula.screen_mode ];
		m_ula.screen_size = 0x8000 - m_ula.screen_base;
		m_ula.vram = (UINT8 *)space.get_read_ptr(m_ula.screen_base );
		logerror( "ULA: screen mode set to %d\n", m_ula.screen_mode );
		m_ula.cassette_motor_mode = ( data >> 6 ) & 0x01;
		cassette_device_image(machine())->change_state(m_ula.cassette_motor_mode ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MOTOR_DISABLED );
		m_ula.capslock_mode = ( data >> 7 ) & 0x01;
		break;
	case 0x08: case 0x0A: case 0x0C: case 0x0E:
		// video_update
		m_ula.current_pal[i+10] = (m_ula.current_pal[i+10] & 0x01) | (((data & 0x80) >> 5) | ((data & 0x08) >> 1));
		m_ula.current_pal[i+8] = (m_ula.current_pal[i+8] & 0x01) | (((data & 0x40) >> 4) | ((data & 0x04) >> 1));
		m_ula.current_pal[i+2] = (m_ula.current_pal[i+2] & 0x03) | ((data & 0x20) >> 3);
		m_ula.current_pal[i] = (m_ula.current_pal[i] & 0x03) | ((data & 0x10) >> 2);
		break;
	case 0x09: case 0x0B: case 0x0D: case 0x0F:
		// video_update
		m_ula.current_pal[i+10] = (m_ula.current_pal[i+10] & 0x06) | ((data & 0x08) >> 3);
		m_ula.current_pal[i+8] = (m_ula.current_pal[i+8] & 0x06) | ((data & 0x04) >> 2);
		m_ula.current_pal[i+2] = (m_ula.current_pal[i+2] & 0x04) | (((data & 0x20) >> 4) | ((data & 0x02) >> 1));
		m_ula.current_pal[i] = (m_ula.current_pal[i] & 0x04) | (((data & 0x10) >> 3) | ((data & 0x01)));
		break;
	}
}

void electron_interrupt_handler(running_machine &machine, int mode, int interrupt)
{
	electron_state *state = machine.driver_data<electron_state>();
	if ( mode == INT_SET )
	{
		state->m_ula.interrupt_status |= interrupt;
	}
	else
	{
		state->m_ula.interrupt_status &= ~interrupt;
	}
	if ( state->m_ula.interrupt_status & state->m_ula.interrupt_control & ~0x83 )
	{
		state->m_ula.interrupt_status |= 0x01;
		machine.device("maincpu")->execute().set_input_line(0, ASSERT_LINE );
	}
	else
	{
		state->m_ula.interrupt_status &= ~0x01;
		machine.device("maincpu")->execute().set_input_line(0, CLEAR_LINE );
	}
}

/**************************************
   Machine Initialisation functions
***************************************/

static TIMER_CALLBACK(setup_beep)
{
	device_t *speaker = machine.device(BEEPER_TAG);
	beep_set_state( speaker, 0 );
	beep_set_frequency( speaker, 300 );
}

static void electron_reset(running_machine &machine)
{
	electron_state *state = machine.driver_data<electron_state>();
	state->membank("bank2")->set_entry(0);

	state->m_ula.communication_mode = 0x04;
	state->m_ula.screen_mode = 0;
	state->m_ula.cassette_motor_mode = 0;
	state->m_ula.capslock_mode = 0;
	state->m_ula.screen_mode = 0;
	state->m_ula.screen_start = 0x3000;
	state->m_ula.screen_base = 0x3000;
	state->m_ula.screen_size = 0x8000 - 0x3000;
	state->m_ula.screen_addr = 0;
	state->m_ula.tape_running = 0;
	state->m_ula.vram = (UINT8 *)machine.device("maincpu")->memory().space(AS_PROGRAM).get_read_ptr(state->m_ula.screen_base);
}

void electron_state::machine_start()
{
	membank("bank2")->configure_entries(0, 16, memregion("user1")->base(), 0x4000);

	m_ula.interrupt_status = 0x82;
	m_ula.interrupt_control = 0x00;
	machine().scheduler().timer_set(attotime::zero, FUNC(setup_beep));
	m_tape_timer = machine().scheduler().timer_alloc(FUNC(electron_tape_timer_handler));
	machine().add_notifier(MACHINE_NOTIFY_RESET, machine_notify_delegate(FUNC(electron_reset),&machine()));
}

