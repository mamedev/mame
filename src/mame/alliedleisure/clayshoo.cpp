// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/***************************************************************************

    Allied Leisure Clay Shoot hardware

    driver by Zsolt Vasvari

    Games supported:
        * Clay Shoot

    Known issues:
        * no sound
        * cocktail mode, dipswitch or alternate romset?
          (cocktail set has a color overlay, upright set has a backdrop)

****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/watchdog.h"
#include "screen.h"


namespace {

class clayshoo_state : public driver_device
{
public:
	clayshoo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram")
	{ }

	void clayshoo(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_videoram;

	void analog_reset_w(uint8_t data);
	uint8_t analog_r();
	void input_port_select_w(uint8_t data);
	uint8_t input_port_r();
	uint32_t screen_update_clayshoo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(reset_analog_bit);
	uint8_t difficulty_input_port_r(int bit);
	void create_analog_timers();

	void main_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;

	emu_timer *m_analog_timer_1 = nullptr, *m_analog_timer_2 = nullptr;
	uint8_t m_input_port_select = 0;
	uint8_t m_analog_port_val = 0;
};


/*************************************
 *
 *  Digital control handling functions
 *
 *************************************/

void clayshoo_state::input_port_select_w(uint8_t data)
{
	m_input_port_select = data;
}


uint8_t clayshoo_state::difficulty_input_port_r( int bit )
{
	uint8_t ret = 0;

	/* read fake port and remap the buttons to 2 bits */
	uint8_t raw = ioport("FAKE")->read();

	if (raw & (1 << (bit + 1)))
		ret = 0x03;     /* expert */
	else if (raw & (1 << (bit + 2)))
		ret = 0x01;     /* pro */
	else
		ret = 0x00;     /* amateur otherwise */

	return ret;
}


uint8_t clayshoo_state::input_port_r()
{
	uint8_t ret = 0;

	switch (m_input_port_select)
	{
	case 0x01: ret = ioport("IN0")->read(); break;
	case 0x02: ret = ioport("IN1")->read(); break;
	case 0x04: ret = (ioport("IN2")->read() & 0xf0) | difficulty_input_port_r(0) | (difficulty_input_port_r(3) << 2); break;
	case 0x08: ret = ioport("IN3")->read(); break;
	case 0x10:
	case 0x20: break;  /* these two are not really used */
	default: logerror("Unexpected port read: %02X\n", m_input_port_select);
	}
	return ret;
}



/*************************************
 *
 *  Analog control handling functions
 *
 *************************************/

TIMER_CALLBACK_MEMBER(clayshoo_state::reset_analog_bit)
{
	m_analog_port_val &= ~param;
}


static attotime compute_duration( device_t *device, int analog_pos )
{
	/* the 58 comes from the length of the loop used to
	   read the analog position */
	return downcast<cpu_device *>(device)->cycles_to_attotime(58 * analog_pos);
}


void clayshoo_state::analog_reset_w(uint8_t data)
{
	/* reset the analog value, and start the two times that will fire
	   off in a short period proportional to the position of the
	   analog control and set the appropriate bit. */
	m_analog_port_val = 0xff;

	m_analog_timer_1->adjust(compute_duration(m_maincpu.target(), ioport("AN1")->read()), 0x02);
	m_analog_timer_2->adjust(compute_duration(m_maincpu.target(), ioport("AN2")->read()), 0x01);
}


uint8_t clayshoo_state::analog_r()
{
	return m_analog_port_val;
}


void clayshoo_state::create_analog_timers(  )
{
	m_analog_timer_1 = timer_alloc(FUNC(clayshoo_state::reset_analog_bit), this);
	m_analog_timer_2 = timer_alloc(FUNC(clayshoo_state::reset_analog_bit), this);
}



/*************************************
 *
 *  Machine setup
 *
 *************************************/

void clayshoo_state::machine_start()
{
	create_analog_timers();

	/* register for state saving */
	save_item(NAME(m_input_port_select));
	save_item(NAME(m_analog_port_val));
}



/*************************************
 *
 *  Video hardware
 *
 *************************************/

uint32_t clayshoo_state::screen_update_clayshoo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	offs_t offs;

	for (offs = 0; offs < m_videoram.bytes(); offs++)
	{
		uint8_t x = offs << 3;
		uint8_t y = ~(offs >> 5);
		uint8_t data = m_videoram[offs];

		for (int i = 0; i < 8; i++)
		{
			pen_t pen = (data & 0x80) ? rgb_t::white() : rgb_t::black();
			bitmap.pix(y, x) = pen;

			data = data << 1;
			x = x + 1;
		}
	}

	return 0;
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void clayshoo_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x4000, 0x47ff).rom();
	map(0x8000, 0x97ff).ram().share("videoram");    /* 6k of video ram according to readme */
	map(0x9800, 0xa800).nopw();      /* not really mapped, but cleared */
	map(0xc800, 0xc800).rw(FUNC(clayshoo_state::analog_r), FUNC(clayshoo_state::analog_reset_w));
}



/*************************************
 *
 *  Port handlers
 *
 *************************************/

void clayshoo_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x20, 0x23).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x30, 0x33).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
//  map(0x40, 0x43).noprw(); // 8253 for sound?
//  map(0x50, 0x50).noprw(); // ?
//  map(0x60, 0x60).noprw(); // ?
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( clayshoo )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_BIT( 0x3c, IP_ACTIVE_LOW, IPT_UNKNOWN )        /* doesn't appear to be used */
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  /* not 100% positive */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )      /* used */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x07, 0x01, "Time/Bonus 1P-2P" )
	PORT_DIPSETTING(    0x00, "60/6k-90/6k" )
	PORT_DIPSETTING(    0x01, "60/6k-120/8k" )
	PORT_DIPSETTING(    0x02, "90/9.5k-150/9.5k" )
	PORT_DIPSETTING(    0x03, "90/9.5k-190/11k" )
	PORT_DIPSETTING(    0x04, "60/8k-90/8k" )
	PORT_DIPSETTING(    0x05, "60/8k-120/10k" )
	PORT_DIPSETTING(    0x06, "90/11.5k-150/11.5k" )
	PORT_DIPSETTING(    0x07, "90/11.5k-190/13k" )
	PORT_BIT( 0xf8, IP_ACTIVE_LOW, IPT_UNKNOWN )    /* doesn't appear to be used */

	PORT_START("IN2")
	PORT_BIT( 0x03, IP_ACTIVE_LOW, IPT_CUSTOM )    /* amateur/expert/pro Player 2 */
	PORT_BIT( 0x0c, IP_ACTIVE_LOW, IPT_CUSTOM )    /* amateur/expert/pro Player 1 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("AN1")  /* IN4 - Fake analog control.  Visible in $c800 bit 1 */
	PORT_BIT( 0x0f, 0x08, IPT_AD_STICK_Y ) PORT_MINMAX(0,0x0f) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("AN2")  /* IN5 - Fake analog control.  Visible in $c800 bit 0 */
	PORT_BIT( 0x0f, 0x08, IPT_AD_STICK_Y ) PORT_MINMAX(0,0x0f) PORT_SENSITIVITY(10) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("FAKE")  /* IN6 - Fake.  Visible in IN2 bits 0-1 and 2-3 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_TOGGLE PORT_PLAYER(2) PORT_NAME("P2 Amateur Difficulty")
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_TOGGLE PORT_PLAYER(2) PORT_NAME("P2 Expert Difficulty")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_TOGGLE PORT_PLAYER(2) PORT_NAME("P2 Pro Difficulty")
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_TOGGLE PORT_PLAYER(1) PORT_NAME("P1 Amateur Difficulty")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_TOGGLE PORT_PLAYER(1) PORT_NAME("P1 Expert Difficulty")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_TOGGLE PORT_PLAYER(1) PORT_NAME("P2 Pro Difficulty")
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void clayshoo_state::machine_reset()
{
	m_input_port_select = 0;
	m_analog_port_val = 0;
}

void clayshoo_state::clayshoo(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 5068000/4);      /* 5.068/4 Mhz (divider is a guess) */
	m_maincpu->set_addrmap(AS_PROGRAM, &clayshoo_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &clayshoo_state::main_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(clayshoo_state::irq0_line_hold));

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(256, 256);
	screen.set_visarea(0, 255, 64, 255);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_screen_update(FUNC(clayshoo_state::screen_update_clayshoo));

	I8255A(config, "ppi8255_0");

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.out_pa_callback().set(FUNC(clayshoo_state::input_port_select_w));
	ppi1.in_pb_callback().set(FUNC(clayshoo_state::input_port_r));
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( clayshoo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0",      0x0000, 0x0800, CRC(9df9d9e3) SHA1(8ce71a6faf5df9c8c3dbb92a443b62c0f376491c) )
	ROM_LOAD( "1",      0x0800, 0x0800, CRC(5134a631) SHA1(f0764a5161934564fd0416be26087cf812e0c422) )
	ROM_LOAD( "2",      0x1000, 0x0800, CRC(5b5a67f6) SHA1(c97b4d44e6dc5dd0c42e04ffceed8934975fe769) )
	ROM_LOAD( "3",      0x1800, 0x0800, CRC(7eda8e44) SHA1(2974f8b06653aee2ffd96ff402707acfc059bc91) )
	ROM_LOAD( "4",      0x4000, 0x0800, CRC(3da16196) SHA1(eb0c0cf0c8fc3db05ac0c469fb20fe92ae6f27ce) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, clayshoo, 0, clayshoo, clayshoo, clayshoo_state, empty_init, ROT0, "Allied Leisure", "Clay Shoot", MACHINE_NO_SOUND | MACHINE_SUPPORTS_SAVE )
