// license: BSD-3-Clause
// copyright-holders: Mathis Rosenhauer, Dirk Best
// thanks-to: Jonathan Gevaryahu
/***************************************************************************

    Beezer

    (c) 1982 Pacific Polytechnical Corp. (PPC) / Tong Electronic

    Notes:
    - To enter test mode, hold down 1P Start and 2P Start, then reset
      (only works for version 9.0)
    - One of the ROMs contains a message that this game was created
      by "Pacific Polytechnical Corporation, Santa Cruz"

    TODO:
    - Improve sound (filters? A reference recording would be nice)
    - Schematics in the sound area seem incomplete, there are
      several unknown connections
    - Watchdog timing (controlled by a 555)
    - Figure out differences between the two sets (test mode isn't
      working in beezer1, instruction screen is different)
    - Verify accuracy of colors

***************************************************************************/

#include "emu.h"

#include "cpu/m6809/m6809.h"
#include "machine/input_merger.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "machine/6522via.h"
#include "machine/6840ptm.h"
#include "sound/mm5837.h"
#include "sound/dac76.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class beezer_state : public driver_device
{
public:
	beezer_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_sysbank(*this, "sysbank"),
		m_banked_roms(*this, "banked"),
		m_rombank{ {*this, "rombank_f1"}, {*this, "rombank_f3"}, {*this, "rombank_e1"}, {*this, "rombank_e3"}, {*this, "rombank_e5"}, {*this, "rombank_f5"}, {*this, "rombank_f7"} },
		m_via_system(*this, "via_u6"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_audiocpu(*this, "audiocpu"),
		m_via_audio(*this, "via_u18"),
		m_ptm(*this, "ptm"),
		m_dac(*this, "dac"),
		m_dac_timer(nullptr),
		m_scanline_timer(nullptr),
		m_count(0), m_noise(0),
		m_pbus(0xff), m_x(0), m_y(0), m_z(0)
	{
		m_ch_sign[0] = m_ch_sign[1] = m_ch_sign[2] = m_ch_sign[3] = 0;
		m_dac_data[0] = m_dac_data[1] = m_dac_data[2] = m_dac_data[3] = 0;
	}

	TIMER_CALLBACK_MEMBER(scanline_cb);
	TIMER_CALLBACK_MEMBER(dac_update_cb);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void palette_init(palette_device &palette);
	void palette_w(offs_t offset, uint8_t data);
	uint8_t line_r();

	void noise_w(int state);
	void dac_w(offs_t offset, uint8_t data);
	uint8_t via_audio_pa_r();
	void via_audio_pa_w(uint8_t data);
	void via_audio_pb_w(uint8_t data);
	void ptm_o1_w(int state);
	void ptm_o2_w(int state);
	void ptm_o3_w(int state);
	void dmod_clr_w(int state);
	void dmod_data_w(int state);

	uint8_t via_system_pa_r();
	uint8_t via_system_pb_r();
	void via_system_pa_w(uint8_t data);
	void via_system_pb_w(uint8_t data);
	void bankswitch_w(uint8_t data);

	void beezer(machine_config &config);
	void main_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	memory_view m_sysbank;
	required_memory_region m_banked_roms;
	required_memory_bank m_rombank[7];
	required_device<via6522_device> m_via_system;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<cpu_device> m_audiocpu;
	required_device<via6522_device> m_via_audio;
	required_device<ptm6840_device> m_ptm;
	required_device<dac76_device> m_dac;

	double m_weights_r[3];
	double m_weights_g[3];
	double m_weights_b[2];

	emu_timer *m_dac_timer;
	emu_timer *m_scanline_timer;

	int m_ch_sign[4];
	uint8_t m_dac_data[4];
	int m_count;
	int m_noise;

	uint8_t m_pbus;
	int m_x, m_y, m_z;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

void beezer_state::main_map(address_map &map)
{
	map(0x0000, 0xbfff).ram().share("videoram");
	map(0xc000, 0xcfff).view(m_sysbank);
	m_sysbank[0](0xc600, 0xc600).mirror(0x1ff).w("watchdog", FUNC(watchdog_timer_device::reset_w));
	m_sysbank[0](0xc800, 0xc80f).mirror(0x1f0).w(FUNC(beezer_state::palette_w));
	m_sysbank[0](0xca00, 0xca00).mirror(0x1ff).r(FUNC(beezer_state::line_r));
	m_sysbank[0](0xce00, 0xce0f).mirror(0x1f0).m("via_u6", FUNC(via6522_device::map));
	m_sysbank[1](0xc000, 0xcfff).bankr("rombank_f1");
	m_sysbank[2](0xc000, 0xcfff).bankr("rombank_f3");
	m_sysbank[3](0xc000, 0xcfff).bankr("rombank_e1");
	m_sysbank[4](0xc000, 0xcfff).bankr("rombank_e3");
	m_sysbank[5](0xc000, 0xcfff).bankr("rombank_e5");
	m_sysbank[6](0xc000, 0xcfff).bankr("rombank_f5");
	m_sysbank[7](0xc000, 0xcfff).bankr("rombank_f7");
	map(0xd000, 0xdfff).rom().region("maincpu", 0x0000).w(FUNC(beezer_state::bankswitch_w)); // g1
	map(0xe000, 0xefff).rom().region("maincpu", 0x1000); // g3
	map(0xf000, 0xffff).rom().region("maincpu", 0x2000); // g5
}

void beezer_state::sound_map(address_map &map)
{
	map(0x0000, 0x07ff).ram(); // 0d
	map(0x0800, 0x0fff).ram(); // 2d, optional (can be rom)
	map(0x1000, 0x1007).mirror(0x07f8).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0x1800, 0x180f).mirror(0x07f0).m(m_via_audio, FUNC(via6522_device::map));
	map(0x8000, 0x8003).mirror(0x1ffc).w(FUNC(beezer_state::dac_w));
//  map(0xa000, 0xbfff).rom(); // 2d (can be ram, unpopulated)
//  map(0xc000, 0xdfff).rom(); // 4d (unpopulated)
	map(0xe000, 0xffff).rom().region("audiocpu", 0); // 6d
}


//**************************************************************************
//  INPUTS
//**************************************************************************

static INPUT_PORTS_START( beezer )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE
	PORT_START("IN2")
	PORT_BIT( 0x0f, 0x00, IPT_TRACKBALL_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE

	// DSWA isn't populated on the board and is pulled to 0xff with a resistor pack
	PORT_START("DSWA")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SWB:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SWB:3")
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWB:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x10, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(    0x20, "30000" )
	PORT_DIPSETTING(    0x10, "60000" )
	PORT_DIPSETTING(    0x00, "90000" )
	PORT_DIPSETTING(    0x30, DEF_STR( No ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

TIMER_CALLBACK_MEMBER( beezer_state::scanline_cb )
{
	const int scanline = m_screen->vpos();

	// TDISP, each 32 lines
	m_via_system->write_ca2((scanline & 32) ? 1 : 0);

	// actually unused by the game (points to a tight loop)
	if (scanline == 240)
		m_maincpu->set_input_line(M6809_FIRQ_LINE, ASSERT_LINE);
	else
		m_maincpu->set_input_line(M6809_FIRQ_LINE, CLEAR_LINE);


	m_scanline_timer->adjust(m_screen->time_until_pos(scanline + 1));
}

uint32_t beezer_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for (int x = cliprect.min_x; x <= cliprect.max_x; x += 2)
		{
			bitmap.pix(y, x + 1) = m_videoram[0x80 * x + y] & 0x0f;
			bitmap.pix(y, x + 0) = m_videoram[0x80 * x + y] >> 4;
		}
	}

	return 0;
}

void beezer_state::palette_init(palette_device &device)
{
	const int resistances_rg[3] = { 1200, 560, 330 };
	const int resistances_b[2]  = { 560, 330 };

	// calculate coefficients for later use
	compute_resistor_weights(0, 255, -1.0,
		3, resistances_rg, m_weights_r, 680, 150,
		3, resistances_rg, m_weights_g, 680, 150,
		2, resistances_b,  m_weights_b, 680, 150);
}

void beezer_state::palette_w(offs_t offset, uint8_t data)
{
	int r = combine_weights(m_weights_r, BIT(data, 0), BIT(data, 1), BIT(data, 2));
	int g = combine_weights(m_weights_g, BIT(data, 3), BIT(data, 4), BIT(data, 5));
	int b = combine_weights(m_weights_b, BIT(data, 6), BIT(data, 7));

	m_palette->set_pen_color(offset, rgb_t(r, g, b));
}

uint8_t beezer_state::line_r()
{
	// d2 to d7 connected to hex buffer u34
	return m_screen->vpos() & 0xfc;
}


//**************************************************************************
//  AUDIO
//**************************************************************************

TIMER_CALLBACK_MEMBER( beezer_state::dac_update_cb )
{
	// channel multiplexer at u52
	int ch = m_count++ & 3;

	m_dac->update();

	m_dac->b1_w(BIT(m_dac_data[ch], 6));
	m_dac->b2_w(BIT(m_dac_data[ch], 5));
	m_dac->b3_w(BIT(m_dac_data[ch], 4));
	m_dac->b4_w(BIT(m_dac_data[ch], 3));
	m_dac->b5_w(BIT(m_dac_data[ch], 2));
	m_dac->b6_w(BIT(m_dac_data[ch], 1));
	m_dac->b7_w(BIT(m_dac_data[ch], 0));

	m_dac->sb_w(m_ch_sign[ch] ^ BIT(m_dac_data[ch], 7));
}

void beezer_state::noise_w(int state)
{
	m_noise = state;
	m_via_audio->write_pb6(m_noise);
}

void beezer_state::dac_w(offs_t offset, uint8_t data)
{
	m_dac_data[offset] = data;
}

uint8_t beezer_state::via_audio_pa_r()
{
	return m_pbus;
}

void beezer_state::via_audio_pa_w(uint8_t data)
{
	m_pbus = data;
}

void beezer_state::via_audio_pb_w(uint8_t data)
{
	// bit 0 - dmod disable
	// bit 1 - fm or am
	// bit 2 - am
	// bit 3 - fmsel0
	// bit 4 - fmsel1
	// -- above bits not handled and only partially visible on the schematic

	// bit 7 - noise gate on rising edge to ptm c1
	if (m_ch_sign[0] == 0 && (BIT(data, 7) == 1))
		m_ptm->set_c1(m_noise);

	m_ch_sign[0] = BIT(data, 7);
}

void beezer_state::ptm_o1_w(int state)
{
	m_ch_sign[1] = state;
}

void beezer_state::ptm_o2_w(int state)
{
	// on rising edge, enable noise input to ptm c3
	if (m_ch_sign[2] == 0 && state == 1)
		m_ptm->set_c3(m_noise);

	m_ch_sign[2] = state;
}

void beezer_state::ptm_o3_w(int state)
{
	m_ch_sign[3] = state;
}

void beezer_state::dmod_clr_w(int state)
{
	// schematics don't show where this is connected
}

void beezer_state::dmod_data_w(int state)
{
	// schematics don't show where this is connected
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

uint8_t beezer_state::via_system_pa_r()
{
	uint8_t data = 0;

	data |= 1 << 4; // N/C
	data |= m_z << 5;
	data |= m_y << 6;
	data |= m_x << 7;

	return data;
}

void beezer_state::via_system_pa_w(uint8_t data)
{
	// bit 3, audio cpu reset line
	m_audiocpu->set_input_line(INPUT_LINE_RESET, BIT(data, 3) ? CLEAR_LINE : ASSERT_LINE);

	// bit 2, enable for ls139
	if (BIT(data, 2) == 0)
	{
		// bit 0-1, input selection
		switch (data & 0x03)
		{
		case 0:
			m_pbus = ioport("IN0")->read();
			break;
		case 1:
			m_pbus = ioport("IN1")->read() | (ioport("IN2")->read() << 4);
			break;
		case 2:
			m_pbus = ioport("DSWB")->read();
			break;
		case 3:
			m_pbus = ioport("DSWA")->read();
			break;
		}
	}
}

uint8_t beezer_state::via_system_pb_r()
{
	return m_pbus;
}

void beezer_state::via_system_pb_w(uint8_t data)
{
	m_pbus = data;
}

void beezer_state::bankswitch_w(uint8_t data)
{
	m_x = BIT(data, 3);
	m_y = BIT(data, 4);
	m_z = BIT(data, 5);

	m_sysbank.select(data & 0x07);

	for (int i = 0; i < 7; i++)
		m_rombank[i]->set_entry(m_x);
}

void beezer_state::machine_start()
{
	// configure rom banks
	for (int i = 0; i < 7; i++)
		m_rombank[i]->configure_entries(0, 2, m_banked_roms->base() + (i * 0x2000), 0x1000);

	// allocate timers
	m_dac_timer = timer_alloc(FUNC(beezer_state::dac_update_cb), this);
	m_scanline_timer = timer_alloc(FUNC(beezer_state::scanline_cb), this);

	// register for state saving
	save_item(NAME(m_ch_sign));
	save_item(NAME(m_dac_data));
	save_item(NAME(m_count));
	save_item(NAME(m_noise));
	save_item(NAME(m_pbus));
	save_item(NAME(m_x));
	save_item(NAME(m_y));
	save_item(NAME(m_z));
}

void beezer_state::machine_reset()
{
	m_pbus = 0xff;

	// initialize memory banks
	bankswitch_w(0);

	// start timer
	m_dac_timer->adjust(attotime::zero, 0, attotime::from_hz((XTAL(4'000'000) / 4) / 16));
	m_scanline_timer->adjust(m_screen->scan_period());
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

void beezer_state::beezer(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, 12_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &beezer_state::main_map);

	MOS6522(config, m_via_system, 12_MHz_XTAL / 12);
	m_via_system->readpa_handler().set(FUNC(beezer_state::via_system_pa_r));
	m_via_system->readpb_handler().set(FUNC(beezer_state::via_system_pb_r));
	m_via_system->writepa_handler().set(FUNC(beezer_state::via_system_pa_w));
	m_via_system->writepb_handler().set(FUNC(beezer_state::via_system_pb_w));
	m_via_system->cb1_handler().set(m_via_audio, FUNC(via6522_device::write_ca2));
	m_via_system->cb2_handler().set(m_via_audio, FUNC(via6522_device::write_ca1));
	m_via_system->irq_handler().set_inputline(m_maincpu, M6809_IRQ_LINE);

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(12_MHz_XTAL / 2, 384, 16, 304, 260, 0, 240); // 288 x 240, correct?
	m_screen->set_screen_update(FUNC(beezer_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(beezer_state::palette_init), 16);

	// sound hardware
	MC6809(config, m_audiocpu, 4_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &beezer_state::sound_map);

	input_merger_device &audio_irqs(INPUT_MERGER_ANY_HIGH(config, "audio_irqs"));
	audio_irqs.output_handler().set_inputline(m_audiocpu, M6809_IRQ_LINE);

	PTM6840(config, m_ptm, 4_MHz_XTAL / 4);
	m_ptm->o1_callback().set(FUNC(beezer_state::ptm_o1_w));
	m_ptm->o2_callback().set(FUNC(beezer_state::ptm_o2_w));
	m_ptm->o3_callback().set(FUNC(beezer_state::ptm_o3_w));
	m_ptm->irq_callback().set("audio_irqs", FUNC(input_merger_device::in_w<0>));
	// schematics show an input labeled VCO to channel 2, but the source is unknown

	MOS6522(config, m_via_audio, 4_MHz_XTAL / 4);
	m_via_audio->readpa_handler().set(FUNC(beezer_state::via_audio_pa_r));
	m_via_audio->writepa_handler().set(FUNC(beezer_state::via_audio_pa_w));
	m_via_audio->writepb_handler().set(FUNC(beezer_state::via_audio_pb_w));
	m_via_audio->ca2_handler().set(m_via_system, FUNC(via6522_device::write_cb1));
	m_via_audio->cb1_handler().set(FUNC(beezer_state::dmod_clr_w));
	m_via_audio->cb2_handler().set(FUNC(beezer_state::dmod_data_w));
	m_via_audio->irq_handler().set("audio_irqs", FUNC(input_merger_device::in_w<1>));

	mm5837_device &noise(MM5837(config, "noise"));
	noise.set_vdd(-12);
	noise.output_callback().set(FUNC(beezer_state::noise_w));

	SPEAKER(config, "speaker").front_center();
	DAC76(config, m_dac, 0);
	m_dac->add_route(ALL_OUTPUTS, "speaker", 1.0);
}


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( beezer )
	ROM_REGION(0xc000, "maincpu", 0)
	ROM_LOAD("g1", 0x0000, 0x1000, CRC(3467a0ec) SHA1(0b094a9bf772b101acd26cf09009c67dd4785ed2))
	ROM_LOAD("g3", 0x1000, 0x1000, CRC(9950cdf2) SHA1(b2b59cc1080357de6ba297392881d626157df809))
	ROM_LOAD("g5", 0x2000, 0x1000, CRC(a4b09879) SHA1(69739dd1d3c88ee6ab310ca3c71b3b50d8ec618f))

	// rom type can be either 2732 or 2764 in all locations
	ROM_REGION(0xe000, "banked", 0)
	ROM_LOAD("f1", 0x0000, 0x2000, CRC(ce1b0b8b) SHA1(8ed1d793928bb7afa041a4f61e0c2f78b4442f2f))
	ROM_LOAD("f3", 0x2000, 0x2000, CRC(6a11072a) SHA1(9700beaec669849da4d0e39d6dbf0b872d7f1b7f))
	ROM_LOAD("e1", 0x4000, 0x1000, CRC(21e4ca9b) SHA1(4024678a4006614051675858ba65db655931a539))
	ROM_RELOAD(0x5000, 0x1000)
	ROM_LOAD("e3", 0x6000, 0x1000, CRC(a4f735d7) SHA1(110061d1c63a331384729951f93a31e62744d0d7))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_LOAD("e5", 0x8000, 0x1000, CRC(0485575b) SHA1(c3be070541459fad4da4a71604883b2f3043374a))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_LOAD("f5", 0xa000, 0x1000, CRC(4b11f572) SHA1(4f283c98a7f1bcf534921b4a54cf564335c53e37))
	ROM_RELOAD(0xb000, 0x1000)
	ROM_LOAD("f7", 0xc000, 0x1000, CRC(bef67473) SHA1(5759ceeca0bb677cee97b74f1a1087d53c25463a))
	ROM_RELOAD(0xd000, 0x1000)

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("d7", 0x1000, 0x1000, CRC(23b0782e) SHA1(7751327b84235a2e2700e4bdd21adec205c54f0e))

	ROM_REGION(0x200, "proms", 0)
	ROM_LOAD("d1.cpu", 0x000, 0x100, CRC(8db17a40) SHA1(0e04a4f5f99b302dbbbfda438808d549f8680fe2))
	ROM_LOAD("e1.cpu", 0x100, 0x100, CRC(3c775c5e) SHA1(ac86f45938c0c9d5fec1245bf86718442baf445b))
ROM_END

ROM_START( beezer1 )
	ROM_REGION(0x3000, "maincpu", 0)
	ROM_LOAD("g1.32", 0x0000, 0x1000, CRC(3134cb93) SHA1(7d4a484378b66ccf2fded31885d6dfb2abae9317))
	ROM_LOAD("g3.32", 0x1000, 0x1000, CRC(a3cb2c2d) SHA1(1e17eb0eaf02f86865845a065a5f714fc51aa7d6))
	ROM_LOAD("g5.32", 0x2000, 0x1000, CRC(5e559bf9) SHA1(cd3713f3ed1215ea5c5640474ba6f005242cd093))

	// rom type can be either 2732 or 2764 in all locations
	ROM_REGION(0xe000, "banked", 0)
	ROM_LOAD("f1.64", 0x0000, 0x2000, CRC(b8a78cca) SHA1(4218ef8c4c8e10d7cc47d6de4c4d189ef3c0f0a1))
	ROM_LOAD("f3.32", 0x2000, 0x1000, CRC(bfa023f5) SHA1(56fb15e2db61197e1aec5a5825beff7c788a4ba3))
	ROM_RELOAD(0x3000, 0x1000)
	ROM_LOAD("e1",    0x4000, 0x1000, CRC(21e4ca9b) SHA1(4024678a4006614051675858ba65db655931a539))
	ROM_RELOAD(0x5000, 0x1000)
	ROM_LOAD("e3",    0x6000, 0x1000, CRC(a4f735d7) SHA1(110061d1c63a331384729951f93a31e62744d0d7))
	ROM_RELOAD(0x7000, 0x1000)
	ROM_LOAD("e5",    0x8000, 0x1000, CRC(0485575b) SHA1(c3be070541459fad4da4a71604883b2f3043374a))
	ROM_RELOAD(0x9000, 0x1000)
	ROM_LOAD("f5",    0xa000, 0x1000, CRC(4b11f572) SHA1(4f283c98a7f1bcf534921b4a54cf564335c53e37))
	ROM_RELOAD(0xb000, 0x1000)
	ROM_LOAD("f7",    0xc000, 0x1000, CRC(bef67473) SHA1(5759ceeca0bb677cee97b74f1a1087d53c25463a))
	ROM_RELOAD(0xd000, 0x1000)

	ROM_REGION(0x2000, "audiocpu", 0)
	ROM_LOAD("d7.32", 0x1000, 0x1000, CRC(b11028b5) SHA1(db8958f0bb12e333ce056da3338f1a824dda36e0))

	ROM_REGION(0x200, "proms", 0)
	ROM_LOAD("d1.cpu", 0x000, 0x100, CRC(8db17a40) SHA1(0e04a4f5f99b302dbbbfda438808d549f8680fe2))
	ROM_LOAD("e1.cpu", 0x100, 0x100, CRC(3c775c5e) SHA1(ac86f45938c0c9d5fec1245bf86718442baf445b))
ROM_END

} // anonymous namespace


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  MACHINE  INPUT   CLASS         INIT        ROTATION  COMPANY                                          FULLNAME                            FLAGS
GAME( 1982, beezer,  0,      beezer,  beezer, beezer_state, empty_init, ROT90,    "Pacific Polytechnical Corp. / Tong Electronic", "Beezer (version 9.0)",             MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // Has test mode, shows version
GAME( 1982, beezer1, beezer, beezer,  beezer, beezer_state, empty_init, ROT90,    "Pacific Polytechnical Corp. / Tong Electronic", "Beezer (unknown earlier version)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // No test mode, possibly earlier?
