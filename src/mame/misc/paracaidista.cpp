// license:BSD-3-Clause
// copyright-holders: Roberto Fresca

/***************************************************************************************

  Paracaidista
  Video Game / Electrogame, 1979.

  Driver by Roberto Fresca.


  There are two versions of the game: The original from 1979 (undumped), and a
  recreation from 2023 done by the same original programmer (Javier Valero, who
  later founded Gaelco) for the same hardware.

  Six PCBs connected by a small custom backplane.

  More info and schematics for the original 1979 version:
   https://www.recreativas.org/el-paracaidista-404-videogame-electrogame

  More info, schematics (minor differences from the original hardware), and source
  code for the 2023 version:
   https://www.recreativas.org/paracaidista-version-2023-08-15294-videogame-electrogame


----------------------------------------------------------------------------------------

  Notes:

  The checksum routine is fixed to FFs values, so don't expect
  the ROMs could pass the test. ROMS probably need to be redumped.

  Routine at $0182 --> call $08de, draws the title.
  Routine at $0185 --> call $09e1, clear the screen.

  bp 0185 to see the title.


  TODO:

  - DMA implementation (3410h)
  - 14bit timer + 2bit control (portmap 64h-65h)


****************************************************************************************/

#include "emu.h"

#include "cpu/i8085/i8085.h"
#include "machine/i8155.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class paracaidista_state : public driver_device
{
public:
	paracaidista_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi8155(*this, "i8155"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram")

	{ }

	void paracaidista(machine_config &config);

private:
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void paraca_map(address_map &map) ATTR_COLD;
	void paraca_portmap(address_map &map) ATTR_COLD;

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<i8155_device> m_ppi8155;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_shared_ptr<u8> m_vram;

	u8 input_r();
	void output_w(u8 data);
};


/*********************************************
*               Video Hardware               *
*********************************************/

uint32_t paracaidista_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= cliprect.bottom(); y++)
	{
		auto const *const src = &m_vram[y << 5];
		auto *const dst = &bitmap.pix(y);
		for (int x = cliprect.left() / 8; x <= (cliprect.right() / 8); x++)
		{
			const u8 pixel_data = src[x];
        
			// unpack 8 pixels from the byte
			for (int bit = 0; bit < 8; bit++)
				dst[(x << 3) | bit] = BIT(pixel_data, bit ^ 7);
		}
	}
	return 0;
}

void paracaidista_state::palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0x00, 0x00, 0x00));    // black
	palette.set_pen_color(1, rgb_t(0xff, 0xff, 0xff));    // white
}	


/*******************************************
*               R/W Handlers               *
*******************************************/

u8 paracaidista_state::input_r()
{
	// what's here?
	return 0;
}

void paracaidista_state::output_w(u8 data)
{
	// what's here?
}


/*********************************************
*           Memory Map Information           *
*********************************************/

void paracaidista_state::paraca_map(address_map &map)
{
	map.global_mask(0x7fff);
	map(0x0000, 0x23ff).rom();
//	map(0x3410, 0x341f).ram();                // DMA channels
	map(0x3800, 0x3800).portr("DSW");
	map(0x3c00, 0x3c00).portr("IN0");
	map(0x3e00, 0x3eff).ram();                // CMOS RAM 256x4 NV
	map(0x4000, 0x5fff).ram().share("vram");  // video RAM (size 1a00)
	map(0x6000, 0x60ff).ram();                // work RAM + stack
	map(0x7001, 0x7001).noprw();              // LEDs to see the status
}

void paracaidista_state::paraca_portmap(address_map &map)
{
	map.global_mask(0xff);
	map(0x60, 0x63).rw(m_ppi8155, FUNC(i8155_device::memory_r), FUNC(i8155_device::memory_w));
//	map(0x64, 0x64).rw timer 8 lower bits
//	map(0x65, 0x65).rw timer 6 higher bits + 2 control bits 

}


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START(paracaidista)
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN0-01")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN0-02")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN0-04")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN0-08")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN0-10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN0-20")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN0-40")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN0-80")

	PORT_START("DSW")  // 3800h
	PORT_DIPNAME( 0x01, 0x01, "DSW-01" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW-02" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW-04" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW-08" )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW-10" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW-20" )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW-40" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW-80" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*********************************************
*              Machine Drivers               *
*********************************************/

void paracaidista_state::paracaidista(machine_config &config)
{
	I8085A(config, m_maincpu, 6'553'600);
	m_maincpu->set_addrmap(AS_PROGRAM, &paracaidista_state::paraca_map);
	m_maincpu->set_addrmap(AS_IO, &paracaidista_state::paraca_portmap);

	I8155(config, m_ppi8155, 6'553'600 / 2);  // port A input, B output, C special mode 2
	m_ppi8155->in_pa_callback().set(FUNC(paracaidista_state::input_r));
	m_ppi8155->out_pb_callback().set(FUNC(paracaidista_state::output_w));
//	m_ppi8155->out_to_callback().set("speaker", FUNC(speaker_sound_device::level_w));

	// Video hardware (probably wrong values)
	screen_device &screen(SCREEN(config, m_screen, SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_screen_update(FUNC(paracaidista_state::screen_update));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 0, 208-1);
	screen.set_palette("palette");

	PALETTE(config, m_palette, FUNC(paracaidista_state::palette), 2);

	// Sound hardware
	SPEAKER(config, "mono").front_center();
}


/*********************************************
*                  Rom Load                  *
*********************************************/

// Recreation from 2023
ROM_START( paraca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "rom1.a3", 0x0000, 0x0400, CRC(5e052f4c) SHA1(4de01c21c3771eaded13b4c5a215ab44650c3133) )
	ROM_LOAD( "rom2.a2", 0x0400, 0x0400, CRC(09f80f7e) SHA1(94f9c7cf4bb55055cb1580e44865acd99000fe25) )
	ROM_LOAD( "rom3.a1", 0x0800, 0x0400, CRC(ea6b27be) SHA1(c4fbe92509faff33bfd7d5169ed21108a019464f) )
	ROM_LOAD( "rom4.b3", 0x0C00, 0x0400, CRC(0d9d9f44) SHA1(06d3e2090e38dcb64bb279c3c128835b05c4e405) )
	ROM_LOAD( "rom5.b2", 0x1000, 0x0400, CRC(33579a33) SHA1(68c7442132d6a816b53ee615b05408683178a3d3) )
	ROM_LOAD( "rom6.b1", 0x1400, 0x0400, CRC(d7719b47) SHA1(a2f3d079aa8919defdaea77450e5defb6c721669) )
	ROM_LOAD( "rom7.c3", 0x1800, 0x0400, CRC(95fa4573) SHA1(761bb8d0c2bcab5b3b3804dc24584e971e05c700) )
	ROM_LOAD( "rom8.c2", 0x1c00, 0x0400, CRC(b7e96680) SHA1(13bdc30c6b083ff08ce1228cb458bedb2060d2f6) )
	ROM_LOAD( "rom9.c1", 0x2000, 0x0400, CRC(eafe8b9e) SHA1(ab3c2e564f02b4c997cb1b58fcc1e978e77ee9db) )
	// ROMs 10, 11, and 12 unused
ROM_END


} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME     PARENT  MACHINE       INPUT         CLASS               INIT        ROT   COMPANY                     FULLNAME                     FLAGS
GAME( 2023, paraca,  0,      paracaidista, paracaidista, paracaidista_state, empty_init, ROT0, "Video Game / Electrogame", "Paracaidista (recreation)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
