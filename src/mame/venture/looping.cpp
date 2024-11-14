// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*

Looping / Sky Bumper

TODO:
- get sound working
- map and test any remaining input ports

---------------------------------------------------------------

Looping
(C)1981 Venture Line

    Main CPU
        TMS9995

    COP420 Microcontroller
        protection

    Sound CPU
        TMS9980
        AY-3-8910
        TMS5220 (SPEECH)

---------------------------------------------------------------

Sky Bumper
(C)1982 Venture Line

    This is a ROM swap for Looping.  There are two 6116's on
    the CPU board, where there is only one on Looping.

===============================================================

LOOPING CHIP PLACEMENT

THERE ARE AT LEAST TWO VERSIONS OF THIS GAME
VERSION NUMBERS FOR THIS PURPOSE ARE CHOSEN AT RANDOM

IC NAME   POSITION   BOARD  TYPE   IC NAME  POSITION  TYPE
VER-1                         VER-2
---------------------------------------------------------------
LOS-2-7   13A        I/O    2532    SAME    13A       2532
LOS-1-1-2 11A         "      "      SAME    11A        "
LOS-3-1   13C         "      "      I-O-V2  13C        "

VLI1      2A         ROM    2764    VLI-7-1 2A         "
VLI3      5A          "      "      VLI-7-2 4A         "
VLI9-5    8A          "      "      VLI-4-3 5A         "
L056-6    9A          "      "      VLI-8-4 7A         "
                      "             LO56-5  8A         "
                      "             LO56-6  9A         "
                      "             VLI-8-7 10A        "
                  ON RIBBON CABLE   18S030  11B             color prom?
                     REAR BD      LOG.1-9-3 6A        2716  tiles
                                  LOG.3     8A         "    tiles
*/

#include "emu.h"

#include "cpu/cop400/cop400.h"
#include "cpu/tms9900/tms9995.h"
#include "cpu/tms9900/tms9980a.h"
#include "machine/74259.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/tms5220.h"
#include "video/resnet.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {


/*************************************
 *
 *  Type definitions
 *
 *************************************/

class looping_state : public driver_device
{
public:
	looping_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_aysnd(*this, "aysnd"),
		m_tms(*this, "tms"),
		m_dac(*this, "dac"),
		m_soundlatch(*this, "soundlatch"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void looping(machine_config &config);

	void init_looping();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	void flip_screen_x_w(int state);
	void flip_screen_y_w(int state);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void level2_irq_set(int state);
	void main_irq_ack_w(int state);
	void souint_clr(int state);
	void ballon_enable_w(int state);
	void out_0_w(uint8_t data);
	void out_2_w(uint8_t data);
	uint8_t adc_r();
	void adc_w(uint8_t data);
	void plr2_w(int state);
	uint8_t cop_unk_r();
	int cop_serial_r();
	void cop_l_w(uint8_t data);
	uint8_t protection_r();
	[[maybe_unused]] void spcint(int state);
	void int_update(int state);
	void sound_sw(uint8_t data);
	void ay_enable_w(int state);
	void speech_enable_w(int state);
	TILE_GET_INFO_MEMBER(get_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
	void map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	// memory pointers
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_spriteram;
	uint8_t m_cop_port_l = 0;

	// tilemaps
	tilemap_t * m_bg_tilemap = nullptr;

	required_device<tms9995_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ay8910_device> m_aysnd;
	required_device<tms5220_device> m_tms;
	required_device<dac_byte_interface> m_dac;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};



/*************************************
 *
 *  Palette conversion
 *
 *************************************/

void looping_state::palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	static constexpr int resistances[3] = { 1000, 470, 220 };

	// compute the color output resistor weights
	double rweights[3], gweights[3], bweights[2];
	compute_resistor_weights(0, 255, -1.0,
			3,  &resistances[0], rweights, 470, 0,
			3,  &resistances[0], gweights, 470, 0,
			2,  &resistances[1], bweights, 470, 0);

	// initialize the palette with these colors
	for (int i = 0; i < 32; i++)
	{
		// red component
		int bit0 = BIT(color_prom[i], 0);
		int bit1 = BIT(color_prom[i], 1);
		int bit2 = BIT(color_prom[i], 2);
		int const r = combine_weights(rweights, bit0, bit1, bit2);

		// green component
		bit0 = BIT(color_prom[i], 3);
		bit1 = BIT(color_prom[i], 4);
		bit2 = BIT(color_prom[i], 5);
		int const g = combine_weights(gweights, bit0, bit1, bit2);

		// blue component
		bit0 = BIT(color_prom[i], 6);
		bit1 = BIT(color_prom[i], 7);
		int const b = combine_weights(bweights, bit0, bit1);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}



/*************************************
 *
 *  Video startup/tilemap config
 *
 *************************************/

TILE_GET_INFO_MEMBER(looping_state::get_tile_info)
{
	int tile_number = m_videoram[tile_index];
	int color = m_colorram[(tile_index & 0x1f) * 2 + 1] & 0x07;
	tileinfo.set(0, tile_number, color, 0);
}


void looping_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(looping_state::get_tile_info)), TILEMAP_SCAN_ROWS, 8,8, 32,32);

	m_bg_tilemap->set_scroll_cols(0x20);
}



/*************************************
 *
 *  Video write handlers
 *
 *************************************/

void looping_state::flip_screen_x_w(int state)
{
	flip_screen_x_set(!state);
	m_bg_tilemap->set_scrollx(0, flip_screen() ? 128 : 0);
}


void looping_state::flip_screen_y_w(int state)
{
	flip_screen_y_set(!state);
	m_bg_tilemap->set_scrollx(0, flip_screen() ? 256 : 0);
}


void looping_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


void looping_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;

	// odd bytes are column color attribute
	if (offset & 1)
	{
		// mark the whole column dirty
		offs_t offs = (offset / 2);
		for (int i = 0; i < 0x20; i++)
			m_bg_tilemap->mark_tile_dirty(i * 0x20 + offs);
	}

	// even bytes are column scroll
	else
		m_bg_tilemap->set_scrolly(offset / 2, data);
}



/*************************************
 *
 *  Video update
 *
 *************************************/

void looping_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (const uint8_t *source = m_spriteram; source < m_spriteram + 0x40; source += 4)
	{
		int sx = source[3];
		int sy = 240 - source[0];
		int flipx = source[1] & 0x40;
		int flipy = source[1] & 0x80;
		int code  = source[1] & 0x3f;
		int color = source[2];

		if (flip_screen_x())
		{
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (flip_screen_y())
		{
			sy = 240 - sy;
			flipy = !flipy;
		}

		m_gfxdecode->gfx(1)->transpen(bitmap, cliprect, code, color, flipx, flipy, sx, sy, 0);
	}
}


uint32_t looping_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprites(bitmap, cliprect);
	return 0;
}



/*************************************
 *
 *  Machine start/reset
 *
 *************************************/

void looping_state::machine_start()
{
	save_item(NAME(m_cop_port_l));
}

void looping_state::machine_reset()
{
	// Disable auto wait state generation by raising the READY line on reset
	m_maincpu->ready_line(ASSERT_LINE);
	m_maincpu->reset_line(ASSERT_LINE);

	m_cop_port_l = 0;
}

/*************************************
 *
 *  Interrupt handling
 *
 *************************************/

INTERRUPT_GEN_MEMBER(looping_state::interrupt)
{
	m_maincpu->set_input_line(INT_9995_INT1, ASSERT_LINE);
}


void looping_state::level2_irq_set(int state)
{
	logerror("Level 2 int = %d\n", state);
	if (state == 0)
		m_maincpu->set_input_line(INT_9995_INT1, ASSERT_LINE);
}


void looping_state::main_irq_ack_w(int state)
{
	if (state == 0)
		m_maincpu->set_input_line(INT_9995_INT1, CLEAR_LINE);
}


void looping_state::souint_clr(int state)
{
	logerror("Soundint clr = %d\n", state);
	if (state == 0)
		m_soundlatch->acknowledge_w();
}


void looping_state::spcint(int state)
{
	logerror("Speech /int = %d\n", state == ASSERT_LINE ? 1 : 0);
	m_audiocpu->set_input_line(INT_9980A_LEVEL4, state == ASSERT_LINE ? CLEAR_LINE : ASSERT_LINE);
}


void looping_state::int_update(int state)
{
	// hack necessitated by flawed input logic in TMS9980A core
	if (m_soundlatch->pending_r())
		m_audiocpu->set_input_line(INT_9980A_LEVEL2, ASSERT_LINE);
	else if (!m_tms->intq_r())
		m_audiocpu->set_input_line(INT_9980A_LEVEL4, ASSERT_LINE);
	else
		m_audiocpu->set_input_line(INT_9980A_CLEAR, CLEAR_LINE);
}


/*************************************
 *
 *  Custom DAC handling
 *
 *************************************/

void looping_state::sound_sw(uint8_t data)
{
	/* this can be improved by adding the missing signals for decay etc. (see schematics)

	    0001 = ASOV
	    0002 = AVOL2
	    0003 = AVOL1
	    0004 = ADECAY1
	    0005 = ADECAY
	    0006 = ASA
	    0007 = AFA
	*/

	m_dac->write(((BIT(~data, 2) << 1) + BIT(~data, 3)) * BIT(~data, 7));
}



/*************************************
 *
 *  Sound controls
 *
 *************************************/

void looping_state::ay_enable_w(int state)
{
	for (int output = 0; output < 3; output++)
		m_aysnd->set_output_gain(output, state ? 1.0 : 0.0);
}


void looping_state::speech_enable_w(int state)
{
	m_tms->set_output_gain(0, state ? 1.0 : 0.0);
}


void looping_state::ballon_enable_w(int state)
{
	logerror("ballon_enable_w = %d\n", state);
}



/*************************************
 *
 *  Misc I/O
 *
 *************************************/

void looping_state::out_0_w(uint8_t data) { osd_printf_debug("out0 = %02X\n", data); }
void looping_state::out_2_w(uint8_t data) { osd_printf_debug("out2 = %02X\n", data); }

uint8_t looping_state::adc_r() { osd_printf_debug("%04X:ADC read\n", m_maincpu->pc()); return 0xff; }
void looping_state::adc_w(uint8_t data) { osd_printf_debug("%04X:ADC write = %02X\n", m_maincpu->pc(), data); }

void looping_state::plr2_w(int state)
{
	/* set to 1 after IDLE, cleared to 0 during processing
	   is this an LED on the PCB? */
}



/*************************************
 *
 *  Protection
 *
 *************************************/

uint8_t looping_state::cop_unk_r()
{
	return 1;
}

int looping_state::cop_serial_r()
{
	return 1;
}

void looping_state::cop_l_w(uint8_t data)
{
	m_cop_port_l = data;
	//logerror("%02x  ",data);
}

uint8_t looping_state::protection_r()
{
//        The code reads ($7002) ($7004) alternately
//        The result must change at least once every 10 reads
//        A read from ($34b0 + result) must == $01

//        Valid values:
//            $61 $67
//            $B7 $BF
//            $DB
//            $E1
//            $F3 $F7 $FD $FF

//        Because they read alternately from different locations,
//        it is trivial to bypass the protection.

//        cop write alternately $02 $01 $08 $04 in port $102
//        cop write randomly fc (unfortunately) but 61,67,b7,bf,db,e1,f3,fd,ff too and only these values

	// missing something
	if(m_cop_port_l != 0xfc) return m_cop_port_l;
	return 0xff;
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void looping_state::map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x7000, 0x7007).r(FUNC(looping_state::protection_r));

	map(0x9000, 0x93ff).ram().w(FUNC(looping_state::videoram_w)).share(m_videoram);

	map(0x9800, 0x983f).mirror(0x0700).ram().w(FUNC(looping_state::colorram_w)).share(m_colorram);
	map(0x9840, 0x987f).mirror(0x0700).ram().share(m_spriteram);
	map(0x9880, 0x98ff).mirror(0x0700).ram();

	map(0xb000, 0xb007).mirror(0x07f8).w("videolatch", FUNC(ls259_device::write_d0));

	map(0xe000, 0xefff).ram();
	map(0xf800, 0xf800).mirror(0x03fc).portr("P1").w(FUNC(looping_state::out_0_w));                      // /OUT0
	map(0xf801, 0xf801).mirror(0x03fc).portr("P2").w("soundlatch", FUNC(generic_latch_8_device::write)); // /OUT1
	map(0xf802, 0xf802).mirror(0x03fc).portr("DSW").w(FUNC(looping_state::out_2_w));                     // /OUT2
	map(0xf803, 0xf803).mirror(0x03fc).rw(FUNC(looping_state::adc_r), FUNC(looping_state::adc_w));
}

void looping_state::io_map(address_map &map)
{
	map(0x0800, 0x080f).w("mainlatch", FUNC(ls259_device::write_d0));
}


// complete memory map derived from schematics
void looping_state::sound_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x37ff).rom();
	map(0x3800, 0x3bff).ram();
	map(0x3c00, 0x3c00).mirror(0x00f4).rw(m_aysnd, FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_w));
	map(0x3c01, 0x3c01).mirror(0x00f6).noprw();
	map(0x3c02, 0x3c02).mirror(0x00f4).nopr().w(m_aysnd, FUNC(ay8910_device::data_w));
	map(0x3e00, 0x3e00).mirror(0x00f4).nopr().w(m_tms, FUNC(tms5220_device::data_w));
	map(0x3e01, 0x3e01).mirror(0x00f6).noprw();
	map(0x3e02, 0x3e02).mirror(0x00f4).r(m_tms, FUNC(tms5220_device::status_r)).nopw();
}

void looping_state::sound_io_map(address_map &map)
{
	map(0x0000, 0x000f).w("sen0", FUNC(ls259_device::write_d0));
	map(0x0010, 0x001f).w("sen1", FUNC(ls259_device::write_d0));
}


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(1,2), RGN_FRAC(0,2) },
	{ STEP8(0,1), STEP8(64,1) },
	{ STEP8(0,8), STEP8(128,8) },
	8*8*4
};


static GFXDECODE_START( gfx_looping )
	GFXDECODE_ENTRY( "gfx", 0, gfx_8x8x2_planar, 0, 8 )
	GFXDECODE_ENTRY( "gfx", 0, sprite_layout,    0, 8 )
GFXDECODE_END


/*************************************
 *
 *  Machine drivers
 *
 *************************************/


void looping_state::looping(machine_config &config)
{
	// CPU TMS9995, standard variant; no line connections
	TMS9995(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &looping_state::map);
	m_maincpu->set_addrmap(AS_IO, &looping_state::io_map);
	m_maincpu->set_vblank_int("screen", FUNC(looping_state::interrupt));

	// CPU TMS9980A for audio subsystem; no line connections
	TMS9980A(config, m_audiocpu, 8_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &looping_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &looping_state::sound_io_map);

	cop420_cpu_device &cop(COP420(config, "mcu", 8_MHz_XTAL / 2)); // unknown guess
	cop.set_config(COP400_CKI_DIVISOR_16, COP400_CKO_OSCILLATOR_OUTPUT, false);
	cop.write_l().set(FUNC(looping_state::cop_l_w));
	cop.read_l().set(FUNC(looping_state::cop_unk_r));
	cop.read_g().set(FUNC(looping_state::cop_unk_r));
	cop.read_in().set(FUNC(looping_state::cop_unk_r));
	cop.read_si().set(FUNC(looping_state::cop_serial_r));

	ls259_device &mainlatch(LS259(config, "mainlatch")); // C9 on CPU board
	// Q0 = A16
	// Q1 = A17
	// Q2 = COLOR 9
	mainlatch.q_out_cb<3>().set(FUNC(looping_state::plr2_w));
	// Q4 = C0
	// Q5 = C1
	mainlatch.q_out_cb<6>().set(FUNC(looping_state::main_irq_ack_w));
	mainlatch.q_out_cb<7>().set("watchdog", FUNC(watchdog_timer_device::reset_line_w)).invert();

	WATCHDOG_TIMER(config, "watchdog");

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.432_MHz_XTAL / 3, 384, 0, 256, 264, 16, 224 + 16);
	screen.set_screen_update(FUNC(looping_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_looping);
	PALETTE(config, m_palette, FUNC(looping_state::palette), 32);

	ls259_device &videolatch(LS259(config, "videolatch")); // E2 on video board
	videolatch.q_out_cb<1>().set(FUNC(looping_state::level2_irq_set));
	videolatch.q_out_cb<6>().set(FUNC(looping_state::flip_screen_x_w));
	videolatch.q_out_cb<7>().set(FUNC(looping_state::flip_screen_y_w));

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	//m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, INT_9980A_LEVEL2);
	m_soundlatch->data_pending_callback().set(FUNC(looping_state::int_update));
	m_soundlatch->set_separate_acknowledge(true);

	AY8910(config, m_aysnd, 8_MHz_XTAL / 4);
	m_aysnd->port_a_read_callback().set("soundlatch", FUNC(generic_latch_8_device::read));
	m_aysnd->add_route(ALL_OUTPUTS, "speaker", 0.2);

	TMS5220(config, m_tms, 640'000);
	//m_tms->irq_cb().set(FUNC(looping_state::spcint));
	m_tms->irq_cb().set(FUNC(looping_state::int_update));
	m_tms->add_route(ALL_OUTPUTS, "speaker", 0.5);

	DAC_2BIT_R2R(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.15); // unknown DAC

	ls259_device &sen0(LS259(config, "sen0")); // B3 on sound board
	sen0.q_out_cb<0>().set(FUNC(looping_state::souint_clr));
	sen0.parallel_out_cb().set(FUNC(looping_state::sound_sw));

	ls259_device &sen1(LS259(config, "sen1")); // A1 on sound board with outputs connected to 4016 at B1
	sen1.q_out_cb<0>().set(FUNC(looping_state::ay_enable_w));
	sen1.q_out_cb<1>().set(FUNC(looping_state::speech_enable_w));
	sen1.q_out_cb<2>().set(FUNC(looping_state::ballon_enable_w));
}



/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( looping )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Shoot")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Accelerate?")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2") // cocktail?
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x18, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Coin_B ) )     PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0e, 0x02, DEF_STR( Coin_A ) )     PORT_DIPLOCATION("DSW:7,6,5")
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_7C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )    PORT_DIPLOCATION("DSW:4")  // Check code at 0x2c00
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Lives ) )      PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "DSW:2" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )    PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
INPUT_PORTS_END

// Same as 'looping' but additional "Infinite Lives" Dip Switch
static INPUT_PORTS_START( skybump )
	PORT_INCLUDE(looping)

	PORT_MODIFY("DSW")
	PORT_DIPNAME( 0x60, 0x40, DEF_STR( Lives ) )      PORT_DIPLOCATION("DSW:3,2")
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPSETTING(    0x20, "Infinite (Cheat)") // duplicate
INPUT_PORTS_END



/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( loopingv )
	ROM_REGION( 0x8000, "maincpu", 0 ) // TMS9995 code
	ROM_LOAD( "vli3.5a",        0x0000, 0x2000, CRC(1ac3ccdf) SHA1(9d1cde8bd4d0f12eaf06225b3ecc4a5c3e4f0c11) )
	ROM_LOAD( "vli1.2a",        0x2000, 0x2000, CRC(97755fd4) SHA1(4a6ef02b0128cd516ff95083a7caaad8f3756f09) )
	ROM_LOAD( "l056-6.9a",      0x4000, 0x2000, CRC(548afa52) SHA1(0b88ac7394feede023519c585a4084591eb9661a) )
	ROM_LOAD( "vli9-5.8a",      0x6000, 0x2000, CRC(5d122f86) SHA1(d1c66b890142bb4d4648f3edec6567f58107dbf0) )

	ROM_REGION( 0x3800, "audiocpu", 0 ) // TMS9980 code
	ROM_LOAD( "i-o.13c",        0x0000, 0x0800, CRC(21e9350c) SHA1(f30a180309e373a17569351944f5e7982c3b3f9d) )
	ROM_LOAD( "i-o.13a",        0x0800, 0x1000, CRC(1de29f25) SHA1(535acb132266d6137b0610ee9a9b946459ae44af) )
	ROM_LOAD( "i-o.11a",        0x2800, 0x1000, CRC(61c74c79) SHA1(9f34d18a919446dd76857b851cea23fc1526f3c2) ) // speech

	ROM_REGION( 0x0400, "mcu", 0 ) // COP420 microcontroller code
	ROM_LOAD( "cop.bin",        0x0000, 0x0400, CRC(d47fecec) SHA1(7eeedcb40f4cd50e1e259c6b01744a3fc97b60aa) )

	ROM_REGION( 0x1000, "gfx", 0 )
	ROM_LOAD( "log2.8a",        0x0000, 0x800, CRC(ef3284ac) SHA1(8719c9df8c972a56c306b3c707aaa53092ffa2d6) )
	ROM_LOAD( "log1-9-3.6a",    0x0800, 0x800, CRC(c434c14c) SHA1(3669aaf7adc6b250378bcf62eb8e7058f55476ef) )

	ROM_REGION( 0x0020, "proms", 0 ) // color prom
	ROM_LOAD( "18s030.11b",     0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( loopingva )
	ROM_REGION( 0x8000, "maincpu", 0 ) // TMS9995 code
	ROM_LOAD( "vli3.5a",        0x0000, 0x2000, CRC(1ac3ccdf) SHA1(9d1cde8bd4d0f12eaf06225b3ecc4a5c3e4f0c11) )
	ROM_LOAD( "vli-4-3",        0x2000, 0x1000, CRC(f32cae2b) SHA1(2c6ef82af438e588b56fd58b95cf969c97bb9a66) )
	ROM_LOAD( "vli-8-4",        0x3000, 0x1000, CRC(611e1dbf) SHA1(0ab6669f1dec30c3f7bca49e158e4790a78fa308) )
	ROM_LOAD( "l056-6.9a",      0x4000, 0x2000, CRC(548afa52) SHA1(0b88ac7394feede023519c585a4084591eb9661a) )
	ROM_LOAD( "vli9-5.8a",      0x6000, 0x2000, CRC(5d122f86) SHA1(d1c66b890142bb4d4648f3edec6567f58107dbf0) )

	ROM_REGION( 0x3800, "audiocpu", 0 ) // TMS9980 code
	ROM_LOAD( "i-o-v2.13c",     0x0000, 0x0800, CRC(09765ebe) SHA1(93b035c3a94f2f6d5e463256e26b600a4dd5d3ea) )
	ROM_LOAD( "i-o.13a",        0x0800, 0x1000, CRC(1de29f25) SHA1(535acb132266d6137b0610ee9a9b946459ae44af) ) // speech
	ROM_LOAD( "i-o.11a",        0x2800, 0x1000, CRC(61c74c79) SHA1(9f34d18a919446dd76857b851cea23fc1526f3c2) )

	ROM_REGION( 0x0400, "mcu", 0 ) // COP420 microcontroller code
	ROM_LOAD( "cop.bin",        0x0000, 0x0400, CRC(d47fecec) SHA1(7eeedcb40f4cd50e1e259c6b01744a3fc97b60aa) )

	ROM_REGION( 0x1000, "gfx", 0 )
	ROM_LOAD( "log2.8a",        0x0000, 0x800, CRC(ef3284ac) SHA1(8719c9df8c972a56c306b3c707aaa53092ffa2d6) )
	ROM_LOAD( "log1-9-3.6a",    0x0800, 0x800, CRC(c434c14c) SHA1(3669aaf7adc6b250378bcf62eb8e7058f55476ef) )

	ROM_REGION( 0x0020, "proms", 0 ) // color prom
	ROM_LOAD( "18s030.11b",     0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( looping )
	ROM_REGION( 0x8000, "maincpu", 0 ) // TMS9995 code
	ROM_LOAD( "loop551.bin",        0x0000, 0x1000, CRC(d6bb6db6) SHA1(074eb3bc101096bfe67c3107f306df829ae38548) )
	ROM_LOAD( "loop552.bin",        0x1000, 0x1000, CRC(bc32956d) SHA1(6ef8d76df1d5b1ed52a4057eae2bf4eb394e4c54) )
	ROM_LOAD( "loop553.bin",        0x2000, 0x1000, CRC(5f8b9aed) SHA1(32be61788e3d54b23d1663025365b1ab6b96dc91) )
	ROM_LOAD( "loop554b.bin",       0x3000, 0x1000, CRC(381a9625) SHA1(07d775125be1f761dad568f8ccce600414a9d15f) )
	ROM_LOAD( "loop555.bin",        0x4000, 0x1000, CRC(0ef4c922) SHA1(df6db0897a51aa10e106865a643588d866ef8c4e) )
	ROM_LOAD( "loop556.bin",        0x5000, 0x1000, CRC(3419a5d5) SHA1(2b0249c54985ab5e12de17c0e3d62caa0c7575e3) )
	ROM_LOAD( "loop557.bin",        0x6000, 0x1000, CRC(d430e287) SHA1(b0edd25ef4d2468cc1f8c10ac49c545a89d398d7) )

	ROM_REGION( 0x3800, "audiocpu", 0 ) // TMS9980 code
	ROM_LOAD( "loopc13.bin",        0x0000, 0x1000, CRC(ff9ac4ec) SHA1(9f8df94cd79d86fe4c384df1d5d729b58a7ca7a8) )
	ROM_LOAD( "loopa13.bin",        0x0800, 0x1000, CRC(1de29f25) SHA1(535acb132266d6137b0610ee9a9b946459ae44af) )
	ROM_LOAD( "loopa11.bin",        0x2800, 0x1000, CRC(61c74c79) SHA1(9f34d18a919446dd76857b851cea23fc1526f3c2) ) // speech

	ROM_REGION( 0x0400, "mcu", 0 ) // COP420 microcontroller code
	ROM_LOAD( "cop.bin",        0x0000, 0x0400, CRC(d47fecec) SHA1(7eeedcb40f4cd50e1e259c6b01744a3fc97b60aa) ) // taken from the other sets

	ROM_REGION( 0x1000, "gfx", 0 )
	ROM_LOAD( "loopaa8.bin",    0x0000, 0x800, CRC(ef3284ac) SHA1(8719c9df8c972a56c306b3c707aaa53092ffa2d6) )
	ROM_LOAD( "loopaa6.bin",    0x0800, 0x800, CRC(c434c14c) SHA1(3669aaf7adc6b250378bcf62eb8e7058f55476ef) )

	ROM_REGION( 0x0020, "proms", 0 ) // color prom
	ROM_LOAD( "loopvp1.bin",        0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END

ROM_START( skybump )
	ROM_REGION( 0x8000, "maincpu", 0 ) // TMS9995 code
	ROM_LOAD( "cpu.5a",         0x0000, 0x2000, CRC(dca38df0) SHA1(86abe04cbabf81399f842f53668fe7a3f7ed3757) )
	ROM_LOAD( "cpu.2a",         0x2000, 0x2000, CRC(6bcc211a) SHA1(245ebae3934df9c3920743a941546d96bb2e7c03) )
	ROM_LOAD( "cpu.9a",         0x4000, 0x2000, CRC(c7a50797) SHA1(60aa0a28ba970f12d0a0e538ae1c6807d105855c) )
	ROM_LOAD( "cpu.8a",         0x6000, 0x2000, CRC(a718c6f2) SHA1(19afa8c353829232cb96c27b87f13b43166ab6fc) )

	ROM_REGION( 0x3800, "audiocpu", 0 ) // TMS9980 code
	ROM_LOAD( "snd.13c",        0x0000, 0x0800, CRC(21e9350c) SHA1(f30a180309e373a17569351944f5e7982c3b3f9d) )
	ROM_LOAD( "snd.13a",        0x0800, 0x1000, CRC(1de29f25) SHA1(535acb132266d6137b0610ee9a9b946459ae44af) )
	ROM_LOAD( "snd.11a",        0x2800, 0x1000, CRC(61c74c79) SHA1(9f34d18a919446dd76857b851cea23fc1526f3c2) )

	ROM_REGION( 0x0400, "mcu", 0 ) // COP420 microcontroller code
	ROM_LOAD( "cop.bin",        0x0000, 0x0400, CRC(d47fecec) SHA1(7eeedcb40f4cd50e1e259c6b01744a3fc97b60aa) )

	ROM_REGION( 0x1000, "gfx", 0 )
	ROM_LOAD( "vid.8a",         0x0000, 0x800, CRC(459ccc55) SHA1(747f6789605b48be9e22f779f9e3f6c98ad4e594) )
	ROM_LOAD( "vid.6a",         0x0800, 0x800, CRC(12ebbe74) SHA1(0f87c81a45d1bf3b8c6a70ee5e1a014069f67755) )

	ROM_REGION( 0x0020, "proms", 0 ) // color prom
	ROM_LOAD( "vid.clr",        0x0000, 0x0020, CRC(6a0c7d87) SHA1(140335d85c67c75b65689d4e76d29863c209cf32) )
ROM_END



/*************************************
 *
 *  Driver config
 *
 *************************************/

void looping_state::init_looping()
{
	int length = memregion("maincpu")->bytes();
	uint8_t *rom = memregion("maincpu")->base();

	// bitswap the TMS9995 ROMs
	for (int i = 0; i < length; i++)
		rom[i] = bitswap<8>(rom[i], 0,1,2,3,4,5,6,7);
}


} // Anonymous namespace


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1982, looping,   0,        looping, looping, looping_state, init_looping, ROT90, "Video Games GmbH", "Looping", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, loopingv,  looping,  looping, looping, looping_state, init_looping, ROT90, "Video Games GmbH (Venture Line license)", "Looping (Venture Line license, set 1)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, loopingva, looping,  looping, looping, looping_state, init_looping, ROT90, "Video Games GmbH (Venture Line license)", "Looping (Venture Line license, set 2)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1982, skybump,   0,        looping, skybump, looping_state, init_looping, ROT90, "Venture Line", "Sky Bumper", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
