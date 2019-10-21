// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*

    CES Classic wall games

    driver by Angelo Salese

    Notes:
    - to play Home Run Classic you have to select a pitcher shot and keep pressed the
      wall strobe. When you release the strobe, batter does the swing.

    TODO:
    - custom layout for dual LCDs
    - artwork and lamps position needed to make progresses
    - U43 and U44 bad in Trap Shoot Classic
    - games are incredibly sluggish by now
    - irq sources are unknown
    - sound doesn't play most samples
    - Trap Shoot Classic runs on a single LCD, needs mods

*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "machine/nvram.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class cesclassic_state : public driver_device
{
public:
	cesclassic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_oki(*this, "oki")
		, m_vram(*this, "vram")
		, m_palette(*this, "palette")
	{ }

	DECLARE_WRITE16_MEMBER(irq2_ack_w);
	DECLARE_WRITE16_MEMBER(irq3_ack_w);
	DECLARE_WRITE16_MEMBER(lamps_w);
	DECLARE_WRITE16_MEMBER(outputs_w);


	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void cesclassic_palette(palette_device &palette) const;
	void cesclassic(machine_config &config);
	void cesclassic_map(address_map &map);
protected:

	// devices
	required_device<cpu_device> m_maincpu;
	required_device<okim6295_device> m_oki;

	required_shared_ptr<uint16_t> m_vram;
	required_device<palette_device> m_palette;
	// driver_device overrides
	virtual void video_start() override;

};


void cesclassic_state::video_start()
{
}

uint32_t cesclassic_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y,xi;

	bitmap.fill(m_palette->black_pen(), cliprect);

	{
		for(y=0;y<64;y++)
		{
			for(x=0;x<16;x++)
			{
				for(xi=0;xi<16;xi++)
				{
					uint8_t color;

					color = (((m_vram[x+y*16+0x400])>>(15-xi)) & 1);
					color |= (((m_vram[x+y*16])>>(15-xi)) & 1)<<1;

					if((x*16+xi)<256 && ((y)+0)<256)
						bitmap.pix32(y, x*16+xi) = m_palette->pen(color);
				}
			}
		}
	}

	return 0;
}

WRITE16_MEMBER( cesclassic_state::irq2_ack_w )
{
	m_maincpu->set_input_line(2, CLEAR_LINE);
}

WRITE16_MEMBER( cesclassic_state::irq3_ack_w )
{
	m_maincpu->set_input_line(3, CLEAR_LINE);
}

WRITE16_MEMBER( cesclassic_state::lamps_w )
{
	//popmessage("%04x",data);
}

WRITE16_MEMBER( cesclassic_state::outputs_w )
{
	/*
	-x-- ---- OKI bankswitch
	--x- ---- probably screen enable
	---- --x- coin counter
	*/
	m_oki->set_rom_bank((data & 0x40) >> 6);
	machine().bookkeeping().coin_counter_w(0, data & 2);
	if(data & ~0x62)
	logerror("Output: %02x\n",data);
}

void cesclassic_state::cesclassic_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x400000, 0x40cfff).ram();
	map(0x40d000, 0x40ffff).ram().share("vram");
	map(0x410000, 0x410001).portr("VBLANK"); //probably m68681 lies there instead
	map(0x410004, 0x410005).w(FUNC(cesclassic_state::irq3_ack_w));
	map(0x410006, 0x410007).w(FUNC(cesclassic_state::irq2_ack_w));
	map(0x480000, 0x481fff).ram().share("nvram"); //8k according to schematics (games doesn't use that much tho)
	map(0x600000, 0x600001).portr("SYSTEM");
	map(0x610000, 0x610001).w(FUNC(cesclassic_state::outputs_w));
//  map(0x640000, 0x640001).nopw();
	map(0x640040, 0x640041).w(FUNC(cesclassic_state::lamps_w));
	map(0x670000, 0x670001).portr("DSW");
	map(0x70ff00, 0x70ff01).nopw(); // writes 0xffff at irq 3 end of service, watchdog?
	map(0x900001, 0x900001).r(m_oki, FUNC(okim6295_device::read)); // unsure about this ...
	map(0x900101, 0x900101).w(m_oki, FUNC(okim6295_device::write));
//  map(0x904000, 0x904001).nopw(); //some kind of serial
}

static INPUT_PORTS_START( cesclassic )
	PORT_START("SYSTEM")
	PORT_DIPNAME( 0x0001, 0x0001, "SYSTEM" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "SYSTEM" ) // hangs system at POST if active
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_BIT(0x1000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT(0x2000, IP_ACTIVE_HIGH, IPT_SERVICE )
	PORT_BIT(0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT(0x8000, IP_ACTIVE_LOW, IPT_BUTTON1 ) // hit strobe

	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, "DSW" )
	PORT_DIPSETTING(    0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, "SYSTEM" )
	PORT_DIPSETTING(    0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) ) // LCD test
	PORT_DIPSETTING(    0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x0000, DEF_STR( On ) )

	PORT_START("VBLANK")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("l_lcd")
INPUT_PORTS_END

void cesclassic_state::cesclassic_palette(palette_device &palette) const
{
	for (int i = 0; i < 4; i++)
		palette.set_pen_color(i, pal2bit(i), 0, 0);
}

void cesclassic_state::cesclassic(machine_config &config)
{
	M68000(config, m_maincpu, 24000000/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &cesclassic_state::cesclassic_map);
	m_maincpu->set_vblank_int("l_lcd", FUNC(cesclassic_state::irq2_line_assert));  // TODO: unknown sources
	m_maincpu->set_periodic_int(FUNC(cesclassic_state::irq3_line_assert), attotime::from_hz(60*8));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "l_lcd", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_screen_update(FUNC(cesclassic_state::screen_update));
	screen.set_size(8*16*2, 8*8+3*8);
	screen.set_visarea(0*8, 8*16*2-1, 0*8, 8*8-1);

	PALETTE(config, m_palette, FUNC(cesclassic_state::cesclassic_palette), 4);

	SPEAKER(config, "mono").front_center();
	OKIM6295(config, m_oki, 24000000/16, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.5);
}


ROM_START(hrclass)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("hrclassic-v121-u44.bin", 0x000000, 0x80000, CRC(cbbbbbdb) SHA1(d406a27a823f5e530a9cf7615c396fe52df1c387) )
	ROM_LOAD16_BYTE("hrclassic-v121-u43.bin", 0x000001, 0x80000, CRC(f136aec3) SHA1(7823e81eb79c7575c1d6c2ae0848c2b9943ee6ef) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "hrclassic-v100-u28.bin", 0x00000, 0x80000, CRC(45d15b3a) SHA1(a11ce27a77ea353034c5f498cb46ef5ed787b0f9) )
ROM_END

ROM_START(ccclass)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD16_BYTE("ccclassic-v110-u44.bin", 0x000000, 0x80000, CRC(63b63f3a) SHA1(d4b6f401815b05ac0c6c259bb066663d4c2ee132) )
	ROM_LOAD16_BYTE("ccclassic-v110-u43.bin", 0x000001, 0x80000, CRC(c1b420df) SHA1(9f1f22e6b27abcede6880a1a8ad5399ce582dab1) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "ccclassic-v100-u28.bin", 0x00000, 0x80000, CRC(94190a55) SHA1(fb219401431747fc3840da02c4e933d4c23049b7) )
ROM_END

ROM_START(tsclass)
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("tsclassic-v100-u43u44.bin", 0x000000, 0x100000, BAD_DUMP CRC(a820ec9a) SHA1(84e38c7e54bb9e80142ed4e7763c9e36df560f42) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "tsclassic-v100-u28.bin", 0x00000, 0x80000, CRC(5bf53ca3) SHA1(5767391175fa9488ba0fb17a16de6d5013712a01) )
ROM_END


GAME(1997, hrclass, 0, cesclassic, cesclassic, cesclassic_state, empty_init, ROT0, "Creative Electronics And Software", "Home Run Classic (v1.21 12-feb-1997)",     MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
GAME(1997, ccclass, 0, cesclassic, cesclassic, cesclassic_state, empty_init, ROT0, "Creative Electronics And Software", "Country Club Classic (v1.10 03-apr-1997)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
GAME(1997, tsclass, 0, cesclassic, cesclassic, cesclassic_state, empty_init, ROT0, "Creative Electronics And Software", "Trap Shoot Classic (v1.0 21-mar-1997)",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND )
