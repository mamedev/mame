// license:BSD-3-Clause
// copyright-holders: Angelo Salese, David Haywood, Roberto Fresca

/*******************************************************************************************

  Champion Super (c) 1999 <unknown>

  Driver by Angelo Salese & David Haywood
  Additional work by Roberto Fresca.

  Notes:
  - To init chsuper3, chmpnum & chmpnuma, losttrea, just keep pressed both service keys
    (9 & 0), and do a soft-reset (F3).

  TODO:
  - sound.
  - ticket dispenser.
  - Trace the hold3 lamp line on the pcb,
    for a proper implementation.
  - Verify losttrea inputs and promote to working if they are ok.

*******************************************************************************************/


#include "emu.h"

#include "cpu/z180/z180.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/ramdac.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "chsuper.lh"


namespace {

class chsuper_state : public driver_device
{
public:
	chsuper_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_palette(*this, "palette")
		, m_lamps(*this, "lamp%u", 0U)
	{ }

	void chsuper(machine_config &config);
	void losttrea(machine_config &config);

	void init_chsuper3();
	void init_chmpnum();
	void init_chsuper2();
	void init_losttrea();

protected:
	virtual void machine_start() override { m_lamps.resolve(); }
	virtual void video_start() override ATTR_COLD;

private:
	int m_tilexor = 0;
	uint8_t m_blacklamp = 0;
	uint8_t m_redlamp = 0;
	std::unique_ptr<uint8_t[]> m_vram;

	required_device<z180_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	output_finder<7> m_lamps;

	void vram_w(offs_t offset, uint8_t data);
	void outporta_w(uint8_t data);
	void outportb_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void losttrea_portmap(address_map &map) ATTR_COLD;
	void portmap(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;
};



/***************************
*      Video Hardware      *
***************************/

void chsuper_state::video_start()
{
	m_vram = make_unique_clear<uint8_t[]>(1 << 14);

	save_item(NAME(m_blacklamp));
	save_item(NAME(m_redlamp));
	save_pointer(NAME(m_vram), 0x4000);
}

uint32_t chsuper_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int count = 0x0000;

	for (int y = 0; y < 64; y++)
	{
		for (int x = 0; x < 128; x++)
		{
			int const tile = ((m_vram[count + 1] << 8) | m_vram[count]) & 0xffff;

			gfx->opaque(bitmap, cliprect, tile, 0, 0, 0, x * 4, y * 8);
			count += 2;
		}
	}

	return 0;
}

void chsuper_state::vram_w(offs_t offset, uint8_t data)
{
	m_vram[offset] = data;
}


/* Output Ports...

  Port EEh:
  - bits -
  7654 3210
  ---- ---x   Coin counter.
  ---- --x-   Hold 1 / Black (Nero) / Bet Max lamp.
  ---- -x--   Payout counter.
  ---- x---   Hold 2 / Low (Bassa) lamp.
  ---x ----   unknown (unused).
  --x- ----   Bet lamp.
  -x-- ----   Ticket Out.
  x--- ----   unknown (unused).

  Port EFh:
  - bits -
  7654 3210
  ---- ---x   unknown (used).
  ---- --x-   unknown (unused).
  ---- -x--   Hold 4 / High (Alta) lamp.
  ---- x---   unknown (unused).
  ---x ----   unknown (unused).
  --x- ----   Hold 5 / Red (Rosso) / Gamble (Raddoppio) lamp.
  -x-- ----   Start / Gamble (Raddoppio) lamp.
  x--- ----   unknown (unused).

  Port EDh seems to be a mirror of EFh, but with bit0 always active.

  After reversing every port, I've not found a proper and dedicated line
  for the Hold 3 lamp, so my guess is that it's created through TTL'ing
  other lines through some kind of circuitry.

  Since holds 1 & 5 lamps *and* holds 2 & 4 lamps are flashing in a
  complementary way for the double-up "Black/Red" and "High/Low"
  functions, I strongly think that hold 3 lamp is lit when both
  of a complementary pair are lit together simultaneously.

  Anyways... Need to be checked against a real PCB.

*/

void chsuper_state::outporta_w(uint8_t data)  // Port EEh
{
	machine().bookkeeping().coin_counter_w(0, data & 0x01);  // Coin counter
	m_lamps[0] = BIT(data, 1);  // Hold 1 / Black (Nero) lamp.
	machine().bookkeeping().coin_counter_w(1, data & 0x04);  // Payout / Ticket Out pulse
	m_lamps[1] = BIT(data, 3);  // Hold 2 / Low (Bassa) lamp.
	// D4: unused...
	m_lamps[5] = BIT(data, 5);  // BET lamp
	// D6: ticket motor...
	// D7: unused...

/*  Workaround to get the HOLD 3 lamp line active,
    from the HOLD 1 and HOLD 5 lamps status...
*/
	m_blacklamp = (data >> 1) & 1;              // latching the BLACK lamp status...

	if ((m_blacklamp == 1) & (m_redlamp == 1))  // if both are ON...
	{
		m_lamps[2] = 1;            // HOLD 3 ON
	}
	else
	{
		m_lamps[2] = 0;            // otherwise HOLD 3 OFF
	}
}

void chsuper_state::outportb_w(uint8_t data)  // Port EFh
{
	// D0: unknown...
	// D1: unused...
	m_lamps[3] = BIT(data, 2);  // Hold 4 / High (Alta) lamp.
	// D3: unused...
	// D4: unused...
	m_lamps[4] = BIT(data, 5);  // Hold 5 / Red (Rosso) / Gamble (Raddoppio) lamp.
	m_lamps[6] = BIT(data, 6);  // Start / Gamble (Raddoppio) lamp.
	// D7: unused...

/*  Workaround to get the HOLD 3 lamp line active,
    from the HOLD 1 and HOLD 5 lamps status...
*/
	m_redlamp = (data >> 5) & 1;    // latching the RED lamp status...

	if ((m_blacklamp == 1) & (m_redlamp == 1))  // if both are ON...
	{
		m_lamps[2] = 1;    // Hold 3 ON
	}
	else
	{
		m_lamps[2] = 0;    // Hold 3 OFF
	}
}


/***************************
*   Memory Map handlers    *
***************************/

void chsuper_state::prg_map(address_map &map)
{
	map(0x00000, 0x1ffff).rom();
	map(0x00000, 0x01fff).w(FUNC(chsuper_state::vram_w));
	map(0xfb000, 0xfbfff).ram().share("nvram");
}

void chsuper_state::losttrea_portmap(address_map &map)
{
	map(0x0000, 0x003f).ram(); // Z180 internal regs
	map(0x00e8, 0x00e8).portr("IN0");
	map(0x00e9, 0x00e9).portr("IN1");
	map(0x00ed, 0x00ed).nopw(); // mirror of EFh, but with bit0 active...
	map(0x00ee, 0x00ee).w(FUNC(chsuper_state::outporta_w));
	map(0x00ef, 0x00ef).w(FUNC(chsuper_state::outportb_w));
	map(0x00fc, 0x00fc).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x00fd, 0x00fd).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x00fe, 0x00fe).w("ramdac", FUNC(ramdac_device::mask_w));
	map(0x8300, 0x8300).portr("IN2");  // valid input port present in test mode.
	map(0xff20, 0xff3f).w("dac", FUNC(dac_byte_interface::data_w)); // unk writes
}

void chsuper_state::portmap(address_map &map)
{
	losttrea_portmap(map);

	map(0x00ea, 0x00ea).portr("DSW");
}

void chsuper_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}


/* About Sound...

  Samples are PCM unsigned 8-bit, mono.
  These are embedded into the program ROM, starting
  at the 0x10000 offset.

  The audio system is currently unknown, but the sound writes
  are the following...

  For any sample triggered:

  FF39h --> 0x02
  FF30h --> 0x30

  then the following 5 writes (per sample number)...

  SND# (hex):  00    01    02    03    04    05    06    07    08    09    0A    0B    0C    0D    0E    0F    10    11    12    13    14
  Writes...
  FF2Ah:      0x00  0xEA  0x01  0x76  0x00  0xF9  0x01  0x0F  0x00  0x84  0x01  0x01  0x00  0x01  0x01  0x01  0x00  0x01  0x01  0x8C  0x00
  FF29h:      0xFF  0xF6  0x04  0x05  0xFF  0x05  0x04  0x75  0xFF  0xF8  0x04  0x00  0xFF  0x00  0x04  0x21  0xFF  0x21  0x04  0x0D  0xFF
  FF28h:      0xFF  0x0F  0xF8  0x69  0xFF  0x75  0x83  0x05  0xFF  0x0D  0x90  0xF8  0xFF  0x00  0x20  0x00  0xFF  0x00  0x00  0x9C  0xFF
  FF2Fh:      0xF7  0x0F  0xFF  0xFF  0x06  0x00  0xFF  0xFF  0xF9  0x0D  0xFD  0xFF  0x01  0x04  0x1C  0xFF  0x22  0xE3  0xFF  0xFF  0xBB
  FF2Eh:      0xF0  0x17  0xFF  0xFF  0x8A  0xF2  0xFF  0xFF  0xF2  0x7D  0x98  0xFF  0xFF  0x20  0xE0  0xFF  0xFF  0x00  0xFF  0xFF  0x1F

  then ending with...

  FF2Ch --> 0x00
  FF2Bh --> 0xF8
  FF30h --> 0x90
  FF39h --> 0x00

*/


/***************************
*  Input Ports definition  *
***************************/

static INPUT_PORTS_START( chsuper )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_Q) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Black (Nero) / Bet Max")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_NAME("Bet / Cancel All / Take (Ritira)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )      PORT_NAME("Start / Double (Radoppio)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Red (Rosso) / Double (Radoppio)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )    PORT_NAME("Service 2 - Statistica Parziale")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )    PORT_NAME("Service 1 - Management")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / High (Alta)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Low (Bassa)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )  // ticket-in in chmpnum
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // ticket out / payout in chsuper2

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Play Graphics" )     PORT_DIPLOCATION("DSW:3")
	PORT_DIPSETTING(    0x04, "Play Numbers" )
	PORT_DIPSETTING(    0x00, "Play Cards" )
	PORT_DIPNAME( 0x08, 0x08, "Abilita" )           PORT_DIPLOCATION("DSW:4")
	PORT_DIPSETTING(    0x08, "Sensa Abilita" )
	PORT_DIPSETTING(    0x00, "Con Abilita" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coinage ) )  PORT_DIPLOCATION("DSW:5,6")
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("DSW:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( losttrea )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER )       PORT_CODE(KEYCODE_Q) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Hold 1 / Black (Nero) / Bet Max")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_BET )  PORT_NAME("Bet / Cancel All / Take (Ritira)")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )      PORT_NAME("Start / Double (Radoppio)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Hold 5 / Red (Rosso) / Double (Radoppio)")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE2 )    PORT_NAME("Service 2 - Statistica Parziale")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE1 )    PORT_NAME("Service 1 - Management")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Hold 4 / High (Alta)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Hold 2 / Low (Bassa)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )  // ticket-in in chmpnum
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  // ticket out / payout in chsuper2

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	// no dips on PCB
INPUT_PORTS_END

/*****************************
*  Graphics Decode Routines  *
*****************************/

static const gfx_layout charlayout =
{
	4,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7},
	{ 2*8,3*8,0*8,1*8 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32},
	8*32
};

static GFXDECODE_START( gfx_chsuper )
	GFXDECODE_ENTRY( "chars", 0x00000, charlayout,   0, 1 )
GFXDECODE_END


/***************************
*     Machine Drivers      *
***************************/

void chsuper_state::chsuper(machine_config &config)
{
	// basic machine hardware
	Z80180(config, m_maincpu, 16_MHz_XTAL); // Z8018006VSC (but can actually take 8 MHz?)
	m_maincpu->set_addrmap(AS_PROGRAM, &chsuper_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &chsuper_state::portmap);
	m_maincpu->set_vblank_int("screen", FUNC(chsuper_state::irq0_line_hold));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(57);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_screen_update(FUNC(chsuper_state::screen_update));
	screen.set_size(64*8, 64*8);
	screen.set_visarea(0*8, 48*8-1, 0, 30*8-1);
	screen.set_palette(m_palette);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_chsuper);
	PALETTE(config, m_palette).set_entries(0x100);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette)); // ADV476KP50
	ramdac.set_addrmap(0, &chsuper_state::ramdac_map);

	// sound hardware
	SPEAKER(config, "speaker").front_center();

	DAC_8BIT_R2R(config, "dac", 0).add_route(ALL_OUTPUTS, "speaker", 0.25); // 74HC273 latch + R2R network (unknown values)
}

void chsuper_state::losttrea(machine_config &config)
{
	chsuper(config);

	m_maincpu->set_addrmap(AS_IO, &chsuper_state::losttrea_portmap);
}


/***************************
*  ROM Regions definition  *
***************************/

ROM_START( chsuper3 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "c.bin",  0x0000, 0x80000, CRC(e987ed1f) SHA1(8d1ee01914356714c7d1f8437d98b41a707a174a) )

	ROM_REGION( 0x100000, "chars", 0 )
	ROM_LOAD( "a.bin",  0x00000, 0x80000, CRC(ace8b591) SHA1(e9ba5efebdc9b655056ed8b2621f062f50e0528f) )
	ROM_LOAD( "b.bin",  0x80000, 0x80000, CRC(5f58c722) SHA1(d339ae27af010b058eae9084fba85fb2fbed3952) )

	ROM_REGION( 0x80000, "adpcm", 0 )
	ROM_COPY( "maincpu", 0x10000, 0x00000, 0x70000 )
ROM_END

ROM_START( chsuper2 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "chsuper2-c.bin",  0x0000, 0x80000, CRC(cbf59e69) SHA1(68e4b167fdf9103fd748cff401f4fe7c1d214552) )

	ROM_REGION( 0x100000, "chars", 0 )
	ROM_LOAD( "chsuper2-a.bin",  0x00000, 0x80000, CRC(7caa8ebe) SHA1(440306a208ec8afd570b15f05b5dc542acc98510) )
	ROM_LOAD( "chsuper2-b.bin",  0x80000, 0x80000, CRC(7bb463d7) SHA1(fb3842ba53e545fa47574c91df7281a9cb417395) )

	ROM_REGION( 0x80000, "adpcm", 0 )
	ROM_COPY( "maincpu", 0x10000, 0x00000, 0x70000 )
ROM_END

ROM_START( chmpnum )
	ROM_REGION( 0x80000, "maincpu", 0 ) // code + samples
	ROM_LOAD( "3.ic11", 0x00000, 0x80000, CRC(46aa2ce7) SHA1(036d67a26c890c4dc26599bfcd2c67f12e30fb52) )

	ROM_REGION( 0x100000, "chars", 0 )
	ROM_LOAD( "1.ic18", 0x00000, 0x80000, CRC(8e202eaa) SHA1(156b498873111e5890c00d447201ba4bcbe6e633) )
	ROM_LOAD( "2.ic19", 0x80000, 0x80000, CRC(dc0790b0) SHA1(4550f85e609338635a3987f7832517ed1d6388d4) )

	ROM_REGION( 0x80000, "adpcm", 0 )
	ROM_COPY( "maincpu", 0x10000, 0x00000, 0x70000 )
ROM_END

/*
  Champion Number (v0.67)
  Year: 1999

  CPUs
  1x Z8018006VSC (8-bit Microprocessor).
  1x TDA2003     (Audio Amplifier).
  1x oscillator 16.000 (xt1).

  ROMs
  1x M27C4001 (3) dumped.
  2x TMS27C040 (1, 2) dumped.

  RAMs
  2x ZMDU6264ADC-07LL.
  1x ADV476KP50.

  PLDs
  1x XC9572-PC84AKJ (read protected).
  1x XC9536-PC44ASJ (read protected).

  Others
  1x 28x2 edge connector.
  1x trimmer (volume).
  1x battery (3,6V).

*/
ROM_START( chmpnuma )
	ROM_REGION( 0x80000, "maincpu", 0 ) // code + samples
	ROM_LOAD( "c.n.v.6.7.ic11", 0x00000, 0x80000, CRC(11a8cfcc) SHA1(a8ac6cea23841df55d636f48e4071ea4ed16119b) )

	ROM_REGION( 0x100000, "chars", 0 )
	ROM_LOAD( "c.number_1.ic18", 0x00000, 0x80000, CRC(8e202eaa) SHA1(156b498873111e5890c00d447201ba4bcbe6e633) )
	ROM_LOAD( "c.number_2.ic19", 0x80000, 0x80000, CRC(dc0790b0) SHA1(4550f85e609338635a3987f7832517ed1d6388d4) )

	ROM_REGION( 0x80000, "adpcm", 0 )
	ROM_COPY( "maincpu", 0x10000, 0x00000, 0x70000 )
ROM_END

ROM_START( losttrea ) // on a slightly newer PCB with more GFX ROM space. Labels are Lost Island, there's still Lost Island GFX in ROMs but title screen is Lost Treasure
	ROM_REGION( 0x80000, "maincpu", 0 ) // code + samples
	ROM_LOAD( "lost_island_1.ic10", 0x00000, 0x80000, CRC(4444cf61) SHA1(d3d84a05042e3cf66c53648148f5e78035800cb9) )

	ROM_REGION( 0x200000, "chars", ROMREGION_ERASE00 )
	ROM_LOAD( "lost_island_2.ic19", 0x000000, 0x080000, CRC(09d37a82) SHA1(92e131d3d6dfdc6a0f5d69cdf7a93539673852a4) )
	ROM_LOAD( "lost_island_3.ic15", 0x080000, 0x080000, CRC(48209662) SHA1(d921f6b39351b0c11818531b5f57fccb49f553af) )
	ROM_LOAD( "lost_island_4.ic16", 0x100000, 0x080000, CRC(866ec65e) SHA1(001ea242c07b79f334bce5c12d488188d1f38d1d) )
	// ic 17 empty socket (seems unused)

	ROM_REGION( 0x80000, "adpcm", 0 )
	ROM_COPY( "maincpu", 0x20000, 0x00000, 0x60000 ) // ?
ROM_END


/*************************
*      Driver Init       *
*************************/

void chsuper_state::init_chsuper2()
{
	std::unique_ptr<uint8_t[]> buffer;
	uint8_t *rom = memregion("chars")->base();

	m_tilexor = 0x7f00;

	buffer = std::make_unique<uint8_t[]>(0x100000);

	for (int i = 0; i < 0x100000; i++)
	{
		int j = i ^ (m_tilexor << 5);

		buffer[j] = rom[i];
	}

	memcpy(rom, buffer.get(), 0x100000);
}

void chsuper_state::init_chsuper3()
{
	std::unique_ptr<uint8_t[]> buffer;
	uint8_t *rom = memregion("chars")->base();

	m_tilexor = 0x0e00;

	buffer = std::make_unique<uint8_t[]>(0x100000);

	for (int i = 0; i < 0x100000; i++)
	{
		int j = i ^ (m_tilexor << 5);

		buffer[j] = rom[i];
	}

	memcpy(rom, buffer.get(), 0x100000);
}

void chsuper_state::init_chmpnum()
{
	std::unique_ptr<uint8_t[]> buffer;
	uint8_t *rom = memregion("chars")->base();

	m_tilexor = 0x1800;

	buffer = std::make_unique<uint8_t[]>(0x100000);

	for (int i = 0; i < 0x100000; i++)
	{
		int j = i ^ (m_tilexor << 5);

		j = bitswap<24>(j , 23, 22, 21, 20, 19, 18, 17, 15, 14, 13, 16, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

		buffer[j] = rom[i];
	}

	memcpy(rom, buffer.get(), 0x100000);
}

void chsuper_state::init_losttrea()
{
	std::unique_ptr<uint8_t[]> buffer;
	uint8_t *rom = memregion("chars")->base();

	m_tilexor = 0x1900;

	buffer = std::make_unique<uint8_t[]>(0x200000);

	for (int i = 0; i < 0x200000; i++)
	{
		int j = i ^ (m_tilexor << 5);

		j = bitswap<24>(j , 23, 22, 21, 20, 19, 18, 17, 15, 13, 14, 16, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

		buffer[j] = rom[i];
	}

	memcpy(rom, buffer.get(), 0x200000);
}

} // anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//     YEAR  NAME      PARENT    MACHINE   INPUT     CLASS          INIT           ROT   COMPANY         FULLNAME                    FLAGS                                                                 LAYOUT
GAMEL( 1999, chsuper3, 0,        chsuper,  chsuper,  chsuper_state, init_chsuper3, ROT0, "<unknown>",    "Champion Super 3 (V0.35)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE,                      layout_chsuper ) //24/02/99
GAMEL( 1999, chsuper2, chsuper3, chsuper,  chsuper,  chsuper_state, init_chsuper2, ROT0, "<unknown>",    "Champion Super 2 (V0.13)", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE,                      layout_chsuper ) //26/01/99
GAME(  1999, chmpnum,  chsuper3, chsuper,  chsuper,  chsuper_state, init_chmpnum,  ROT0, "<unknown>",    "Champion Number (V0.74)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )                                      //10/11/99
GAME(  1999, chmpnuma, chsuper3, chsuper,  chsuper,  chsuper_state, init_chmpnum,  ROT0, "<unknown>",    "Champion Number (V0.67)",  MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )                                      //21/10/99
GAME(  2001, losttrea, 0,        losttrea, losttrea, chsuper_state, init_losttrea, ROT0, "<unknown>",    "Lost Treasure (V1.03)",    MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )                                      //04/05/01, there's also a Lost Island string
