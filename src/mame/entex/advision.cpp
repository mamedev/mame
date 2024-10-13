// license:BSD-3-Clause
// copyright-holders:Dan Boris, hap
/*******************************************************************************

Entex Adventure Vision, tabletop game console

Hardware notes:
- INS8048-11 @ 11MHz (1KB internal ROM)
- COP411 for the sound, 1-bit speaker with volume control
- Molex socket for 4KB cartridges
- 1KB external RAM (2*MM2114N)
- 40 small rectangular red LEDs, a motor with a fast spinning mirror gives the
  illusion of a 150*40 screen
- 4-way joystick, 8 buttons (other than having buttons 2/4 swapped, left and
  right button panels are electronically the same)
- expansion port (unused)

The cartridge slot is different from what is common on other consoles. A game
cartridge is basically an EPROM chip wearing a jacket. Only 4 games were released
in total.

The mirror rotates at around 7Hz, the motor speed is not controlled by software,
and it differs a bit per console. This can be adjusted after enabling -cheat.
There's a mirror on both sides so the display refreshes at around 14Hz. A similar
technology was later used in the Nintendo Virtual Boy.

The display is faked in MAME. On the real thing, the picture is not as stable. The
width of 150 is specified by the BIOS, but it's possible to update the leds at a
different rate, hence MAME configures a larger screen. In fact, the homebrew demo
Code Red doesn't use the BIOS for it, and runs at 50*40 to save some RAM.

TODO:
- display refresh is actually ~14Hz, but doing that will make MAME very sluggish

BTANB:
- 2 thin vertical seams (do a hyperspace in defender to see them more clearly)
- glitches at the right edge during gameplay in scobra, and also during some
  explosion sounds in defender
- scobra screen is shifted to the right when the game scrolls

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/cop400/cop400.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/flt_vol.h"

#include "screen.h"
#include "softlist_dev.h"
#include "speaker.h"

namespace {

class advision_state : public driver_device
{
public:
	advision_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_dac(*this, "dac"),
		m_volume(*this, "volume"),
		m_screen(*this, "screen"),
		m_mirror_sync(*this, "mirror_sync"),
		m_led_update(*this, "led_update"),
		m_led_off(*this, "led_off"),
		m_joy(*this, "JOY"),
		m_conf(*this, "CONF")
	{ }

	void advision(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(set_screensize);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<i8048_device> m_maincpu;
	required_device<cop411_cpu_device> m_soundcpu;
	required_device<dac_1bit_device> m_dac;
	required_device<filter_volume_device> m_volume;
	required_device<screen_device> m_screen;
	required_device<timer_device> m_mirror_sync;
	required_device<timer_device> m_led_update;
	required_device<timer_device> m_led_off;
	required_ioport m_joy;
	required_ioport m_conf;

	static constexpr u32 DISPLAY_WIDTH = 0x400;

	bool m_video_strobe = false;
	bool m_video_enable = false;
	u8 m_video_bank = 0;
	u32 m_video_hpos = 0;
	u8 m_led_output[5] = { };
	u8 m_led_latch[5] = { };
	std::unique_ptr<u8[]> m_display;

	std::vector<u8> m_ext_ram;
	u16 m_rambank = 0;
	u8 m_sound_cmd = 0;

	void io_map(address_map &map) ATTR_COLD;
	void program_map(address_map &map) ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void av_control_w(u8 data);
	void vblank(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(led_update);
	TIMER_DEVICE_CALLBACK_MEMBER(led_off);
	int vsync_r();

	TIMER_CALLBACK_MEMBER(sound_cmd_sync);
	u8 sound_cmd_r();
	void sound_g_w(u8 data);
	void sound_d_w(u8 data);

	void bankswitch_w(u8 data);
	u8 ext_ram_r(offs_t offset);
	void ext_ram_w(offs_t offset, u8 data);
	u8 controller_r();
};



/*******************************************************************************
    Video
*******************************************************************************/

INPUT_CHANGED_MEMBER(advision_state::set_screensize)
{
	// reconfigure screen size when settings changed
	const int width = 9600 / (m_conf->read() & 0x3f);
	const int height = 40 * 4 + 3 - (m_conf->read() >> 6 & 3);
	m_screen->set_visible_area(0, width - 1, 0, height - 1);
}

u32 advision_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u8 led_height = (m_conf->read() >> 6 & 3) + 1;

	bitmap.fill(0, cliprect);

	for (int y = 0; y < 40; y++)
	{
		u8 *src = &m_display[y * DISPLAY_WIDTH];

		for (int x = cliprect.left(); x <= cliprect.right(); x++)
		{
			u8 red = src[x] ? 0xff : 0;
			u8 green = red / 16;
			u8 blue = red / 12;

			int dy = y * 4 + (4 - led_height);

			for (int i = 0; i < led_height; i++)
			{
				if (cliprect.contains(x, dy + i))
					bitmap.pix(dy + i, x) = red << 16 | green << 8 | blue;
			}
		}
	}

	return 0;
}

void advision_state::av_control_w(u8 data)
{
	// P25-P27: led latch select
	m_video_bank = data >> 5 & 7;

	// disable led outputs (there is some delay before it takes effect)
	// see for example codered twister and anime girl, gaps between the 'pixels' should be visible but minimal
	if (m_video_bank == 0 && m_led_off->expire().is_never())
		m_led_off->adjust(attotime::from_usec(39));

	// P24 rising edge: transfer led latches to outputs
	if (!m_video_strobe && bool(data & 0x10))
	{
		std::copy_n(m_led_latch, std::size(m_led_output), m_led_output);
		m_led_off->adjust(attotime::never);
		m_video_enable = true;
	}
	m_video_strobe = bool(data & 0x10);

	// sync soundlatch (using gen_latch device here would give lots of logerror)
	machine().scheduler().synchronize(timer_expired_delegate(FUNC(advision_state::sound_cmd_sync), this), data >> 4);
}

void advision_state::vblank(int state)
{
	if (state && (m_screen->frame_number() & 3) == 0)
	{
		// starting a new frame
		std::fill_n(m_display.get(), DISPLAY_WIDTH * 40, 0);

		m_mirror_sync->adjust(attotime::from_usec(100));

		m_video_hpos = 0;
		m_led_update->adjust(attotime::zero);
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(advision_state::led_update)
{
	// write current leds to display buffer
	for (int y = 0; y < 8; y++)
	{
		for (int b = 0; b < 5; b++)
		{
			int pixel = m_video_enable ? BIT(m_led_output[b], y ^ 7) : 0;
			m_display[(b * 8 + y) * DISPLAY_WIDTH + m_video_hpos] = pixel;
		}
	}

	if (m_video_hpos < DISPLAY_WIDTH - 1)
	{
		// for official games, 1 'pixel' is 60us, but there are two spots that have
		// a longer duration: at x=50 and x=100 (see BTANB note about seams)
		m_led_update->adjust(attotime::from_usec(m_conf->read() & 0x3f));
		m_video_hpos++;
	}
}

TIMER_DEVICE_CALLBACK_MEMBER(advision_state::led_off)
{
	m_video_enable = false;
}

int advision_state::vsync_r()
{
	// T1: mirror sync pulse (half rotation)
	return (m_mirror_sync->enabled()) ? 0 : 1;
}



/*******************************************************************************
    Sound
*******************************************************************************/

TIMER_CALLBACK_MEMBER(advision_state::sound_cmd_sync)
{
	m_sound_cmd = param;
}

u8 advision_state::sound_cmd_r()
{
	// L0-L3: sound command
	return m_sound_cmd;
}

void advision_state::sound_g_w(u8 data)
{
	// G0: speaker out
	m_dac->write(data & 1);
}

void advision_state::sound_d_w(u8 data)
{
	// D0: speaker volume
	m_volume->set_gain((data & 1) ? 0.5 : 1.0);
}



/*******************************************************************************
    Memory
*******************************************************************************/

void advision_state::bankswitch_w(u8 data)
{
	// P10,P11: RAM bank
	m_rambank = data & 3;

	// P12: 8048 EA pin
	m_maincpu->set_input_line(MCS48_INPUT_EA, BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
}

u8 advision_state::ext_ram_r(offs_t offset)
{
	// read from external RAM
	u8 data = m_ext_ram[m_rambank << 8 | offset];
	if (machine().side_effects_disabled())
		return data;

	// transfer data to led latch
	if (m_video_bank > 0 && m_video_bank < 6)
		m_led_latch[5 - m_video_bank] = data ^ 0xff;

	// reset sound cpu
	else if (m_video_bank == 6)
		m_soundcpu->set_input_line(INPUT_LINE_RESET, (data & 1) ? CLEAR_LINE : ASSERT_LINE);

	return data;
}

void advision_state::ext_ram_w(offs_t offset, u8 data)
{
	// write to external RAM
	m_ext_ram[m_rambank << 8 | offset] = data;
}

void advision_state::program_map(address_map &map)
{
	map(0x0000, 0x0fff).r("cartslot", FUNC(generic_slot_device::read_rom));
}

void advision_state::io_map(address_map &map)
{
	map(0x00, 0xff).rw(FUNC(advision_state::ext_ram_r), FUNC(advision_state::ext_ram_w));
}



/*******************************************************************************
    Inputs
*******************************************************************************/

u8 advision_state::controller_r()
{
	u8 data = m_joy->read();

	// some of the buttons share the same bitmask as joystick directions
	if (~data & 0x01) data &= 0xcf; // button 1: down + up
	if (~data & 0x02) data &= 0xaf; // button 2: down + right
	if (~data & 0x04) data &= 0x6f; // button 4: down + left

	return data | 0x07;
}

static INPUT_PORTS_START( advision )
	PORT_START("JOY")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // u
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // r
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) // l
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) // d
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY

	PORT_START("CONF")
	PORT_CONFNAME( 0x3f, 20, "Screen Width" ) PORT_CHANGED_MEMBER(DEVICE_SELF, advision_state, set_screensize, 0) // factors of 60
	PORT_CONFSETTING(    60, "160" )
	PORT_CONFSETTING(    30, "320" )
	PORT_CONFSETTING(    20, "480" )
	PORT_CONFSETTING(    15, "640" )
	PORT_CONFSETTING(    12, "800" )
	PORT_CONFSETTING(    10, "960" )
	PORT_CONFNAME( 0xc0, 0x80, "LED Height" ) PORT_CHANGED_MEMBER(DEVICE_SELF, advision_state, set_screensize, 0)
	PORT_CONFSETTING(    0x00, "25%" )
	PORT_CONFSETTING(    0x40, "50%" )
	PORT_CONFSETTING(    0x80, "75%" )
	PORT_CONFSETTING(    0xc0, "100%" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Initialization
*******************************************************************************/

void advision_state::machine_start()
{
	// allocate display buffer
	m_display = std::make_unique<u8 []>(DISPLAY_WIDTH * 40);
	std::fill_n(m_display.get(), DISPLAY_WIDTH * 40, 0);
	save_pointer(NAME(m_display), DISPLAY_WIDTH * 40);

	// allocate external RAM
	m_ext_ram.resize(0x400);
	save_item(NAME(m_ext_ram));

	save_item(NAME(m_led_output));
	save_item(NAME(m_led_latch));

	save_item(NAME(m_rambank));
	save_item(NAME(m_video_strobe));
	save_item(NAME(m_video_enable));
	save_item(NAME(m_video_bank));
	save_item(NAME(m_sound_cmd));
	save_item(NAME(m_video_hpos));
}

void advision_state::machine_reset()
{
	m_video_hpos = 0;
	m_led_update->adjust(attotime::never);

	// reset sound CPU
	m_soundcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
}



/*******************************************************************************
    Machine Config
*******************************************************************************/

void advision_state::advision(machine_config &config)
{
	// basic machine hardware
	I8048(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &advision_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &advision_state::io_map);
	m_maincpu->p1_in_cb().set(FUNC(advision_state::controller_r));
	m_maincpu->p1_out_cb().set(FUNC(advision_state::bankswitch_w));
	m_maincpu->p2_out_cb().set(FUNC(advision_state::av_control_w));
	m_maincpu->t1_in_cb().set(FUNC(advision_state::vsync_r));

	COP411(config, m_soundcpu, 200000); // COP411L-KCN/N, R11=82k, C8=56pF
	m_soundcpu->set_config(COP400_CKI_DIVISOR_4, COP400_CKO_RAM_POWER_SUPPLY, false);
	m_soundcpu->read_l().set(FUNC(advision_state::sound_cmd_r));
	m_soundcpu->write_g().set(FUNC(advision_state::sound_g_w));
	m_soundcpu->write_d().set(FUNC(advision_state::sound_d_w));

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(4 * 14); // see notes
	m_screen->set_vblank_time(0);
	m_screen->set_size(960, 40 * 4 + 4);
	m_screen->set_visarea(0, 480 - 1, 0, 40 * 4 - 0); // default setting
	m_screen->set_screen_update(FUNC(advision_state::screen_update));
	m_screen->screen_vblank().set(FUNC(advision_state::vblank));

	TIMER(config, "mirror_sync").configure_generic(nullptr);
	TIMER(config, "led_update").configure_generic(FUNC(advision_state::led_update));
	TIMER(config, "led_off").configure_generic(FUNC(advision_state::led_off));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "volume", 0.25);
	FILTER_VOLUME(config, m_volume).add_route(ALL_OUTPUTS, "speaker", 1.0);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "advision_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("advision");
}



/*******************************************************************************
    ROMs
*******************************************************************************/

ROM_START( advision )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	ROM_LOAD( "ins8048-11kdp_n.u5", 0x000, 0x400, CRC(279e33d1) SHA1(bf7b0663e9125c9bfb950232eab627d9dbda8460) ) // "<natsemi logo> /B8225 \\ INS8048-11KDP/N"

	ROM_REGION( 0x200, "soundcpu", 0 )
	ROM_LOAD( "cop411l-kcn_n.u8", 0x000, 0x200, CRC(81e95975) SHA1(8b6f8c30dd3e9d8e43f1ea20fba2361b383790eb) ) // "<natsemi logo> /B8223 \\ COP411L-KCN/N"
ROM_END

} // anonymous namespace



/*******************************************************************************
    Driver
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME            FLAGS
SYST( 1982, advision, 0,      0,      advision, advision, advision_state, empty_init, "Entex", "Adventure Vision", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
