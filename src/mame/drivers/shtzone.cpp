// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

"Sega Shooting Zone" aka "Sega Sharp Shooter"

This is an SMS with a timer system, official Sega product.  Cabinet has a lightgun, it runs the SMS light gun games.

It has 2 IO controllers, and 1 VDP, so I'm guessing the BIOS just displays to some kind of Segment LED display.

---------------------------------

Shooting Zone by SEGA 1987

834-6294

CPU(s) : D780C (x2)

Xtal : 10.7380 Mhz

RAMS(s) : D4168C (x3)
        : MB8464-12L

Eprom : Epr10894A.20

PAL : 315-5287

Customs IC's :  315-5216 (x2)
                315-5124

GAMES for this system :

Black Belt (mpr10150.ic1)

Shooting Gallery(2)

Gangster Town

Marksman Shooting / Trap Shooting / Safari Hunt (315-5028.ic1 + Mpr10157.ic2)

Fantasy Zone(1)

---------------------------------

Notes:
(1) apparently.... seems a bit odd, because it's not a gun game
(2) an auction has been observed with Out Run instead of this

Stuck at 'set cartridge', currently only the VDP and basic inputs for the timer CPU are hooked up.
It's possible to enter test mode.

TODO:
- hook up SMS CPU and carts reading, SMS-side inputs;
- outputs (test mode lists LED lamp, gun solenoid and buzzer);
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "video/315_5124.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class shtzone_state : public driver_device
{
public:
	shtzone_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	void shtzone(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	void prg_map(address_map &map);
	void io_map(address_map &map);
};

void shtzone_state::prg_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4fff).ram();
	map(0x5000, 0x5fff).ram().share("nvram");
	map(0x8000, 0x8000).portr("DSW2");
	map(0x8001, 0x8001).portr("DSW1");
	// TODO: everything below
	map(0x7ff0, 0x7ff0).nopr(); // checks for cart here?
	map(0x8000, 0x8000).nopw();
	map(0xa000, 0xa000).nopw();
	map(0xc000, 0xc000).nopw(); // bits 4-6 slot selection?
	map(0xc400, 0xc400).nopw();
	// end of TODO
	map(0xd800, 0xd800).portr("IN0");
	map(0xdc00, 0xdc00).portr("IN1");
}

void shtzone_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x3e, 0x3f).nopw(); // TODO
	map(0x7e, 0x7e).r("vdp", FUNC(sega315_5124_device::vcount_read));
	map(0x7e, 0x7f).w("vdp", FUNC(sega315_5124_device::psg_w));
	map(0xbe, 0xbe).rw("vdp", FUNC(sega315_5124_device::data_read), FUNC(sega315_5124_device::data_write));
	map(0xbf, 0xbf).rw("vdp", FUNC(sega315_5124_device::control_read), FUNC(sega315_5124_device::control_write));
	map(0xc0, 0xc0).nopr(); // TODO
}


static INPUT_PORTS_START( shtzone )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) // called Select button 1 in test mode
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) // " 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) // " 3
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) // " 4
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) // " 5
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_SERVICE_NO_TOGGLE(0x80, IP_ACTIVE_LOW)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1  )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) // does nothing in test mode
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) // "
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) // called gun 1 in test mode
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) // " 2
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) // does nothing in test mode
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // active high or nothing on screen (?)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("DSW1:8,7,6,5")
	PORT_DIPSETTING(    0x0f, DEF_STR (1C_1C) )
	PORT_DIPSETTING(    0x0e, DEF_STR (1C_2C) )
	PORT_DIPSETTING(    0x0d, DEF_STR (1C_3C) )
	PORT_DIPSETTING(    0x0c, DEF_STR (1C_4C) )
	PORT_DIPSETTING(    0x0b, DEF_STR (1C_5C) )
	PORT_DIPSETTING(    0x0a, DEF_STR (1C_6C) )
	PORT_DIPSETTING(    0x09, DEF_STR (2C_1C) )
	PORT_DIPSETTING(    0x08, DEF_STR (3C_1C) )
	PORT_DIPSETTING(    0x07, DEF_STR (4C_1C) )
	PORT_DIPSETTING(    0x06, DEF_STR (5C_1C) )
	PORT_DIPSETTING(    0x05, DEF_STR (6C_1C) )
	PORT_DIPSETTING(    0x04, DEF_STR (3C_2C) )
	PORT_DIPSETTING(    0x03, DEF_STR (4C_3C) )
	PORT_DIPSETTING(    0x02, DEF_STR (5C_3C) )
	PORT_DIPSETTING(    0x01, DEF_STR (2C_3C) )
	PORT_DIPSETTING(    0x00, DEF_STR (4C_5C) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("DSW1:4,3,2,1")
	PORT_DIPSETTING(    0xf0, DEF_STR (1C_1C) )
	PORT_DIPSETTING(    0xe0, DEF_STR (1C_2C) )
	PORT_DIPSETTING(    0xd0, DEF_STR (1C_3C) )
	PORT_DIPSETTING(    0xc0, DEF_STR (1C_4C) )
	PORT_DIPSETTING(    0xb0, DEF_STR (1C_5C) )
	PORT_DIPSETTING(    0xa0, DEF_STR (1C_6C) )
	PORT_DIPSETTING(    0x90, DEF_STR (2C_1C) )
	PORT_DIPSETTING(    0x80, DEF_STR (3C_1C) )
	PORT_DIPSETTING(    0x70, DEF_STR (4C_1C) )
	PORT_DIPSETTING(    0x60, DEF_STR (5C_1C) )
	PORT_DIPSETTING(    0x50, DEF_STR (6C_1C) )
	PORT_DIPSETTING(    0x40, DEF_STR (3C_2C) )
	PORT_DIPSETTING(    0x30, DEF_STR (4C_3C) )
	PORT_DIPSETTING(    0x20, DEF_STR (5C_3C) )
	PORT_DIPSETTING(    0x10, DEF_STR (2C_3C) )
	PORT_DIPSETTING(    0x00, DEF_STR (4C_5C) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:8") // no use shown in test mode
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:7") // no use shown in test mode
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:6") // no use shown in test mode
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("DSW2:5") // no use shown in test mode
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Game_Time ) ) PORT_DIPLOCATION("DSW2:4,3,2,1")
	PORT_DIPSETTING(    0xf0, "0:30" )
	PORT_DIPSETTING(    0xe0, "1:00" )
	PORT_DIPSETTING(    0xd0, "1:30" )
	PORT_DIPSETTING(    0xc0, "2:00" )
	PORT_DIPSETTING(    0xb0, "2:30" )
	PORT_DIPSETTING(    0xa0, "3:00" )
	PORT_DIPSETTING(    0x90, "3:30" )
	PORT_DIPSETTING(    0x80, "4:00" )
	PORT_DIPSETTING(    0x70, "4:30" )
	PORT_DIPSETTING(    0x60, "5:00" )
	PORT_DIPSETTING(    0x50, "5:30" )
	PORT_DIPSETTING(    0x40, "6:00" )
	PORT_DIPSETTING(    0x30, "6:30" )
	PORT_DIPSETTING(    0x20, "7:00" )
	PORT_DIPSETTING(    0x10, "7:30" )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
INPUT_PORTS_END


void shtzone_state::machine_start()
{
}

void shtzone_state::machine_reset()
{
}

void shtzone_state::video_start()
{
}

void shtzone_state::shtzone(machine_config &config)
{
	/* basic machine hardware */
	z80_device &timercpu(Z80(config, "timercpu", 10.738_MHz_XTAL / 4));
	timercpu.set_addrmap(AS_PROGRAM, &shtzone_state::prg_map);
	timercpu.set_addrmap(AS_IO, &shtzone_state::io_map);

	/* + SMS CPU */


	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(10.738_MHz_XTAL / 2,
			sega315_5124_device::WIDTH , sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH, sega315_5124_device::LBORDER_START + sega315_5124_device::LBORDER_WIDTH + 256,
			sega315_5124_device::HEIGHT_NTSC, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_192_TBORDER_HEIGHT, sega315_5124_device::TBORDER_START + sega315_5124_device::NTSC_192_TBORDER_HEIGHT + 192);
	screen.set_screen_update("vdp", FUNC(sega315_5124_device::screen_update));

	PALETTE(config, "palette").set_entries(0x100);

	sega315_5124_device &vdp(SEGA315_5124(config, "vdp", 10.738_MHz_XTAL));
	vdp.set_screen("screen");
	vdp.set_is_pal(false);
	vdp.n_int().set_inputline("timercpu", 0);
	vdp.n_nmi().set_inputline("timercpu", INPUT_LINE_NMI);
	vdp.add_route(ALL_OUTPUTS, "mono", 1.00);

	SPEAKER(config, "mono").front_center();
}


ROM_START( shtzone )
	ROM_REGION( 0x4000, "timercpu", 0 )
	ROM_LOAD( "epr10894a.20", 0x00000, 0x04000, CRC(ea8901d9) SHA1(43fd8bfc395e3b2e3fbe9645d692a5eb04783d9c) )
ROM_END

} // Anonymous namespace


GAME( 1987, shtzone, 0, shtzone, shtzone, shtzone_state, empty_init, ROT0, "Sega", "Shooting Zone System BIOS", MACHINE_IS_SKELETON | MACHINE_IS_BIOS_ROOT )
