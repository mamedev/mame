// license:BSD-3-Clause
// copyright-holders:Stefan Jokisch
/***************************************************************************

Atari Destroyer Driver

TODO:
- missing language roms means DIP switches related to these do not function

***************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/74259.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "machine/netlist.h"

#include "netlist/nl_setup.h"
#include "nl_destroyr.h"

#include "destroyr.lh"


namespace {

class destroyr_state : public driver_device
{
public:
	destroyr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_watchdog(*this, "watchdog")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_inputs(*this, "IN%u", 0U)
		, m_paddle(*this, "PADDLE")
		, m_alpha_num_ram(*this, "alpha_nuram")
		, m_major_obj_ram(*this, "major_obj_ram")
		, m_minor_obj_ram(*this, "minor_obj_ram")
		, m_sound_motor_speed(*this, "sound_nl:motor_speed")
		, m_sound_noise(*this, "sound_nl:noise")
		, m_sound_attract(*this, "sound_nl:attract")
		, m_sound_songate(*this, "sound_nl:songate")
		, m_sound_launch(*this, "sound_nl:launch")
		, m_sound_explo(*this, "sound_nl:explo")
		, m_sound_sonlat(*this, "sound_nl:sonlat")
		, m_sound_hexplo(*this, "sound_nl:hexplo")
		, m_sound_lexplo(*this, "sound_nl:lexplo")
	{ }

	void destroyr(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void main_map(address_map &map) ATTR_COLD;

	void misc_w(uint8_t data);
	void cursor_load_w(uint8_t data);
	void interrupt_ack_w(uint8_t data);
	uint8_t input_r(offs_t offset);
	uint8_t scanline_r();

	void palette_init(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	TIMER_CALLBACK_MEMBER(dial_callback);
	TIMER_CALLBACK_MEMBER(frame_callback);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline_irq);

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_ioport_array<3> m_inputs;
	required_ioport m_paddle;

	/* memory pointers */
	required_shared_ptr<uint8_t> m_alpha_num_ram;
	required_shared_ptr<uint8_t> m_major_obj_ram;
	required_shared_ptr<uint8_t> m_minor_obj_ram;

	/* audio triggers */
	required_device<netlist_mame_logic_input_device> m_sound_motor_speed;
	required_device<netlist_mame_logic_input_device> m_sound_noise;
	required_device<netlist_mame_logic_input_device> m_sound_attract;
	required_device<netlist_mame_logic_input_device> m_sound_songate;
	required_device<netlist_mame_logic_input_device> m_sound_launch;
	required_device<netlist_mame_logic_input_device> m_sound_explo;
	required_device<netlist_mame_logic_input_device> m_sound_sonlat;
	required_device<netlist_mame_logic_input_device> m_sound_hexplo;
	required_device<netlist_mame_logic_input_device> m_sound_lexplo;

	/* video-related */
	int            m_cursor;
	int            m_wavemod;

	/* misc */
	int            m_potmask[2];
	int            m_potsense[2];
	int            m_attract;
	emu_timer      *m_dial_timer;
	emu_timer      *m_frame_timer;
};


uint32_t destroyr_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	/* draw major objects */
	for (int i = 0; i < 16; i++)
	{
		int attr = m_major_obj_ram[2 * i + 0] ^ 0xff;
		int horz = m_major_obj_ram[2 * i + 1];

		int num = attr & 3;
		int scan = attr & 4;
		int flipx = attr & 8;

		if (scan == 0)
		{
			if (horz >= 192)
				horz -= 256;
		}
		else
		{
			if (horz < 192)
				continue;
		}

		m_gfxdecode->gfx(2)->transpen(bitmap,cliprect, num, 0, flipx, 0, horz, 16 * i, 0);
	}

	/* draw alpha numerics */
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 32; j++)
		{
			int num = m_alpha_num_ram[32 * i + j];

			m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, num, 0, 0, 0, 8 * j, 8 * i, 0);
		}
	}

	/* draw minor objects */
	for (int i = 0; i < 2; i++)
	{
		int num = i << 4 | (m_minor_obj_ram[i + 0] & 0xf);
		int horz = 256 - m_minor_obj_ram[i + 2];
		int vert = 256 - m_minor_obj_ram[i + 4];

		m_gfxdecode->gfx(1)->transpen(bitmap,cliprect, num, 0, 0, 0, horz, vert, 0);
	}

	/* draw waves */
	for (int i = 0; i < 4; i++)
	{
		m_gfxdecode->gfx(3)->transpen(bitmap,cliprect, m_wavemod ? 1 : 0, 0, 0, 0, 64 * i, 0x4e, 0);
	}

	/* draw cursor */
	for (int i = 0; i < 256; i++)
	{
		if (i & 4)
			bitmap.pix(m_cursor ^ 0xff, i) = 7;
	}
	return 0;
}


TIMER_CALLBACK_MEMBER(destroyr_state::dial_callback)
{
	int dial = param;

	/* Analog inputs come from the player's depth control potentiometer.
	   The voltage is compared to a voltage ramp provided by a discrete
	   analog circuit that conditions the VBLANK signal. When the ramp
	   voltage exceeds the input voltage an NMI signal is generated. The
	   computer then reads the VSYNC data functions to tell where the
	   cursor should be located. */

	m_potsense[dial] = 1;

	if (m_potmask[dial])
	{
		m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
	}
}


TIMER_CALLBACK_MEMBER(destroyr_state::frame_callback)
{
	m_potsense[0] = 0;
	m_potsense[1] = 0;

	/* PCB supports two dials, but cab has only got one */
	m_dial_timer->adjust(m_screen->time_until_pos(m_paddle->read()));
	m_frame_timer->adjust(m_screen->time_until_pos(0));
}


TIMER_DEVICE_CALLBACK_MEMBER(destroyr_state::scanline_irq)
{
	// 16V clocks LS74 flip-flop with D = 32V and /Q output connected to /IRQ on 6800
	m_maincpu->set_input_line(M6800_IRQ_LINE, BIT(param, 5) ? ASSERT_LINE : CLEAR_LINE);
}


void destroyr_state::machine_reset()
{
	m_frame_timer->adjust(m_screen->time_until_pos(0));

	m_cursor = 0;
	m_wavemod = 0;
	m_potmask[0] = 0;
	m_potmask[1] = 0;
	m_potsense[0] = 0;
	m_potsense[1] = 0;
	m_attract = 0;
}


void destroyr_state::misc_w(uint8_t data)
{
	/* bits 0 to 2 connect to the sound circuits */
	m_attract = BIT(data, 0);
	m_sound_attract->write(m_attract);
	m_sound_noise->write(BIT(data, 1));
	m_sound_motor_speed->write(BIT(data, 2));
	m_potmask[0] = BIT(data, 3);
	m_wavemod = BIT(data, 4);
	m_potmask[1] = BIT(data, 5);

	machine().bookkeeping().coin_lockout_w(0, !m_attract);
	machine().bookkeeping().coin_lockout_w(1, !m_attract);
}


void destroyr_state::cursor_load_w(uint8_t data)
{
	m_cursor = data;
	m_watchdog->watchdog_reset();
}


void destroyr_state::interrupt_ack_w(uint8_t data)
{
	m_maincpu->set_input_line(M6800_IRQ_LINE, CLEAR_LINE);
}


uint8_t destroyr_state::input_r(offs_t offset)
{
	if (offset & 1)
	{
		return m_inputs[1]->read();
	}

	else
	{
		uint8_t ret = m_inputs[0]->read();
		ret |= (m_potsense[0] && m_potmask[0]) ? (1 << 2) : 0;
		ret |= (m_potsense[1] && m_potmask[1]) ? (1 << 3) : 0;
		return ret;
	}
}


uint8_t destroyr_state::scanline_r()
{
	return m_screen->vpos();
}


void destroyr_state::main_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x00ff).mirror(0xf00).ram();
	map(0x1000, 0x1001).mirror(0xffe).r(FUNC(destroyr_state::input_r));
	map(0x1000, 0x1007).mirror(0xff0).w("outlatch", FUNC(f9334_device::write_d0));
	map(0x1008, 0x1008).mirror(0xff7).w(FUNC(destroyr_state::misc_w));
	map(0x2000, 0x2000).mirror(0xfff).portr("IN2");
	map(0x3000, 0x30ff).mirror(0xf00).writeonly().share("alpha_nuram");
	map(0x4000, 0x401f).mirror(0xfe0).writeonly().share("major_obj_ram");
	map(0x5000, 0x5000).mirror(0xff8).w(FUNC(destroyr_state::cursor_load_w));
	map(0x5001, 0x5001).mirror(0xff8).w(FUNC(destroyr_state::interrupt_ack_w));
	map(0x5002, 0x5007).mirror(0xff8).writeonly().share("minor_obj_ram");
	map(0x6000, 0x6000).mirror(0xfff).r(FUNC(destroyr_state::scanline_r));
	map(0x7000, 0x7fff).rom();
}


static INPUT_PORTS_START( destroyr )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED ) /* call 7400 */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNUSED ) /* potsense1 */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNUSED ) /* potsense2 */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0xc0, 0x80, "Extended Play" ) PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING( 0x40, "1500 points" )
	PORT_DIPSETTING( 0x80, "2500 points" )
	PORT_DIPSETTING( 0xc0, "3500 points" )
	PORT_DIPSETTING( 0x00, "never" )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW,  IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_NAME("Speed Control") PORT_TOGGLE
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")

	PORT_START("IN2")
	PORT_DIPNAME( 0x03, 0x02, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:4,3")
	PORT_DIPSETTING( 0x03, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING( 0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING( 0x01, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING( 0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x08, "Play Time" ) PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING( 0x00, "50 seconds" )
	PORT_DIPSETTING( 0x04, "75 seconds" )
	PORT_DIPSETTING( 0x08, "100 seconds" )
	PORT_DIPSETTING( 0x0c, "125 seconds" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Language ) ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING( 0x30, DEF_STR( German ) )
	PORT_DIPSETTING( 0x20, DEF_STR( French ) )
	PORT_DIPSETTING( 0x10, DEF_STR( Spanish ) )
	PORT_DIPSETTING( 0x00, DEF_STR( English ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PADDLE")
	PORT_BIT( 0xff, 0x00, IPT_PADDLE_V ) PORT_MINMAX(0,160) PORT_SENSITIVITY(30) PORT_KEYDELTA(10) PORT_CENTERDELTA(0) PORT_REVERSE
INPUT_PORTS_END


static const gfx_layout destroyr_alpha_num_layout =
{
	8, 8,     /* width, height */
	64,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		0x4, 0x5, 0x6, 0x7, 0xC, 0xD, 0xE, 0xF
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80      /* increment */
};


static const gfx_layout destroyr_minor_object_layout =
{
	16, 16,   /* width, height */
	32,       /* total         */
	1,        /* planes        */
	{ 0 },    /* plane offsets */
	{
		0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F,
		0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F
	},
	{
		0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
		0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0, 0x1e0
	},
	0x200     /* increment */
};

static const uint32_t destroyr_major_object_layout_xoffset[64] =
{
	0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
	0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
	0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E,
	0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E,
	0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E,
	0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E,
	0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6C, 0x6E,
	0x70, 0x72, 0x74, 0x76, 0x78, 0x7A, 0x7C, 0x7E
};

static const gfx_layout destroyr_major_object_layout =
{
	64, 16,   /* width, height */
	4,        /* total         */
	2,        /* planes        */
	{ 1, 0 },  /* plane offsets */
	EXTENDED_XOFFS,
	{
		0x000, 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380,
		0x400, 0x480, 0x500, 0x580, 0x600, 0x680, 0x700, 0x780
	},
	0x0800,   /* increment */
	destroyr_major_object_layout_xoffset,
	nullptr
};

static const uint32_t destroyr_waves_layout_xoffset[64] =
{
	0x00, 0x01, 0x02, 0x03, 0x08, 0x09, 0x0A, 0x0B,
	0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1A, 0x1B,
	0x20, 0x21, 0x22, 0x23, 0x28, 0x29, 0x2A, 0x2B,
	0x30, 0x31, 0x32, 0x33, 0x38, 0x39, 0x3A, 0x3B,
	0x40, 0x41, 0x42, 0x43, 0x48, 0x49, 0x4A, 0x4B,
	0x50, 0x51, 0x52, 0x53, 0x58, 0x59, 0x5A, 0x5B,
	0x60, 0x61, 0x62, 0x63, 0x68, 0x69, 0x6A, 0x6B,
	0x70, 0x71, 0x72, 0x73, 0x78, 0x79, 0x7A, 0x7B
};

static const gfx_layout destroyr_waves_layout =
{
	64, 2,    /* width, height */
	2,        /* total         */
	1,        /* planes        */
	{ 0 },
	EXTENDED_XOFFS,
	{ 0x00, 0x80 },
	0x04,     /* increment */
	destroyr_waves_layout_xoffset,
	nullptr
};


static GFXDECODE_START( gfx_destroyr )
	GFXDECODE_ENTRY( "gfx1", 0, destroyr_alpha_num_layout, 4, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, destroyr_minor_object_layout, 4, 1 )
	GFXDECODE_ENTRY( "gfx3", 0, destroyr_major_object_layout, 0, 1 )
	GFXDECODE_ENTRY( "gfx4", 0, destroyr_waves_layout, 4, 1 )
GFXDECODE_END


void destroyr_state::palette_init(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00));   // major objects
	palette.set_pen_color(1, rgb_t(0x50, 0x50, 0x50));
	palette.set_pen_color(2, rgb_t(0xAF, 0xAF, 0xAF));
	palette.set_pen_color(3, rgb_t(0xFF ,0xFF, 0xFF));
	palette.set_pen_color(4, rgb_t(0x00, 0x00, 0x00));   // alpha numerics, waves, minor objects
	palette.set_pen_color(5, rgb_t(0xFF, 0xFF, 0xFF));
	palette.set_pen_color(6, rgb_t(0x00, 0x00, 0x00));   // cursor
	palette.set_pen_color(7, rgb_t(0x78, 0x78, 0x78));
}


void destroyr_state::machine_start()
{
	m_dial_timer = timer_alloc(FUNC(destroyr_state::dial_callback), this);
	m_frame_timer = timer_alloc(FUNC(destroyr_state::frame_callback), this);

	save_item(NAME(m_cursor));
	save_item(NAME(m_wavemod));
	save_item(NAME(m_attract));
	save_item(NAME(m_potmask));
	save_item(NAME(m_potsense));
}

void destroyr_state::destroyr(machine_config &config)
{
	/* basic machine hardware */
	M6800(config, m_maincpu, 12.096_MHz_XTAL / 16);
	m_maincpu->set_addrmap(AS_PROGRAM, &destroyr_state::main_map);

	TIMER(config, "scantimer").configure_scanline(FUNC(destroyr_state::scanline_irq), m_screen, 16, 32);

	f9334_device &outlatch(F9334(config, "outlatch")); // F8
	outlatch.q_out_cb<0>().set_output("led0").invert(); // LED 1
	outlatch.q_out_cb<1>().set_output("led1").invert(); // LED 2 (no second LED present on cab)
	outlatch.q_out_cb<2>().set(m_sound_songate, FUNC(netlist_mame_logic_input_device::write));
	outlatch.q_out_cb<3>().set(m_sound_launch, FUNC(netlist_mame_logic_input_device::write));
	outlatch.q_out_cb<4>().set(m_sound_explo, FUNC(netlist_mame_logic_input_device::write));
	outlatch.q_out_cb<5>().set(m_sound_sonlat, FUNC(netlist_mame_logic_input_device::write));
	outlatch.q_out_cb<6>().set(m_sound_hexplo, FUNC(netlist_mame_logic_input_device::write));
	outlatch.q_out_cb<7>().set(m_sound_lexplo, FUNC(netlist_mame_logic_input_device::write));

	WATCHDOG_TIMER(config, m_watchdog);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(12.096_MHz_XTAL / 2, 384, 0, 256, 263, 0, 240);
	m_screen->set_screen_update(FUNC(destroyr_state::screen_update));
	m_screen->set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_destroyr);
	PALETTE(config, m_palette, FUNC(destroyr_state::palette_init), 8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NETLIST_SOUND(config, "sound_nl", 48000)
		.set_source(NETLIST_NAME(destroyr))
		.add_route(ALL_OUTPUTS, "mono", 1.0);

	NETLIST_LOGIC_INPUT(config, "sound_nl:motor_speed", "MOTOR_SPEED.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:noise", "NOISE.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:attract", "ATTRACT.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:songate", "SONGATE.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:launch", "LAUNCH.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:explo", "EXPLO.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:sonlat", "SONLAT.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:hexplo", "HE.IN", 0);
	NETLIST_LOGIC_INPUT(config, "sound_nl:lexplo", "LE.IN", 0);

	NETLIST_STREAM_OUTPUT(config, "sound_nl:cout0", 0, "OUTPUT").set_mult_offset(1.0, 0.0);
}


ROM_START( destroyr )
	ROM_REGION( 0x8000, "maincpu", 0 )  /* program code */
	ROM_LOAD( "030138.rom",0x7000, 0x0800, NO_DUMP ) // optional add-on translation rom
	ROM_LOAD( "030146-01.c3", 0x7800, 0x0800, CRC(e560c712) SHA1(0505ab57eee5421b4ff4e87d14505e02b18fd54c) )

	ROM_REGION( 0x0400, "gfx1", 0 )     /* alpha numerics */
	ROM_LOAD( "030135-01.p4", 0x0000, 0x0400, CRC(184824cf) SHA1(713cfd1d41ef7b1c345ea0038b652c4ba3f08301) )

	ROM_REGION( 0x0800, "gfx2", 0 )     /* minor objects */
	ROM_LOAD( "030132-01.f4", 0x0000, 0x0400, CRC(e09d3d55) SHA1(b26013397ef2cb32d0416ecb118387b9c2dffa9a) )
	ROM_LOAD( "030132-01.k4", 0x0400, 0x0400, CRC(e09d3d55) SHA1(b26013397ef2cb32d0416ecb118387b9c2dffa9a) ) // identical to f4

	ROM_REGION( 0x0400, "gfx3", 0 )     /* major objects */
	ROM_LOAD_NIB_HIGH( "030134-01.p8", 0x0000, 0x0400, CRC(6259e007) SHA1(049f5f7160305cb4f4b499dd113cb11eea73fc95) )
	ROM_LOAD_NIB_LOW ( "030133-01.n8", 0x0000, 0x0400, CRC(108d3e2c) SHA1(8c993369d37c6713670483af78e6d04d38f4b4fc) )

	ROM_REGION( 0x0020, "gfx4", 0 )     /* waves */
	ROM_LOAD( "030136-01.k2", 0x0000, 0x0020, CRC(532c11b1) SHA1(18ab5369a3f2cfcc9a44f38fa8649524bea5b203) )

	ROM_REGION( 0x0100, "syncprom", 0 )    /* used for vsync/vblank signals */
	ROM_LOAD( "030131-01.m1", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )
ROM_END

ROM_START( destroyr1 )
	ROM_REGION( 0x8000, "maincpu", 0 )  /* program code */
	ROM_LOAD( "030138.rom",0x7000, 0x0800, NO_DUMP ) // optional add-on translation rom
	ROM_LOAD_NIB_HIGH( "030142-01.f3", 0x7800, 0x0400, CRC(9e9a08d3) SHA1(eb31bab1537caf43ab8c3d23a6c9cc2009fcb98e) )
	ROM_LOAD_NIB_LOW ( "030141-01.e2", 0x7800, 0x0400, CRC(c924fbce) SHA1(53aa9a3c4c6e90fb94500ddfa6c2ae3076eee2ef) )
	ROM_LOAD_NIB_HIGH( "030144-01.j3", 0x7c00, 0x0400, CRC(0c7135c6) SHA1(6a0180353a0a6f34639dadc23179f6323aae8d62) )
	ROM_LOAD_NIB_LOW ( "030143-01.h2", 0x7c00, 0x0400, CRC(b946e6f0) SHA1(b906024bb0e03a644fff1d5516637c24916b096e) )

	ROM_REGION( 0x0400, "gfx1", 0 )     /* alpha numerics */
	ROM_LOAD( "030135-01.p4", 0x0000, 0x0400, CRC(184824cf) SHA1(713cfd1d41ef7b1c345ea0038b652c4ba3f08301) )

	ROM_REGION( 0x0800, "gfx2", 0 )     /* minor objects */
	ROM_LOAD( "030132-01.f4", 0x0000, 0x0400, CRC(e09d3d55) SHA1(b26013397ef2cb32d0416ecb118387b9c2dffa9a) )
	ROM_LOAD( "030132-01.k4", 0x0400, 0x0400, CRC(e09d3d55) SHA1(b26013397ef2cb32d0416ecb118387b9c2dffa9a) ) // identical to f4

	ROM_REGION( 0x0400, "gfx3", 0 )     /* major objects */
	ROM_LOAD_NIB_HIGH( "030134-01.p8", 0x0000, 0x0400, CRC(6259e007) SHA1(049f5f7160305cb4f4b499dd113cb11eea73fc95) )
	ROM_LOAD_NIB_LOW ( "030133-01.n8", 0x0000, 0x0400, CRC(108d3e2c) SHA1(8c993369d37c6713670483af78e6d04d38f4b4fc) )

	ROM_REGION( 0x0020, "gfx4", 0 )     /* waves */
	ROM_LOAD( "030136-01.k2", 0x0000, 0x0020, CRC(532c11b1) SHA1(18ab5369a3f2cfcc9a44f38fa8649524bea5b203) )

	ROM_REGION( 0x0100, "syncprom", 0 )    /* used for vsync/vblank signals */
	ROM_LOAD( "030131-01.m1", 0x0000, 0x0100, CRC(b8094b4c) SHA1(82dc6799a19984f3b204ee3aeeb007e55afc8be3) )
ROM_END

} // anonymous namespace


GAMEL( 1977, destroyr,  0,        destroyr, destroyr, destroyr_state, empty_init, ORIENTATION_FLIP_X, "Atari", "Destroyer (Atari, version O2)", MACHINE_SUPPORTS_SAVE, layout_destroyr )
GAMEL( 1977, destroyr1, destroyr, destroyr, destroyr, destroyr_state, empty_init, ORIENTATION_FLIP_X, "Atari", "Destroyer (Atari, version O1)", MACHINE_SUPPORTS_SAVE, layout_destroyr )
