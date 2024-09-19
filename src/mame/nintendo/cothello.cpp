// license:BSD-3-Clause
// copyright-holders:hap
/*******************************************************************************

Nintendo Computer Othello (コンピューター オセロ)

This is Nintendo's 1st microprocessor-based arcade game.
It's a cocktail cabinet, P1 side has 10 buttons, and P2 side has 4.

In 1980, they also sold a home version. It was called Computer TV Game (model CTG-HC10)
and is presumed to be on the same base hardware as the arcade version.

Hardware notes:
- PCB label: COG CPU
- M58710S (8080A), 3.579545MHz XTAL
- 3*1KB M58732S 2708 ROM, 4th socket is empty
- 256 bytes RAM (2*M58722P 2111A)
- 0.5KB DRAM (M58755S) for framebuffer
- M58741P Color TV Interface, 64*64 pixels (192 scanlines), chip supports 3bpp
  and the software does write 3bpp, but the VRAM only has 1 bit per pixel
- 7seg time counter (not software controlled)
- beeper

TODO:
- verify video timing, maybe 3.57MHz / 2 / (262*114)
- beeper duration and base frequency is approximated (divisor is correct)
- CPU speed is wrong, it's likely running at 1.79MHz. But that's way too fast
  compared to videos of the game. 0.9MHz(XTAL/4) is also too fast. It's probably
  due to the M58741P HOLD pin halting the 8080, which is not emulated.
- Is there a button select somewhere? I really can't find any. Or is current
  input emulation correct where for example P1 can move P2 cursor?
- It's not known if the screen is color or B&W + overlay, but since the video
  chip is meant for a color tv, let's assume the green tint is from the screen
  itself. Photos of the home version also show a green tint.
- Is Universal's "Computer R-3" a modified version of this game?

*******************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "sound/beep.h"

#include "screen.h"
#include "speaker.h"

#include "cothello.lh"


namespace {

class cothello_state : public driver_device
{
public:
	cothello_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_vram(*this, "vram"),
		m_beeper(*this, "beeper"),
		m_inputs(*this, "IN.%u", 0),
		m_digits(*this, "digit%u", 0U)
	{ }

	void cothello(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(insert_coin);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_shared_ptr<u8> m_vram;
	required_device<beep_device> m_beeper;
	required_ioport_array<4> m_inputs;
	output_finder<3> m_digits;

	u16 m_counter = 0;
	u8 m_sound_data = 0;
	emu_timer *m_counter_timer;
	emu_timer *m_beeper_off;

	void main_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	u8 coin_r();
	u8 input_r();
	void sound_w(u8 data);

	TIMER_CALLBACK_MEMBER(counter_tick);
	TIMER_CALLBACK_MEMBER(beeper_off) { m_beeper->set_state(0); }
};

void cothello_state::machine_start()
{
	m_digits.resolve();

	m_counter_timer = timer_alloc(FUNC(cothello_state::counter_tick), this);
	m_beeper_off = timer_alloc(FUNC(cothello_state::beeper_off), this);

	save_item(NAME(m_counter));
	save_item(NAME(m_sound_data));
}



/*******************************************************************************
    Video
*******************************************************************************/

u32 cothello_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			int pixel = m_vram[((y / 3) << 8 | x) & 0x3fff] & 1;
			bitmap.pix(y, x) = pixel ? rgb_t(0x00, 0xff, 0x80) : rgb_t::black();
		}
	}

	return 0;
}



/*******************************************************************************
    I/O
*******************************************************************************/

INPUT_CHANGED_MEMBER(cothello_state::insert_coin)
{
	if (!newval || m_counter_timer->enabled())
		return;

	// reset counter
	m_counter = ~0;
	m_counter_timer->adjust(attotime::zero);
}

TIMER_CALLBACK_MEMBER(cothello_state::counter_tick)
{
	m_counter++;

	// highest digit is compared to a digit wheel setting
	u8 max = std::clamp(int(m_inputs[3]->read()), 1, 9);
	u8 high = (m_counter / 100) % 10;

	if (high != max)
		m_counter_timer->adjust(attotime::from_seconds(1));

	// output counter to 7segs (don't know which decoder chip, but 6/9 has roof/tail)
	static const u8 led_map[] = { 0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f };

	m_digits[0] = led_map[m_counter % 10];
	m_digits[1] = led_map[(m_counter / 10) % 10];
	m_digits[2] = led_map[high];
}

u8 cothello_state::coin_r()
{
	// d0-d3: coin status
	// d4-d7: unused
	return m_counter_timer->enabled() ? 0xfb : 0xff;
}

u8 cothello_state::input_r()
{
	u8 data = 0xff;

	for (int i = 0; i < 3; i++)
		data &= m_inputs[i]->read();

	return data;
}

void cothello_state::sound_w(u8 data)
{
	if (data & 0x80)
	{
		// d3: beeper start
		if (~m_sound_data & data & 8)
		{
			m_beeper->set_state(1);
			m_beeper_off->adjust(attotime::from_msec(80)); // approximation
		}

		// d0-d2: beeper frequency
		u8 freq = (data & 0x7) + 1;
		const u32 base = 3500; // approximation
		m_beeper->set_clock(base / freq);
	}

	m_sound_data = data;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void cothello_state::main_map(address_map &map)
{
	map(0x0000, 0x0fff).rom();
	map(0x4000, 0x40ff).ram();
	map(0x6000, 0x6000).r(FUNC(cothello_state::input_r));
	map(0x8000, 0x8000).w(FUNC(cothello_state::sound_w));
	map(0xa000, 0xa000).r(FUNC(cothello_state::coin_r));
	map(0xc000, 0xffff).writeonly().share("vram");
	map(0xc040, 0xc0ff).mirror(0x3f00).nopw();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( cothello )
	PORT_START("IN.0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("1 Player Start Sente")  // 4
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 ) PORT_NAME("1 Player Start Gote")   // 3
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START3 ) PORT_NAME("2 Players Start Sente") // 2
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START4 ) PORT_NAME("2 Players Start Gote")  // 1
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Abort")
	PORT_SERVICE( 0x80, IP_ACTIVE_LOW )

	PORT_START("IN.1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Pass")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Set")
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL PORT_16WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Pass")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Set")
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN.3")
	PORT_CONFNAME( 0x0f, 0x05, "Time" )
	PORT_DIPSETTING(     0x01, "100 seconds" )
	PORT_DIPSETTING(     0x02, "200 seconds" )
	PORT_DIPSETTING(     0x03, "300 seconds" )
	PORT_DIPSETTING(     0x04, "400 seconds" )
	PORT_DIPSETTING(     0x05, "500 seconds" )
	PORT_DIPSETTING(     0x06, "600 seconds" )
	PORT_DIPSETTING(     0x07, "700 seconds" )
	PORT_DIPSETTING(     0x08, "800 seconds" )
	PORT_DIPSETTING(     0x09, "900 seconds" )

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cothello_state, insert_coin, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void cothello_state::cothello(machine_config &config)
{
	// basic machine hardware
	I8080A(config, m_maincpu, 750000); // wrong, see TODO
	m_maincpu->set_vblank_int("screen", FUNC(cothello_state::irq0_line_hold));
	m_maincpu->set_addrmap(AS_PROGRAM, &cothello_state::main_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(64, 192);
	m_screen->set_visarea(0, 64-1, 0, 192-1);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	m_screen->set_screen_update(FUNC(cothello_state::screen_update));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 0).add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( cothello )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "13.ic13", 0x0000, 0x0400, CRC(c4b2802b) SHA1(b7e568c3503722143815b051de2dd3b60cde635a) )
	ROM_LOAD( "12.ic12", 0x0400, 0x0400, CRC(293eee03) SHA1(d3bf755104f2fcbc99ebdc9556b3f42cdfacf94e) )
	ROM_LOAD( "11.ic11", 0x0800, 0x0400, CRC(dea6486e) SHA1(4e11699dfee0e34c67872427372ea0b33bd16d09) )
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT  MACHINE    INPUT     CLASS           INIT        SCREEN  COMPANY, FULLNAME, FLAGS
GAMEL(1978, cothello, 0,      cothello,  cothello, cothello_state, empty_init, ROT0,   "Nintendo", "Computer Othello", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND | MACHINE_IMPERFECT_TIMING, layout_cothello )
