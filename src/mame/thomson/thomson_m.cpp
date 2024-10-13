// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit computers

**********************************************************************/

#include "emu.h"
#include "thomson.h"
#include "machine/6821pia.h"
#include "machine/ram.h"

#define LOG_KBD    (1U << 1) /* TO8 / TO9 / TO9+ keyboard */
#define LOG_BANK   (1U << 2)
#define LOG_VIDEO  (1U << 3) /* video & lightpen */
#define LOG_EXTRA  (1U << 4)
#define LOG_ERRORS (1U << 5)

#define VERBOSE (LOG_ERRORS)
#include "logmacro.h"


#define PRINT(x) osd_printf_info x

/* This set to 1 handle the .k7 files without passing through .wav */
/* It must be set accordingly in formats/thom_cas.c */
#define K7_SPEED_HACK 0


/*-------------- TO7 ------------*/


/* On the TO7 & compatible (TO7/70,TO8,TO9, but not MO5,MO6), bits are coded
   in FM format with a 1.1 ms period (900 bauds):
   - 0 is 5 periods at 4.5 kHz
   - 1 is 7 periods at 6.3 kHz

   Moreover, a byte is represented using 11 bits:
   - one 0 start bit
   - eight data bits (low bit first)
   - two 1 stop bits

   There are also long (1 s) sequences of 1 bits to re-synchronize the
   cassette at places the motor can be cut off and back on (e.g., between
   files).

   The computer outputs a modulated wave that is directly put on the cassette.
   However, the input is demodulated by the cassette-reader before being
   sent to the computer: we got 0 when the signal is around 4.5 kHz and
   1 when the signal is around 6.3 kHz.
*/

#define TO7_BIT_LENGTH 0.001114

/* 1-bit cassette input to the computer
   inside the controller, two frequency filters (adjusted to 6.3 and 4.5 kHz)
   and a comparator demodulate the raw signal into 0s and 1s.
*/
int thomson_state::to7_get_cassette()
{
	if ( m_cassette->exists() )
	{
		cassette_image* cass = m_cassette->get_image();
		cassette_state state = m_cassette->get_state();
		double pos = m_cassette->get_position();
		int bitpos = pos / TO7_BIT_LENGTH;

		if ( (state & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_DISABLED )
			return 1;

		if ( K7_SPEED_HACK && m_to7_k7_bits )
		{
			/* hack, feed existing bits */
			if ( bitpos >= m_to7_k7_bitsize )
				bitpos = m_to7_k7_bitsize -1;
			LOGMASKED(LOG_EXTRA, "$%04x %f to7_get_cassette: state=$%X pos=%f samppos=%i bit=%i\n",
				m_maincpu->pc(), machine().time().as_double(), state, pos, bitpos,
				m_to7_k7_bits[ bitpos ]);
			return m_to7_k7_bits[ bitpos ];
		}
		else
		{
			/* demodulate wave signal on-the-fly */
			/* we simply count sign changes... */
			int k, chg;
			int8_t data[40];
			cass->get_samples( 0, pos, TO7_BIT_LENGTH * 15. / 14., 40, 1, data, 0 );

			for ( k = 1, chg = 0; k < 40; k++ )
			{
				if ( data[ k - 1 ] >= 0 && data[ k ] < 0 )
					chg++;
				if ( data[ k - 1 ] <= 0 && data[ k ] > 0 )
					chg++;
			}
			k = ( chg >= 13 ) ? 1 : 0;
			LOGMASKED(LOG_EXTRA, "$%04x %f to7_get_cassette: state=$%X pos=%f samppos=%i bit=%i (%i)\n",
				m_maincpu->pc(), machine().time().as_double(), state, pos, bitpos,
				k, chg);
			return k;
		}

	}
	else
		return 0;
}



/* 1-bit cassette output */
void thomson_state::to7_set_cassette(int state)
{
	m_cassette->output(state ? 1. : -1. );
}



void thomson_state::to7_set_cassette_motor(int state)
{
	cassette_state cassstate =  m_cassette->get_state();
	double pos = m_cassette->get_position();

	LOG("$%04x %f to7_set_cassette_motor: cassette motor %s bitpos=%i\n",
			m_maincpu->pc(), machine().time().as_double(), state ? "off" : "on",
			(int) (pos / TO7_BIT_LENGTH));

	if ( (cassstate & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_DISABLED && !state && pos > 0.3 )
	{
		/* rewind a little before starting the motor */
		m_cassette->seek(-0.3, SEEK_CUR );
	}

	m_cassette->change_state(state ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR );
}



/*-------------- MO5 ------------*/


/* Each byte is represented as 8 bits without start or stop bit (unlike TO7).
   Bits are coded in MFM, and the MFM signal is directly fed to the
   computer which has to decode it in software (unlike TO7).
   A 1 bit is one period at 1200 Hz; a 0 bit is one half-period at 600 Hz.
   Bit-order is most significant bit first (unlike TO7).

   Double-density MO6 cassettes follow the exact same mechanism, but with
   at double frequency (periods at 2400 Hz, and half-periods at 1200 Hz).
*/


#define MO5_BIT_LENGTH   0.000833
#define MO5_HBIT_LENGTH (MO5_BIT_LENGTH / 2.)


int thomson_state::mo5_get_cassette()
{
	if ( m_cassette->exists() )
	{
		cassette_image* cass = m_cassette->get_image();
		cassette_state state = m_cassette->get_state();
		double pos = m_cassette->get_position();
		int32_t hbit;

		if ( (state & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_DISABLED )
			return 1;

		cass->get_sample( 0, pos, 0, &hbit );
		hbit = hbit >= 0;

		LOGMASKED(LOG_EXTRA, "$%04x %f mo5_get_cassette: state=$%X pos=%f hbitpos=%i hbit=%i\n",
			m_maincpu->pc(), machine().time().as_double(), state, pos,
			(int) (pos / MO5_HBIT_LENGTH), hbit);
		return hbit;
	}
	else
		return 0;
}



void thomson_state::mo5_set_cassette( int data )
{
	m_cassette->output(data ? 1. : -1. );
}



void thomson_state::mo5_set_cassette_motor(int state)
{
	cassette_state cassstate = m_cassette->get_state();
	double pos = m_cassette->get_position();

	LOG("$%04x %f mo5_set_cassette_motor: cassette motor %s hbitpos=%i\n",
			m_maincpu->pc(), machine().time().as_double(), state ? "off" : "on",
			(int) (pos / MO5_HBIT_LENGTH));

	if ( (cassstate & CASSETTE_MASK_MOTOR) == CASSETTE_MOTOR_DISABLED &&  !state && pos > 0.3 )
	{
		/* rewind a little before starting the motor */
		m_cassette->seek(-0.3, SEEK_CUR );
	}

	m_cassette->change_state(state ? CASSETTE_MOTOR_DISABLED : CASSETTE_MOTOR_ENABLED,CASSETTE_MASK_MOTOR );
}




/*************************** utilities ********************************/



/* ------------ IRQs ------------ */


void thomson_state::thom_irq_reset()
{
	m_mainfirq->in_w<2>(0);
}



/***************************** TO7 / T9000 *************************/

DEVICE_IMAGE_LOAD_MEMBER( thomson_state::to7_cartridge )
{
	offs_t size;
	if (!image.loaded_through_softlist())
		size = image.length();
	else
		size = image.get_software_region_length("rom");

	// get size & number of 16-KB banks
	if (size <= 0x04000)
		m_thom_cart_nb_banks = 1;
	else if (size == 0x08000)
		m_thom_cart_nb_banks = 2;
	else if (size == 0x0c000)
		m_thom_cart_nb_banks = 3;
	else if (size == 0x10000)
		m_thom_cart_nb_banks = 4;
	else
	{
		return std::make_pair(
				image_error::INVALIDLENGTH,
				util::string_format("Invalid cartridge size %u", size));
	}

	uint8_t *const pos = &m_cart_rom[0];
	if (!image.loaded_through_softlist())
	{
		if (image.fread(pos, size) != size)
			return std::make_pair(image_error::UNSPECIFIED, "Error reading file");
	}
	else
	{
		memcpy(pos, image.get_software_region("rom"), size);
	}

	// extract name
	int i,j;
	char name[129];
	for (i = 0; i < size && pos[i] != ' '; i++);
	for (i++, j = 0; i + j < size && j < 128 && pos[i+j] >= 0x20; j++)
		name[j] = pos[i+j];
	name[j] = 0;

	// sanitize name
	for (i = 0; name[i]; i++)
	{
		if ( name[i] < ' ' || name[i] >= 127 )
			name[i] = '?';
	}

	PRINT (( "to7_cartridge_load: cartridge \"%s\" banks=%i, size=%i\n", name, m_thom_cart_nb_banks, size ));

	return std::make_pair(std::error_condition(), std::string());
}



void thomson_state::to7_update_cart_bank()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	int bank = 0;
	if ( m_thom_cart_nb_banks )
	{
		bank = m_thom_cart_bank % m_thom_cart_nb_banks;
		if ( bank != m_old_cart_bank && m_old_cart_bank < 0 )
		{
			space.install_read_handler(0x0000, 0x0003, read8sm_delegate(*this, FUNC(thomson_state::to7_cartridge_r)) );
		}
	}
	if ( bank != m_old_cart_bank )
	{
		m_cartbank->set_entry( bank );
		m_old_cart_bank = bank;
		LOGMASKED(LOG_BANK, "to7_update_cart_bank: CART is cartridge bank %i\n", bank);
	}
}



void thomson_state::to7_update_cart_bank_postload()
{
	to7_update_cart_bank();
}



/* write signal to 0000-1fff generates a bank switch */
void thomson_state::to7_cartridge_w(offs_t offset, uint8_t data)
{
	if ( offset >= 0x2000 )
		return;

	m_thom_cart_bank = offset & 3;
	to7_update_cart_bank();
}



/* read signal to 0000-0003 generates a bank switch */
uint8_t thomson_state::to7_cartridge_r(offs_t offset)
{
	uint8_t data = m_cart_rom[offset + (m_thom_cart_bank % m_thom_cart_nb_banks) * 0x4000];
	if ( !machine().side_effects_disabled() )
	{
		m_thom_cart_bank = offset & 3;
		to7_update_cart_bank();
	}
	return data;
}



/* ------------ 6846 (timer, I/O) ------------ */



void thomson_state::to7_timer_port_out(uint8_t data)
{
	thom_set_mode_point( data & 1 );          /* bit 0: video bank switch */
	m_caps_led = (data & 8) ? 1 : 0; /* bit 3: keyboard led */
	thom_set_border_color( ((data & 0x10) ? 1 : 0) |           /* bits 4-6: border color */
							((data & 0x20) ? 2 : 0) |
							((data & 0x40) ? 4 : 0) );
}



uint8_t thomson_state::to7_timer_port_in()
{
	int lightpen = (m_io_lightpen_button->read() & 1) ? 2 : 0;
	int cass = to7_get_cassette() ? 0x80 : 0;
	return lightpen | cass;
}



/* ------------ lightpen automaton ------------ */


void thomson_state::to7_lightpen_cb( int step )
{
	if ( ! m_to7_lightpen )
		return;

	LOGMASKED(LOG_VIDEO, "%f to7_lightpen_cb: step=%i\n", machine().time().as_double(), step);
	m_pia_sys->cb1_w( 1 );
	m_pia_sys->cb1_w( 0 );
	m_to7_lightpen_step = step;
}



/* ------------ video ------------ */



void thomson_state::to7_set_init( int init )
{
	/* INIT signal wired to system PIA 6821 */

	LOGMASKED(LOG_VIDEO, "%f to7_set_init: init=%i\n", machine().time().as_double(), init);
	m_pia_sys->ca1_w( init );
}



/* ------------ system PIA 6821 ------------ */



void thomson_state::to7_sys_cb2_out(int state)
{
	m_to7_lightpen = !state;
}



void thomson_state::to7_sys_portb_out(uint8_t data)
{
	/* value fetched in to7_sys_porta_in */
}



#define TO7_LIGHTPEN_DECAL 17 /* horizontal lightpen shift, stored in $60D2 */



uint8_t thomson_state::to7_sys_porta_in()
{
	if ( m_to7_lightpen )
	{
		/* lightpen hi */
		return to7_lightpen_gpl( TO7_LIGHTPEN_DECAL, m_to7_lightpen_step ) >> 8;
	}
	else
	{
		/* keyboard  */
		int keyline = m_pia_sys->b_output();
		uint8_t val = 0xff;
		int i;

		for ( i = 0; i < 8; i++ )
		{
			if ( ! (keyline & (1 << i)) )
				val &= m_io_keyboard[i]->read();
		}
		return val;
	}
}



uint8_t thomson_state::to7_sys_portb_in()
{
	/* lightpen low */
	return to7_lightpen_gpl( TO7_LIGHTPEN_DECAL, m_to7_lightpen_step ) & 0xff;
}



/* ------------ SX 90-018 (model 2) music & game extension ------------ */

/* features:
   - 6821 PIA
   - two 8-position, 2-button game pads
   - 2-button mouse (exclusive with pads)
     do not confuse with the TO9-specific mouse
   - 6-bit DAC sound

   extends the CM 90-112 (model 1) with one extra button per pad and a mouse
*/



#define TO7_GAME_POLL_PERIOD  attotime::from_usec( 500 )


/* The mouse is a very simple phase quadrature one.
   Each axis provides two 1-bit signals, A and B, that are toggled by the
   axis rotation. The two signals are not in phase, so that whether A is
   toggled before B or B before A gives the direction of rotation.
   This is similar Atari & Amiga mouses.
   Returns: 0 0 0 0 0 0 YB XB YA XA
 */
uint8_t thomson_state::to7_get_mouse_signal()
{
	uint8_t xa, xb, ya, yb;
	uint16_t dx = m_io_mouse_x->read(); /* x axis */
	uint16_t dy = m_io_mouse_y->read(); /* y axis */
	xa = ((dx + 1) & 3) <= 1;
	xb = (dx & 3) <= 1;
	ya = ((dy + 1) & 3) <= 1;
	yb = (dy & 3) <= 1;
	return xa | (ya << 1) | (xb << 2) | (yb << 3);
}



void thomson_state::to7_game_sound_update()
{
	m_dac->write(m_to7_game_mute ? 0 : m_to7_game_sound );
}



uint8_t thomson_state::to7_game_porta_in()
{
	uint8_t data;
	if ( m_io_config->read() & 1 )
	{
		/* mouse */
		data = to7_get_mouse_signal() & 0x0c;             /* XB, YB */
		data |= m_io_mouse_button->read() & 3; /* buttons */
	}
	else
	{
		/* joystick */
		data = m_io_game_port_directions->read();
		/* bit 0=0 => P1 up      bit 4=0 => P2 up
		   bit 1=0 => P1 down    bit 5=0 => P2 down
		   bit 2=0 => P1 left    bit 6=0 => P2 left
		   bit 3=0 => P1 right   bit 7=0 => P2 right
		*/
		/* remove impossible combinations: up+down, left+right */
		if ( ! ( data & 0x03 ) )
			data |= 0x03;
		if ( ! ( data & 0x0c ) )
			data |= 0x0c;
		if ( ! ( data & 0x30 ) )
			data |= 0x30;
		if ( ! ( data & 0xc0 ) )
			data |= 0xc0;
		if ( ! ( data & 0x03 ) )
			data |= 0x03;
		if ( ! ( data & 0x0c ) )
			data |= 0x0c;
		if ( ! ( data & 0x30 ) )
			data |= 0x30;
		if ( ! ( data & 0xc0 ) )
			data |= 0xc0;
	}
	return data;
}



uint8_t thomson_state::to7_game_portb_in()
{
	uint8_t data;
	if ( m_io_config->read() & 1 )
	{
		/* mouse */
		uint8_t mouse =  to7_get_mouse_signal();
		data = 0;
		if ( mouse & 1 )
			data |= 0x04; /* XA */
		if ( mouse & 2 )
			data |= 0x40; /* YA */
	}
	else
	{
		/* joystick */
		/* bits 6-7: action buttons A (0=pressed) */
		/* bits 2-3: action buttons B (0=pressed) */
		/* bits 4-5: unused (ouput) */
		/* bits 0-1: unknown! */
		data = m_io_game_port_buttons->read();
	}
	return data;
}



void thomson_state::to7_game_portb_out(uint8_t data)
{
	/* 6-bit DAC sound */
	m_to7_game_sound = data & 0x3f;
	to7_game_sound_update();
}



void thomson_state::to7_game_cb2_out(int state)
{
	/* undocumented */
	/* some TO8 games (e.g.: F15) seem to write here a lot */
}



/* this should be called periodically */
TIMER_CALLBACK_MEMBER(thomson_state::to7_game_update_cb)
{
	if ( m_io_config->read() & 1 )
	{
		/* mouse */
		uint8_t mouse = to7_get_mouse_signal();
		m_pia_game->ca1_w( (mouse & 1) ? 1 : 0 ); /* XA */
		m_pia_game->ca2_w( (mouse & 2) ? 1 : 0 ); /* YA */
	}
	else
	{
		/* joystick */
		uint8_t in = m_io_game_port_buttons->read();
		m_pia_game->cb2_w( (in & 0x80) ? 1 : 0 ); /* P2 action A */
		m_pia_game->ca2_w( (in & 0x40) ? 1 : 0 ); /* P1 action A */
		m_pia_game->cb1_w( (in & 0x08) ? 1 : 0 ); /* P2 action B */
		m_pia_game->ca1_w( (in & 0x04) ? 1 : 0 ); /* P1 action B */
		/* TODO:
		   it seems that CM 90-112 behaves differently
		   - ca1 is P1 action A, i.e., in & 0x40
		   - ca2 is P2 action A, i.e., in & 0x80
		   - cb1, cb2 are not connected (should not be a problem)
		*/
		/* Note: the MO6 & MO5NR have slightly different connections
		   (see mo6_game_update_cb)
		*/
	}
}



void thomson_state::to7_game_init()
{
	LOG("to7_game_init called\n");
	m_to7_game_timer = timer_alloc(FUNC(thomson_state::to7_game_update_cb), this);
	m_to7_game_timer->adjust(TO7_GAME_POLL_PERIOD, 0, TO7_GAME_POLL_PERIOD);
	save_item(NAME(m_to7_game_sound));
	save_item(NAME(m_to7_game_mute));
}



void thomson_state::to7_game_reset()
{
	LOG("to7_game_reset called\n");
	m_pia_game->ca1_w( 0 );
	m_to7_game_sound = 0;
	m_to7_game_mute = 0;
	to7_game_sound_update();
}



/* ------------ init / reset ------------ */



MACHINE_RESET_MEMBER( thomson_state, to7 )
{
	LOG("to7: machine reset called\n");

	/* subsystems */
	thom_irq_reset();
	to7_game_reset();

	m_extension->rom_map(m_maincpu->space(AS_PROGRAM), 0xe000, 0xe7bf);
	m_extension->io_map (m_maincpu->space(AS_PROGRAM), 0xe7c0, 0xe7ff);

	/* video */
	thom_set_video_mode( THOM_VMODE_TO770 );
	m_thom_init_cb = &thomson_state::to7_set_init;
	m_thom_lightpen_cb = std::bind(&thomson_state::to7_lightpen_cb, this,  std::placeholders::_1);
	thom_set_lightpen_callback( 3 );
	thom_set_mode_point( 0 );
	thom_set_border_color( 0 );
	m_pia_sys->cb1_w( 0 );

	/* memory */
	m_old_cart_bank = -1;
	to7_update_cart_bank();
	/* thom_cart_bank not reset */

	/* lightpen */
	m_to7_lightpen = 0;
}



MACHINE_START_MEMBER( thomson_state, to7 )
{
	uint8_t* cartmem = &m_cart_rom[0];
	uint8_t* ram = m_ram->pointer();

	LOG("to7: machine start called\n");

	/* subsystems */
	to7_game_init();

	m_extension->rom_map(m_maincpu->space(AS_PROGRAM), 0xe000, 0xe7bf);
	m_extension->io_map (m_maincpu->space(AS_PROGRAM), 0xe7c0, 0xe7ff);

	/* memory */
	m_thom_cart_bank = 0;
	m_thom_vram = ram;
	m_basebank->configure_entry( 0, ram + 0x4000);
	m_vrambank->configure_entries( 0, 2, m_thom_vram, 0x2000 );
	m_cartbank->configure_entries( 0, 4, cartmem, 0x4000 );
	m_basebank->set_entry( 0 );
	m_vrambank->set_entry( 0 );
	m_cartbank->set_entry( 0 );

	address_space& space = m_maincpu->space(AS_PROGRAM);
	space.unmap_readwrite(0x8000, 0xdfff);

	if ( m_ram->size() > 24*1024 )
	{
		/* install 16 KB or 16 KB + 8 KB memory extensions */
		/* BASIC instruction to see free memory: ?FRE(0) */
		int extram = m_ram->size() - 24*1024;
		space.install_write_bank(0x8000, 0x8000 + extram - 1, m_rambank);
		space.install_read_bank(0x8000, 0x8000 + extram - 1, m_rambank);
		m_rambank->configure_entry( 0, ram + 0x6000);
		m_rambank->set_entry( 0 );
	}

	/* force 2 topmost color bits to 1 */
	memset( m_thom_vram + 0x2000, 0xc0, 0x2000 );

	/* save-state */
	save_item(NAME(m_thom_cart_nb_banks));
	save_item(NAME(m_thom_cart_bank));
	save_item(NAME(m_to7_lightpen));
	save_item(NAME(m_to7_lightpen_step));
	save_pointer(NAME(cartmem), 0x10000 );
	machine().save().register_postload(save_prepost_delegate(FUNC(thomson_state::to7_update_cart_bank_postload),this));
}



/***************************** TO7/70 *************************/



/* ------------ system PIA 6821 ------------ */



void thomson_state::to770_sys_cb2_out(int state)
{
	/* video overlay: black pixels are transparent and show TV image underneath */
	LOG("$%04x to770_sys_cb2_out: video overlay %i\n", m_maincpu->pc(), state);
}



uint8_t thomson_state::to770_sys_porta_in()
{
	/* keyboard */
	int keyline = m_pia_sys->b_output() & 7;

	return m_io_keyboard[7 - keyline]->read();
}



void thomson_state::to770_update_ram_bank()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	uint8_t portb = m_pia_sys->port_b_z_mask();
	int bank;

	switch (portb & 0xf8)
	{
		/* 2 * 16 KB internal RAM */
	case 0xf0: bank = 0; break;
	case 0xe8: bank = 1; break;

		/* 4 * 16 KB extended RAM */
	case 0x18: bank = 2; break;
	case 0x98: bank = 3; break;
	case 0x58: bank = 4; break;
	case 0xd8: bank = 5; break;

		/* none selected */
	case 0xf8: return;

	default:
		LOGMASKED(LOG_ERRORS, "to770_update_ram_bank unknown bank $%02X\n", portb & 0xf8);
		return;
	}

	if ( bank != m_old_ram_bank )
	{
		if ( m_ram->size() == 128*1024 || bank < 2 )
		{
			m_rambank->set_entry( bank );
		}
		else
		{
			/* RAM size is 48 KB only and unavailable bank
			 * requested */
			space.nop_readwrite(0xa000, 0xdfff);
		}
		m_old_ram_bank = bank;
		LOGMASKED(LOG_BANK, "to770_update_ram_bank: RAM bank change %i\n", bank);
	}
}



void thomson_state::to770_update_ram_bank_postload()
{
	to770_update_ram_bank();
}



void thomson_state::to770_sys_portb_out(uint8_t data)
{
	to770_update_ram_bank();
}



/* ------------ 6846 (timer, I/O) ------------ */



void thomson_state::to770_timer_port_out(uint8_t data)
{
	thom_set_mode_point( data & 1 );          /* bit 0: video bank switch */
	m_caps_led = (data & 8) ? 1 : 0; /* bit 3: keyboard led */
	thom_set_border_color( ((data & 0x10) ? 1 : 0) |          /* 4-bit border color */
							((data & 0x20) ? 2 : 0) |
							((data & 0x40) ? 4 : 0) |
							((data & 0x04) ? 0 : 8) );
}


/* ------------ gate-array ------------ */



uint8_t thomson_state::to770_gatearray_r(offs_t offset)
{
	struct thom_vsignal v = thom_get_vsignal();
	struct thom_vsignal l = thom_get_lightpen_vsignal( TO7_LIGHTPEN_DECAL, m_to7_lightpen_step - 1, 0 );
	int count, inil, init, lt3;
	count = m_to7_lightpen ? l.count : v.count;
	inil  = m_to7_lightpen ? l.inil  : v.inil;
	init  = m_to7_lightpen ? l.init  : v.init;
	lt3   = m_to7_lightpen ? l.lt3   : v.lt3;

	switch ( offset )
	{
	case 0: return (count >> 8) & 0xff;
	case 1: return count & 0xff;
	case 2: return (lt3 << 7) | (inil << 6);
	case 3: return (init << 7);
	default:
		LOGMASKED(LOG_ERRORS, "$%04x to770_gatearray_r: invalid offset %i\n", m_maincpu->pc(), offset);
		return 0;
	}
}



void thomson_state::to770_gatearray_w(offs_t offset, uint8_t data)
{
	if ( ! offset )
		m_to7_lightpen = data & 1;
}



/* ------------ init / reset ------------ */



MACHINE_RESET_MEMBER( thomson_state, to770 )
{
	LOG("to770: machine reset called\n");

	/* subsystems */
	thom_irq_reset();
	to7_game_reset();

	m_extension->rom_map(m_maincpu->space(AS_PROGRAM), 0xe000, 0xe7bf);
	m_extension->io_map (m_maincpu->space(AS_PROGRAM), 0xe7c0, 0xe7ff);

	/* video */
	thom_set_video_mode( THOM_VMODE_TO770 );
	m_thom_init_cb = &thomson_state::to7_set_init;
	m_thom_lightpen_cb = std::bind(&thomson_state::to7_lightpen_cb, this,  std::placeholders::_1);
	thom_set_lightpen_callback( 3 );
	thom_set_mode_point( 0 );
	thom_set_border_color( 8 );
	m_pia_sys->cb1_w( 0 );

	/* memory */
	m_old_ram_bank = -1;
	m_old_cart_bank = -1;
	to7_update_cart_bank();
	to770_update_ram_bank();
	/* thom_cart_bank not reset */

	/* lightpen */
	m_to7_lightpen = 0;
}



MACHINE_START_MEMBER( thomson_state, to770 )
{
	uint8_t* cartmem = &m_cart_rom[0];
	uint8_t* ram = m_ram->pointer();

	LOG("to770: machine start called\n");

	/* subsystems */
	to7_game_init();

	m_extension->rom_map(m_maincpu->space(AS_PROGRAM), 0xe000, 0xe7bf);
	m_extension->io_map (m_maincpu->space(AS_PROGRAM), 0xe7c0, 0xe7ff);

	/* memory */
	m_thom_cart_bank = 0;
	m_thom_vram = ram;
	m_basebank->configure_entry( 0, ram + 0x4000);
	m_rambank->configure_entries( 0, 6, ram + 0x8000, 0x4000 );
	m_vrambank->configure_entries( 0, 2, m_thom_vram, 0x2000 );
	m_cartbank->configure_entries( 0, 4, cartmem, 0x4000 );
	m_basebank->set_entry( 0 );
	m_rambank->set_entry( 0 );
	m_vrambank->set_entry( 0 );
	m_cartbank->set_entry( 0 );

	/* save-state */
	save_item(NAME(m_thom_cart_nb_banks));
	save_item(NAME(m_thom_cart_bank));
	save_item(NAME(m_to7_lightpen));
	save_item(NAME(m_to7_lightpen_step));
	save_pointer(NAME(cartmem), 0x10000 );
	machine().save().register_postload(save_prepost_delegate(FUNC(thomson_state::to770_update_ram_bank_postload), this));
	machine().save().register_postload(save_prepost_delegate(FUNC(thomson_state::to7_update_cart_bank_postload), this));
}



/***************************** MO5 *************************/



/* ------------ lightpen automaton ------------ */



void mo5_state::mo5_lightpen_cb( int step )
{
	/* MO5 signals ca1 (TO7 signals cb1) */
	if ( ! m_to7_lightpen )
		return;

	m_pia_sys->ca1_w( 1 );
	m_pia_sys->ca1_w( 0 );
	m_to7_lightpen_step = step;
}


/* ------------ periodic interrupt ------------ */

/* the MO5 & MO6 do not have a MC 6846 timer,
   they have a fixed 50 Hz timer instead
*/


TIMER_CALLBACK_MEMBER(thomson_state::mo5_periodic_cb)
{
	/* pulse */
	m_pia_sys->cb1_w( 1 );
	m_pia_sys->cb1_w( 0 );
}



void thomson_state::mo5_init_timer()
{
	/* time is a little faster than 50 Hz to match video framerate */
	m_mo5_periodic_timer->adjust(attotime::zero, 0, attotime::from_usec( 19968 ));
}



/* ------------ system PIA 6821 ------------ */



void mo5_state::mo5_sys_porta_out(uint8_t data)
{
	thom_set_mode_point( data & 1 );       /* bit 0: video bank switch */
	thom_set_border_color( (data >> 1) & 15 ); /* bit 1-4: border color */
	mo5_set_cassette( (data & 0x40) ? 1 : 0 ); /* bit 6: cassette output */
}



uint8_t mo5_state::mo5_sys_porta_in()
{
	return
		((m_io_lightpen_button->read() & 1) ? 0x20 : 0) | /* bit 5: lightpen button */
		(mo5_get_cassette() ? 0x80 : 0) |                 /* bit 7: cassette input */
		0x5f;                                             /* other bits are unconnected and pulled hi internally */
}



uint8_t mo5_state::mo5_sys_portb_in()
{
	uint8_t portb = m_pia_sys->b_output();
	int col = (portb >> 1) & 7;       /* key column */
	int lin = 7 - ((portb >> 4) & 7); /* key line */

	return ( m_io_keyboard[lin]->read() & (1 << col) ) ? 0x80 : 0;
}



/* ------------ gate-array ------------ */



#define MO5_LIGHTPEN_DECAL 12



uint8_t mo5_state::mo5_gatearray_r(offs_t offset)
{
	struct thom_vsignal v = thom_get_vsignal();
	struct thom_vsignal l = thom_get_lightpen_vsignal( MO5_LIGHTPEN_DECAL, m_to7_lightpen_step - 1, 0 );
	int count, inil, init, lt3;
	count = m_to7_lightpen ? l.count : v.count;
	inil  = m_to7_lightpen ? l.inil  : v.inil;
	init  = m_to7_lightpen ? l.init  : v.init;
	lt3   = m_to7_lightpen ? l.lt3   : v.lt3;

	switch ( offset ) {
	case 0: return (count >> 8) & 0xff;
	case 1: return count & 0xff;
	case 2: return (lt3 << 7) | (inil << 6);
	case 3: return (init << 7);
	default:
		LOGMASKED(LOG_ERRORS, "$%04x mo5_gatearray_r: invalid offset %i\n",  m_maincpu->pc(), offset);
		return 0;
	}
}



void mo5_state::mo5_gatearray_w(offs_t offset, uint8_t data)
{
	if ( ! offset )
		m_to7_lightpen = data & 1;
}



/* ------------ cartridge / extended RAM ------------ */



DEVICE_IMAGE_LOAD_MEMBER( thomson_state::mo5_cartridge )
{
	uint64_t size, i;
	int j;

	if (!image.loaded_through_softlist())
		size = image.length();
	else
		size = image.get_software_region_length("rom");

	// get size & number of 16-KB banks
	if (size > 32 && size <= 0x04000)
		m_thom_cart_nb_banks = 1;
	else if (size == 0x08000)
		m_thom_cart_nb_banks = 2;
	else if (size == 0x0c000)
		m_thom_cart_nb_banks = 3;
	else if (size == 0x10000)
		m_thom_cart_nb_banks = 4;
	else
	{
		return std::make_pair(
				image_error::INVALIDLENGTH,
				util::string_format("Invalid cartridge size %u", size));
	}

	uint8_t *const pos = &m_cart_rom[0];
	if (!image.loaded_through_softlist())
	{
		if (image.fread(pos, size) != size)
			return std::make_pair(image_error::UNSPECIFIED, "Error reading file");
	}
	else
	{
		memcpy(pos, image.get_software_region("rom"), size);
	}

	// extract name
	i = size - 32;
	char name[129];
	while (i < size && !pos[i]) i++;
	for (j = 0; i < size && pos[i] >= 0x20; j++, i++)
		name[j] = pos[i];
	name[j] = 0;

	// sanitize name
	for ( j = 0; name[j]; j++)
	{
		if ( name[j] < ' ' || name[j] >= 127 ) name[j] = '?';
	}

	PRINT (( "mo5_cartridge_load: cartridge \"%s\" banks=%i, size=%u\n", name, m_thom_cart_nb_banks, (unsigned) size ));

	return std::make_pair(std::error_condition(), std::string());
}



void mo5_state::mo5_update_cart_bank()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	int rom_is_ram = m_mo5_reg_cart & 4;
	int bank = 0;
	int bank_is_read_only = 0;


	if ( rom_is_ram && m_thom_cart_nb_banks == 4 )
	{
		/* 64 KB ROM from "JANE" cartridge */
		bank = m_mo5_reg_cart & 3;
		if ( bank != m_old_cart_bank )
		{
			if ( m_old_cart_bank < 0 || m_old_cart_bank > 3 )
			{
				space.install_read_bank( 0xb000, 0xefff, m_cartbank);
				space.nop_write( 0xb000, 0xefff);
			}
			LOGMASKED(LOG_BANK, "mo5_update_cart_bank: CART is cartridge bank %i (A7CB style)\n", bank);
		}
	}
	else if ( rom_is_ram )
	{
		/* 64 KB RAM from network extension */
		bank = 4 + ( m_mo5_reg_cart & 3 );
		bank_is_read_only = (( m_mo5_reg_cart & 8 ) == 0);

		if ( bank != m_old_cart_bank || bank_is_read_only != m_old_cart_bank_was_read_only)
		{
			if ( bank_is_read_only )
			{
				space.install_read_bank( 0xb000, 0xefff, m_cartbank);
				space.nop_write( 0xb000, 0xefff );
			}
			else
			{
				space.install_readwrite_bank( 0xb000, 0xefff, m_cartbank);
			}
			LOGMASKED(LOG_BANK, "mo5_update_cart_bank: CART is nanonetwork RAM bank %i (%s)\n",
						m_mo5_reg_cart & 3,
						bank_is_read_only ? "read-only":"read-write");
			m_old_cart_bank_was_read_only = bank_is_read_only;
		}
	}
	else
	{
		/* regular cartridge bank switch */
		if ( m_thom_cart_nb_banks )
		{
			bank = m_thom_cart_bank % m_thom_cart_nb_banks;
			if ( bank != m_old_cart_bank )
			{
				if ( m_old_cart_bank < 0 )
				{
					space.install_read_bank( 0xb000, 0xefff, m_cartbank);
					space.install_write_handler( 0xb000, 0xefff, write8sm_delegate(*this, FUNC(mo5_state::mo5_cartridge_w)) );
					space.install_read_handler( 0xbffc, 0xbfff, read8sm_delegate(*this, FUNC(mo5_state::mo5_cartridge_r)) );
				}
				LOGMASKED(LOG_BANK, "mo5_update_cart_bank: CART is cartridge bank %i\n", bank);
			}
		}
		else
		{
			/* internal ROM */
			if ( m_old_cart_bank != 0 )
						{
				space.install_read_bank( 0xb000, 0xefff, m_cartbank);
				space.install_write_handler( 0xb000, 0xefff, write8sm_delegate(*this, FUNC(mo5_state::mo5_cartridge_w)) );
				LOGMASKED(LOG_BANK, "mo5_update_cart_bank: CART is internal\n");
			}
		}
	}
	if ( bank != m_old_cart_bank )
	{
		m_cartbank->set_entry( bank );
		m_old_cart_bank = bank;
	}
}



void mo5_state::mo5_update_cart_bank_postload()
{
	mo5_update_cart_bank();
}



/* write signal to b000-cfff generates a bank switch */
void mo5_state::mo5_cartridge_w(offs_t offset, uint8_t data)
{
	if ( offset >= 0x2000 )
		return;

	m_thom_cart_bank = offset & 3;
	mo5_update_cart_bank();
}



/* read signal to bffc-bfff generates a bank switch */
uint8_t mo5_state::mo5_cartridge_r(offs_t offset)
{
	uint8_t data = m_cart_rom[offset + 0x3ffc + (m_thom_cart_bank % m_thom_cart_nb_banks) * 0x4000];
	if ( !machine().side_effects_disabled() )
	{
		m_thom_cart_bank = offset & 3;
		mo5_update_cart_bank();
	}
	return data;
}



/* 0xa7cb bank-switch register */
void mo5_state::mo5_ext_w(uint8_t data)
{
	m_mo5_reg_cart = data;
	mo5_update_cart_bank();
}



/* ------------ init / reset ------------ */



MACHINE_RESET_MEMBER( mo5_state, mo5 )
{
	LOG("mo5: machine reset called\n");

	/* subsystems */
	thom_irq_reset();
	to7_game_reset();
	mo5_init_timer();

	/* video */
	thom_set_video_mode( THOM_VMODE_MO5 );
	m_thom_lightpen_cb = std::bind(&mo5_state::mo5_lightpen_cb, this,  std::placeholders::_1);
	thom_set_lightpen_callback( 3 );
	thom_set_mode_point( 0 );
	thom_set_border_color( 0 );
	m_pia_sys->ca1_w( 0 );

	/* memory */
	m_old_cart_bank = -1;
	mo5_update_cart_bank();
	/* mo5_reg_cart not reset */
	/* thom_cart_bank not reset */

	/* lightpen */
	m_to7_lightpen = 0;
}



MACHINE_START_MEMBER( mo5_state, mo5 )
{
	uint8_t* mem = memregion("maincpu")->base();
	uint8_t* cartmem = &m_cart_rom[0];
	uint8_t* ram = m_ram->pointer();

	LOG("mo5: machine start called\n");

	/* subsystems */
	to7_game_init();
	m_mo5_periodic_timer = timer_alloc(FUNC(mo5_state::mo5_periodic_cb), this);

	m_extension->rom_map(m_maincpu->space(AS_PROGRAM), 0xa000, 0xa7bf);
	m_extension->io_map (m_maincpu->space(AS_PROGRAM), 0xa7c0, 0xa7ff);

	/* memory */
	m_thom_cart_bank = 0;
	m_mo5_reg_cart = 0;
	m_thom_vram = ram;
	m_basebank->configure_entry( 0, ram + 0x4000);
	m_cartbank->configure_entry( 0, mem + 0x10000);
	m_cartbank->configure_entries( 1, 3, cartmem + 0x4000, 0x4000 );
	m_cartbank->configure_entries( 4, 4, ram + 0xc000, 0x4000 );
	m_vrambank->configure_entries( 0, 2, m_thom_vram, 0x2000 );
	m_basebank->set_entry( 0 );
	m_cartbank->set_entry( 0 );
	m_vrambank->set_entry( 0 );

	/* save-state */
	save_item(NAME(m_thom_cart_nb_banks));
	save_item(NAME(m_thom_cart_bank));
	save_item(NAME(m_to7_lightpen));
	save_item(NAME(m_to7_lightpen_step));
	save_item(NAME(m_mo5_reg_cart));
	save_pointer(NAME(cartmem), 0x10000 );
	machine().save().register_postload(save_prepost_delegate(FUNC(mo5_state::mo5_update_cart_bank_postload), this));
}



/***************************** TO9 *************************/



/* ------------ IEEE extension ------------ */



/* TODO: figure out what this extension is... IEEE-488 ??? */



void to9_state::to9_ieee_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_ERRORS, "$%04x %f to9_ieee_w: unhandled write $%02X to register %i\n", m_maincpu->pc(), machine().time().as_double(), data, offset);
}



uint8_t to9_state::to9_ieee_r(offs_t offset)
{
	LOGMASKED(LOG_ERRORS, "$%04x %f to9_ieee_r: unhandled read from register %i\n", m_maincpu->pc(), machine().time().as_double(), offset);
	return 0;
}



/* ------------ system gate-array ------------ */



#define TO9_LIGHTPEN_DECAL 8



uint8_t to9_state::to9_gatearray_r(offs_t offset)
{
	struct thom_vsignal v = thom_get_vsignal();
	struct thom_vsignal l = thom_get_lightpen_vsignal( TO9_LIGHTPEN_DECAL, m_to7_lightpen_step - 1, 0 );
	int count, inil, init, lt3;
	count = m_to7_lightpen ? l.count : v.count;
	inil  = m_to7_lightpen ? l.inil  : v.inil;
	init  = m_to7_lightpen ? l.init  : v.init;
	lt3   = m_to7_lightpen ? l.lt3   : v.lt3;

	switch ( offset )
	{
	case 0: return (count >> 8) & 0xff;
	case 1: return count & 0xff;
	case 2: return (lt3 << 7) | (inil << 6);
	case 3: return (v.init << 7) | (init << 6); /* != TO7/70 */
	default:
		LOGMASKED(LOG_ERRORS, "$%04x to9_gatearray_r: invalid offset %i\n", m_maincpu->pc(), offset);
		return 0;
	}
}



void to9_state::to9_gatearray_w(offs_t offset, uint8_t data)
{
	if ( ! offset )
		m_to7_lightpen = data & 1;
}



/* ------------ video gate-array ------------ */



/* style: 0 => TO9, 1 => TO8/TO9, 2 => MO6 */
void to9_state::to9_set_video_mode( uint8_t data, int style )
{
	switch ( data & 0x7f )
	{
	case 0x00:
		if ( style == 2 )
			thom_set_video_mode( THOM_VMODE_MO5 );
		else if ( style == 1 )
			thom_set_video_mode( THOM_VMODE_TO770 );
		else
			thom_set_video_mode( THOM_VMODE_TO9 );
		break;

	// undocumented, but tested on a real TO8D
	case 0x20: thom_set_video_mode( THOM_VMODE_MO5_ALT );     break;

	case 0x21: thom_set_video_mode( THOM_VMODE_BITMAP4 );     break;

	case 0x41: thom_set_video_mode( THOM_VMODE_BITMAP4_ALT ); break;

	// also undocumented but tested
	case 0x59: thom_set_video_mode( THOM_VMODE_BITMAP4_ALT_HALF ); break;

	case 0x2a:
		if ( style==0 )
			thom_set_video_mode( THOM_VMODE_80_TO9 );
		else
			thom_set_video_mode( THOM_VMODE_80 );
		break;

	case 0x7b: thom_set_video_mode( THOM_VMODE_BITMAP16 );    break;

	case 0x24: thom_set_video_mode( THOM_VMODE_PAGE1 );       break;

	case 0x25: thom_set_video_mode( THOM_VMODE_PAGE2 );       break;

	case 0x26: thom_set_video_mode( THOM_VMODE_OVERLAY );     break;

		// undocumented 160x200 variant of overlay
	case 0x3e: thom_set_video_mode( THOM_VMODE_OVERLAY_HALF );     break;

	case 0x3f: thom_set_video_mode( THOM_VMODE_OVERLAY3 );    break;

	// undocumented variant enconding for bitmap16
	case 0x5b: thom_set_video_mode( THOM_VMODE_BITMAP16_ALT ); break;

	default:
		LOGMASKED(LOG_ERRORS, "to9_set_video_mode: unknown mode $%02X tr=%i phi=%i mod=%i\n", data, (data >> 5) & 3, (data >> 3) & 2, data & 7);
	}
}



uint8_t to9_state::to9_vreg_r(offs_t offset)
{
	switch ( offset )
	{
	case 0: /* palette data */
	{
		uint8_t c =  m_to9_palette_data[ m_to9_palette_idx ];
		if ( !machine().side_effects_disabled() )
		{
			m_to9_palette_idx = ( m_to9_palette_idx + 1 ) & 31;
		}
		return c;
	}

	case 1: /* palette address */
		return m_to9_palette_idx;

	case 2:
	case 3:
		return 0;

	default:
		LOGMASKED(LOG_ERRORS, "to9_vreg_r: invalid read offset %i\n", offset);
		return 0;
	}
}



void to9_state::to9_vreg_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_VIDEO, "$%04x %f to9_vreg_w: off=%i ($%04X) data=$%02X\n", m_maincpu->pc(), machine().time().as_double(), offset, 0xe7da + offset, data);

	switch ( offset )
	{
	case 0: /* palette data */
	{
		uint16_t color, idx;
		m_to9_palette_data[ m_to9_palette_idx ] = data;
		idx = m_to9_palette_idx / 2;
		color = m_to9_palette_data[ 2 * idx + 1 ];
		color = m_to9_palette_data[ 2 * idx ] | (color << 8);
		thom_set_palette( idx ^ 8, color & 0x1fff );
		m_to9_palette_idx = ( m_to9_palette_idx + 1 ) & 31;
	}
	break;

	case 1: /* palette address */
		m_to9_palette_idx = data & 31;
		break;

	case 2: /* video mode */
		to9_set_video_mode( data, 0 );
		break;

	case 3: /* border color */
		thom_set_border_color( data & 15 );
		break;

	default:
		LOGMASKED(LOG_ERRORS, "to9_vreg_w: invalid write offset %i data=$%02X\n", offset, data);
	}
}



void to9_state::to9_palette_init()
{
	m_to9_palette_idx = 0;
	memset( m_to9_palette_data, 0, sizeof( m_to9_palette_data ) );
	save_item(NAME(m_to9_palette_idx));
	save_item(NAME(m_to9_palette_data));
}



/* ------------ RAM / ROM banking ------------ */


void to9_state::to9_update_cart_bank()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	int bank = 0;
	int slot = ( m_mc6846->get_output_port() >> 4 ) & 3; /* bits 4-5: ROM bank */

	switch ( slot )
	{
	case 0:
		/* BASIC (64 KB) */
		bank = 4 + m_to9_soft_bank;
		if ( bank != m_old_cart_bank )
		{
			if ( m_old_cart_bank < 4)
			{
				space.install_read_bank( 0x0000, 0x3fff, m_cartbank );
			}
			LOGMASKED(LOG_BANK, "to9_update_cart_bank: CART is BASIC bank %i\n", m_to9_soft_bank);
		}
		break;
	case 1:
		/* software 1 (32 KB) */
		bank = 8 + (m_to9_soft_bank & 1);
		if ( bank != m_old_cart_bank )
		{
			if ( m_old_cart_bank < 4)
			{
				space.install_read_bank( 0x0000, 0x3fff, m_cartbank );
			}
			LOGMASKED(LOG_BANK, "to9_update_cart_bank: CART is software 1 bank %i\n", m_to9_soft_bank);
		}
		break;
	case 2:
		/* software 2 (32 KB) */
		bank = 10 + (m_to9_soft_bank & 1);
		if ( bank != m_old_cart_bank )
		{
			if ( m_old_cart_bank < 4)
			{
				space.install_read_bank( 0x0000, 0x3fff, m_cartbank );
			}
			LOGMASKED(LOG_BANK, "to9_update_cart_bank: CART is software 2 bank %i\n", m_to9_soft_bank);
		}
		break;
	case 3:
		/* external cartridge */
		if ( m_thom_cart_nb_banks )
		{
			bank = m_thom_cart_bank % m_thom_cart_nb_banks;
			if ( bank != m_old_cart_bank )
			{
				if ( m_old_cart_bank < 0 || m_old_cart_bank > 3 )
				{
					space.install_read_bank( 0x0000, 0x3fff, m_cartbank );
					space.install_write_handler( 0x0000, 0x3fff, write8sm_delegate(*this, FUNC(to9_state::to9_cartridge_w)) );
					space.install_read_handler( 0x0000, 0x0003, read8sm_delegate(*this, FUNC(to9_state::to9_cartridge_r)) );
				}
				LOGMASKED(LOG_BANK, "to9_update_cart_bank: CART is cartridge bank %i\n", m_thom_cart_bank);
			}
		}
		else
		{
			if ( m_old_cart_bank != 0 )
			{
				space.nop_read( 0x0000, 0x3fff);
				LOGMASKED(LOG_BANK, "to9_update_cart_bank: CART is unmapped\n");
			}
		}
		break;
	}
	if ( bank != m_old_cart_bank )
	{
		m_cartbank->set_entry( bank );
		m_old_cart_bank = bank;
	}
}



void to9_state::to9_update_cart_bank_postload()
{
	to9_update_cart_bank();
}



/* write signal to 0000-1fff generates a bank switch */
void to9_state::to9_cartridge_w(offs_t offset, uint8_t data)
{
	int slot = ( m_mc6846->get_output_port() >> 4 ) & 3; /* bits 4-5: ROM bank */

	if ( offset >= 0x2000 )
		return;

	if ( slot == 3 )
		m_thom_cart_bank = offset & 3;
	else
		m_to9_soft_bank = offset & 3;
	to9_update_cart_bank();
}



/* read signal to 0000-0003 generates a bank switch */
uint8_t to9_state::to9_cartridge_r(offs_t offset)
{
	uint8_t data = m_cart_rom[offset + (m_thom_cart_bank % m_thom_cart_nb_banks) * 0x4000];
	if ( !machine().side_effects_disabled() )
	{
		m_thom_cart_bank = offset & 3;
		to9_update_cart_bank();
	}
	return data;
}



void to9_state::to9_update_ram_bank()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	uint8_t port = m_mc6846->get_output_port();
	uint8_t portb = m_pia_sys->port_b_z_mask();
	uint8_t disk = ((port >> 2) & 1) | ((port >> 5) & 2); /* bits 6,2: RAM bank */
	int bank;

	switch ( portb & 0xf8 )
	{
		/* TO7/70 compatible */
	case 0xf0: bank = 0; break;
	case 0xe8: bank = 1; break;
	case 0x18: bank = 2; break;
	case 0x98: bank = 3; break;
	case 0x58: bank = 4; break;
	case 0xd8: bank = 5; break;

		/* 64 KB of virtual disk */
	case 0xf8: bank = 6 + disk ; break;

		/* none selected */
	case 0: return;

	default:
		LOGMASKED(LOG_ERRORS, "to9_update_ram_bank: unknown RAM bank pia=$%02X disk=%i\n", portb & 0xf8, disk);
		return;
	}

	if ( m_old_ram_bank != bank )
	{
		if ( m_ram->size() == 192*1024 || bank < 6 )
		{
			m_rambank->set_entry( bank );
		}
		else
		{
			space.nop_readwrite( 0xa000, 0xdfff);
		}
		m_old_ram_bank = bank;
		LOGMASKED(LOG_BANK, "to9_update_ram_bank: bank %i selected (pia=$%02X disk=%i)\n", bank, portb & 0xf8, disk);
	}
}



void to9_state::to9_update_ram_bank_postload()
{
	to9_update_ram_bank();
}


/* ------------ system PIA 6821 ------------ */

uint8_t to9_state::to9_sys_porta_in()
{
	uint8_t ktest = m_to9_kbd->ktest_r();

	LOGMASKED(LOG_KBD, "to9_sys_porta_in: ktest=%i\n", ktest);

	// PB1-7 are not connected, and are pulled hi internally
	return ktest | 0xfe;
}



void to9_state::to9_sys_porta_out(uint8_t data)
{
	m_centronics->write_data1(BIT(data, 1));
	m_centronics->write_data2(BIT(data, 2));
	m_centronics->write_data3(BIT(data, 3));
	m_centronics->write_data4(BIT(data, 4));
	m_centronics->write_data5(BIT(data, 5));
	m_centronics->write_data6(BIT(data, 6));
	m_centronics->write_data7(BIT(data, 7));
}



void to9_state::to9_sys_portb_out(uint8_t data)
{
	m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_strobe(BIT(data, 1));

	to9_update_ram_bank();

	if ( data & 4 ) /* bit 2: video overlay (TODO) */
		LOG("to9_sys_portb_out: video overlay not handled\n");
}



/* ------------ 6846 (timer, I/O) ------------ */



void to9_state::to9_timer_port_out(uint8_t data)
{
	thom_set_mode_point( data & 1 ); /* bit 0: video bank */
	to9_update_ram_bank();
	to9_update_cart_bank();
}


/* ------------ init / reset ------------ */



MACHINE_RESET_MEMBER( to9_state, to9 )
{
	LOG("to9: machine reset called\n");

	/* subsystems */
	thom_irq_reset();
	to7_game_reset();

	m_extension->rom_map(m_maincpu->space(AS_PROGRAM), 0xe000, 0xe7bf);
	m_extension->io_map (m_maincpu->space(AS_PROGRAM), 0xe7c0, 0xe7ff);

	/* video */
	thom_set_video_mode( THOM_VMODE_TO9 );
	m_thom_lightpen_cb = std::bind(&to9_state::to7_lightpen_cb, this,  std::placeholders::_1);
	thom_set_lightpen_callback( 3 );
	thom_set_border_color( 8 );
	thom_set_mode_point( 0 );
	m_pia_sys->cb1_w( 0 );

	/* memory */
	m_old_ram_bank = -1;
	m_old_cart_bank = -1;
	m_to9_soft_bank = 0;
	to9_update_cart_bank();
	to9_update_ram_bank();
	/* thom_cart_bank not reset */

	/* lightpen */
	m_to7_lightpen = 0;
}



MACHINE_START_MEMBER( to9_state, to9 )
{
	uint8_t* mem = memregion("maincpu")->base();
	uint8_t* cartmem = &m_cart_rom[0];
	uint8_t* ram = m_ram->pointer();

	LOG("to9: machine start called\n");

	/* subsystems */
	to7_game_init();
	to9_palette_init();

	m_extension->rom_map(m_maincpu->space(AS_PROGRAM), 0xe000, 0xe7bf);
	m_extension->io_map (m_maincpu->space(AS_PROGRAM), 0xe7c0, 0xe7ff);

	/* memory */
	m_thom_vram = ram;
	m_thom_cart_bank = 0;
	m_vrambank->configure_entries( 0,  2, m_thom_vram, 0x2000 );
	m_cartbank->configure_entries( 0,  4, cartmem, 0x4000 );
	m_cartbank->configure_entries( 4,  8, mem + 0x10000, 0x4000 );
	m_basebank->configure_entry( 0,  ram + 0x4000);
	m_rambank->configure_entries( 0, 10, ram + 0x8000, 0x4000 );
	m_vrambank->set_entry( 0 );
	m_cartbank->set_entry( 0 );
	m_basebank->set_entry( 0 );
	m_rambank->set_entry( 0 );

	/* save-state */
	save_item(NAME(m_thom_cart_nb_banks));
	save_item(NAME(m_thom_cart_bank));
	save_item(NAME(m_to7_lightpen));
	save_item(NAME(m_to7_lightpen_step));
	save_item(NAME(m_to9_soft_bank));
	save_pointer(NAME(cartmem), 0x10000 );
	machine().save().register_postload(save_prepost_delegate(FUNC(to9_state::to9_update_ram_bank_postload), this));
	machine().save().register_postload(save_prepost_delegate(FUNC(to9_state::to9_update_cart_bank_postload), this));
}



/***************************** TO8 *************************/


/* ------------ RAM / ROM banking ------------ */

void to9_state::to8_update_ram_bank()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	uint8_t bank = 0;

	if ( m_to8_reg_sys1 & 0x10 )
	{
		bank = m_to8_reg_ram & 31;
	}
	else
	{
		uint8_t portb = m_pia_sys->port_b_z_mask();

		switch ( portb & 0xf8 )
		{
			/*  in compatibility mode, banks 5 and 6 are reversed wrt TO7/70 */
		case 0xf0: bank = 2; break;
		case 0xe8: bank = 3; break;
		case 0x18: bank = 4; break;
		case 0x58: bank = 5; break;
		case 0x98: bank = 6; break;
		case 0xd8: bank = 7; break;
		case 0xf8: return;
		default:
			LOGMASKED(LOG_ERRORS, "to8_update_ram_bank: unknown RAM bank=$%02X\n", portb & 0xf8);
			return;
		}
	}

	/*  due to addressing distortion, the 16 KB banked memory space is
	    split into two 8 KB spaces:
	    - 0xa000-0xbfff maps to 0x2000-0x3fff in 16 KB bank
	    - 0xc000-0xdfff maps to 0x0000-0x1fff in 16 KB bank
	    this is important if we map a bank that is also reachable by another,
	    undistorted space, such as cartridge, page 0 (video), or page 1
	*/
	if ( bank != m_old_ram_bank)
	{
		if (m_ram->size() == 512*1024 || m_to8_data_vpage < 16)
		{
			m_datalobank->set_entry( bank );
			m_datahibank->set_entry( bank );
		}
		else
		{
			/* RAM size is 256 KB only and unavailable
			 * bank requested */
			space.nop_readwrite( 0xa000, 0xbfff);
			space.nop_readwrite( 0xc000, 0xdfff);
		}
		m_to8_data_vpage = bank;
		m_old_ram_bank = bank;
		LOGMASKED(LOG_BANK, "to8_update_ram_bank: select bank %i (%s style)\n", bank, (m_to8_reg_sys1 & 0x10) ? "new" : "old");
	}
}



void to9_state::to8_update_ram_bank_postload()
{
	to8_update_ram_bank();
}



void to9_state::to8_update_cart_bank()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	int bank = 0;
	int bank_is_read_only = 0;

	if ( m_to8_reg_cart & 0x20 )
	{
		/* RAM space */
		m_to8_cart_vpage = m_to8_reg_cart & 31;
		bank = 8 + m_to8_cart_vpage;
		bank_is_read_only = (( m_to8_reg_cart & 0x40 ) == 0);
		if ( bank != m_old_cart_bank )
		{
			/* mapping to VRAM */
			if (m_ram->size() == 512*1024 || m_to8_cart_vpage < 16)
			{
				if (m_to8_cart_vpage < 4)
				{
					if (m_old_cart_bank < 8 || m_old_cart_bank > 11)
					{
						space.install_read_bank( 0x0000, 0x3fff, m_cartbank );
						if ( bank_is_read_only )
						{
							space.nop_write( 0x0000, 0x3fff);
						}
						else
						{
							space.install_write_handler( 0x0000, 0x3fff, write8sm_delegate(*this, FUNC(to9_state::to8_vcart_w)));
						}
					}
				}
				else
				{
					if (m_old_cart_bank < 12)
					{
						if ( bank_is_read_only )
						{
							space.install_read_bank( 0x0000, 0x3fff, m_cartbank );
							space.nop_write( 0x0000, 0x3fff);
						}
						else
						{
							space.install_readwrite_bank(0x0000, 0x3fff, m_cartbank);
						}
					}
				}
			}
			else
			{
				/* RAM size is 256 KB only and unavailable
				 * bank requested */
				space.nop_readwrite( 0x0000, 0x3fff);
			}
			LOGMASKED(LOG_BANK, "to8_update_cart_bank: CART is RAM bank %i (%s)\n", m_to8_cart_vpage, bank_is_read_only ? "read-only" : "read-write");
		}
		else
		{
			if ( bank_is_read_only != m_old_cart_bank_was_read_only )
			{
				if ( bank_is_read_only )
				{
					space.nop_write( 0x0000, 0x3fff);
				}
				else
				{
					if (m_to8_cart_vpage < 4)
					{
						space.install_write_handler( 0x0000, 0x3fff, write8sm_delegate(*this, FUNC(to9_state::to8_vcart_w)));
					}
					else
					{
						space.install_readwrite_bank( 0x0000, 0x3fff, m_cartbank );
					}
				}
				LOGMASKED(LOG_BANK, "to8_update_cart_bank: update CART bank %i write status to %s\n", m_to8_cart_vpage, bank_is_read_only ? "read-only" : "read-write");
			}
		}
		m_old_cart_bank_was_read_only = bank_is_read_only;
	}
	else
	{
		if ( m_to8_soft_select )
		{
			/* internal software ROM space */
			bank = 4 + m_to8_soft_bank;
			if ( bank != m_old_cart_bank )
			{
				if ( m_old_cart_bank < 4 || m_old_cart_bank > 7 )
				{
					space.install_read_bank( 0x0000, 0x3fff, m_cartbank );
					space.install_write_handler( 0x0000, 0x3fff, write8sm_delegate(*this, FUNC(to9_state::to8_cartridge_w)) );
				}
				LOGMASKED(LOG_BANK, "to8_update_cart_bank: CART is internal bank %i\n", m_to8_soft_bank);
			}
		}
		else
		{
			/* external cartridge ROM space */
			if ( m_thom_cart_nb_banks )
			{
				bank = m_thom_cart_bank % m_thom_cart_nb_banks;
				if ( bank != m_old_cart_bank )
				{
					if ( m_old_cart_bank < 0 || m_old_cart_bank > 3 )
					{
						space.install_read_bank( 0x0000, 0x3fff, m_cartbank );
						space.install_write_handler( 0x0000, 0x3fff, write8sm_delegate(*this, FUNC(to9_state::to8_cartridge_w)) );
						space.install_read_handler( 0x0000, 0x0003, read8sm_delegate(*this, FUNC(to9_state::to8_cartridge_r)) );
					}
					LOGMASKED(LOG_BANK, "to8_update_cart_bank: CART is external cartridge bank %i\n", bank);
				}
			}
			else
			{
				if ( m_old_cart_bank != 0 )
				{
					space.nop_read( 0x0000, 0x3fff);
					LOGMASKED(LOG_BANK, "to8_update_cart_bank: CART is unmapped\n");
				}
			}
		}
	}
	if ( bank != m_old_cart_bank )
	{
		m_cartbank->set_entry( bank );
		m_old_cart_bank = bank;
	}
}



void to9_state::to8_update_cart_bank_postload()
{
	to8_update_cart_bank();
}



/* ROM bank switch */
void to9_state::to8_cartridge_w(offs_t offset, uint8_t data)
{
	if ( offset >= 0x2000 )
		return;

	if ( m_to8_soft_select )
		m_to8_soft_bank = offset & 3;
	else
		m_thom_cart_bank = offset & 3;

	to8_update_cart_bank();
}



/* read signal to 0000-0003 generates a bank switch */
uint8_t to9_state::to8_cartridge_r(offs_t offset)
{
	uint8_t data = m_cart_rom[offset + (m_thom_cart_bank % m_thom_cart_nb_banks) * 0x4000];
	if ( !machine().side_effects_disabled() )
	{
		m_thom_cart_bank = offset & 3;
		to8_update_cart_bank();
	}
	return data;
}


/* ------------ system gate-array ------------ */



#define TO8_LIGHTPEN_DECAL 16



uint8_t to9_state::to8_gatearray_r(offs_t offset)
{
	struct thom_vsignal v = thom_get_vsignal();
	struct thom_vsignal l = thom_get_lightpen_vsignal( TO8_LIGHTPEN_DECAL, m_to7_lightpen_step - 1, 6 );
	int count, inil, init, lt3;
	uint8_t res;
	count = m_to7_lightpen ? l.count : v.count;
	inil  = m_to7_lightpen ? l.inil  : v.inil;
	init  = m_to7_lightpen ? l.init  : v.init;
	lt3   = m_to7_lightpen ? l.lt3   : v.lt3;

	switch ( offset )
	{
	case 0: /* system 2 / lightpen register 1 */
		if ( m_to7_lightpen )
			res = (count >> 8) & 0xff;
		else
			res = m_to8_reg_sys2 & 0xf0;
		break;

	case 1: /* ram register / lightpen register 2 */
		if ( m_to7_lightpen )
		{
			if ( !machine().side_effects_disabled() )
			{
				m_mainfirq->in_w<2>(0);
				m_to8_lightpen_intr = 0;
			}
			res = count & 0xff;
		}
		else
			res = m_to8_reg_ram & 0x1f;
		break;

	case 2: /* cartrige register / lightpen register 3 */
		if ( m_to7_lightpen )
			res = (lt3 << 7) | (inil << 6);
		else
			res = m_to8_reg_cart;
		break;

	case 3: /* lightpen register 4 */
		res = (v.init << 7) | (init << 6) | (v.inil << 5) | (m_to8_lightpen_intr << 1) | m_to7_lightpen;
		break;

	default:
		LOGMASKED(LOG_ERRORS, "$%04x to8_gatearray_r: invalid offset %i\n", m_maincpu->pc(), offset);
		res = 0;
	}

	LOGMASKED(LOG_VIDEO, "$%04x %f to8_gatearray_r: off=%i ($%04X) res=$%02X lightpen=%i\n", m_maincpu->pc(), machine().time().as_double(), offset, 0xe7e4 + offset, res, m_to7_lightpen);

	return res;
}



void to9_state::to8_gatearray_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_VIDEO, "$%04x %f to8_gatearray_w: off=%i ($%04X) data=$%02X\n", m_maincpu->pc(), machine().time().as_double(), offset, 0xe7e4 + offset, data);

	switch ( offset )
	{
	case 0: /* switch */
		m_to7_lightpen = data & 1;
		break;

	case 1: /* ram register */
		if ( m_to8_reg_sys1 & 0x10 )
		{
			m_to8_reg_ram = data;
			to8_update_ram_bank();
		}
		break;

	case 2: /* cartridge register */
		m_to8_reg_cart = data;
		to8_update_cart_bank();
		break;

	case 3: /* system register 1 */
		m_to8_reg_sys1 = data;
		to8_update_ram_bank();
		to8_update_cart_bank();
		break;

	default:
		LOGMASKED(LOG_ERRORS, "$%04x to8_gatearray_w: invalid offset %i (data=$%02X)\n", m_maincpu->pc(), offset, data);
	}
}



/* ------------ video gate-array ------------ */



uint8_t to9_state::to8_vreg_r(offs_t offset)
{
	/* 0xe7dc from external floppy drive aliases the video gate-array */
	if ( ( offset == 3 ) && ( m_to8_reg_ram & 0x80 ) && ( m_to8_reg_sys1 & 0x80 ) )
	{
		if ( machine().side_effects_disabled() )
			return 0;

		abort(); // return to7_floppy_r( 0xc );
	}

	switch ( offset )
	{
	case 0: /* palette data */
	{
		uint8_t c =  m_to9_palette_data[ m_to9_palette_idx ];
		if ( !machine().side_effects_disabled() )
		{
			m_to9_palette_idx = ( m_to9_palette_idx + 1 ) & 31;
		}
		return c;
	}

	case 1: /* palette address */
		return m_to9_palette_idx;

	case 2:
	case 3:
		return 0;

	default:
		LOGMASKED(LOG_ERRORS, "to8_vreg_r: invalid read offset %i\n", offset);
		return 0;
	}
}



void to9_state::to8_vreg_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_VIDEO, "$%04x %f to8_vreg_w: off=%i ($%04X) data=$%02X\n", m_maincpu->pc(), machine().time().as_double(), offset, 0xe7da + offset, data);

	switch ( offset )
	{
	case 0: /* palette data */
	{
		uint16_t color, idx;
		m_to9_palette_data[ m_to9_palette_idx ] = data;
		idx = m_to9_palette_idx / 2;
		color = m_to9_palette_data[ 2 * idx + 1 ];
		color = m_to9_palette_data[ 2 * idx ] | (color << 8);
		thom_set_palette( idx, color & 0x1fff );
		m_to9_palette_idx = ( m_to9_palette_idx + 1 ) & 31;
	}
	break;

	case 1: /* palette address */
		m_to9_palette_idx = data & 31;
		break;

	case 2: /* display register */
		to9_set_video_mode( data, 1 );
		break;

	case 3: /* system register 2 */
		/* 0xe7dc from external floppy drive aliases the video gate-array */
		if ( ( offset == 3 ) && ( m_to8_reg_ram & 0x80 ) && ( m_to8_reg_sys1 & 0x80 ) )
		{
			abort(); // to7_floppy_w( 0xc, data );
		}
		else
		{
			m_to8_reg_sys2 = data;
			thom_set_video_page( data >> 6 );
			thom_set_border_color( data & 15 );
		}
		break;

	default:
		LOGMASKED(LOG_ERRORS, "to8_vreg_w: invalid write offset %i data=$%02X\n", offset, data);
	}
}



/* ------------ system PIA 6821 ------------ */


uint8_t to9_state::to8_sys_porta_in()
{
	int ktest = m_to8_kbd->ktest_r();

	LOGMASKED(LOG_KBD, "$%04x %f: to8_sys_porta_in ktest=%i\n", m_maincpu->pc(), machine().time().as_double(), ktest);

	// PB1-7 are not connected, and are pulled hi internally
	return ktest | 0xfe;
}



void to9_state::to8_sys_portb_out(uint8_t data)
{
	m_centronics->write_data0(BIT(data, 0));
	m_centronics->write_strobe(BIT(data, 1));

	to8_update_ram_bank();

	if ( data & 4 ) /* bit 2: video overlay (TODO) */
		LOG("to8_sys_portb_out: video overlay not handled\n");
}



/* ------------ 6846 (timer, I/O) ------------ */


void to9_state::write_centronics_busy(int state)
{
	m_centronics_busy = state;
}

uint8_t to9_state::to8_timer_port_in()
{
	int lightpen = (m_io_lightpen_button->read() & 1) ? 2 : 0;
	int cass = to7_get_cassette() ? 0x80 : 0;
	int dtr = m_centronics_busy << 6;
	int lock = m_to8_kbd->caps_r() ? 0 : 8; /* undocumented! */
	return lightpen | cass | dtr | lock;
}



void to9_state::to8_timer_port_out(uint8_t data)
{
	int ack = (data & 0x20) ? 1 : 0;       /* bit 5: keyboard ACK */
	m_to8_bios_bank = (data & 0x10) ? 1 : 0; /* bit 4: BIOS bank*/
	thom_set_mode_point( data & 1 );       /* bit 0: video bank switch */
	m_biosbank->set_entry( m_to8_bios_bank );
	m_to8_soft_select = (data & 0x04) ? 1 : 0; /* bit 2: internal ROM select */
	to8_update_cart_bank();
	m_to8_kbd->set_ack(ack);
}



void to9_state::to8_timer_cp2_out(int state)
{
	/* mute */
	m_to7_game_mute = state;
	to7_game_sound_update();
}

/* ------------ lightpen ------------ */



/* direct connection to interrupt line instead of through a PIA */
void to9_state::to8_lightpen_cb( int step )
{
	if ( ! m_to7_lightpen )
		return;

	m_mainfirq->in_w<2>(1);
	m_to7_lightpen_step = step;
	m_to8_lightpen_intr = 1;
}



/* ------------ init / reset ------------ */



MACHINE_RESET_MEMBER( to9_state, to8 )
{
	LOG("to8: machine reset called\n");

	/* subsystems */
	thom_irq_reset();
	to7_game_reset();

	/* gate-array */
	m_to7_lightpen = 0;
	m_to8_reg_ram = 0;
	m_to8_reg_cart = 0;
	m_to8_reg_sys1 = 0;
	m_to8_reg_sys2 = 0;
	m_to8_lightpen_intr = 0;
	m_to8_soft_select = 0;

	/* video */
	thom_set_video_mode( THOM_VMODE_TO770 );
	m_thom_lightpen_cb = std::bind(&to9_state::to8_lightpen_cb, this,  std::placeholders::_1);
	thom_set_lightpen_callback( 4 );
	thom_set_border_color( 0 );
	thom_set_mode_point( 0 );
	m_pia_sys->cb1_w( 0 );

	/* memory */
	m_old_ram_bank = -1;
	m_old_cart_bank = -1;
	m_old_cart_bank_was_read_only = 0;
	m_to8_cart_vpage = 0;
	m_to8_data_vpage = 0;
	m_to8_soft_bank = 0;
	m_to8_bios_bank = 0;
	to8_update_ram_bank();
	to8_update_cart_bank();
	m_biosbank->set_entry( 0 );
	/* thom_cart_bank not reset */
}



MACHINE_START_MEMBER( to9_state, to8 )
{
	uint8_t* mem = memregion("maincpu")->base();
	uint8_t* cartmem = &m_cart_rom[0];
	uint8_t* ram = m_ram->pointer();

	LOG("to8: machine start called\n");

	/* subsystems */
	to7_game_init();
	to9_palette_init();

	m_extension->rom_map(m_maincpu->space(AS_PROGRAM), 0xe000, 0xe7bf);
	m_extension->io_map (m_maincpu->space(AS_PROGRAM), 0xe7c0, 0xe7ff);

	/* memory */
	m_thom_cart_bank = 0;
	m_thom_vram = ram;
	m_cartbank->configure_entries( 0, 4, cartmem, 0x4000 );
	m_cartbank->configure_entries( 4, 4, mem + 0x10000, 0x4000 );
	if ( m_ram->size() == 256*1024 )
	{
		m_cartbank->configure_entries( 8,    16, ram, 0x4000 );
		m_cartbank->configure_entries( 8+16, 16, ram, 0x4000 );
		m_datalobank->configure_entries( 0, 16, ram + 0x2000, 0x4000 );
		m_datalobank->configure_entries( 16, 16, ram + 0x2000, 0x4000 );
		m_datahibank->configure_entries( 0, 16, ram + 0x0000, 0x4000 );
		m_datahibank->configure_entries( 16, 16, ram + 0x0000, 0x4000 );
	}
	else
	{
		m_cartbank->configure_entries( 8, 32, ram, 0x4000 );
		m_datalobank->configure_entries( 0, 32, ram + 0x2000, 0x4000 );
		m_datahibank->configure_entries( 0, 32, ram + 0x0000, 0x4000 );
	}
	m_vrambank->configure_entries( 0,  2, ram, 0x2000 );
	m_syslobank->configure_entry( 0,  ram + 0x6000);
	m_syshibank->configure_entry( 0,  ram + 0x4000);
	m_biosbank->configure_entries( 0,  2, mem + 0x20000, 0x2000 );
	m_cartbank->set_entry( 0 );
	m_vrambank->set_entry( 0 );
	m_syslobank->set_entry( 0 );
	m_syshibank->set_entry( 0 );
	m_datalobank->set_entry( 0 );
	m_datahibank->set_entry( 0 );
	m_biosbank->set_entry( 0 );

	/* save-state */
	save_item(NAME(m_thom_cart_nb_banks));
	save_item(NAME(m_thom_cart_bank));
	save_item(NAME(m_to7_lightpen));
	save_item(NAME(m_to7_lightpen_step));
	save_item(NAME(m_to8_reg_ram));
	save_item(NAME(m_to8_reg_cart));
	save_item(NAME(m_to8_reg_sys1));
	save_item(NAME(m_to8_reg_sys2));
	save_item(NAME(m_to8_soft_select));
	save_item(NAME(m_to8_soft_bank));
	save_item(NAME(m_to8_bios_bank));
	save_item(NAME(m_to8_lightpen_intr));
	save_item(NAME(m_to8_data_vpage));
	save_item(NAME(m_to8_cart_vpage));
	save_pointer(NAME(cartmem), 0x10000 );
	machine().save().register_postload(save_prepost_delegate(FUNC(to9_state::to8_update_ram_bank_postload), this));
	machine().save().register_postload(save_prepost_delegate(FUNC(to9_state::to8_update_cart_bank_postload), this));
}



/***************************** TO9+ *************************/



/* ------------ system PIA 6821 ------------ */



/* ------------ 6846 (timer, I/O) ------------ */



uint8_t to9_state::to9p_timer_port_in()
{
	int lightpen = (m_io_lightpen_button->read() & 1) ? 2 : 0;
	int cass = to7_get_cassette() ? 0x80 : 0;
	int dtr = m_centronics_busy << 6;
	return lightpen | cass | dtr;
}



void to9_state::to9p_timer_port_out(uint8_t data)
{
	int bios_bank = (data & 0x10) ? 1 : 0; /* bit 4: BIOS bank */
	thom_set_mode_point( data & 1 );       /* bit 0: video bank switch */
	m_biosbank->set_entry( bios_bank );
	m_to8_soft_select = (data & 0x04) ? 1 : 0; /* bit 2: internal ROM select */
	to8_update_cart_bank();
}

/* ------------ init / reset ------------ */

MACHINE_RESET_MEMBER( to9_state, to9p )
{
	LOG("to9p: machine reset called\n");

	/* subsystems */
	thom_irq_reset();
	to7_game_reset();

	/* gate-array */
	m_to7_lightpen = 0;
	m_to8_reg_ram = 0;
	m_to8_reg_cart = 0;
	m_to8_reg_sys1 = 0;
	m_to8_reg_sys2 = 0;
	m_to8_lightpen_intr = 0;
	m_to8_soft_select = 0;

	/* video */
	thom_set_video_mode( THOM_VMODE_TO770 );
	m_thom_lightpen_cb = std::bind(&to9_state::to8_lightpen_cb, this,  std::placeholders::_1);
	thom_set_lightpen_callback( 4 );
	thom_set_border_color( 0 );
	thom_set_mode_point( 0 );
	m_pia_sys->cb1_w( 0 );

	/* memory */
	m_old_ram_bank = -1;
	m_old_cart_bank = -1;
	m_to8_cart_vpage = 0;
	m_to8_data_vpage = 0;
	m_to8_soft_bank = 0;
	m_to8_bios_bank = 0;
	to8_update_ram_bank();
	to8_update_cart_bank();
	m_biosbank->set_entry( 0 );
	/* thom_cart_bank not reset */
}



MACHINE_START_MEMBER( to9_state, to9p )
{
	uint8_t* mem = memregion("maincpu")->base();
	uint8_t* cartmem = &m_cart_rom[0];
	uint8_t* ram = m_ram->pointer();

	LOG("to9p: machine start called\n");

	/* subsystems */
	to7_game_init();
	to9_palette_init();

	m_extension->rom_map(m_maincpu->space(AS_PROGRAM), 0xe000, 0xe7bf);
	m_extension->io_map (m_maincpu->space(AS_PROGRAM), 0xe7c0, 0xe7ff);

	/* memory */
	m_thom_cart_bank = 0;
	m_thom_vram = ram;
	m_cartbank->configure_entries( 0,  4, cartmem, 0x4000 );
	m_cartbank->configure_entries( 4,  4, mem + 0x10000, 0x4000 );
	m_cartbank->configure_entries( 8, 32, ram, 0x4000 );
	m_vrambank->configure_entries( 0,  2, ram, 0x2000 );
	m_syslobank->configure_entry( 0,  ram + 0x6000 );
	m_syshibank->configure_entry( 0,  ram + 0x4000 );
	m_datalobank->configure_entries( 0, 32, ram + 0x2000, 0x4000 );
	m_datahibank->configure_entries( 0, 32, ram + 0x0000, 0x4000 );
	m_biosbank->configure_entries( 0,  2, mem + 0x20000, 0x2000 );
	m_cartbank->set_entry( 0 );
	m_vrambank->set_entry( 0 );
	m_syslobank->set_entry( 0 );
	m_syshibank->set_entry( 0 );
	m_datalobank->set_entry( 0 );
	m_datahibank->set_entry( 0 );
	m_biosbank->set_entry( 0 );

	/* save-state */
	save_item(NAME(m_thom_cart_nb_banks));
	save_item(NAME(m_thom_cart_bank));
	save_item(NAME(m_to7_lightpen));
	save_item(NAME(m_to7_lightpen_step));
	save_item(NAME(m_to8_reg_ram));
	save_item(NAME(m_to8_reg_cart));
	save_item(NAME(m_to8_reg_sys1));
	save_item(NAME(m_to8_reg_sys2));
	save_item(NAME(m_to8_soft_select));
	save_item(NAME(m_to8_soft_bank));
	save_item(NAME(m_to8_bios_bank));
	save_item(NAME(m_to8_lightpen_intr));
	save_item(NAME(m_to8_data_vpage));
	save_item(NAME(m_to8_cart_vpage));
	save_pointer(NAME(cartmem), 0x10000 );
	machine().save().register_postload(save_prepost_delegate(FUNC(to9_state::to8_update_ram_bank_postload), this));
	machine().save().register_postload(save_prepost_delegate(FUNC(to9_state::to8_update_cart_bank_postload), this));
}



/****************** MO6 / Olivetti Prodest PC 128 *******************/



/* ------------ RAM / ROM banking ------------ */



void mo6_state::mo6_update_ram_bank()
{
	uint8_t bank = 0;

	if ( m_to8_reg_sys1 & 0x10 )
	{
		bank = m_to8_reg_ram & 7; /* 128 KB RAM only = 8 pages */
	}
	if ( bank != m_to8_data_vpage ) {
		m_datalobank->set_entry( bank );
		m_datahibank->set_entry( bank );
		m_to8_data_vpage = bank;
		m_old_ram_bank = bank;
		LOGMASKED(LOG_BANK, "mo6_update_ram_bank: select bank %i (new style)\n", bank);
	}
}



void mo6_state::mo6_update_ram_bank_postload()
{
	mo6_update_ram_bank();
}



void mo6_state::mo6_update_cart_bank()
{
	address_space& space = m_maincpu->space(AS_PROGRAM);
	int b = (m_pia_sys->a_output() >> 5) & 1;
	int bank = 0;
	int bank_is_read_only = 0;

	if ( ( ( m_to8_reg_sys1 & 0x40 ) && ( m_to8_reg_cart & 0x20 ) ) || ( ! ( m_to8_reg_sys1 & 0x40 ) && ( m_mo5_reg_cart & 4 ) ) )
	{
		/* RAM space */
		if ( m_to8_reg_sys1 & 0x40 )
		{
			/* use a7e6 */
			m_to8_cart_vpage = m_to8_reg_cart & 7; /* 128 KB RAM only = 8 pages */
			bank = 8 + m_to8_cart_vpage;
			bank_is_read_only = (( m_to8_reg_cart & 0x40 ) == 0);
			if ( bank != m_old_cart_bank )
			{
				if (m_to8_cart_vpage < 4)
				{
					if (m_old_cart_bank < 8 || m_old_cart_bank > 11)
					{
						space.install_read_bank( 0xb000, 0xbfff, m_cartlobank );
						space.install_read_bank( 0xc000, 0xefff, m_carthibank );
						if ( bank_is_read_only )
						{
							space.nop_write( 0xb000, 0xefff);
						}
						else
						{
							space.install_write_handler( 0xb000, 0xbfff, write8sm_delegate(*this, FUNC(mo6_state::mo6_vcart_lo_w)));
							space.install_write_handler( 0xc000, 0xefff, write8sm_delegate(*this, FUNC(mo6_state::mo6_vcart_hi_w)));
						}
					}
				}
				else
				{
					if (m_old_cart_bank < 12)
					{
						if ( bank_is_read_only )
						{
							space.install_read_bank( 0xb000, 0xbfff, m_cartlobank );
							space.install_read_bank( 0xc000, 0xefff, m_carthibank );
							space.nop_write( 0xb000, 0xefff);
						}
						else
						{
							space.install_readwrite_bank( 0xb000, 0xbfff, m_cartlobank );
							space.install_readwrite_bank( 0xc000, 0xefff, m_carthibank );
						}
					}
				}
				LOGMASKED(LOG_BANK, "mo6_update_cart_bank: CART is RAM bank %i (%s)\n", m_to8_cart_vpage, bank_is_read_only ? "read-only" : "read-write");
			}
			else if ( bank_is_read_only != m_old_cart_bank_was_read_only )
			{
				if ( bank_is_read_only )
				{
					space.nop_write( 0xb000, 0xefff);
				}
				else
				{
					if (m_to8_cart_vpage < 4)
					{
						space.install_write_handler( 0xb000, 0xbfff, write8sm_delegate(*this, FUNC(mo6_state::mo6_vcart_lo_w)));
						space.install_write_handler( 0xc000, 0xefff, write8sm_delegate(*this, FUNC(mo6_state::mo6_vcart_hi_w)));
					}
					else
					{
						space.install_readwrite_bank( 0xb000, 0xbfff, m_cartlobank );
						space.install_readwrite_bank( 0xc000, 0xefff, m_carthibank );
					}
				}
				LOGMASKED(LOG_BANK, "mo6_update_cart_bank: update CART bank %i write status to %s\n", m_to8_cart_vpage, bank_is_read_only ? "read-only" : "read-write");
			}
			m_old_cart_bank_was_read_only = bank_is_read_only;
		}
		else if ( m_thom_cart_nb_banks == 4 )
		{
			/* "JANE"-style cartridge bank switching */
			bank = m_mo5_reg_cart & 3;
			if ( bank != m_old_cart_bank )
			{
				if ( m_old_cart_bank < 0 || m_old_cart_bank > 3 )
				{
										space.install_read_bank( 0xb000, 0xbfff, m_cartlobank );
										space.install_read_bank( 0xc000, 0xefff, m_carthibank );
					space.nop_write( 0xb000, 0xefff);
				}
				LOGMASKED(LOG_BANK, "mo6_update_cart_bank: CART is external cartridge bank %i (A7CB style)\n", bank);
			}
		}
		else
		{
			/* RAM from MO5 network extension */
			int bank_is_read_only = (( m_mo5_reg_cart & 8 ) == 0);
			m_to8_cart_vpage = (m_mo5_reg_cart & 3) | 4;
			bank = 8 + m_to8_cart_vpage;
			if ( bank != m_old_cart_bank )
			{
				if ( m_old_cart_bank < 12 )
				{
					if ( bank_is_read_only )
					{
						space.install_read_bank( 0xb000, 0xbfff, m_cartlobank );
						space.install_read_bank( 0xc000, 0xefff, m_carthibank );
						space.nop_write( 0xb000, 0xefff);
					}
					else
					{
						space.install_readwrite_bank( 0xb000, 0xbfff, m_cartlobank );
						space.install_readwrite_bank( 0xc000, 0xefff, m_carthibank );
					}
				}
				LOGMASKED(LOG_BANK, "mo6_update_cart_bank: CART is RAM bank %i (MO5 compat.) (%s)\n", m_to8_cart_vpage, bank_is_read_only ? "read-only" : "read-write");
			}
			else if ( bank_is_read_only != m_old_cart_bank_was_read_only )
			{
				if ( bank_is_read_only )
				{
					space.install_read_bank( 0xb000, 0xbfff, m_cartlobank );
					space.install_read_bank( 0xc000, 0xefff, m_carthibank );
					space.nop_write( 0xb000, 0xefff);
				}
				else
				{
					space.install_readwrite_bank( 0xb000, 0xbfff, m_cartlobank );
					space.install_readwrite_bank( 0xc000, 0xefff, m_carthibank );
				}
				LOGMASKED(LOG_BANK, "mo5_update_cart_bank: update CART bank %i write status to %s\n", m_to8_cart_vpage, bank_is_read_only ? "read-only" : "read-write");
			}
			m_old_cart_bank_was_read_only = bank_is_read_only;
		}
	}
	else
	{
		/* ROM space */
		if ( m_to8_reg_sys2 & 0x20 )
		{
			/* internal ROM */
			if ( m_to8_reg_sys2 & 0x10)
			{
				bank = b + 6; /* BASIC 128 */
			}
			else
			{
				bank = b + 4;                      /* BASIC 1 */
			}
			if ( bank != m_old_cart_bank )
			{
				if ( m_old_cart_bank < 4 || m_old_cart_bank > 7 )
				{
					space.install_read_bank( 0xb000, 0xbfff, m_cartlobank );
					space.install_read_bank( 0xc000, 0xefff, m_carthibank );
					space.install_write_handler( 0xb000, 0xefff, write8sm_delegate(*this, FUNC(mo6_state::mo6_cartridge_w)) );
				}
				LOGMASKED(LOG_BANK, "mo6_update_cart_bank: CART is internal ROM bank %i\n", b);
			}
		}
		else
		{
			/* cartridge */
			if ( m_thom_cart_nb_banks )
			{
				bank = m_thom_cart_bank % m_thom_cart_nb_banks;
				if ( bank != m_old_cart_bank )
				{
					if ( m_old_cart_bank < 0 || m_old_cart_bank > 3 )
					{
						space.install_read_bank( 0xb000, 0xbfff, m_cartlobank );
						space.install_read_bank( 0xc000, 0xefff, m_carthibank );
						space.install_write_handler( 0xb000, 0xefff, write8sm_delegate(*this, FUNC(mo6_state::mo6_cartridge_w)) );
						space.install_read_handler( 0xbffc, 0xbfff, read8sm_delegate(*this, FUNC(mo6_state::mo6_cartridge_r)) );
					}
					LOGMASKED(LOG_BANK, "mo6_update_cart_bank: CART is external cartridge bank %i\n", bank);
				}
			}
			else
			{
				if ( m_old_cart_bank != 0 )
				{
					space.nop_read( 0xb000, 0xefff );
					LOGMASKED(LOG_BANK, "mo6_update_cart_bank: CART is unmapped\n");
				}
			}
		}
	}
	if ( bank != m_old_cart_bank )
	{
		m_cartlobank->set_entry( bank );
		m_carthibank->set_entry( bank );
		m_biosbank->set_entry( b );
		m_old_cart_bank = bank;
	}
}



void mo6_state::mo6_update_cart_bank_postload()
{
	mo6_update_cart_bank();
}



/* write signal generates a bank switch */
void mo6_state::mo6_cartridge_w(offs_t offset, uint8_t data)
{
	if ( offset >= 0x2000 )
		return;

	m_thom_cart_bank = offset & 3;
	mo6_update_cart_bank();
}



/* read signal generates a bank switch */
uint8_t mo6_state::mo6_cartridge_r(offs_t offset)
{
	uint8_t data = m_cart_rom[offset + 0x3ffc + (m_thom_cart_bank % m_thom_cart_nb_banks) * 0x4000];
	if ( !machine().side_effects_disabled() )
	{
		m_thom_cart_bank = offset & 3;
		mo6_update_cart_bank();
	}
	return data;
}



void mo6_state::mo6_ext_w(uint8_t data)
{
	/* MO5 network extension compatible */
	m_mo5_reg_cart = data;
	mo6_update_cart_bank();
}



/* ------------ game 6821 PIA ------------ */

/* similar to SX 90-018, but with a few differences: mute, printer */


void mo6_state::mo6_centronics_busy(int state)
{
	m_pia_game->cb1_w(state);
}


void mo6_state::mo6_game_porta_out(uint8_t data)
{
	LOG("$%04x %f mo6_game_porta_out: CENTRONICS set data=$%02X\n", m_maincpu->pc(), machine().time().as_double(), data);

	/* centronics data */
	m_cent_data_out->write(data);
}



void mo6_state::mo6_game_cb2_out(int state)
{
	LOG("$%04x %f mo6_game_cb2_out: CENTRONICS set strobe=%i\n", m_maincpu->pc(), machine().time().as_double(), state);

	/* centronics strobe */
	m_centronics->write_strobe(state);
}



TIMER_CALLBACK_MEMBER(mo6_state::mo6_game_update_cb)
{
	/* unlike the TO8, CB1 & CB2 are not connected to buttons */
	if ( m_io_config->read() & 1 )
	{
		uint8_t mouse = to7_get_mouse_signal();
		m_pia_game->ca1_w( BIT(mouse, 0) ); /* XA */
		m_pia_game->ca2_w( BIT(mouse, 1) ); /* YA */
	}
	else
	{
		/* joystick */
		uint8_t in = m_io_game_port_buttons->read();
		m_pia_game->ca1_w( BIT(in, 2) ); /* P1 action B */
		m_pia_game->ca2_w( BIT(in, 6) ); /* P1 action A */
	}
}



void mo6_state::mo6_game_init()
{
	LOG("mo6_game_init called\n");
	m_to7_game_timer = timer_alloc(FUNC(mo6_state::mo6_game_update_cb), this);
	m_to7_game_timer->adjust(TO7_GAME_POLL_PERIOD, 0, TO7_GAME_POLL_PERIOD);
	save_item(NAME(m_to7_game_sound));
	save_item(NAME(m_to7_game_mute));
}



void mo6_state::mo6_game_reset()
{
	LOG("mo6_game_reset called\n");
	m_pia_game->ca1_w( 0 );
	m_to7_game_sound = 0;
	m_to7_game_mute = 0;
	to7_game_sound_update();
}



/* ------------ system PIA 6821 ------------ */



uint8_t mo6_state::mo6_sys_porta_in()
{
	return
		((m_io_lightpen_button->read() & 1) ? 2 : 0) | /* bit 1: lightpen button */
		8 |                                            /* bit 3: kbd-line float up to 1 */
		(mo5_get_cassette() ? 0x80 : 0) |              /* bit 7: cassette input */
		0x75;                                          /* other bits are unconnected and pulled hi internally */
}



uint8_t mo6_state::mo6_sys_portb_in()
{
	/* keyboard: 9 lines of 8 keys */
	uint8_t porta = m_pia_sys->a_output();
	uint8_t portb = m_pia_sys->b_output();
	int col = (portb >> 4) & 7;    /* B bits 4-6: kbd column */
	int lin = (portb >> 1) & 7;    /* B bits 1-3: kbd line */

	if ( ! (porta & 8) )
		lin = 8;     /* A bit 3: 9-th kbd line select */

	return
		( m_io_keyboard[lin]->read() & (1 << col) ) ?  0x80 : 0;
	/* bit 7: key up */
}



void mo6_state::mo6_sys_porta_out(uint8_t data)
{
	thom_set_mode_point( data & 1 );                /* bit 0: video bank switch */
	m_to7_game_mute = data & 4;                       /* bit 2: sound mute */
	m_caps_led = (data & 16) ? 0 : 1;      /* bit 4: keyboard led */
	mo5_set_cassette( (data & 0x40) ? 1 : 0 );     /* bit 6: cassette output */
	mo6_update_cart_bank();                  /* bit 5: rom bank */
	to7_game_sound_update();
}



void mo6_state::mo6_sys_cb2_out(int state)
{
	/* SCART pin 8 = slow switch (?) */
	LOG("mo6_sys_cb2_out: SCART slow switch set to %i\n", state);
}



/* ------------ system gate-array ------------ */

#define MO6_LIGHTPEN_DECAL 12



uint8_t mo6_state::mo6_gatearray_r(offs_t offset)
{
	struct thom_vsignal v = thom_get_vsignal();
	struct thom_vsignal l = thom_get_lightpen_vsignal( MO6_LIGHTPEN_DECAL, m_to7_lightpen_step - 1, 6 );
	int count, inil, init, lt3;
	uint8_t res;
	count = m_to7_lightpen ? l.count : v.count;
	inil  = m_to7_lightpen ? l.inil  : v.inil;
	init  = m_to7_lightpen ? l.init  : v.init;
	lt3   = m_to7_lightpen ? l.lt3   : v.lt3;

	switch ( offset )
	{
	case 0: /* system 2 / lightpen register 1 */
		if ( m_to7_lightpen )
			res = (count >> 8) & 0xff;
		else
			res = m_to8_reg_sys2 & 0xf0;
		break;

	case 1: /* ram register / lightpen register 2 */
		if ( m_to7_lightpen )
		{
			if ( !machine().side_effects_disabled() )
			{
				m_mainfirq->in_w<2>(0);
				m_to8_lightpen_intr = 0;
			}
			res =  count & 0xff;
		}
		else
			res = m_to8_reg_ram & 0x1f;
		break;

	case 2: /* cartrige register / lightpen register 3 */
		if ( m_to7_lightpen )
			res = (lt3 << 7) | (inil << 6);
		else
			res = 0;
		break;

	case 3: /* lightpen register 4 */
		res = (v.init << 7) | (init << 6) | (v.inil << 5) | (m_to8_lightpen_intr << 1) | m_to7_lightpen;
		break;

	default:
		LOGMASKED(LOG_ERRORS, "$%04x mo6_gatearray_r: invalid offset %i\n", m_maincpu->pc(), offset);
		res = 0;
	}

	LOGMASKED(LOG_VIDEO, "$%04x %f mo6_gatearray_r: off=%i ($%04X) res=$%02X lightpen=%i\n", m_maincpu->pc(), machine().time().as_double(), offset, 0xa7e4 + offset, res, m_to7_lightpen);

	return res;
}



void mo6_state::mo6_gatearray_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_VIDEO, "$%04x %f mo6_gatearray_w: off=%i ($%04X) data=$%02X\n", m_maincpu->pc(), machine().time().as_double(), offset, 0xa7e4 + offset, data);

	switch ( offset )
	{
	case 0: /* switch */
		m_to7_lightpen = data & 1;
		break;

	case 1: /* ram register */
		if ( m_to8_reg_sys1 & 0x10 )
		{
			m_to8_reg_ram = data;
			mo6_update_ram_bank();
		}
		break;

	case 2: /* cartridge register */
		m_to8_reg_cart = data;
		mo6_update_cart_bank();
		break;

	case 3: /* system register 1 */
		m_to8_reg_sys1 = data;
		mo6_update_ram_bank();
		mo6_update_cart_bank();
		break;

	default:
		LOGMASKED(LOG_ERRORS, "$%04x mo6_gatearray_w: invalid offset %i (data=$%02X)\n", m_maincpu->pc(), offset, data);
	}
}


/* ------------ video gate-array ------------ */



uint8_t mo6_state::mo6_vreg_r(offs_t offset)
{
	/* 0xa7dc from external floppy drive aliases the video gate-array */
	if ( ( offset == 3 ) && ( m_to8_reg_ram & 0x80 ) )
		{
		if ( !machine().side_effects_disabled() )
			abort(); // return to7_floppy_r( 0xc );
		}

	switch ( offset )
	{
	case 0: /* palette data */
	case 1: /* palette address */
		return to8_vreg_r( offset );

	case 2:
	case 3:
		return 0;

	default:
		LOGMASKED(LOG_ERRORS, "mo6_vreg_r: invalid read offset %i\n", offset);
		return 0;
	}
}



void mo6_state::mo6_vreg_w(offs_t offset, uint8_t data)
{
	LOGMASKED(LOG_VIDEO, "$%04x %f mo6_vreg_w: off=%i ($%04X) data=$%02X\n", m_maincpu->pc(), machine().time().as_double(), offset, 0xa7da + offset, data);

	switch ( offset )
	{
	case 0: /* palette data */
	case 1: /* palette address */
		to8_vreg_w( offset, data );
		return;

	case 2: /* display / external floppy register */
		if ( ( m_to8_reg_sys1 & 0x80 ) && ( m_to8_reg_ram & 0x80 ) )
			abort(); // to7_floppy_w( 0xc, data );
		else
			to9_set_video_mode( data, 2 );
		break;

	case 3: /* system register 2 */
		/* 0xa7dc from external floppy drive aliases the video gate-array */
		if ( ( offset == 3 ) && ( m_to8_reg_ram & 0x80 ) )
			abort(); // to7_floppy_w( 0xc, data );
		else
		{
			m_to8_reg_sys2 = data;
			thom_set_video_page( data >> 6 );
			thom_set_border_color( data & 15 );
			mo6_update_cart_bank();
		}
		break;

	default:
		LOGMASKED(LOG_ERRORS, "mo6_vreg_w: invalid write offset %i data=$%02X\n", offset, data);
	}
}



/* ------------ init / reset ------------ */



MACHINE_RESET_MEMBER( mo6_state, mo6 )
{
	LOG("mo6: machine reset called\n");

	/* subsystems */
	thom_irq_reset();
	mo6_game_reset();
	mo5_init_timer();

	/* gate-array */
	m_to7_lightpen = 0;
	m_to8_reg_ram = 0;
	m_to8_reg_cart = 0;
	m_to8_reg_sys1 = 0;
	m_to8_reg_sys2 = 0;
	m_to8_lightpen_intr = 0;

	/* video */
	thom_set_video_mode( THOM_VMODE_MO5 );
	m_thom_lightpen_cb = std::bind(&mo6_state::to8_lightpen_cb, this,  std::placeholders::_1);
	thom_set_lightpen_callback( 3 );
	thom_set_border_color( 0 );
	thom_set_mode_point( 0 );
	m_pia_sys->ca1_w( 0 );

	/* memory */
	m_old_ram_bank = -1;
	m_old_cart_bank = -1;
	m_to8_cart_vpage = 0;
	m_to8_data_vpage = 0;
	mo6_update_ram_bank();
	mo6_update_cart_bank();
	/* mo5_reg_cart not reset */
	/* thom_cart_bank not reset */
}



MACHINE_START_MEMBER( mo6_state, mo6 )
{
	uint8_t* mem = memregion("maincpu")->base();
	uint8_t* cartmem = &m_cart_rom[0];
	uint8_t* ram = m_ram->pointer();

	LOG("mo6: machine start called\n");

	/* subsystems */
	mo6_game_init();
	to9_palette_init();
	m_mo5_periodic_timer = timer_alloc(FUNC(mo6_state::mo5_periodic_cb), this);

	m_extension->rom_map(m_maincpu->space(AS_PROGRAM), 0xa000, 0xa7bf);
	m_extension->io_map (m_maincpu->space(AS_PROGRAM), 0xa7c0, 0xa7ff);

	/* memory */
	m_thom_cart_bank = 0;
	m_mo5_reg_cart = 0;
	m_thom_vram = ram;
	m_cartlobank->configure_entries( 0, 4, cartmem, 0x4000 );
	m_cartlobank->configure_entry( 4, cartmem + 0xf000 ); // FIXME: this is wrong
	m_cartlobank->configure_entry( 5, mem + 0x13000 ); // FIXME: this is wrong
	m_cartlobank->configure_entries( 6, 2, mem + 0x18000, 0x4000 );
	m_cartlobank->configure_entries( 8, 8, ram + 0x3000, 0x4000 );
	m_carthibank->configure_entries( 0, 4, cartmem + 0x1000, 0x4000 );
	m_carthibank->configure_entries( 4, 2, mem + 0x10000, 0x4000 );
	m_carthibank->configure_entries( 6, 2, mem + 0x18000 + 0x1000, 0x4000 );
	m_carthibank->configure_entries( 8, 8, ram, 0x4000 );
	m_vrambank->configure_entries( 0, 2, ram, 0x2000 );
	m_syslobank->configure_entry( 0, ram + 0x6000);
	m_syshibank->configure_entry( 0, ram + 0x4000);
	m_datalobank->configure_entries( 0, 8, ram + 0x2000, 0x4000 );
	m_datahibank->configure_entries( 0, 8, ram + 0x0000, 0x4000 );
	m_biosbank->configure_entries( 0, 2, mem + 0x13000, 0x4000 );
	m_cartlobank->set_entry( 0 );
	m_carthibank->set_entry( 0 );
	m_vrambank->set_entry( 0 );
	m_syslobank->set_entry( 0 );
	m_syshibank->set_entry( 0 );
	m_datalobank->set_entry( 0 );
	m_datahibank->set_entry( 0 );
	m_biosbank->set_entry( 0 );

	/* save-state */
	save_item(NAME(m_thom_cart_nb_banks));
	save_item(NAME(m_thom_cart_bank));
	save_item(NAME(m_to7_lightpen));
	save_item(NAME(m_to7_lightpen_step));
	save_item(NAME(m_to8_reg_ram));
	save_item(NAME(m_to8_reg_cart));
	save_item(NAME(m_to8_reg_sys1));
	save_item(NAME(m_to8_reg_sys2));
	save_item(NAME(m_to8_lightpen_intr));
	save_item(NAME(m_to8_data_vpage));
	save_item(NAME(m_to8_cart_vpage));
	save_item(NAME(m_mo5_reg_cart));
	save_pointer(NAME(cartmem), 0x10000 );
	machine().save().register_postload(save_prepost_delegate(FUNC(mo6_state::mo6_update_ram_bank_postload), this));
	machine().save().register_postload(save_prepost_delegate(FUNC(mo6_state::mo6_update_cart_bank_postload), this));
}



/***************************** MO5 NR *************************/



/* ------------ system PIA 6821 ------------ */



uint8_t mo5nr_state::mo5nr_sys_portb_in()
{
	/* keyboard: only 8 lines of 8 keys (MO6 has 9 lines) */
	uint8_t portb = m_pia_sys->b_output();
	int col = (portb >> 4) & 7;    /* B bits 4-6: kbd column */
	int lin = (portb >> 1) & 7;    /* B bits 1-3: kbd line */

	return ( m_io_keyboard[lin]->read() & (1 << col) ) ? 0x80 : 0;
	/* bit 7: key up */
}



void mo5nr_state::mo5nr_sys_porta_out(uint8_t data)
{
	/* no keyboard LED */
	thom_set_mode_point( data & 1 );           /* bit 0: video bank switch */
	m_to7_game_mute = data & 4;                       /* bit 2: sound mute */
	mo5_set_cassette( (data & 0x40) ? 1 : 0 );     /* bit 6: cassette output */
	mo6_update_cart_bank();                  /* bit 5: rom bank */
	to7_game_sound_update();
}



/* ------------ game 6821 PIA ------------ */

/* similar to the MO6, without the printer */



void mo5nr_state::mo5nr_game_init()
{
	LOG("mo5nr_game_init called\n");
	m_to7_game_timer = timer_alloc(FUNC(mo5nr_state::mo6_game_update_cb), this);
	m_to7_game_timer->adjust( TO7_GAME_POLL_PERIOD, 0, TO7_GAME_POLL_PERIOD );
	save_item(NAME(m_to7_game_sound));
	save_item(NAME(m_to7_game_mute));
}



void mo5nr_state::mo5nr_game_reset()
{
	LOG("mo5nr_game_reset called\n");
	m_pia_game->ca1_w( 0 );
	m_to7_game_sound = 0;
	m_to7_game_mute = 0;
	to7_game_sound_update();
}



/* ------------ init / reset ------------ */


uint8_t mo5nr_state::id_r()
{
	return (m_nanoreseau_config->read() >> 1) & 0x1f;
}

MACHINE_RESET_MEMBER( mo5nr_state, mo5nr )
{
	LOG("mo5nr: machine reset called\n");

	m_extension_view.select(m_nanoreseau_config->read() & 1);

	/* subsystems */
	thom_irq_reset();
	mo5nr_game_reset();
	mo5_init_timer();

	/* gate-array */
	m_to7_lightpen = 0;
	m_to8_reg_ram = 0;
	m_to8_reg_cart = 0;
	m_to8_reg_sys1 = 0;
	m_to8_reg_sys2 = 0;
	m_to8_lightpen_intr = 0;

	/* video */
	thom_set_video_mode( THOM_VMODE_MO5 );
	m_thom_lightpen_cb = std::bind(&mo5nr_state::to8_lightpen_cb, this,  std::placeholders::_1);
	thom_set_lightpen_callback( 3 );
	thom_set_border_color( 0 );
	thom_set_mode_point( 0 );
	m_pia_sys->ca1_w( 0 );

	/* memory */
	m_old_ram_bank = -1;
	m_old_cart_bank = -1;
	m_to8_cart_vpage = 0;
	m_to8_data_vpage = 0;
	mo6_update_ram_bank();
	mo6_update_cart_bank();
	/* mo5_reg_cart not reset */
	/* thom_cart_bank not reset */
}



MACHINE_START_MEMBER( mo5nr_state, mo5nr )
{
	uint8_t* mem = memregion("maincpu")->base();
	uint8_t* cartmem = &m_cart_rom[0];
	uint8_t* ram = m_ram->pointer();

	LOG("mo5nr: machine start called\n");

	/* subsystems */
	mo5nr_game_init();
	to9_palette_init();
	m_mo5_periodic_timer = timer_alloc(FUNC(mo5nr_state::mo5_periodic_cb), this);

	m_extension->rom_map(m_extension_view[0], 0xa000, 0xa7bf);
	m_extension->io_map (m_extension_view[0], 0xa7c0, 0xa7ff);
	m_extension_view[1].install_device(0xa000, 0xa7bf, *m_nanoreseau, &nanoreseau_device::rom_map );
	m_extension_view[1].install_device(0xa7c0, 0xa7ff, *m_nanoreseau, &nanoreseau_device::io_map  );

	/* memory */
	m_thom_cart_bank = 0;
	m_mo5_reg_cart = 0;
	m_thom_vram = ram;

	m_cartlobank->configure_entries( 0, 4, cartmem, 0x4000 );
	m_cartlobank->configure_entry( 4, cartmem + 0xf000 ); // FIXME: this is wrong
	m_cartlobank->configure_entry( 5, mem + 0x13000 ); // FIXME: this is wrong
	m_cartlobank->configure_entries( 6, 2, mem + 0x18000, 0x4000 );
	m_cartlobank->configure_entries( 8, 8, ram + 0x3000, 0x4000 );
	m_carthibank->configure_entries( 0, 4, cartmem + 0x1000, 0x4000 );
	m_carthibank->configure_entries( 4, 2, mem + 0x10000, 0x4000 );
	m_carthibank->configure_entries( 6, 2, mem + 0x18000 + 0x1000, 0x4000 );
	m_carthibank->configure_entries( 8, 8, ram, 0x4000 );
	m_vrambank->configure_entries( 0, 2, ram, 0x2000 );
	m_syslobank->configure_entry( 0, ram + 0x6000);
	m_syshibank->configure_entry( 0, ram + 0x4000);
	m_datalobank->configure_entries( 0, 8, ram + 0x2000, 0x4000 );
	m_datahibank->configure_entries( 0, 8, ram + 0x0000, 0x4000 );
	m_biosbank->configure_entries( 0, 2, mem + 0x13000, 0x4000 );
	m_cartlobank->set_entry( 0 );
	m_carthibank->set_entry( 0 );
	m_vrambank->set_entry( 0 );
	m_syslobank->set_entry( 0 );
	m_syshibank->set_entry( 0 );
	m_datalobank->set_entry( 0 );
	m_datahibank->set_entry( 0 );
	m_biosbank->set_entry( 0 );

	/* save-state */
	save_item(NAME(m_thom_cart_nb_banks));
	save_item(NAME(m_thom_cart_bank));
	save_item(NAME(m_to7_lightpen));
	save_item(NAME(m_to7_lightpen_step));
	save_item(NAME(m_to8_reg_ram));
	save_item(NAME(m_to8_reg_cart));
	save_item(NAME(m_to8_reg_sys1));
	save_item(NAME(m_to8_reg_sys2));
	save_item(NAME(m_to8_lightpen_intr));
	save_item(NAME(m_to8_data_vpage));
	save_item(NAME(m_to8_cart_vpage));
	save_item(NAME(m_mo5_reg_cart));
	save_pointer(NAME(cartmem), 0x10000 );
	machine().save().register_postload(save_prepost_delegate(FUNC(mo5nr_state::mo6_update_ram_bank_postload), this));
	machine().save().register_postload(save_prepost_delegate(FUNC(mo5nr_state::mo6_update_cart_bank_postload), this));
}
