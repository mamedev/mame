// license:BSD-3-Clause
// copyright-holders:Mariusz Wojcieszek, Aaron Giles
/******************************************************************************

    Exidy Max-A-Flex driver

    by Mariusz Wojcieszek

    Based on Atari 400/800 MAME Driver by Juergen Buchmueller

******************************************************************************/

#include "emu.h"
#include "atari400.h"

#include "cpu/m6502/m6502.h"
#include "cpu/m6805/m68705.h"
#include "machine/6821pia.h"
#include "machine/input_merger.h"
#include "machine/rescap.h"
#include "machine/timer.h"
#include "sound/spkrdev.h"
#include "sound/pokey.h"
#include "video/pwm.h"

#include "gtia.h"

#include "screen.h"
#include "speaker.h"

#include "maxaflex.lh"


namespace {

class maxaflex_state : public atari_common_state
{
public:
	maxaflex_state(const machine_config &mconfig, device_type type, const char *tag)
		: atari_common_state(mconfig, type, tag)
		, m_mcu(*this, "mcu")
		, m_speaker(*this, "speaker")
		, m_digits_pwm(*this, "digits_pwm")
		, m_region_maincpu(*this, "maincpu")
		, m_dsw(*this, "dsw")
		, m_coin(*this, "coin")
		, m_console(*this, "console")
		, m_joy01(*this, "djoy_0_1")
		, m_joy23(*this, "djoy_2_3")
		, m_lamps(*this, "lamp%u", 0U)
		, m_digits(*this, "digit%u", 0U)
	{
	}

	void maxaflex(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	uint8_t mcu_porta_r();
	void mcu_porta_w(uint8_t data);
	void mcu_portb_w(uint8_t data);
	void mcu_portc_w(uint8_t data);
	uint8_t pia_pa_r();
	uint8_t pia_pb_r();
	void pia_pb_w(uint8_t data) { mmu(data); }
	void pia_cb2_w(int state) { } // This is used by Floppy drive on Atari 8bits Home Computers
	TIMER_DEVICE_CALLBACK_MEMBER(mf_interrupt);

	bool atari_input_disabled() const { return !BIT(m_portb_out, 7); }
	void mmu(uint8_t new_mmu);

	void a600xl_mem(address_map &map) ATTR_COLD;

	uint8_t m_portb_out;
	uint8_t m_portc_out;

	required_device<m68705p3_device> m_mcu;
	required_device<speaker_sound_device> m_speaker;
	required_device<pwm_display_device> m_digits_pwm;
	required_region_ptr<uint8_t> m_region_maincpu;
	required_ioport m_dsw;
	required_ioport m_coin;
	required_ioport m_console;
	required_ioport m_joy01;
	required_ioport m_joy23;
	output_finder<4> m_lamps;
	output_finder<3> m_digits;
};


void maxaflex_state::mmu(uint8_t new_mmu)
{
	// check if self-test ROM changed
	if (new_mmu & 0x80)
	{
		logerror("%s MMU SELFTEST RAM\n", machine().system().name);
		m_maincpu->space(AS_PROGRAM).nop_readwrite(0x5000, 0x57ff);
	}
	else
	{
		logerror("%s MMU SELFTEST ROM\n", machine().system().name);
		m_maincpu->space(AS_PROGRAM).install_rom(0x5000, 0x57ff, &m_region_maincpu[0xd000]);
		m_maincpu->space(AS_PROGRAM).unmap_write(0x5000, 0x57ff);
	}
}


// Supervisor board emulation

/* Port A:
    0   (in)  DSW
    1   (in)  DSW
    2   (in)  DSW
    3   (in)  DSW
    4   (in)  coin
    5   (in)  START button
    6   -
    7   (out) AUDIO
*/

uint8_t maxaflex_state::mcu_porta_r()
{
	return
			((m_dsw->read()     << 0) & 0x0f) |
			((m_coin->read()    << 4) & 0x10) |
			((m_console->read() << 5) & 0x20) |
			0xc0;
}

void maxaflex_state::mcu_porta_w(uint8_t data)
{
	m_speaker->level_w(BIT(data, 7));
}


/* Port B:
    0   (out)   Select 7-segment display to control by writing port C
    1   (out)   "
    2   (out)   clear coin interrupt
    3   (out)   STRKEY - line connected to keyboard input in 600XL, seems to be not used
    4   (out)   RES600 - reset 600
    5   (out)   AUDMUTE - mutes audio
    6   (out)   latch for lamps
    7   (out)   TOFF - enables/disables user controls
*/

void maxaflex_state::mcu_portb_w(uint8_t data)
{
	const uint8_t diff = data ^ m_portb_out;
	m_portb_out = data;

	// 74139 to 7segs
	m_digits_pwm->write_my(1 << (data & 3));

	// clear coin interrupt
	if (BIT(data, 2))
		m_mcu->set_input_line(M6805_IRQ_LINE, CLEAR_LINE);

	// RES600
	m_maincpu->set_input_line(INPUT_LINE_RESET, BIT(data, 4) ? CLEAR_LINE : ASSERT_LINE);

	// AUDMUTE
	machine().sound().system_mute(!BIT(data, 5));

	// clock 74175 for lamps
	if (BIT(diff, 6) && !BIT(data, 6))
	{
		m_lamps[0] = BIT(m_portc_out, 0);
		m_lamps[1] = BIT(m_portc_out, 1);
		m_lamps[2] = BIT(m_portc_out, 2);
		m_lamps[3] = BIT(m_portc_out, 3);
	}
}


/* Port C:
    0   (out)   7segs + lamp COIN
    1   (out)   7segs + lamp PLAY
    2   (out)   7segs + lamp START
    3   (out)   7segs + lamp OVER */

void maxaflex_state::mcu_portc_w(uint8_t data)
{
	m_portc_out = data & 0x0f;

	// uses a 7447A, which is equivalent to an LS47/48
	constexpr static uint8_t ls48_map[16] =
		{ 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0x58, 0x4c, 0x62, 0x69, 0x78, 0x00 };

	m_digits_pwm->write_mx(ls48_map[m_portc_out]);
}

INPUT_CHANGED_MEMBER(maxaflex_state::coin_inserted)
{
	if (!newval)
		m_mcu->set_input_line(M6805_IRQ_LINE, ASSERT_LINE);
}



void maxaflex_state::a600xl_mem(address_map &map)
{
	map(0x0000, 0x3fff).ram();
	map(0x5000, 0x57ff).rom().region("maincpu", 0xd000); // self test
	map(0x8000, 0xbfff).rom(); // game cartridge
	map(0xc000, 0xcfff).rom(); // OS
	map(0xd000, 0xd0ff).rw(m_gtia, FUNC(gtia_device::read), FUNC(gtia_device::write));
	map(0xd100, 0xd1ff).noprw();
	map(0xd200, 0xd2ff).rw(m_pokey, FUNC(pokey_device::read), FUNC(pokey_device::write));
	map(0xd300, 0xd3ff).rw("pia", FUNC(pia6821_device::read_alt), FUNC(pia6821_device::write_alt));
	map(0xd400, 0xd4ff).rw(m_antic, FUNC(antic_device::read), FUNC(antic_device::write));
	map(0xd500, 0xd7ff).noprw();
	map(0xd800, 0xffff).rom(); // OS
}


static INPUT_PORTS_START( a600xl )
	PORT_START("console")
	PORT_BIT(0x04, 0x04, IPT_KEYPAD) PORT_NAME("Option") PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x02, 0x02, IPT_KEYPAD) PORT_NAME("Select") PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x01, 0x01, IPT_START1 )

	PORT_START("djoy_0_1")
	PORT_BIT(0x01, 0x01, IPT_JOYSTICK_UP) PORT_PLAYER(1)
	PORT_BIT(0x02, 0x02, IPT_JOYSTICK_DOWN) PORT_PLAYER(1)
	PORT_BIT(0x04, 0x04, IPT_JOYSTICK_LEFT) PORT_PLAYER(1)
	PORT_BIT(0x08, 0x08, IPT_JOYSTICK_RIGHT) PORT_PLAYER(1)
	// player #2 input is not connected
	PORT_BIT(0x10, 0x10, IPT_JOYSTICK_UP) PORT_PLAYER(2)
	PORT_BIT(0x20, 0x20, IPT_JOYSTICK_DOWN) PORT_PLAYER(2)
	PORT_BIT(0x40, 0x40, IPT_JOYSTICK_LEFT) PORT_PLAYER(2)
	PORT_BIT(0x80, 0x80, IPT_JOYSTICK_RIGHT) PORT_PLAYER(2)

	PORT_START("djoy_2_3")
	// not connected
	PORT_BIT(0x01, 0x01, IPT_JOYSTICK_UP) PORT_PLAYER(3)
	PORT_BIT(0x02, 0x02, IPT_JOYSTICK_DOWN) PORT_PLAYER(3)
	PORT_BIT(0x04, 0x04, IPT_JOYSTICK_LEFT) PORT_PLAYER(3)
	PORT_BIT(0x08, 0x08, IPT_JOYSTICK_RIGHT) PORT_PLAYER(3)
	PORT_BIT(0x10, 0x10, IPT_JOYSTICK_UP) PORT_PLAYER(4)
	PORT_BIT(0x20, 0x20, IPT_JOYSTICK_DOWN) PORT_PLAYER(4)
	PORT_BIT(0x40, 0x40, IPT_JOYSTICK_LEFT) PORT_PLAYER(4)
	PORT_BIT(0x80, 0x80, IPT_JOYSTICK_RIGHT) PORT_PLAYER(4)

	PORT_START("djoy_b")
	PORT_BIT(0x01, 0x01, IPT_BUTTON1) PORT_PLAYER(1)
	PORT_BIT(0x02, 0x02, IPT_BUTTON1) PORT_PLAYER(2)
	PORT_BIT(0x04, 0x04, IPT_BUTTON1) PORT_PLAYER(3)
	PORT_BIT(0x08, 0x08, IPT_BUTTON1) PORT_PLAYER(4)

	// Max-A-Flex specific ports
	PORT_START("coin")
	PORT_BIT(0x1, IP_ACTIVE_LOW, IPT_COIN1) PORT_CHANGED_MEMBER(DEVICE_SELF, maxaflex_state, coin_inserted, 0)

	PORT_START("dsw")
	PORT_DIPNAME(0xf, 0x9, "Coin/Time" )
	PORT_DIPSETTING( 0x0, "30 sec" )
	PORT_DIPSETTING( 0x1, "60 sec" )
	PORT_DIPSETTING( 0x2, "90 sec" )
	PORT_DIPSETTING( 0x3, "120 sec" )
	PORT_DIPSETTING( 0x4, "150 sec" )
	PORT_DIPSETTING( 0x5, "180 sec" )
	PORT_DIPSETTING( 0x6, "210 sec" )
	PORT_DIPSETTING( 0x7, "240 sec" )
	PORT_DIPSETTING( 0x8, "270 sec" )
	PORT_DIPSETTING( 0x9, "300 sec" )
	PORT_DIPSETTING( 0xa, "330 sec" )
	PORT_DIPSETTING( 0xb, "360 sec" )
	PORT_DIPSETTING( 0xc, "390 sec" )
	PORT_DIPSETTING( 0xd, "420 sec" )
	PORT_DIPSETTING( 0xe, "450 sec" )
	PORT_DIPSETTING( 0xf, "480 sec" )
INPUT_PORTS_END


uint8_t maxaflex_state::pia_pa_r()
{
	return atari_input_disabled() ? 0xff : m_joy01->read();
}

uint8_t maxaflex_state::pia_pb_r()
{
	return atari_input_disabled() ? 0xff : m_joy23->read();
}


void maxaflex_state::machine_start()
{
	atari_common_state::machine_start();

	m_lamps.resolve();
	m_digits.resolve();

	save_item(NAME(m_portb_out));
	save_item(NAME(m_portc_out));
}

void maxaflex_state::machine_reset()
{
	atari_common_state::machine_reset();

	m_pokey->write(15, 0);

	// Supervisor board reset
	m_portb_out = 0xff;
	m_portc_out = 0xff;

	std::fill(std::begin(m_lamps), std::end(m_lamps), 0);
	std::fill(std::begin(m_digits), std::end(m_digits), 0x00);
}

TIMER_DEVICE_CALLBACK_MEMBER( maxaflex_state::mf_interrupt )
{
	m_antic->generic_interrupt(2);
}

void maxaflex_state::maxaflex(machine_config &config)
{
	// basic machine hardware
	m6502_device &maincpu(M6502(config, "maincpu", pokey_device::FREQ_17_EXACT));
	maincpu.set_addrmap(AS_PROGRAM, &maxaflex_state::a600xl_mem);
	TIMER(config, "scantimer").configure_scanline(FUNC(maxaflex_state::mf_interrupt), "screen", 0, 1);

	INPUT_MERGER_ANY_HIGH(config, "mainirq").output_handler().set_inputline("maincpu", m6502_device::IRQ_LINE);

	M68705P3(config, m_mcu, 3.579545_MHz_XTAL);
	m_mcu->porta_r().set(FUNC(maxaflex_state::mcu_porta_r));
	m_mcu->porta_w().set(FUNC(maxaflex_state::mcu_porta_w));
	m_mcu->portb_w().set(FUNC(maxaflex_state::mcu_portb_w));
	m_mcu->portc_w().set(FUNC(maxaflex_state::mcu_portc_w));

	ATARI_GTIA(config, m_gtia, 0);
	m_gtia->set_region(GTIA_NTSC);
	m_gtia->read_callback().set_ioport("console");
	m_gtia->trigger_callback().set_ioport("djoy_b");

	ATARI_ANTIC(config, m_antic, 0);
	m_antic->set_gtia_tag(m_gtia);

	pia6821_device &pia(PIA6821(config, "pia"));
	pia.readpa_handler().set(FUNC(maxaflex_state::pia_pa_r));
	pia.readpb_handler().set(FUNC(maxaflex_state::pia_pb_r));
	pia.writepb_handler().set(FUNC(maxaflex_state::pia_pb_w));
	pia.cb2_handler().set(FUNC(maxaflex_state::pia_cb2_w));
	pia.irqa_handler().set("mainirq", FUNC(input_merger_device::in_w<1>));
	pia.irqb_handler().set("mainirq", FUNC(input_merger_device::in_w<2>));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	config_ntsc_screen(config);
	//screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	//screen.set_visarea(antic_device::MIN_X, antic_device::MAX_X, antic_device::MIN_Y, antic_device::MAX_Y);
	//screen.set_refresh_hz(antic_device::FRAME_RATE_60HZ);
	//screen.set_size(antic_device::HWIDTH * 8, antic_device::TOTAL_LINES_60HZ);
	m_screen->set_screen_update("antic", FUNC(antic_device::screen_update));
	m_screen->set_palette("palette");

	PALETTE(config, "palette", FUNC(maxaflex_state::atari_palette), 256);

	PWM_DISPLAY(config, m_digits_pwm).set_size(3, 7);
	m_digits_pwm->set_segmask(0x7, 0x7f);
	m_digits_pwm->output_digit().set([this](offs_t offset, u64 data) { m_digits[offset] = data; });

	config.set_default_layout(layout_maxaflex);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	POKEY(config, m_pokey, pokey_device::FREQ_17_EXACT);
	m_pokey->irq_w().set("mainirq", FUNC(input_merger_device::in_w<0>));
	m_pokey->set_output_rc(RES_K(1), CAP_U(0.0), 5.0);
	m_pokey->add_route(ALL_OUTPUTS, "mono", 1.0);

	SPEAKER_SOUND(config, m_speaker);
	m_speaker->add_route(ALL_OUTPUTS, "mono", 0.50);

	RAM(config, RAM_TAG).set_default_size("16K");
}


ROM_START(maxaflex)
	ROM_REGION(0x10000,"maincpu",0) // 64K for the CPU
	ROM_LOAD("atarixl.rom",  0xc000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55))

	ROM_REGION( 0x0800, "mcu", 0 ) // 2k for the microcontroller
	ROM_LOAD("maxaflex.uc",  0x0000, 0x0800, CRC(fe9cf53c) SHA1(4b02bc2f0c8a1eab799814fac82d5812c0160206))

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("maxprom.prm",  0x0000, 0x0200, CRC(edf5c950) SHA1(9ad046ea41a61585dd8d2f2d4167a3cc39d2928f)) // for simulating keystrokes ?
ROM_END

ROM_START(mf_bdash)
	ROM_REGION(0x10000,"maincpu",0) // 64K for the CPU
	ROM_LOAD("bd-acs-1.rom", 0x8000, 0x2000, CRC(2b11750e) SHA1(43e9ae44eb1767621920bb94a4370ed602d81056))
	ROM_LOAD("bd-acs-2.rom", 0xa000, 0x2000, CRC(e9ea2658) SHA1(189ede7201ef122cf2b72fc847a896b9dbe007e5))
	ROM_LOAD("atarixl.rom",  0xc000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55))

	ROM_REGION( 0x0800, "mcu", 0 ) // 2k for the microcontroller
	ROM_LOAD("maxaflex.uc",  0x0000, 0x0800, CRC(fe9cf53c) SHA1(4b02bc2f0c8a1eab799814fac82d5812c0160206))

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("maxprom.prm",  0x0000, 0x0200, CRC(edf5c950) SHA1(9ad046ea41a61585dd8d2f2d4167a3cc39d2928f)) // for simulating keystrokes ?
ROM_END

ROM_START(mf_achas)
	ROM_REGION(0x10000,"maincpu",0) // 64K for the CPU
	ROM_LOAD("ac.rom",       0x8000, 0x4000, CRC(18752991) SHA1(f508b89d2251c53d017cff6cb23b8e9880a0cc0b))
	ROM_LOAD("atarixl.rom",  0xc000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55))

	ROM_REGION( 0x0800, "mcu", 0 ) // 2k for the microcontroller
	ROM_LOAD("maxaflex.uc",  0x0000, 0x0800, CRC(fe9cf53c) SHA1(4b02bc2f0c8a1eab799814fac82d5812c0160206))

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("maxprom.prm",  0x0000, 0x0200, CRC(edf5c950) SHA1(9ad046ea41a61585dd8d2f2d4167a3cc39d2928f)) // for simulating keystrokes ?
ROM_END

ROM_START(mf_brist)
	ROM_REGION(0x10000,"maincpu",0) // 64K for the CPU
	ROM_LOAD("brist.rom",    0x8000, 0x4000, CRC(4263d64d) SHA1(80a041bceb499e1466516488013aa4439b3db6f2))
	ROM_LOAD("atarixl.rom",  0xc000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55))

	ROM_REGION( 0x0800, "mcu", 0 ) // 2k for the microcontroller
	ROM_LOAD("maxaflex.uc",  0x0000, 0x0800, CRC(fe9cf53c) SHA1(4b02bc2f0c8a1eab799814fac82d5812c0160206))

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("maxprom.prm",  0x0000, 0x0200, CRC(edf5c950) SHA1(9ad046ea41a61585dd8d2f2d4167a3cc39d2928f)) // for simulating keystrokes ?
ROM_END

ROM_START(mf_flip)
	ROM_REGION(0x10000,"maincpu",0) // 64K for the CPU
	ROM_LOAD("flipflop.rom", 0x8000, 0x4000, CRC(8ae057be) SHA1(ba26d6a3790ebdb754c1192b2c28f0fe93aca377))
	ROM_LOAD("atarixl.rom",  0xc000, 0x4000, CRC(1f9cd270) SHA1(ae4f523ba08b6fd59f3cae515a2b2410bbd98f55))

	ROM_REGION( 0x0800, "mcu", 0 ) // 2k for the microcontroller
	ROM_LOAD("maxaflex.uc",  0x0000, 0x0800, CRC(fe9cf53c) SHA1(4b02bc2f0c8a1eab799814fac82d5812c0160206))

	ROM_REGION( 0x200, "proms", 0 )
	ROM_LOAD("maxprom.prm",  0x0000, 0x0200, CRC(edf5c950) SHA1(9ad046ea41a61585dd8d2f2d4167a3cc39d2928f)) // for simulating keystrokes ?
ROM_END

} // anonymous namespace


//    YEAR  NAME      PARENT    MACHINE   INPUT   CLASS           INIT        SCREEN  COMPANY,                       FULLNAME,                    FLAGS
GAME( 1984, maxaflex, 0,        maxaflex, a600xl, maxaflex_state, empty_init, ROT0,   "Exidy",                       "Max-A-Flex",                MACHINE_IS_BIOS_ROOT )
GAME( 1982, mf_achas, maxaflex, maxaflex, a600xl, maxaflex_state, empty_init, ROT0,   "Exidy / First Star Software", "Astro Chase (Max-A-Flex)",  0 )
GAME( 1983, mf_brist, maxaflex, maxaflex, a600xl, maxaflex_state, empty_init, ROT0,   "Exidy / First Star Software", "Bristles (Max-A-Flex)",     0 )
GAME( 1983, mf_flip,  maxaflex, maxaflex, a600xl, maxaflex_state, empty_init, ROT0,   "Exidy / First Star Software", "Flip & Flop (Max-A-Flex)",  0 )
GAME( 1984, mf_bdash, maxaflex, maxaflex, a600xl, maxaflex_state, empty_init, ROT0,   "Exidy / First Star Software", "Boulder Dash (Max-A-Flex)", 0 )
