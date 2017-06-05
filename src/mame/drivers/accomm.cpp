// license:BSD-3-Clause
// copyright-holders:R. Belmont, Wilbert Pol
/***************************************************************************

    Acorn Communicator

    Driver-in-progress by R. Belmont
    Electron ULA emulation by Wilbert Pol

    Main CPU: 65C816
    Other chips: 6850 UART, 6522 VIA, SAA5240(video?), AM7910 modem, PCF0335(?)

****************************************************************************/

#include "emu.h"
#include "cpu/g65816/g65816.h"
#include "machine/nvram.h"
#include "machine/bankdev.h"
#include "bus/rs232/rs232.h"
#include "screen.h"
#include "speaker.h"

/* Interrupts */
#define INT_HIGH_TONE       0x40
#define INT_TRANSMIT_EMPTY  0x20
#define INT_RECEIVE_FULL    0x10
#define INT_RTC             0x08
#define INT_DISPLAY_END     0x04
#define INT_SET             0x100
#define INT_CLEAR           0x200

/* ULA context */

struct ULA
{
	uint8_t interrupt_status;
	uint8_t interrupt_control;
	uint8_t rompage;
	uint16_t screen_start;
	uint16_t screen_base;
	int screen_size;
	uint16_t screen_addr;
	uint8_t *vram;
	int current_pal[16];
	int communication_mode;
	int screen_mode;
	int cassette_motor_mode;
	int capslock_mode;
//  int scanline;
	/* tape reading related */
	uint32_t tape_value;
	int tape_steps;
	int bit_count;
	int high_tone_set;
	int start_bit;
	int stop_bit;
	int tape_running;
	uint8_t tape_byte;
};

class accomm_state : public driver_device
{
public:
	accomm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_bank0dev(*this, "bank0dev"),
			m_vram(*this, "vram"),
			m_keybd(*this, "LINE.%u", 0),
			m_keybd2(*this, "LINE2.%u", 0)
	{ }

	virtual void machine_reset() override;
	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(b0_rom_disable_w);
	DECLARE_READ8_MEMBER(read_keyboard);
	DECLARE_READ8_MEMBER(read_keyboard2);
	DECLARE_READ8_MEMBER(sheila_r);
	DECLARE_WRITE8_MEMBER(sheila_w);

	DECLARE_PALETTE_INIT(accomm);
	INTERRUPT_GEN_MEMBER(vbl_int);

protected:

	// devices
	required_device<g65816_device> m_maincpu;
	required_device<address_map_bank_device> m_bank0dev;
	required_shared_ptr<uint8_t> m_vram;
	required_ioport_array<14> m_keybd, m_keybd2;

	// driver_device overrides
	virtual void video_start() override;

	void interrupt_handler(int mode, int interrupt);
	inline uint8_t read_vram( uint16_t addr );
	inline void plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color);

private:
	ULA m_ula;
	int m_map4[256];
	int m_map16[256];
};

static const rgb_t electron_palette[8]=
{
	rgb_t(0x0ff,0x0ff,0x0ff),
	rgb_t(0x000,0x0ff,0x0ff),
	rgb_t(0x0ff,0x000,0x0ff),
	rgb_t(0x000,0x000,0x0ff),
	rgb_t(0x0ff,0x0ff,0x000),
	rgb_t(0x000,0x0ff,0x000),
	rgb_t(0x0ff,0x000,0x000),
	rgb_t(0x000,0x000,0x000)
};

PALETTE_INIT_MEMBER(accomm_state, accomm)
{
	palette.set_pen_colors(0, electron_palette, ARRAY_LENGTH(electron_palette));
}

READ8_MEMBER(accomm_state::read_keyboard)
{
	uint8_t data = 0;

	//printf( "keyboard read @ %x\n", offset );
	for (int i = 0; i < 14; i++)
	{
		if (!(offset & 1))
			data |= m_keybd[i]->read() & 0x0f;

		offset = offset >> 1;
	}
	//logerror( ", data: %02x\n", data );
	return data;
}

READ8_MEMBER(accomm_state::read_keyboard2)
{
	uint8_t data = 0;

	//printf( "keyboard read @ %x\n", offset );
	for (int i = 0; i < 14; i++)
	{
		if (!(offset & 1))
			data |= m_keybd2[i]->read() & 0x0f;

		offset = offset >> 1;
	}
	//logerror( ", data: %02x\n", data );
	return data;
}

INTERRUPT_GEN_MEMBER(accomm_state::vbl_int)
{
	interrupt_handler( INT_SET, INT_DISPLAY_END );
}

void accomm_state::machine_reset()
{
	m_ula.communication_mode = 0x04;
	m_ula.screen_mode = 0;
	m_ula.cassette_motor_mode = 0;
	m_ula.capslock_mode = 0;
	m_ula.screen_start = 0x3000;
	m_ula.screen_base = 0x3000;
	m_ula.screen_size = 0x8000 - 0x3000;
	m_ula.screen_addr = 0;
	m_ula.tape_running = 0;
	m_ula.interrupt_status = 0x82;
	m_ula.interrupt_control = 0;
	m_ula.vram = (uint8_t *)m_vram.target() + m_ula.screen_base;
}

void accomm_state::machine_start()
{
	m_ula.interrupt_status = 0x82;
	m_ula.interrupt_control = 0x00;
}

void accomm_state::video_start()
{
	int i;
	for( i = 0; i < 256; i++ ) {
		m_map4[i] = ( ( i & 0x10 ) >> 3 ) | ( i & 0x01 );
		m_map16[i] = ( ( i & 0x40 ) >> 3 ) | ( ( i & 0x10 ) >> 2 ) | ( ( i & 0x04 ) >> 1 ) | ( i & 0x01 );
	}
}

WRITE8_MEMBER(accomm_state::b0_rom_disable_w)
{
	m_bank0dev->set_bank(1);
}

inline uint8_t accomm_state::read_vram(uint16_t addr)
{
	return m_ula.vram[ addr % m_ula.screen_size ];
}

inline void accomm_state::plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color)
{
	bitmap.pix16(y, x) = (uint16_t)color;
}

uint32_t accomm_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int i;
	int x = 0;
	int pal[16];
	int scanline = screen.vpos();
	rectangle r = cliprect;
	r.min_y = r.max_y = scanline;

	if (scanline == 0)
	{
		m_ula.screen_addr = m_ula.screen_start - m_ula.screen_base;
	}

	/* set up palette */
	switch( m_ula.screen_mode )
	{
	case 0: case 3: case 4: case 6: case 7: /* 2 colour mode */
		pal[0] = m_ula.current_pal[0];
		pal[1] = m_ula.current_pal[8];
		break;
	case 1: case 5: /* 4 colour mode */
		pal[0] = m_ula.current_pal[0];
		pal[1] = m_ula.current_pal[2];
		pal[2] = m_ula.current_pal[8];
		pal[3] = m_ula.current_pal[10];
		break;
	case 2: /* 16 colour mode */
		for( i = 0; i < 16; i++ )
			pal[i] = m_ula.current_pal[i];
	}

	/* draw line */
	switch( m_ula.screen_mode )
	{
	case 0:
		for( i = 0; i < 80; i++ )
		{
			uint8_t pattern = read_vram( m_ula.screen_addr + (i << 3) );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>7)& 1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>6)& 1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>5)& 1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>4)& 1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>3)& 1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>2)& 1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>1)& 1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>0)& 1] );
		}
		m_ula.screen_addr++;
		if ( ( scanline & 0x07 ) == 7 )
			m_ula.screen_addr += 0x278;
		break;

	case 1:
		for( i = 0; i < 80; i++ )
		{
			uint8_t pattern = read_vram( m_ula.screen_addr + i * 8 );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>3]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>3]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>2]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>2]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>1]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>1]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>0]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>0]] );
		}
		m_ula.screen_addr++;
		if ( ( scanline & 0x07 ) == 7 )
			m_ula.screen_addr += 0x278;
		break;

	case 2:
		for( i = 0; i < 80; i++ )
		{
			uint8_t pattern = read_vram( m_ula.screen_addr + i * 8 );
			plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>1]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>1]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>1]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>1]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>0]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>0]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>0]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map16[pattern>>0]] );
		}
		m_ula.screen_addr++;
		if ( ( scanline & 0x07 ) == 7 )
			m_ula.screen_addr += 0x278;
		break;

	case 3:
		if ( ( scanline > 249 ) || ( scanline % 10 >= 8 ) )
			bitmap.fill(7, r );
		else
		{
			for( i = 0; i < 80; i++ )
			{
				uint8_t pattern = read_vram( m_ula.screen_addr + i * 8 );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>7)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>6)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>5)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>4)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>3)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>2)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>1)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>0)&1] );
			}
			m_ula.screen_addr++;
		}
		if ( scanline % 10 == 9 )
			m_ula.screen_addr += 0x278;
		break;

	case 4:
	case 7:
		for( i = 0; i < 40; i++ )
		{
			uint8_t pattern = read_vram( m_ula.screen_addr + i * 8 );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>7)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>7)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>6)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>6)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>5)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>5)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>4)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>4)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>3)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>3)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>2)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>2)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>1)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>1)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>0)&1] );
			plot_pixel( bitmap, x++, scanline, pal[(pattern>>0)&1] );
		}
		m_ula.screen_addr++;
		if ( ( scanline & 0x07 ) == 7 )
			m_ula.screen_addr += 0x138;
		break;

	case 5:
		for( i = 0; i < 40; i++ )
		{
			uint8_t pattern = read_vram( m_ula.screen_addr + i * 8 );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>3]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>3]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>3]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>3]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>2]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>2]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>2]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>2]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>1]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>1]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>1]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>1]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>0]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>0]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>0]] );
			plot_pixel( bitmap, x++, scanline, pal[m_map4[pattern>>0]] );
		}
		m_ula.screen_addr++;
		if ( ( scanline & 0x07 ) == 7 )
			m_ula.screen_addr += 0x138;
		break;

	case 6:
		if ( ( scanline > 249 ) || ( scanline % 10 >= 8 ) )
			bitmap.fill(7, r );
		else
		{
			for( i = 0; i < 40; i++ )
			{
				uint8_t pattern = read_vram( m_ula.screen_addr + i * 8 );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>7)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>7)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>6)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>6)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>5)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>5)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>4)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>4)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>3)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>3)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>2)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>2)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>1)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>1)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>0)&1] );
				plot_pixel( bitmap, x++, scanline, pal[(pattern>>0)&1] );
			}
			m_ula.screen_addr++;
			if ( ( scanline % 10 ) == 7 )
				m_ula.screen_addr += 0x138;
		}
		break;
	}

	return 0;
}


READ8_MEMBER(accomm_state::sheila_r)
{
	uint8_t data = 0;
	switch ( offset & 0x0f )
	{
	case 0x00:  /* Interrupt status */
		data = m_ula.interrupt_status;
		m_ula.interrupt_status &= ~0x02;
		break;
	case 0x01:  /* Unknown */
		break;
	case 0x04:  /* Cassette data shift register */
		interrupt_handler(INT_CLEAR, INT_RECEIVE_FULL );
		data = m_ula.tape_byte;
		break;
	}
	logerror( "ULA: read offset %02x: %02x\n", offset, data );
	return data;
}

static const int palette_offset[4] = { 0, 4, 5, 1 };
static const uint16_t screen_base[8] = { 0x3000, 0x3000, 0x3000, 0x4000, 0x5800, 0x5800, 0x6000, 0x5800 };

WRITE8_MEMBER(accomm_state::sheila_w)
{
	int i = palette_offset[(( offset >> 1 ) & 0x03)];
	logerror( "ULA: write offset %02x <- %02x\n", offset & 0x0f, data );
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
		m_ula.screen_start = ( m_ula.screen_start & 0x1c0 ) | ( ( data & 0x3f ) << 9 );
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
		}
		if ( data & 0x10 )
		{
			interrupt_handler( INT_CLEAR, INT_DISPLAY_END );
		}
		if ( data & 0x20 )
		{
			interrupt_handler( INT_CLEAR, INT_RTC );
		}
		if ( data & 0x40 )
		{
			interrupt_handler( INT_CLEAR, INT_HIGH_TONE );
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
			//m_beeper->set_clock( 1000000 / ( 32 * ( data + 1 ) ) );
		}
		break;
	case 0x07:  /* Misc. */
		m_ula.communication_mode = ( data >> 1 ) & 0x03;
		switch( m_ula.communication_mode )
		{
		case 0x00:  /* cassette input */
			//m_beeper->set_state( 0 );
			//_start();
			break;
		case 0x01:  /* sound generation */
			//m_beeper->set_state( 1 );
			//electron_tape_stop();
			break;
		case 0x02:  /* cassette output */
			//m_beeper->set_state( 0 );
			//electron_tape_stop();
			break;
		case 0x03:  /* not used */
			//m_beeper->set_state( 0 );
			//electron_tape_stop();
			break;
		}
		m_ula.screen_mode = ( data >> 3 ) & 0x07;
		m_ula.screen_base = screen_base[ m_ula.screen_mode ];
		m_ula.screen_size = 0x8000 - m_ula.screen_base;
		m_ula.vram = (uint8_t *)m_vram.target() + m_ula.screen_base;
		logerror( "ULA: screen mode set to %d\n", m_ula.screen_mode );
		m_ula.cassette_motor_mode = ( data >> 6 ) & 0x01;
		//m_cassette->change_state(m_ula.cassette_motor_mode ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MOTOR_DISABLED );
		m_ula.capslock_mode = ( data >> 7 ) & 0x01;
		break;
	case 0x08: case 0x0A: case 0x0C: case 0x0E:
		// video_update
		m_ula.current_pal[i+10] = (m_ula.current_pal[i+10] & 0x01) | (((data & 0x80) >> 5) | ((data & 0x08) >> 1));
		m_ula.current_pal[i+8] = (m_ula.current_pal[i+8] & 0x01) | (((data & 0x40) >> 4) | ((data & 0x04) >> 1));
		m_ula.current_pal[i+2] = (m_ula.current_pal[i+2] & 0x03) | ((data & 0x20) >> 3);
		m_ula.current_pal[i] = (m_ula.current_pal[
		i] & 0x03) | ((data & 0x10) >> 2);
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

void accomm_state::interrupt_handler(int mode, int interrupt)
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

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, accomm_state )
	AM_RANGE(0x000000, 0x007fff) AM_RAM
	AM_RANGE(0x008000, 0x00ffff) AM_DEVICE("bank0dev", address_map_bank_device, amap8)
	AM_RANGE(0x010000, 0x08ffff) AM_RAM // "576K RAM"

	AM_RANGE(0x440000, 0x440000) AM_WRITE(b0_rom_disable_w)
	AM_RANGE(0x450000, 0x457fff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0x458000, 0x459fff) AM_READ(read_keyboard)
	AM_RANGE(0x45a000, 0x45bfff) AM_READ(read_keyboard2)
	AM_RANGE(0x45fe00, 0x45feff) AM_READWRITE(sheila_r, sheila_w)
	AM_RANGE(0x460000, 0x467fff) AM_RAM // nvram?
	AM_RANGE(0xfc0000, 0xffffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( b0dev_map, AS_PROGRAM, 8, accomm_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("maincpu", 0x38000)
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( accomm )
	PORT_START("LINE.0")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	// 04 = underscore
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("LINE.1")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("LINE.2")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("LINE.3") // unk, unk, unk, help
	
	PORT_START("LINE.4") // question, slash, question, modifier? (CTRL?)

	PORT_START("LINE.5") // X, W, C, 3
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("LINE.6") // shift or capslock, unk, 1, Z
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("LINE.7")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE.8") // 9, 6, =, 3
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('=') PORT_CHAR('+')

	PORT_START("LINE.9") // backspace, unk, down arrow, unk
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_DOWN)      PORT_CODE(KEYCODE_DOWN)     PORT_CHAR(10)

	PORT_START("LINE.10") // unk, KP4, KP0, unk

	PORT_START("LINE.11")

	PORT_START("LINE.12")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1) PORT_NAME("F1")
	
	PORT_START("LINE.13")

	PORT_START("LINE2.0")	// colon, caret, open bracket, semicolon


	PORT_START("LINE2.1")	// L, 0, O, K
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR('@')
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("LINE2.2")	// G, 6, T, F
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')

	PORT_START("LINE2.3")	// unk, unk, unk, help?


	PORT_START("LINE2.4")	// unk, unk, unk, unk

	
	PORT_START("LINE2.5")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')

	PORT_START("LINE2.6")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_NAME("Caps Lock")
	
	PORT_START("LINE2.7")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')

	PORT_START("LINE2.8")	// 

	PORT_START("LINE2.9")	// right, unk, up, left?
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_RIGHT)     PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_UP)        PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME(UTF8_LEFT)      PORT_CODE(KEYCODE_LEFT)

	PORT_START("LINE2.10")	// 8, 2, 5, 7


	PORT_START("LINE2.11")

	
	PORT_START("LINE2.12")


	PORT_START("LINE2.13")	// module back, space, calculator, enter
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("RETURN") PORT_CODE(KEYCODE_ENTER)     PORT_CHAR(13)

	PORT_START("BRK")       /* BREAK */
	//PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("BREAK") PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12)) PORT_CHANGED_MEMBER(DEVICE_SELF, accomm_state, trigger_reset, 0)
INPUT_PORTS_END

static MACHINE_CONFIG_START( accomm )
	MCFG_CPU_ADD("maincpu", G65816, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", accomm_state, vbl_int)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 50.08 )
	MCFG_SCREEN_SIZE( 640, 312 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 640-1, 0, 256-1 )
	MCFG_SCREEN_UPDATE_DRIVER(accomm_state, screen_update)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_SCANLINE)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD( "palette", 16 )
	MCFG_PALETTE_INIT_OWNER(accomm_state, accomm)

	MCFG_DEVICE_ADD("bank0dev", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(b0dev_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x8000)

	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END

ROM_START(accomm)
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_LOAD( "romv100-3.rom", 0x000000, 0x010000, CRC(bd87a157) SHA1(b9b9ed1aab9ffef2de988b2cfeac293afa11448a) )
	ROM_LOAD( "romv100-2.rom", 0x010000, 0x010000, CRC(3438adee) SHA1(cd9d5522d9430cb2e1936210b77d2edd280f9419) )
	ROM_LOAD( "romv100-1.rom", 0x020000, 0x010000, CRC(adc6a073) SHA1(3e87f21fafc1d69f33c5b541a20a98e82aacbfab) )
	ROM_LOAD( "romv100-0.rom", 0x030000, 0x010000, CRC(6d22950d) SHA1(d4cbdccf8d2bc836fb81182b2ed344d7134fe5c9) )
ROM_END

GAME( 1986,  accomm,  0,  accomm,  accomm, accomm_state,  0,  ROT0,  "Acorn", "Acorn Communicator", MACHINE_NOT_WORKING )
