// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Lee Taylor, AJR
/***************************************************************************

Cosmic Guerilla by Universal (board number 7907A)

Notes:
- background should be blue
- board can operate in b&w mode if there is no PROM, in this case
  a colour overlay should be used.

***************************************************************************/

#include "emu.h"

#include "cpu/tms9900/tms9980a.h"
#include "sound/dac.h"
#include "sound/samples.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class cosmicg_state : public driver_device
{
public:
	cosmicg_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_palette(*this, "palette")
		, m_samples(*this, "samples")
		, m_dac(*this, "dac")
		, m_in_ports(*this, "IN%u", 0)
		, m_videoram(*this, "videoram")
		, m_colormap(*this, "colormap")
	{
	}

	void cosmicg(machine_config &config);
	void init_cosmicg();

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void cosmicg_palette(palette_device &palette);
	MC6845_UPDATE_ROW(draw_scanline);

	void cosmicg_output_w(offs_t offset, u8 data);
	u8 cosmicg_port_0_r(offs_t offset);
	u8 cosmicg_port_1_r(offs_t offset);
	void cosmic_color_register_w(offs_t offset, u8 data);

	void mem_map(address_map &map) ATTR_COLD;
	void cru_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_device<palette_device> m_palette;
	required_device<samples_device> m_samples;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<3> m_in_ports;
	required_shared_ptr<u8> m_videoram;
	required_region_ptr<u8> m_colormap;

	/* video-related */
	int            m_color_registers[2]{};
	bool           m_flip_screen = false;

	/* sound-related */
	int            m_sound_enabled = 0;
	int            m_march_select = 0;
	int            m_gun_die_select = 0;
};


/*
 * Cosmic guerilla table setup
 *
 * Use AA for normal, FF for Full Red
 * Bit 0 = R, bit 1 = G, bit 2 = B, bit 4 = High Red
 *
 * It's possible that the background is dark gray and not black, as the
 * resistor chain would never drop to zero, Anybody know ?
 */
void cosmicg_state::cosmicg_palette(palette_device &palette)
{
	for (int i = 0; i < palette.entries(); i++)
	{
		int const r = (i > 8) ? 0xff : 0xaa * BIT(i, 0);
		int const g = 0xaa * BIT(i, 1);
		int const b = 0xaa * BIT(i, 2);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void cosmicg_state::cosmicg_output_w(offs_t offset, u8 data)
{
	/* Sound Enable / Disable */
	if (offset == 12)
	{
		int count;

		m_sound_enabled = data;
		if (data == 0)
			for (count = 0; count < 9; count++)
				m_samples->stop(count);
	}
	if (m_sound_enabled)
	{
		switch (offset)
		{
		/* The schematics show a direct link to the sound amp  */
		/* as other cosmic series games, but it is toggled     */
		/* once during game over. It is implemented for sake   */
		/* of completeness.                                    */
		case 1: m_dac->write(BIT(data, 0)); break; /* Game Over */
		case 2: if (data) m_samples->start(0, m_march_select); break;   /* March Sound */
		case 3: m_march_select = (m_march_select & 0xfe) | data; break;
		case 4: m_march_select = (m_march_select & 0xfd) | (data << 1); break;
		case 5: m_march_select = (m_march_select & 0xfb) | (data << 2); break;

		case 6: if (data)                           /* Killer Attack (crawly thing at bottom of screen) */
					m_samples->start(1, 8, true);
				else
					m_samples->stop(1);
				break;

		case 7: if (data)                               /* Bonus Chance & Got Bonus */
				{
					m_samples->stop(4);
					m_samples->start(4, 10);
				}
				break;

		case 8: if (data)
				{
					if (!m_samples->playing(4)) m_samples->start(4, 9, true);
				}
				else
					m_samples->stop(4);
				break;

		case 9: if (data) m_samples->start(3, 11); break;   /* Got Ship */
		case 11: /* watchdog_reset_w(0, 0); */ break;   /* Watchdog? only toggles during game play */
		case 13:    if (data) m_samples->start(8, 13 - m_gun_die_select); break;  /* Got Monster / Gunshot */
		case 14:    m_gun_die_select = data; break;
		case 15:    if (data) m_samples->start(5, 14); break;   /* Coin Extend (extra base) */
		}
	}

	if (offset == 10)
		m_flip_screen = BIT(data, 0);

	#ifdef MAME_DEBUG
	if (offset != 11) logerror("cosmicg_output_w %x=%x\n", offset, data);
	#endif
}

u8 cosmicg_state::cosmicg_port_0_r(offs_t offset)
{
	/* The top four address lines from the CRTC are bits 0-3 */
	if (offset >= 4)
		return BIT(m_in_ports[0]->read(), offset);
	else
		return BIT(m_crtc->get_ma(), offset + 8);
}

u8 cosmicg_state::cosmicg_port_1_r(offs_t offset)
{
	return BIT(m_in_ports[1]->read(), offset);
}


void cosmicg_state::cosmic_color_register_w(offs_t offset, u8 data)
{
	m_color_registers[offset] = data ? 1 : 0;
}


MC6845_UPDATE_ROW(cosmicg_state::draw_scanline)
{
	u32 *pix = &bitmap.pix(y);
	u8 const *const colors = &m_colormap[m_color_registers[0] << 8 | m_color_registers[1] << 9];

	// MA7-MA12 on schematics are actually MA6-MA11, with RA0 being nominal MA6
	if (m_flip_screen && BIT(m_in_ports[2]->read(), 1))
	{
		while (x_count-- != 0)
		{
			u16 offs = (~ma & 0x001f) | (~ra & 1) << 5 | (~ma & 0x0fe0) << 1;
			u8 data = m_videoram[offs];
			u8 color = colors[(~offs & 0x1e00) >> 5 | (~offs & 0x001e) >> 1] >> 4;
			for (int n = 8; n != 0; n--)
			{
				*pix++ = m_palette->pen(BIT(data, 0) ? color : color & 0x08);
				data >>= 1;
			}
			ma++;
		}
	}
	else
	{
		while (x_count-- != 0)
		{
			u16 offs = (ma & 0x001f) | (ra & 1) << 5 | (ma & 0x0fe0) << 1;
			u8 data = m_videoram[offs];
			u8 color = colors[(offs & 0x1e00) >> 5 | (offs & 0x001e) >> 1] & 0x0f;
			for (int n = 8; n != 0; n--)
			{
				*pix++ = m_palette->pen(BIT(data, 7) ? color : color & 0x08);
				data <<= 1;
			}
			ma++;
		}
	}
}


void cosmicg_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).rom().region("program", 0);
	map(0x2000, 0x3fff).ram().share("videoram");

	// CRTC is accessed through a small hole in the video RAM area with A11 = RS, A12 = R/W & A13 = EN
	map(0x3ff0, 0x3ff7).noprw();
	map(0x3ff1, 0x3ff1).w("crtc", FUNC(mc6845_device::address_w));
	map(0x3ff5, 0x3ff5).w("crtc", FUNC(mc6845_device::register_w));
	map(0x3ff7, 0x3ff7).r("crtc", FUNC(mc6845_device::register_r));
}

void cosmicg_state::cru_map(address_map &map)
{
	map(0x0000, 0x000f).r(FUNC(cosmicg_state::cosmicg_port_0_r));
	map(0x0010, 0x001f).r(FUNC(cosmicg_state::cosmicg_port_1_r));
	map(0x0000, 0x002b).w(FUNC(cosmicg_state::cosmicg_output_w));
	map(0x002c, 0x002f).w(FUNC(cosmicg_state::cosmic_color_register_w));
}


/* These are used for the CR handling - This can be used to */
/* from 1 to 16 bits from any bit offset between 0 and 4096 */

/* Offsets are in BYTES, so bits 0-7 are at offset 0 etc.   */

INPUT_CHANGED_MEMBER(cosmicg_state::coin_inserted)
{
	m_maincpu->set_input_line(INT_9980A_LEVEL4, newval ? ASSERT_LINE : CLEAR_LINE);
}

static INPUT_PORTS_START( cosmicg )
	PORT_START("IN0")   /* 4-7 */
	PORT_BIT( 0x0f, IP_ACTIVE_LOW, IPT_CUSTOM )    /* pixel clock */
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_2WAY

	PORT_START("IN1")   /* 8-15 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_2WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT) PORT_2WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT) PORT_2WAY PORT_COCKTAIL
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SW:2,1")
	PORT_DIPSETTING(    0x10, "1000" )
	PORT_DIPSETTING(    0x20, "1500" )
	PORT_DIPSETTING(    0x30, "2000" )
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW:3")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW:4")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x80, "5" )

	PORT_START("IN2")   /* Hard wired settings */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, cosmicg_state, coin_inserted, 0)

	/* This dip switch is not read by the program at any time   */
	/* but is wired to enable or disable the flip screen output */

	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Cocktail ) )

	/* This odd setting is marked as shown on the schematic,    */
	/* and again, is not read by the program, but wired into    */
	/* the watchdog circuit. The book says to leave it off      */

	PORT_DIPUNUSED_DIPLOC( 0x04, 0x00, "SW:6" )
INPUT_PORTS_END


static const char *const cosmicg_sample_names[] =
{
	"*cosmicg",
	"cg_m0",    /* 8 Different pitches of March Sound */
	"cg_m1",
	"cg_m2",
	"cg_m3",
	"cg_m4",
	"cg_m5",
	"cg_m6",
	"cg_m7",
	"cg_att",   /* Killer Attack */
	"cg_chnc",  /* Bonus Chance  */
	"cg_gotb",  /* Got Bonus - have not got correct sound for */
	"cg_dest",  /* Gun Destroy */
	"cg_gun",   /* Gun Shot */
	"cg_gotm",  /* Got Monster */
	"cg_ext",   /* Coin Extend */
	nullptr
};


void cosmicg_state::machine_start()
{
	m_sound_enabled = 0;
	m_march_select = 0;
	m_gun_die_select = 0;

	save_item(NAME(m_sound_enabled));
	save_item(NAME(m_march_select));
	save_item(NAME(m_gun_die_select));

	save_item(NAME(m_color_registers));
	save_item(NAME(m_flip_screen));
}

void cosmicg_state::machine_reset()
{
	m_color_registers[0] = 0;
	m_color_registers[1] = 0;
	m_flip_screen = false;

	m_maincpu->set_input_line(INT_9980A_RESET, ASSERT_LINE);
	m_maincpu->set_input_line(INT_9980A_RESET, CLEAR_LINE);
}


void cosmicg_state::cosmicg(machine_config &config)
{
	/* basic machine hardware */
	TMS9980A(config, m_maincpu, 9.828_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &cosmicg_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &cosmicg_state::cru_map);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(9.828_MHz_XTAL / 2, 312, 0, 256, 263, 0, 192);
	screen.set_screen_update(m_crtc, FUNC(mc6845_device::screen_update));

	PALETTE(config, m_palette, FUNC(cosmicg_state::cosmicg_palette), 16);

	MC6845(config, m_crtc, 9.828_MHz_XTAL / 16); // character clock is phase-locked to CPU
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(8);
	m_crtc->set_update_row_callback(FUNC(cosmicg_state::draw_scanline));

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();

	SAMPLES(config, m_samples);
	m_samples->set_channels(9);
	m_samples->set_samples_names(cosmicg_sample_names);
	m_samples->add_route(ALL_OUTPUTS, "speaker", 0.25);

	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // NE556
	// Other DACs include 3-bit binary-weighted (100K/50K/25K) DAC combined with another NE556 for attack march
}


ROM_START( cosmicg )
	ROM_REGION( 0x2000, "program", 0 )  /* 8k for code */
	ROM_LOAD( "cosmicg1.bin", 0x0000, 0x0400, CRC(e1b9f894) SHA1(bab7fd9b3db145a889542653191905b6efc5ce75) )
	ROM_LOAD( "cosmicg2.bin", 0x0400, 0x0400, CRC(35c75346) SHA1(4e50eaa0b50ab04802dc63992ad2600c227301ad) )
	ROM_LOAD( "cosmicg3.bin", 0x0800, 0x0400, CRC(82a49b48) SHA1(4cf9f684f3eb18b99a88ca879bb7083b1334f0cc) )
	ROM_LOAD( "cosmicg4.bin", 0x0c00, 0x0400, CRC(1c1c934c) SHA1(011b2b3ec4d31869fda13a3654c7bc51f3ce4dc2) )
	ROM_LOAD( "cosmicg5.bin", 0x1000, 0x0400, CRC(b1c00fbf) SHA1(136267f75e2d5b445695cabef4538f986e6f1b10) )
	ROM_LOAD( "cosmicg6.bin", 0x1400, 0x0400, CRC(f03454ce) SHA1(32c87f369475c7154fe3243d2c7be4a25444e530) )
	ROM_LOAD( "cosmicg7.bin", 0x1800, 0x0400, CRC(f33ebae7) SHA1(915bca53d5356e12c94ec765103ceced7306d1dd) )
	ROM_LOAD( "cosmicg8.bin", 0x1c00, 0x0400, CRC(472e4990) SHA1(d5797b9d89446aa6533f7515e6a5fc8368d82f91) )

	ROM_REGION( 0x0400, "colormap", 0 ) /* color map */
	ROM_LOAD( "cosmicg9.bin", 0x0000, 0x0400, CRC(689c2c96) SHA1(ddfdc3fd29c56fdebd3b1c3443a7c39f567d5355) )
ROM_END


ROM_START( cosmicgi )
	ROM_REGION( 0x2000, "program", 0 )  /* 8k for code */
	ROM_LOAD( "1g118.2h", 0x0000, 0x0400, CRC(4bda1711) SHA1(746fd15dbe08c9e2af74547c19a55a84f7b65303) )
	ROM_LOAD( "2g118.3h", 0x0400, 0x0400, CRC(3c10b2ba) SHA1(127a950d90420417a91aa3c8fabec7d7e7d526f5) )
	ROM_LOAD( "3.4h",     0x0800, 0x0400, CRC(82a49b48) SHA1(4cf9f684f3eb18b99a88ca879bb7083b1334f0cc) )
	ROM_LOAD( "4g118.5h", 0x0c00, 0x0400, CRC(42bb0611) SHA1(3894e4372f1443402ea7145b1101e1219fe2cde2) ) // changes in here cause trails when you move the ship, PCB does the same and ROM gives the same read every time, possible a bit has been flipped tho. check
	ROM_LOAD( "5.6h",     0x1000, 0x0400, CRC(b1c00fbf) SHA1(136267f75e2d5b445695cabef4538f986e6f1b10) )
	ROM_LOAD( "6.7h",     0x1400, 0x0400, CRC(f03454ce) SHA1(32c87f369475c7154fe3243d2c7be4a25444e530) )
	ROM_LOAD( "7.8h",     0x1800, 0x0400, CRC(84656c97) SHA1(2180faa07dd5bc618c80ae033babfc1191a0b890) ) // standard label but different anyway?
	ROM_LOAD( "8g128.9h", 0x1c00, 0x0400, CRC(7f48307c) SHA1(5929c131d790b0c8f9113730715531809c6840e2) )

	ROM_REGION( 0x0400, "colormap", 0 ) /* color map */ // population of this is optional, board runs as b&w without (this board didn't have it populated)
	ROM_LOAD( "cosmicg9.bin", 0x0000, 0x0400, CRC(689c2c96) SHA1(ddfdc3fd29c56fdebd3b1c3443a7c39f567d5355) )
ROM_END


void cosmicg_state::init_cosmicg()
{
	/* Program ROMs have data pins connected different from normal */
	offs_t len = memregion("program")->bytes();
	u8 *rom = memregion("program")->base();

	/* convert dummy instruction to meaningful one */

	rom[0x1e9b] ^= 0x20;
	rom[0x1e9f] ^= 0x20;

	for (offs_t offs = 0; offs < len; offs++)
	{
		u8 scrambled = rom[offs];

		u8 normal = (scrambled >> 3 & 0x11)
						| (scrambled >> 1 & 0x22)
						| (scrambled << 1 & 0x44)
						| (scrambled << 3 & 0x88);

		rom[offs] = normal;
	}
}

} // anonymous namespace


GAME( 1979, cosmicg,  0,       cosmicg, cosmicg, cosmicg_state, init_cosmicg, ROT270, "Universal", "Cosmic Guerilla", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
GAME( 1979, cosmicgi, cosmicg, cosmicg, cosmicg, cosmicg_state, init_cosmicg, ROT270, "bootleg (Inder)", "Cosmic Guerilla (Spanish bootleg)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
