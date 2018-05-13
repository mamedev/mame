// license:BSD-3-Clause
// copyright-holders:R. Belmont, Wilbert Pol
/***************************************************************************

    Acorn Communicator

    Driver-in-progress by R. Belmont
    Electron ULA emulation by Wilbert Pol

    Main CPU: 65C816
    Other chips: 6850 UART, 6522 VIA, SAA5240(video?), AM7910 modem, PCF0335(?), PCF8573P

****************************************************************************/

#include "emu.h"
#include "cpu/g65816/g65816.h"
#include "machine/6522via.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/mc6854.h"
#include "machine/ram.h"
#include "machine/nvram.h"
#include "machine/bankdev.h"
#include "sound/beep.h"
#include "bus/econet/econet.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "screen.h"
#include "speaker.h"

#include "accomm.lh"

/* Interrupts */
#define INT_HIGH_TONE       0x40
#define INT_TRANSMIT_EMPTY  0x20
#define INT_RECEIVE_FULL    0x10
#define INT_RTC             0x08
#define INT_DISPLAY_END     0x04
#define INT_SET             0x100
#define INT_CLEAR           0x200

class accomm_state : public driver_device
{
public:
	accomm_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_beeper(*this, "beeper"),
		m_ram(*this, RAM_TAG),
		m_via(*this, "via6522"),
		m_acia(*this, "acia"),
		m_acia_clock(*this, "acia_clock"),
		m_adlc(*this, "mc6854"),
		m_vram(*this, "vram"),
		m_keybd1(*this, "LINE1.%u", 0),
		m_keybd2(*this, "LINE2.%u", 0),
		m_ch00rom_enabled(true)
	{ }

	void accomm(machine_config &config);

protected:
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_WRITE8_MEMBER(ch00switch_w);
	DECLARE_READ8_MEMBER(read_keyboard1);
	DECLARE_READ8_MEMBER(read_keyboard2);
	DECLARE_READ8_MEMBER(ram_r);
	DECLARE_WRITE8_MEMBER(ram_w);
	DECLARE_READ8_MEMBER(sheila_r);
	DECLARE_WRITE8_MEMBER(sheila_w);
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);
	DECLARE_WRITE_LINE_MEMBER(econet_clk_w);

	DECLARE_PALETTE_INIT(accomm);
	INTERRUPT_GEN_MEMBER(vbl_int);

	virtual void machine_reset() override;
	virtual void machine_start() override;
	void main_map(address_map &map);

	// devices
	required_device<g65816_device> m_maincpu;
	required_device<beep_device> m_beeper;
	required_device<ram_device> m_ram;
	required_device<via6522_device> m_via;
	required_device<acia6850_device> m_acia;
	required_device<clock_device> m_acia_clock;
	required_device<mc6854_device> m_adlc;
	required_shared_ptr<uint8_t> m_vram;
	required_ioport_array<14> m_keybd1, m_keybd2;

	// driver_device overrides
	virtual void video_start() override;

	void interrupt_handler(int mode, int interrupt);
	inline uint8_t read_vram( uint16_t addr );
	inline void plot_pixel(bitmap_ind16 &bitmap, int x, int y, uint32_t color);

private:
	bool m_ch00rom_enabled;

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
		int shiftlock_mode;
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

READ8_MEMBER(accomm_state::read_keyboard1)
{
	uint8_t data = 0;

	for (int i = 0; i < 14; i++)
	{
		if (!(offset & 1))
			data |= m_keybd1[i]->read() & 0x0f;

		offset = offset >> 1;
	}
	return data;
}

READ8_MEMBER(accomm_state::read_keyboard2)
{
	uint8_t data = 0;

	for (int i = 0; i < 14; i++)
	{
		if (!(offset & 1))
			data |= m_keybd2[i]->read() & 0x0f;

		offset = offset >> 1;
	}
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
	m_ula.shiftlock_mode = 0;
	m_ula.capslock_mode = 0;
	m_ula.screen_start = 0x3000;
	m_ula.screen_base = 0x3000;
	m_ula.screen_size = 0x8000 - 0x3000;
	m_ula.screen_addr = 0;
	m_ula.tape_running = 0;
	m_ula.interrupt_status = 0x82;
	m_ula.interrupt_control = 0;
	m_ula.vram = (uint8_t *)m_vram.target() + m_ula.screen_base;

	m_ch00rom_enabled = true;
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

WRITE8_MEMBER(accomm_state::ch00switch_w)
{
	if (!machine().side_effects_disabled())
		m_ch00rom_enabled = false;
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


READ8_MEMBER(accomm_state::ram_r)
{
	uint8_t data = 0xff;

	if (m_ch00rom_enabled && (offset < 0x10000))
	{
		data = memregion("maincpu")->base()[0x30000 + offset];
	}
	else
	{
		switch (m_ram->size())
		{
		case 512 * 1024:
			data = m_ram->pointer()[offset & 0x7ffff];
			break;

		case 1024 * 1024:
			data = m_ram->pointer()[offset & 0xfffff];
			break;
		}
	}
	return data;
}

WRITE8_MEMBER(accomm_state::ram_w)
{
	switch (m_ram->size())
	{
	case 512 * 1024:
		m_ram->pointer()[offset & 0x7ffff] = data;
		break;

	case 1024 * 1024:
		m_ram->pointer()[offset & 0xfffff] = data;
		break;
	}
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
		 * the Electron Advanced Users manual says this is the correct algorithm
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
			break;
		case 0x01:  /* sound generation */
			m_beeper->set_state( 1 );
			break;
		case 0x02:  /* cassette output */
			m_beeper->set_state( 0 );
			break;
		case 0x03:  /* not used */
			m_beeper->set_state( 0 );
			break;
		}
		m_ula.screen_mode = ( data >> 3 ) & 0x07;
		m_ula.screen_base = screen_base[ m_ula.screen_mode ];
		m_ula.screen_size = 0x8000 - m_ula.screen_base;
		m_ula.vram = (uint8_t *)m_vram.target() + m_ula.screen_base;
		logerror( "ULA: screen mode set to %d\n", m_ula.screen_mode );
		m_ula.shiftlock_mode = !BIT(data, 6);
		output().set_value("shiftlock_led", m_ula.shiftlock_mode);
		m_ula.capslock_mode = BIT(data, 7);
		output().set_value("capslock_led", m_ula.capslock_mode);
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

WRITE_LINE_MEMBER(accomm_state::write_acia_clock)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

WRITE_LINE_MEMBER(accomm_state::econet_clk_w)
{
	m_adlc->rxc_w(state);
	m_adlc->txc_w(state);
}

void accomm_state::main_map(address_map &map)
{
	map(0x000000, 0x1fffff).rw(this, FUNC(accomm_state::ram_r), FUNC(accomm_state::ram_w));                                       /* System RAM */
	map(0x200000, 0x3fffff).noprw();                                                           /* External expansion RAM */
	map(0x400000, 0x400000).noprw();                                                           /* MODEM */
	map(0x410000, 0x410000).ram();                                                           /* Econet ID */
	map(0x420000, 0x42000f).rw(m_via, FUNC(via6522_device::read), FUNC(via6522_device::write));          /* 6522 VIA (printer etc) */
	map(0x430000, 0x430001).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));            /* 2641 ACIA (RS423) */
	map(0x440000, 0x440000).w(this, FUNC(accomm_state::ch00switch_w));                                           /* CH00SWITCH */
	map(0x450000, 0x457fff).ram().share("vram");                                          /* Video RAM */
	map(0x458000, 0x459fff).r(this, FUNC(accomm_state::read_keyboard1));                                          /* Video ULA */
	map(0x45a000, 0x45bfff).r(this, FUNC(accomm_state::read_keyboard2));                                          /* Video ULA */
	map(0x45fe00, 0x45feff).rw(this, FUNC(accomm_state::sheila_r), FUNC(accomm_state::sheila_w));                                 /* Video ULA */
	map(0x460000, 0x467fff).ram().share("nvram");                                         /* CMOS RAM */
	map(0x470000, 0x47001f).rw(m_adlc, FUNC(mc6854_device::read), FUNC(mc6854_device::write));            /* 68B54 (Econet) */
	map(0x480000, 0x7fffff).noprw();                                                           /* Reserved */
	map(0x800000, 0xbfffff).noprw();                                                           /* External expansion IO  */
	map(0xc00000, 0xf7ffff).noprw();                                                           /* External expansion ROM */
	map(0xf80000, 0xf9ffff).noprw();                                                           /* Empty */
	map(0xfa0000, 0xfbffff).noprw();                                                           /* ROM bank 4 */
	map(0xfc0000, 0xfcffff).rom().region("maincpu", 0x000000);                            /* ROM bank 3 */
	map(0xfd0000, 0xfdffff).rom().region("maincpu", 0x010000);                            /* ROM bank 2 */
	map(0xfe0000, 0xfeffff).rom().region("maincpu", 0x020000);                            /* ROM bank 0 */
	map(0xff0000, 0xffffff).rom().region("maincpu", 0x030000);                            /* ROM bank 1 */
}

static INPUT_PORTS_START( accomm )
	PORT_START("LINE1.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)      PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)          PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)  PORT_CHAR('_') PORT_CHAR('@')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS)      PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("LINE1.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)      PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)          PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)       PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9)          PORT_CHAR('9') PORT_CHAR(')')

	PORT_START("LINE1.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)          PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)          PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)          PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5)          PORT_CHAR('5') PORT_CHAR('%')

	PORT_START("LINE1.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TILDE)      PORT_CHAR(UCHAR_MAMEKEY(TILDE))     PORT_NAME("Function")

	PORT_START("LINE1.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RSHIFT)     PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("LINE1.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)          PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)          PORT_CHAR('w') PORT_CHAR('W')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)          PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3)          PORT_CHAR('3') PORT_CHAR('#')

	PORT_START("LINE1.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LALT)       PORT_CHAR(UCHAR_MAMEKEY(LALT))      PORT_NAME("Shift Lock")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_TAB)        PORT_CHAR(9)                        PORT_NAME("Tab")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)          PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1)          PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("LINE1.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)          PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)          PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)          PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7)          PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("LINE1.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_9_PAD)      PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6_PAD)      PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER_PAD)  PORT_CHAR(UCHAR_MAMEKEY(EQUALS_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_3_PAD)      PORT_CHAR(UCHAR_MAMEKEY(3_PAD))

	PORT_START("LINE1.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)        PORT_CHAR(UCHAR_MAMEKEY(BACKSPACE)) PORT_NAME("Del CE")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)        PORT_CHAR(UCHAR_MAMEKEY(END))       PORT_NAME("Copy EE")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)       PORT_CHAR(10)                       PORT_NAME(UTF8_DOWN" +")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)       PORT_CHAR(UCHAR_MAMEKEY(HOME))      PORT_NAME("Home %")

	PORT_START("LINE1.10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL_PAD)    PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))   PORT_NAME("Keypad .")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4_PAD)      PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0_PAD)      PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_1_PAD)      PORT_CHAR(UCHAR_MAMEKEY(1_PAD))

	PORT_START("LINE1.11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']')  PORT_CHAR('}')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)        PORT_CHAR(UCHAR_MAMEKEY(F10))       PORT_NAME("Phone")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)  PORT_CHAR(UCHAR_MAMEKEY(ESC))       PORT_NAME("Escape")

	PORT_START("LINE1.12")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)         PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)         PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)         PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)         PORT_CHAR(UCHAR_MAMEKEY(F1))

	PORT_START("LINE1.13")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE2.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)      PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)     PORT_CHAR('^') PORT_CHAR('~')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)  PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_COLON)      PORT_CHAR(';') PORT_CHAR('+')

	PORT_START("LINE2.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)          PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_0)          PORT_CHAR('0') PORT_CHAR(0xa3)   // £
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)          PORT_CHAR('o') PORT_CHAR('O')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)          PORT_CHAR('k') PORT_CHAR('K')

	PORT_START("LINE2.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)          PORT_CHAR('g') PORT_CHAR('G')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_6)          PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)          PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)          PORT_CHAR('f') PORT_CHAR('F')

	PORT_START("LINE2.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ESC)        PORT_NAME("Help")

	PORT_START("LINE2.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LCONTROL)   PORT_CHAR(UCHAR_MAMEKEY(LCONTROL)) PORT_NAME("Ctrl")

	PORT_START("LINE2.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)          PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_4)          PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)          PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)          PORT_CHAR('s') PORT_CHAR('S')

	PORT_START("LINE2.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)          PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2)          PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)          PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_CAPSLOCK)   PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK))

	PORT_START("LINE2.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)          PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8)          PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)          PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)          PORT_CHAR('h') PORT_CHAR('H')

	PORT_START("LINE2.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LINE2.9")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)      PORT_CHAR(UCHAR_MAMEKEY(RIGHT))    PORT_NAME(UTF8_RIGHT" -")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_INSERT)     PORT_CHAR(UCHAR_MAMEKEY(INSERT))   PORT_NAME("Insert " UTF8_DIVIDE)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)         PORT_CHAR(UCHAR_MAMEKEY(UP))       PORT_NAME(UTF8_UP" " UTF8_MULTIPLY)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)       PORT_CHAR(UCHAR_MAMEKEY(LEFT))     PORT_NAME(UTF8_LEFT" AC")

	PORT_START("LINE2.10")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_8_PAD)      PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_2_PAD)      PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_5_PAD)      PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_7_PAD)      PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("LINE2.11")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)         PORT_CHAR(UCHAR_MAMEKEY(F9))       PORT_NAME("Stop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)      PORT_CHAR(' ')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F12)        PORT_CHAR(UCHAR_MAMEKEY(F12))      PORT_NAME("Calc")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)      PORT_CHAR(13)

	PORT_START("LINE2.12")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)         PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)         PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)         PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)         PORT_CHAR(UCHAR_MAMEKEY(F3))

	PORT_START("LINE2.13")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END

MACHINE_CONFIG_START(accomm_state::accomm)
	MCFG_DEVICE_ADD("maincpu", G65816, 16_MHz_XTAL / 8)
	MCFG_DEVICE_PROGRAM_MAP(main_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", accomm_state, vbl_int)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 50.08 )
	MCFG_SCREEN_SIZE( 640, 312 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 640-1, 0, 256-1 )
	MCFG_SCREEN_UPDATE_DRIVER(accomm_state, screen_update)
	MCFG_SCREEN_VIDEO_ATTRIBUTES(VIDEO_UPDATE_SCANLINE)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD( "palette", 16 )
	MCFG_PALETTE_INIT_OWNER(accomm_state, accomm)

	MCFG_DEFAULT_LAYOUT(layout_accomm)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("512K")
	MCFG_RAM_EXTRA_OPTIONS("1M")

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* sound */
	SPEAKER(config, "mono").front_center();
	MCFG_DEVICE_ADD("beeper", BEEP, 300)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	/* rtc pcf8573 */

	/* via */
	MCFG_DEVICE_ADD("via6522", VIA6522, XTAL(16'000'000) / 16)
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8("cent_data_out", output_latch_device, write))
	MCFG_VIA6522_CA2_HANDLER(WRITELINE("centronics", centronics_device, write_strobe))

	/* acia */
	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE("serial", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(WRITELINE("serial", rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(INPUTLINE("maincpu", G65816_LINE_IRQ))

	MCFG_DEVICE_ADD("serial", RS232_PORT, default_rs232_devices, nullptr)
	MCFG_RS232_RXD_HANDLER(WRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_DCD_HANDLER(WRITELINE("acia", acia6850_device, write_dcd))
	MCFG_RS232_CTS_HANDLER(WRITELINE("acia", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, XTAL(16'000'000) / 13)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(*this, accomm_state, write_acia_clock))

	/* econet */
	MCFG_DEVICE_ADD("mc6854", MC6854, 0)
	MCFG_MC6854_OUT_TXD_CB(WRITELINE(ECONET_TAG, econet_device, data_w))
	MCFG_MC6854_OUT_IRQ_CB(INPUTLINE("maincpu", G65816_LINE_NMI))
	MCFG_ECONET_ADD()
	MCFG_ECONET_CLK_CALLBACK(WRITELINE(*this, accomm_state, econet_clk_w))
	MCFG_ECONET_DATA_CALLBACK(WRITELINE("mc6854", mc6854_device, set_rx))
	MCFG_ECONET_SLOT_ADD("econet254", 254, econet_devices, nullptr)

	/* printer */
	MCFG_CENTRONICS_ADD("centronics", centronics_devices, "printer")
	MCFG_CENTRONICS_ACK_HANDLER(WRITELINE("via6522", via6522_device, write_ca1))
	MCFG_CENTRONICS_OUTPUT_LATCH_ADD("cent_data_out", "centronics")
MACHINE_CONFIG_END

ROM_START(accomm)
	ROM_REGION(0x40000, "maincpu", 0)
	ROM_DEFAULT_BIOS("100")
	ROM_SYSTEM_BIOS(0, "100", "1.00 13/Nov/86") /* Version 1.00 13/Nov/86 (C)1986 */
	ROMX_LOAD( "romv100-3.rom", 0x000000, 0x010000, CRC(bd87a157) SHA1(b9b9ed1aab9ffef2de988b2cfeac293afa11448a), ROM_BIOS(1) )
	ROMX_LOAD( "romv100-2.rom", 0x010000, 0x010000, CRC(3438adee) SHA1(cd9d5522d9430cb2e1936210b77d2edd280f9419), ROM_BIOS(1) )
	ROMX_LOAD( "romv100-1.rom", 0x020000, 0x010000, CRC(adc6a073) SHA1(3e87f21fafc1d69f33c5b541a20a98e82aacbfab), ROM_BIOS(1) )
	ROMX_LOAD( "romv100-0.rom", 0x030000, 0x010000, CRC(6d22950d) SHA1(d4cbdccf8d2bc836fb81182b2ed344d7134fe5c9), ROM_BIOS(1) )
	/* Version 1.70 04/Jun/87 (C)1987 */
	/* Versone 3.00 13/gen/88 (C)1988 */
ROM_END

COMP( 1986, accomm, 0, 0, accomm, accomm, accomm_state, empty_init, "Acorn", "Acorn Communicator", MACHINE_NOT_WORKING )
