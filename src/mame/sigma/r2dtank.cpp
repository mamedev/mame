// license:BSD-3-Clause
// copyright-holders:David Haywood, Pierpaolo Prazzoli
/*******************************************************************
R2D Tank (c) 1980 Sigma Ent. Inc.

driver by: David Haywood & Pierpaolo Prazzoli


from the readme
----------------------------------------------------
Orca board number OVG-17A

r2d1.1c is ROM #1 at board position 1C, and so on.

1 = 2716
2 = 2732
3 = 2732
4 = 2732
5 = 2716 Sound

CPU = 6809
other = HD46505SP (6845) (CRT controller)
other = MB14282(x2)
other = HD468458SP
other = MB14282
other = MB14368
other = HD6821 (x2) (PIA)
other = HD46802
other = M5L8226 (x2)
RAM = 4116 (x11)

----------------------------------------------------

XTAL values appear to be 3579.545 (X1) and 11.200 (X2).

********************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/74123.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "machine/rescap.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define LOG_AUDIO_COMM  (1U << 1)

#define VERBOSE (0)
#include "logmacro.h"


namespace {

#define MAIN_CPU_MASTER_CLOCK   (11.2_MHz_XTAL)
#define PIXEL_CLOCK             (MAIN_CPU_MASTER_CLOCK / 2)
#define CRTC_CLOCK              (MAIN_CPU_MASTER_CLOCK / 16)

class r2dtank_state : public driver_device
{
public:
	r2dtank_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_audiocpu(*this, "audiocpu"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_pia_main(*this, "pia_main"),
		m_pia_audio(*this, "pia_audio"),
		m_ay1(*this, "ay1"),
		m_ay2(*this, "ay2")
	{ }

	void r2dtank(machine_config &config);

	int ttl74123_output_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<cpu_device> m_audiocpu;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_device<pia6821_device> m_pia_main;
	required_device<pia6821_device> m_pia_audio;
	required_device<ay8910_device> m_ay1;
	required_device<ay8910_device> m_ay2;

	uint8_t m_flipscreen = 0;
	uint32_t m_ttl74123_output = 0;
	uint8_t m_AY8910_selected = 0;

	uint8_t audio_command_r();
	void audio_command_w(uint8_t data);
	uint8_t audio_answer_r();
	void audio_answer_w(uint8_t data);
	void main_cpu_irq(int state);
	void AY8910_select_w(uint8_t data);
	uint8_t AY8910_port_r();
	void AY8910_port_w(uint8_t data);
	void flipscreen_w(int state);
	void pia_comp_w(offs_t offset, uint8_t data);

	void ttl74123_output_changed(int state);

	MC6845_UPDATE_ROW(crtc_update_row);

	void r2dtank_audio_map(address_map &map) ATTR_COLD;
	void r2dtank_main_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Interrupt generation
 *
 *************************************/

void r2dtank_state::main_cpu_irq(int state)
{
	int combined_state = m_pia_main->irq_a_state() | m_pia_main->irq_b_state() |
							m_pia_audio->irq_a_state() | m_pia_audio->irq_b_state();

	m_maincpu->set_input_line(M6809_IRQ_LINE,  combined_state ? ASSERT_LINE : CLEAR_LINE);
}



/*************************************
 *
 *  Audio system - CPU 1
 *
 *************************************/

uint8_t r2dtank_state::audio_command_r()
{
	uint8_t ret = m_soundlatch->read();

	LOGMASKED(LOG_AUDIO_COMM, "%08X  CPU#1  Audio Command Read: %x\n", m_audiocpu->pc(), ret);

	return ret;
}


void r2dtank_state::audio_command_w(uint8_t data)
{
	m_soundlatch->write(~data);
	m_audiocpu->set_input_line(M6802_IRQ_LINE, HOLD_LINE);

	LOGMASKED(LOG_AUDIO_COMM, "%08X   CPU#0  Audio Command Write: %x\n", m_maincpu->pc(), data^0xff);
}


uint8_t r2dtank_state::audio_answer_r()
{
	uint8_t ret = m_soundlatch2->read();
	LOGMASKED(LOG_AUDIO_COMM, "%08X  CPU#0  Audio Answer Read: %x\n", m_maincpu->pc(), ret);

	return ret;
}


void r2dtank_state::audio_answer_w(uint8_t data)
{
	/* HACK - prevents lock-up, but causes game to end some in-between screens prematurely */
	if (m_audiocpu->pc() == 0xfb12)
		data = 0x00;

	m_soundlatch2->write(data);
	m_maincpu->set_input_line(M6809_IRQ_LINE, HOLD_LINE);

	LOGMASKED(LOG_AUDIO_COMM, "%08X  CPU#1  Audio Answer Write: %x\n", m_audiocpu->pc(), data);
}


void r2dtank_state::AY8910_select_w(uint8_t data)
{
	/* not sure what all the bits mean:
	   D0 - ????? definitely used
	   D1 - not used?
	   D2 - selects ay8910 control or port
	   D3 - selects ay8910 #0
	   D4 - selects ay8910 #1
	   D5-D7 - not used */
	m_AY8910_selected = data;

	LOGMASKED(LOG_AUDIO_COMM, "%s:  CPU#1  AY8910_select_w: %x\n", machine().describe_context(), data);
}


uint8_t r2dtank_state::AY8910_port_r()
{
	uint8_t ret = 0;

	if (m_AY8910_selected & 0x08)
		ret = m_ay1->data_r();

	if (m_AY8910_selected & 0x10)
		ret = m_ay2->data_r();

	return ret;
}


void r2dtank_state::AY8910_port_w(uint8_t data)
{
	if (m_AY8910_selected & 0x08)
		m_ay1->data_address_w(m_AY8910_selected >> 2, data);

	if (m_AY8910_selected & 0x10)
		m_ay2->data_address_w(m_AY8910_selected >> 2, data);
}


/*************************************
 *
 *  74123
 *
 *  This timer is responsible for
 *  delaying the PIA1's port input.
 *  This delay ensures that
 *  CA1 is only changed in the VBLANK
 *  region, but not in HBLANK
 *
 *************************************/

void r2dtank_state::ttl74123_output_changed(int state)
{
	m_pia_main->ca1_w(state);
	m_ttl74123_output = state;
}


int r2dtank_state::ttl74123_output_r()
{
	return m_ttl74123_output;
}

/*************************************
 *
 *  Machine start
 *
 *************************************/

void r2dtank_state::machine_start()
{
	/* setup for save states */
	save_item(NAME(m_flipscreen));
	save_item(NAME(m_ttl74123_output));
	save_item(NAME(m_AY8910_selected));
}



/*************************************
 *
 *  Video system
 *
 *************************************/


void r2dtank_state::flipscreen_w(int state)
{
	m_flipscreen = !state;
}


MC6845_UPDATE_ROW( r2dtank_state::crtc_update_row )
{
	uint8_t x = 0;

	for (uint8_t cx = 0; cx < x_count; cx++)
	{
		/* the memory is hooked up to the MA, RA lines this way */
		offs_t offs = ((ma << 3) & 0x1f00) |
						((ra << 5) & 0x00e0) |
						((ma << 0) & 0x001f);

		if (m_flipscreen)
			offs = offs ^ 0x1fff;

		uint8_t data = m_videoram[offs];
		uint8_t fore_color = (m_colorram[offs] >> 5) & 0x07;

		for (int i = 0; i < 8; i++)
		{
			uint8_t bit;

			if (m_flipscreen)
			{
				bit = data & 0x01;
				data >>= 1;
			}
			else
			{
				bit = data & 0x80;
				data <<= 1;
			}

			uint8_t const color = bit ? fore_color : 0;
			bitmap.pix(y, x) = m_palette->pen_color(color);

			x++;
		}

		ma++;
	}
}



/*************************************
 *
 *  Memory handlers
 *
 *************************************/

void r2dtank_state::pia_comp_w(offs_t offset, uint8_t data)
{
	m_pia_main->write(offset, ~data);
}


void r2dtank_state::r2dtank_main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("videoram");
	map(0x2000, 0x3fff).ram();
	map(0x4000, 0x5fff).ram().share("colorram");
	map(0x6000, 0x7fff).ram();
	map(0x8000, 0x8003).r("pia_main", FUNC(pia6821_device::read)).w(FUNC(r2dtank_state::pia_comp_w));
	map(0x8004, 0x8004).rw(FUNC(r2dtank_state::audio_answer_r), FUNC(r2dtank_state::audio_command_w));
	map(0xb000, 0xb000).w("crtc", FUNC(mc6845_device::address_w));
	map(0xb001, 0xb001).w("crtc", FUNC(mc6845_device::register_w));
	map(0xc000, 0xc007).ram().share("nvram");
	map(0xc800, 0xffff).rom();
}


void r2dtank_state::r2dtank_audio_map(address_map &map)
{
	map(0xd000, 0xd003).rw("pia_audio", FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xf000, 0xf000).rw(FUNC(r2dtank_state::audio_command_r), FUNC(r2dtank_state::audio_answer_w));
	map(0xf800, 0xffff).rom();
}



/*************************************
 *
 *  Port definition
 *
 *************************************/

static INPUT_PORTS_START( r2dtank )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x04, IP_ACTIVE_LOW)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(r2dtank_state, ttl74123_output_r)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "5000" )
	PORT_DIPSETTING(    0x08, "10000" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coinage ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Free_Play ) )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

INPUT_PORTS_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

void r2dtank_state::r2dtank(machine_config &config)
{
	MC6809(config, m_maincpu, MAIN_CPU_MASTER_CLOCK / 4); // divider guessed
	m_maincpu->set_addrmap(AS_PROGRAM, &r2dtank_state::r2dtank_main_map);

	M6802(config, m_audiocpu, 3.579545_MHz_XTAL);
	m_audiocpu->set_addrmap(AS_PROGRAM, &r2dtank_state::r2dtank_audio_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PIXEL_CLOCK, 360, 0, 256, 276, 0, 224);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, palette_device::BGR_3BIT);

	mc6845_device &crtc(MC6845(config, "crtc", CRTC_CLOCK));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.set_update_row_callback(FUNC(r2dtank_state::crtc_update_row));
	crtc.out_de_callback().set("74123", FUNC(ttl74123_device::a_w));

	/* 74LS123 */

	ttl74123_device &ttl74123(TTL74123(config, "74123", 0));
	ttl74123.set_connection_type(TTL74123_GROUNDED);    /* the hook up type */
	ttl74123.set_resistor_value(RES_K(22));             /* resistor connected to RCext */
	ttl74123.set_capacitor_value(CAP_U(0.01));          /* capacitor connected to Cext and RCext */
	ttl74123.set_a_pin_value(1);                        /* A pin - driven by the CRTC */
	ttl74123.set_b_pin_value(1);                        /* B pin - pulled high */
	ttl74123.set_clear_pin_value(1);                    /* Clear pin - pulled high */
	ttl74123.out_cb().set(FUNC(r2dtank_state::ttl74123_output_changed));

	PIA6821(config, m_pia_main);
	m_pia_main->readpa_handler().set_ioport("IN0");
	m_pia_main->readpb_handler().set_ioport("IN1");
	m_pia_main->cb2_handler().set(FUNC(r2dtank_state::flipscreen_w));
	m_pia_main->irqa_handler().set(FUNC(r2dtank_state::main_cpu_irq));
	m_pia_main->irqb_handler().set(FUNC(r2dtank_state::main_cpu_irq));

	PIA6821(config, m_pia_audio);
	m_pia_audio->readpa_handler().set(FUNC(r2dtank_state::AY8910_port_r));
	m_pia_audio->writepa_handler().set(FUNC(r2dtank_state::AY8910_port_w));
	m_pia_audio->writepb_handler().set(FUNC(r2dtank_state::AY8910_select_w));
	m_pia_audio->irqa_handler().set(FUNC(r2dtank_state::main_cpu_irq));
	m_pia_audio->irqb_handler().set(FUNC(r2dtank_state::main_cpu_irq));

	/* audio hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	GENERIC_LATCH_8(config, m_soundlatch2);

	AY8910(config, m_ay1, 3.579545_MHz_XTAL / 4); // probably E clock from MC6802
	m_ay1->port_a_read_callback().set_ioport("DSWB");
	m_ay1->add_route(ALL_OUTPUTS, "mono", 0.25);

	AY8910(config, m_ay2, 3.579545_MHz_XTAL / 4);
	m_ay2->port_a_read_callback().set_ioport("IN1");
	m_ay2->port_b_read_callback().set_ioport("DSWA");
	m_ay2->add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*************************************
 *
 *  ROM definition
 *
 *************************************/

ROM_START( r2dtank )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "r2d1.1c",      0xc800, 0x0800, CRC(20606a0f) SHA1(9a55e595c7ea332bdc89142338947be8a28a92a3) )
	ROM_LOAD( "r2d2.1a",      0xd000, 0x1000, CRC(7561c67f) SHA1(cccc7bbd7975db340fe571a4c31c25b41b2563b8) )
	ROM_LOAD( "r2d3.2c",      0xe000, 0x1000, CRC(fc53c538) SHA1(8f9a2edcf7a2cb2a8ddd084828b52f1bf45f434a) )
	ROM_LOAD( "r2d4.2a",      0xf000, 0x1000, CRC(56636225) SHA1(dcfc6e29b4c51a45cfbecf6790b7d88b89af433b) )

	ROM_REGION( 0x10000, "audiocpu", 0 )
	ROM_LOAD( "r2d5.7l",      0xf800, 0x0800, CRC(c49bed15) SHA1(ffa635a65c024c532bb13fb91bbd3e54923e81bf) )
ROM_END

} // anonymous namespace


/*************************************
 *
 *  Game driver
 *
 *************************************/

GAME( 1980, r2dtank, 0, r2dtank, r2dtank, r2dtank_state, empty_init, ROT270, "Sigma Enterprises Inc.", "R2D Tank", MACHINE_SUPPORTS_SAVE)
