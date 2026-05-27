// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari
/*******************************************************************************

Allied Leisure Clay Shoot hardware
driver by Zsolt Vasvari

TODO:
- missing SN76477 sound effects
- cocktail mode, dipswitch or alternate romset? (cocktail set has a color overlay,
  upright set has a backdrop)

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/pit8253.h"
#include "machine/watchdog.h"
#include "sound/dac.h"

#include "screen.h"
#include "speaker.h"


namespace {

class clayshoo_state : public driver_device
{
public:
	clayshoo_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_videoram(*this, "videoram"),
		m_ppi(*this, "ppi%u", 0),
		m_pit(*this, "pit"),
		m_dac(*this, "dac"),
		m_in(*this, "IN%u", 0),
		m_an(*this, "AN%u", 0)
	{ }

	void clayshoo(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<uint8_t> m_videoram;
	required_device_array<i8255_device, 2> m_ppi;
	required_device<pit8253_device> m_pit;
	required_device<dac_1bit_device> m_dac;
	required_ioport_array<4> m_in;
	required_ioport_array<2> m_an;

	emu_timer *m_analog_timer[2];
	uint8_t m_input_port_select = 0;
	uint8_t m_analog_port_val = 0xff;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void input_port_select_w(uint8_t data);
	uint8_t input_port_r();

	TIMER_CALLBACK_MEMBER(reset_analog_bit);
	void analog_reset_w(uint8_t data);
	uint8_t analog_r();
	void create_analog_timers();

	void main_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Machine setup
 *
 *************************************/

void clayshoo_state::machine_start()
{
	create_analog_timers();

	// register for state saving
	save_item(NAME(m_input_port_select));
	save_item(NAME(m_analog_port_val));
}



/*************************************
 *
 *  Video hardware
 *
 *************************************/

uint32_t clayshoo_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (offs_t offs = 0; offs < m_videoram.bytes(); offs++)
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
 *  Digital control handling functions
 *
 *************************************/

void clayshoo_state::input_port_select_w(uint8_t data)
{
	m_input_port_select = data;
}

uint8_t clayshoo_state::input_port_r()
{
	uint8_t data = 0xff;

	for (int i = 0; i < 4; i++)
		if (BIT(m_input_port_select, i))
			data &= m_in[i]->read();

	return data;
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

void clayshoo_state::analog_reset_w(uint8_t data)
{
	/* reset the analog value, and start the two timers that will fire
	   off in a short period proportional to the position of the
	   analog control and set the appropriate bit. */
	m_analog_port_val = 0xff;

	for (int i = 0; i < 2; i++)
	{
		// the 58 comes from the length of the loop used to read the analog position
		attotime duration = m_maincpu->cycles_to_attotime(58 * m_an[i]->read());
		const int bit = i ? 0x01 : 0x02;

		m_analog_timer[i]->adjust(duration, bit);
	}
}

uint8_t clayshoo_state::analog_r()
{
	return m_analog_port_val;
}

void clayshoo_state::create_analog_timers()
{
	for (int i = 0; i < 2; i++)
		m_analog_timer[i] = timer_alloc(FUNC(clayshoo_state::reset_analog_bit), this);
}



/*************************************
 *
 *  Address maps
 *
 *************************************/

void clayshoo_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x23ff).ram();
	map(0x4000, 0x47ff).rom();
	map(0x8000, 0x97ff).ram().share("videoram"); // 6k of video ram according to readme
	map(0x9800, 0xa800).nopw(); // not really mapped, but cleared
	map(0xc800, 0xc800).rw(FUNC(clayshoo_state::analog_r), FUNC(clayshoo_state::analog_reset_w));
}

void clayshoo_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	map(0x20, 0x23).rw(m_ppi[0], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x30, 0x33).rw(m_ppi[1], FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x40, 0x43).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0x50, 0x50).unmapw(); // ?
	map(0x60, 0x60).unmapw(); // ?
}



/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( clayshoo )
	PORT_START("IN0")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1C:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Free_Play ) )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x00, "SW1C:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1C:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1C:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1C:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW1C:2" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW1C:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x07, 0x00, "Time/Bonus 1P-2P" ) PORT_DIPLOCATION("SW1B:8,7,6")
	PORT_DIPSETTING(    0x00, "60/6k-90/6k" )
	PORT_DIPSETTING(    0x01, "60/6k-120/8k" )
	PORT_DIPSETTING(    0x02, "90/9.5k-150/9.5k" )
	PORT_DIPSETTING(    0x03, "90/9.5k-190/11k" )
	PORT_DIPSETTING(    0x04, "60/8k-90/8k" )
	PORT_DIPSETTING(    0x05, "60/8k-120/10k" )
	PORT_DIPSETTING(    0x06, "90/11.5k-150/11.5k" )
	PORT_DIPSETTING(    0x07, "90/11.5k-190/13k" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x00, "SW1B:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x00, "SW1B:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x00, "SW1B:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x00, "SW1B:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x00, "SW1B:1" )

	PORT_START("IN2") // skill level switch is on the control panel and can be changed mid-game
	PORT_CONFNAME( 0x03, 0x00, "P2 Skill Level" )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x03, "Expert" )
	PORT_CONFSETTING(    0x01, "Pro" )
	PORT_CONFNAME( 0x0c, 0x00, "P1 Skill Level" )
	PORT_CONFSETTING(    0x00, "Amateur" )
	PORT_CONFSETTING(    0x0c, "Expert" )
	PORT_CONFSETTING(    0x04, "Pro" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("AN0") // fake analog control, visible in $c800 bit 1
	PORT_BIT( 0x0f, 0x08, IPT_PADDLE_V ) PORT_MINMAX(0x01,0x0f) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("AN1") // fake analog control, visible in $c800 bit 0
	PORT_BIT( 0x0f, 0x08, IPT_PADDLE_V ) PORT_MINMAX(0x01,0x0f) PORT_SENSITIVITY(50) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_REVERSE PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END



/*************************************
 *
 *  Machine config
 *
 *************************************/

void clayshoo_state::clayshoo(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 5.0688_MHz_XTAL / 2); // divider is a guess
	m_maincpu->set_addrmap(AS_PROGRAM, &clayshoo_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &clayshoo_state::main_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(clayshoo_state::irq0_line_hold));

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_size(256, 256);
	screen.set_visarea(0, 255, 64, 255);
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_screen_update(FUNC(clayshoo_state::screen_update));

	I8255(config, m_ppi[0]);

	I8255(config, m_ppi[1]);
	m_ppi[1]->out_pa_callback().set(FUNC(clayshoo_state::input_port_select_w));
	m_ppi[1]->in_pb_callback().set(FUNC(clayshoo_state::input_port_r));

	// sound hardware
	SPEAKER(config, "mono").front_center();

	PIT8253(config, m_pit);
	m_pit->set_clk<0>(5.0688_MHz_XTAL / 2);
	m_pit->set_clk<1>(5.0688_MHz_XTAL / 2);
	m_pit->set_clk<2>(5.0688_MHz_XTAL / 2);
	m_pit->out_handler<0>().set(m_dac, FUNC(dac_bit_interface::write)).invert();

	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( clayshoo )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "0.d8", 0x0000, 0x0800, CRC(9df9d9e3) SHA1(8ce71a6faf5df9c8c3dbb92a443b62c0f376491c) )
	ROM_LOAD( "1.d7", 0x0800, 0x0800, CRC(5134a631) SHA1(f0764a5161934564fd0416be26087cf812e0c422) )
	ROM_LOAD( "2.d6", 0x1000, 0x0800, CRC(5b5a67f6) SHA1(c97b4d44e6dc5dd0c42e04ffceed8934975fe769) )
	ROM_LOAD( "3.d5", 0x1800, 0x0800, CRC(7eda8e44) SHA1(2974f8b06653aee2ffd96ff402707acfc059bc91) )
	ROM_LOAD( "4.d4", 0x4000, 0x0800, CRC(3da16196) SHA1(eb0c0cf0c8fc3db05ac0c469fb20fe92ae6f27ce) )
ROM_END

} // anonymous namespace



/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1979, clayshoo, 0, clayshoo, clayshoo, clayshoo_state, empty_init, ROT0, "Allied Leisure", "Clay Shoot", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
