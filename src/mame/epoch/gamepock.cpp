// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/******************************************************************************

Epoch Game Pocket Computer
Japanese LCD handheld console

Hardware notes:
- NEC uPD78C06AG
- x

TODO:
- use hd44102_device

******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/upd7810/upd7810.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace {

class gamepock_state : public driver_device
{
public:
	gamepock_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_speaker(*this, "speaker")
	{ }

	void gamepock(machine_config &config);

private:
	struct HD44102CH {
		uint8_t   enabled = 0U;
		uint8_t   start_page = 0U;
		uint8_t   address = 0U;
		uint8_t   y_inc = 0U;
		uint8_t   ram[256]{};   // There are actually 50 x 4 x 8 bits. This just makes addressing easier.
	};

	virtual void machine_reset() override;

	void hd44102ch_w(int which, int c_d, uint8_t data);
	void hd44102ch_init(int which);
	void lcd_update();

	void port_a_w(uint8_t data);
	uint8_t port_b_r();
	void port_b_w(uint8_t data);
	uint8_t port_c_r();
	uint32_t screen_update_gamepock(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gamepock_mem(address_map &map);

	uint8_t m_port_a = 0U;
	uint8_t m_port_b = 0U;
	HD44102CH m_hd44102ch[3];

	required_device<upd78c06_device> m_maincpu;
	required_device<speaker_sound_device> m_speaker;
};


void gamepock_state::hd44102ch_w( int which, int c_d, uint8_t data )
{
	if ( c_d )
	{
		uint8_t   y;
		/* Data */
		m_hd44102ch[which].ram[ m_hd44102ch[which].address ] = data;

		/* Increment/decrement Y counter */
		y = ( m_hd44102ch[which].address & 0x3F ) + m_hd44102ch[which].y_inc;
		if ( y == 0xFF )
		{
			y = 49;
		}
		if ( y == 50 )
		{
			y = 0;
		}
		m_hd44102ch[which].address = ( m_hd44102ch[which].address & 0xC0 ) | y;
	}
	else
	{
		/* Command */
		switch ( data )
		{
		case 0x38:      /* Display off */
			m_hd44102ch[which].enabled = 0;
			break;
		case 0x39:      /* Display on */
			m_hd44102ch[which].enabled = 1;
			break;
		case 0x3A:      /* Y decrement mode */
			m_hd44102ch[which].y_inc = 0xFF;
			break;
		case 0x3B:      /* Y increment mode */
			m_hd44102ch[which].y_inc = 0x01;
			break;
		case 0x3E:      /* Display start page #0 */
		case 0x7E:      /* Display start page #1 */
		case 0xBE:      /* Display start page #2 */
		case 0xFE:      /* Display start page #3 */
			m_hd44102ch[which].start_page = data & 0xC0;
			break;
		default:
			if ( ( data & 0x3F ) < 50 )
			{
				m_hd44102ch[which].address = data;
			}
			break;
		}
	}
}


void gamepock_state::hd44102ch_init( int which )
{
	memset( &m_hd44102ch[which], 0, sizeof( HD44102CH ) );
	m_hd44102ch[which].y_inc = 0x01;
}


void gamepock_state::lcd_update()
{
	/* Check whether HD44102CH #1 is enabled */
	if ( m_port_a & 0x08 )
	{
		hd44102ch_w( 0, m_port_a & 0x04, m_port_b );
	}

	/* Check whether HD44102CH #2 is enabled */
	if ( m_port_a & 0x10 )
	{
		hd44102ch_w( 1, m_port_a & 0x04, m_port_b );
	}

	/* Check whether HD44102CH #3 is enabled */
	if ( m_port_a & 0x20 )
	{
		hd44102ch_w( 2, m_port_a & 0x04, m_port_b );
	}
}


void gamepock_state::port_a_w(uint8_t data)
{
	uint8_t   old_port_a = m_port_a;

	m_port_a = data;

	if ( ! ( old_port_a & 0x02 ) && ( m_port_a & 0x02 ) )
	{
		lcd_update();
	}
}


void gamepock_state::port_b_w(uint8_t data)
{
	m_port_b = data;
}


uint8_t gamepock_state::port_b_r()
{
	logerror("gamepock_port_b_r: not implemented\n");
	return 0xFF;
}


uint8_t gamepock_state::port_c_r()
{
	uint8_t   data = 0xFF;

	if ( m_port_a & 0x80 )
	{
		data &= ioport("IN0")->read();
	}

	if ( m_port_a & 0x40 )
	{
		data &= ioport("IN1")->read();
	}

	return data;
}


void gamepock_state::machine_reset()
{
	hd44102ch_init( 0 );
	hd44102ch_init( 1 );
	hd44102ch_init( 2 );
}

uint32_t gamepock_state::screen_update_gamepock(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t   ad;

	/* Handle HD44102CH #0 */
	ad = m_hd44102ch[0].start_page;
	for ( int i = 0; i < 4; i++ )
	{
		for ( int j = 0; j < 50; j++ )
		{
			bitmap.pix(i * 8 + 0, 49 - j ) = ( m_hd44102ch[0].ram[ad+j] & 0x01 ) ? 0 : 1;
			bitmap.pix(i * 8 + 1, 49 - j ) = ( m_hd44102ch[0].ram[ad+j] & 0x02 ) ? 0 : 1;
			bitmap.pix(i * 8 + 2, 49 - j ) = ( m_hd44102ch[0].ram[ad+j] & 0x04 ) ? 0 : 1;
			bitmap.pix(i * 8 + 3, 49 - j ) = ( m_hd44102ch[0].ram[ad+j] & 0x08 ) ? 0 : 1;
			bitmap.pix(i * 8 + 4, 49 - j ) = ( m_hd44102ch[0].ram[ad+j] & 0x10 ) ? 0 : 1;
			bitmap.pix(i * 8 + 5, 49 - j ) = ( m_hd44102ch[0].ram[ad+j] & 0x20 ) ? 0 : 1;
			bitmap.pix(i * 8 + 6, 49 - j ) = ( m_hd44102ch[0].ram[ad+j] & 0x40 ) ? 0 : 1;
			bitmap.pix(i * 8 + 7, 49 - j ) = ( m_hd44102ch[0].ram[ad+j] & 0x80 ) ? 0 : 1;
		}
		ad += 0x40;
	}

	/* Handle HD44102CH #1 */
	ad = m_hd44102ch[1].start_page;
	for ( int i = 4; i < 8; i++ )
	{
		for ( int j = 0; j < 50; j++ )
		{
			bitmap.pix(i * 8 + 0, j ) = ( m_hd44102ch[1].ram[ad+j] & 0x01 ) ? 0 : 1;
			bitmap.pix(i * 8 + 1, j ) = ( m_hd44102ch[1].ram[ad+j] & 0x02 ) ? 0 : 1;
			bitmap.pix(i * 8 + 2, j ) = ( m_hd44102ch[1].ram[ad+j] & 0x04 ) ? 0 : 1;
			bitmap.pix(i * 8 + 3, j ) = ( m_hd44102ch[1].ram[ad+j] & 0x08 ) ? 0 : 1;
			bitmap.pix(i * 8 + 4, j ) = ( m_hd44102ch[1].ram[ad+j] & 0x10 ) ? 0 : 1;
			bitmap.pix(i * 8 + 5, j ) = ( m_hd44102ch[1].ram[ad+j] & 0x20 ) ? 0 : 1;
			bitmap.pix(i * 8 + 6, j ) = ( m_hd44102ch[1].ram[ad+j] & 0x40 ) ? 0 : 1;
			bitmap.pix(i * 8 + 7, j ) = ( m_hd44102ch[1].ram[ad+j] & 0x80 ) ? 0 : 1;
		}
		ad += 0x40;
	}

	/* Handle HD44102CH #2 */
	ad = m_hd44102ch[2].start_page;
	for ( int i = 0; i < 4; i++ )
	{
		for ( int j = 0; j < 25; j++ )
		{
			bitmap.pix(i * 8 + 0, 50 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x01 ) ? 0 : 1;
			bitmap.pix(i * 8 + 1, 50 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x02 ) ? 0 : 1;
			bitmap.pix(i * 8 + 2, 50 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x04 ) ? 0 : 1;
			bitmap.pix(i * 8 + 3, 50 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x08 ) ? 0 : 1;
			bitmap.pix(i * 8 + 4, 50 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x10 ) ? 0 : 1;
			bitmap.pix(i * 8 + 5, 50 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x20 ) ? 0 : 1;
			bitmap.pix(i * 8 + 6, 50 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x40 ) ? 0 : 1;
			bitmap.pix(i * 8 + 7, 50 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x80 ) ? 0 : 1;
		}
		for ( int j = 25; j < 50; j++ )
		{
			bitmap.pix(32 + i * 8 + 0, 25 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x01 ) ? 0 : 1;
			bitmap.pix(32 + i * 8 + 1, 25 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x02 ) ? 0 : 1;
			bitmap.pix(32 + i * 8 + 2, 25 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x04 ) ? 0 : 1;
			bitmap.pix(32 + i * 8 + 3, 25 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x08 ) ? 0 : 1;
			bitmap.pix(32 + i * 8 + 4, 25 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x10 ) ? 0 : 1;
			bitmap.pix(32 + i * 8 + 5, 25 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x20 ) ? 0 : 1;
			bitmap.pix(32 + i * 8 + 6, 25 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x40 ) ? 0 : 1;
			bitmap.pix(32 + i * 8 + 7, 25 + j ) = ( m_hd44102ch[2].ram[ad+j] & 0x80 ) ? 0 : 1;
		}
		ad += 0x40;
	}

	return 0;
}


void gamepock_state::gamepock_mem(address_map &map)
{
	map.unmap_value_high();
	// 0x0000-0x0fff is internal ROM
	map(0x1000, 0x3fff).noprw();
	map(0x4000, 0xbfff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0xc000, 0xc7ff).mirror(0x0800).ram();
	// 0xff80-0xffff is internal RAM
}


static INPUT_PORTS_START( gamepock )
	PORT_START("IN0")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_SELECT )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON3 )

	PORT_START("IN1")
	PORT_BIT ( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT ( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT ( 0x04, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT ( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT ( 0x10, IP_ACTIVE_LOW, IPT_BUTTON4 )
INPUT_PORTS_END


void gamepock_state::gamepock(machine_config &config)
{
	UPD78C06(config, m_maincpu, 6_MHz_XTAL); // uPD78C06AG
	m_maincpu->set_addrmap(AS_PROGRAM, &gamepock_state::gamepock_mem);
	m_maincpu->pa_out_cb().set(FUNC(gamepock_state::port_a_w));
	m_maincpu->pb_in_cb().set(FUNC(gamepock_state::port_b_r));
	m_maincpu->pb_out_cb().set(FUNC(gamepock_state::port_b_w));
	m_maincpu->pc_in_cb().set(FUNC(gamepock_state::port_c_r));
	m_maincpu->to_func().set(m_speaker, FUNC(speaker_sound_device::level_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_size(75, 64);
	screen.set_visarea(0, 74, 0, 63);
	screen.set_screen_update(FUNC(gamepock_state::screen_update_gamepock));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker).add_route(ALL_OUTPUTS, "mono", 0.50);

	/* cartridge */
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "gamepock_cart");

	/* Software lists */
	SOFTWARE_LIST(config, "cart_list").set_original("gamepock");
}


ROM_START( gamepock )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD( "egpcboot.bin", 0x0000, 0x1000, CRC(ee1ea65d) SHA1(9c7731b5ead721d2cc7f7e2655c5fed9e56db8b0) )
ROM_END

} // anonymous namespace


CONS( 1984, gamepock, 0, 0, gamepock, gamepock, gamepock_state, empty_init, "Epoch", "Game Pocket Computer", 0 )
