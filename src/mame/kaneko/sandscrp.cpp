// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*

    Sand Scorpion    (C) 1992 FACE

driver by   Luca Elia (l.elia@tin.it)

PCB Number: Z03VA-001

      CPU: TMP68HC000N-12
Sound CPU: Z8400AB1 (Z80A)
    Sound: OKI6295, YM2203C & Y3014B
      OSC: 16.000mhz & 12.000mhz
     Dips: Two 8-way dipswitch banks

Kaneko custom (surfaced scratched):
   PX79C480FP-3 PANDORA-CHIP (160pin PQFP)  <- Sprites
   VIEW2-CHIP (144pin PQFP)                 <- Tilemaps
   HELP1-CHIP (64pin PQFP)
   CALC1-CHIP (40pin DIP)                   <- Collision Detection etc.


+-------------------------------------+
|  VOL   M6295 7.IC55   Z80A   HELP1  |
|        Y3014B YM2203C 8.IC51        |
|            DSW1 DSW2 LH5168D  4.IC32|
|        12MHz 16MHz   LH5168D  3.IC33|
|J                     LH5168D   IC30*|
|A                       VIEW2   IC31*|
|M       LH5116D             M51257AL |
|M       LH5116D        PAL  M51257AL |
|A                               2.IC5|
|         PAL                    1.IC4|
|         IC15*       PANDORA    68000|
|  IC14* 5.IC16   M41464 M41464  CALC1|
|  IC18* 6.IC17   M41464 M41464       |
+-------------------------------------+

IC18 unpopulated 28 pin socket (silkscreened EPROM)
IC14 & IC15 unpopulated 32 pin socket (silkscreened MASK)
IC30 & IC31 unpopulated 42 pin socket (silkscreened MASK)

RAM:
 OKI M41464C-10 (x4)
 OKI M51257AL-10 (x2)
 Sharp LH5116D-10 (x2)
 Sharp LH5168D-10L (x3)

PALS:
 AMI 18CV8PC-25 (Stamped IC25D located at IC25)
 AMI 18CV8PC-25 (Stamped IC26 located at IC26)

Chips simply labeled 1 through 8

ROM ID    ROM Type
===================
1.ic4     27C2001
2.ic5     27C2001
3.ic33    27C040
4.ic32    27C040
5.ic16    27C040
6.ic17    27C040
7.ic55    27C2001
8.ic51    27C1001

Alternate Program roms:
11.ic4    27C2001
12.ic5    27C2001

Is there another alt program rom set labeled 9 & 10?

*/

#include "emu.h"

#include "kan_pand.h"
#include "kaneko_hit.h"
#include "kaneko_tmap.h"

#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/watchdog.h"
#include "sound/okim6295.h"
#include "sound/ymopn.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class sandscrp_state : public driver_device
{
public:
	sandscrp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_pandora(*this, "pandora")
		, m_view2(*this, "view2")
		, m_soundlatch(*this, "soundlatch%u", 1)
		, m_audiobank(*this, "audiobank")
	{ }

	void sandscrp(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<kaneko_pandora_device> m_pandora;
	required_device<kaneko_view2_tilemap_device> m_view2;
	required_device_array<generic_latch_8_device, 2> m_soundlatch;

	required_memory_bank m_audiobank;

	u8 m_sprite_irq;
	u8 m_unknown_irq;
	u8 m_vblank_irq;
	bool m_latch_full[2];

	u8 irq_cause_r();
	void irq_cause_w(u8 data);
	void coincounter_w(u8 data);
	template<unsigned Latch> u8 soundlatch_r();
	template<unsigned Latch> void soundlatch_w(u8 data);
	u8 latchstatus_68k_r();
	void latchstatus_68k_w(u8 data);
	void bankswitch_w(u8 data);
	u8 latchstatus_r();

	virtual void machine_start() override ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank(int state);

	INTERRUPT_GEN_MEMBER(interrupt);
	void update_irq_state();
	void sandscrp_mem(address_map &map) ATTR_COLD;
	void sandscrp_soundmem(address_map &map) ATTR_COLD;
	void sandscrp_soundport(address_map &map) ATTR_COLD;
};


u32 sandscrp_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	screen.priority().fill(0, cliprect);

	m_view2->prepare(bitmap, cliprect);

	for ( int l = 0; l < 4; l++ )
	{
		m_view2->render_tilemap(screen,bitmap,cliprect,l);
	}

	// copy sprite bitmap to screen
	m_pandora->update(bitmap, cliprect);

	for ( int h = 4; h < 8; h++ ) // high bit of tile priority : above sprites
	{
		m_view2->render_tilemap(screen,bitmap,cliprect,h);
	}

	return 0;
}


void sandscrp_state::machine_start()
{
	m_audiobank->configure_entries(0, 8, memregion("audiocpu")->base(), 0x4000);

	save_item(NAME(m_sprite_irq));
	save_item(NAME(m_unknown_irq));
	save_item(NAME(m_vblank_irq));
	save_item(NAME(m_latch_full));
}


/* Update the IRQ state based on all possible causes */
void sandscrp_state::update_irq_state()
{
	if (m_vblank_irq || m_sprite_irq || m_unknown_irq)
		m_maincpu->set_input_line(1, ASSERT_LINE);
	else
		m_maincpu->set_input_line(1, CLEAR_LINE);
}


/* Called once/frame to generate the VBLANK interrupt */
INTERRUPT_GEN_MEMBER(sandscrp_state::interrupt)
{
	m_vblank_irq = 1;
	update_irq_state();
}


void sandscrp_state::screen_vblank(int state)
{
	// rising edge
	if (state)
	{
		m_sprite_irq = 1;
		update_irq_state();
		m_pandora->eof();
	}
}

/* Reads the cause of the interrupt */
u8 sandscrp_state::irq_cause_r()
{
	return  ( m_sprite_irq  ?  0x08  : 0 ) |
			( m_unknown_irq ?  0x10  : 0 ) |
			( m_vblank_irq  ?  0x20  : 0 ) ;
}


/* Clear the cause of the interrupt */
void sandscrp_state::irq_cause_w(u8 data)
{
//  m_sprite_flipx  =   data & 1;
//  m_sprite_flipy  =   data & 1;

	if (BIT(data, 3))    m_sprite_irq  = 0;
	if (BIT(data, 4))    m_unknown_irq = 0;
	if (BIT(data, 5))    m_vblank_irq  = 0;

	update_irq_state();
}


/***************************************************************************
                                Sand Scorpion
***************************************************************************/

void sandscrp_state::coincounter_w(u8 data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 0));
	machine().bookkeeping().coin_counter_w(1, BIT(data, 1));
}


u8 sandscrp_state::latchstatus_68k_r()
{
	return  (m_latch_full[0] ? 0x80 : 0) |
			(m_latch_full[1] ? 0x40 : 0) ;
}

void sandscrp_state::latchstatus_68k_w(u8 data)
{
	m_latch_full[0] = BIT(data, 7);
	m_latch_full[1] = BIT(data, 6);
}

template<unsigned Latch>
u8 sandscrp_state::soundlatch_r()
{
	if (!machine().side_effects_disabled())
		m_latch_full[Latch] = false;
	return m_soundlatch[Latch]->read();
}

template<unsigned Latch>
void sandscrp_state::soundlatch_w(u8 data)
{
	m_latch_full[Latch] = true;
	m_soundlatch[Latch]->write(data);
}

void sandscrp_state::sandscrp_mem(address_map &map)
{
	map(0x000000, 0x07ffff).rom();     // ROM
	map(0x100001, 0x100001).w(FUNC(sandscrp_state::irq_cause_w)); // IRQ Ack

	map(0x200000, 0x20001f).rw("calc1_mcu", FUNC(kaneko_hit_device::kaneko_hit_r), FUNC(kaneko_hit_device::kaneko_hit_w));
	map(0x300000, 0x30001f).rw(m_view2, FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0x400000, 0x403fff).m(m_view2, FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x500000, 0x501fff).rw(m_pandora, FUNC(kaneko_pandora_device::spriteram_LSB_r), FUNC(kaneko_pandora_device::spriteram_LSB_w)); // sprites
	map(0x600000, 0x600fff).ram().w("palette", FUNC(palette_device::write16)).share("palette");    // Palette
	map(0x700000, 0x70ffff).ram();     // RAM
	map(0x800001, 0x800001).r(FUNC(sandscrp_state::irq_cause_r));  // IRQ Cause
	map(0xa00001, 0xa00001).w(FUNC(sandscrp_state::coincounter_w));  // Coin Counters (Lockout unused)
	map(0xb00000, 0xb00001).portr("P1");
	map(0xb00002, 0xb00003).portr("P2");
	map(0xb00004, 0xb00005).portr("SYSTEM");
	map(0xb00006, 0xb00007).portr("UNK");
	map(0xe00001, 0xe00001).rw(FUNC(sandscrp_state::soundlatch_r<1>), FUNC(sandscrp_state::soundlatch_w<0>));   // From/To Sound CPU
	map(0xe40001, 0xe40001).rw(FUNC(sandscrp_state::latchstatus_68k_r), FUNC(sandscrp_state::latchstatus_68k_w)); //
	map(0xec0000, 0xec0001).r("watchdog", FUNC(watchdog_timer_device::reset16_r));
}


/***************************************************************************
                                Sand Scorpion
***************************************************************************/

void sandscrp_state::bankswitch_w(u8 data)
{
	m_audiobank->set_entry(data & 7);
}

u8 sandscrp_state::latchstatus_r()
{
	return  (m_latch_full[1] ? 0x80 : 0) |    // swapped!?
			(m_latch_full[0] ? 0x40 : 0) ;
}

void sandscrp_state::sandscrp_soundmem(address_map &map)
{
	map(0x0000, 0x7fff).rom();     // ROM
	map(0x8000, 0xbfff).bankr("audiobank");    // Banked ROM
	map(0xc000, 0xdfff).ram();     // RAM
}

void sandscrp_state::sandscrp_soundport(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(sandscrp_state::bankswitch_w));    // ROM Bank
	map(0x02, 0x03).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));        // PORTA/B read
	map(0x04, 0x04).w("oki", FUNC(okim6295_device::write));     // OKIM6295
	map(0x06, 0x06).w(FUNC(sandscrp_state::soundlatch_w<1>));    //
	map(0x07, 0x07).r(FUNC(sandscrp_state::soundlatch_r<0>));     //
	map(0x08, 0x08).r(FUNC(sandscrp_state::latchstatus_r));    //
}


/***************************************************************************
                                Sand Scorpion
***************************************************************************/

static INPUT_PORTS_START( sandscrp )
	PORT_START("P1")    /* $b00000.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN   ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")    /* $b00002.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN   ) PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SYSTEM")    /* $b00004.w */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_START1   )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_START2   )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_COIN1    )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_COIN2    )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_TILT     )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN  )
	PORT_BIT( 0xff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("UNK")   /* $b00006.w */
	PORT_BIT( 0xffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")  /* read by the Z80 through the sound chip */
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x02, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x0c, 0x0c, "Bombs" )         PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "1" )
	PORT_DIPSETTING(    0x04, "2" )
	PORT_DIPSETTING(    0x0c, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x20, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x30, DEF_STR( Easy )    )
	PORT_DIPSETTING(    0x20, DEF_STR( Normal )  )
	PORT_DIPSETTING(    0x10, DEF_STR( Hard )    )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x80, "100K, 300K" )
	PORT_DIPSETTING(    0xc0, "200K, 500K" )
	PORT_DIPSETTING(    0x40, "500K, 1000K" )
	PORT_DIPSETTING(    0x00, "1000K, 3000K" )

	PORT_START("DSW2")  /* read by the Z80 through the sound chip */
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW2:1,2,3,4")
	PORT_DIPSETTING(    0x0a, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 8C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 5C_3C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Allow_Continue ) )   PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW2:8" )
INPUT_PORTS_END


static GFXDECODE_START( gfx_sandscrp_spr )
	GFXDECODE_ENTRY( "sprites", 0, gfx_8x8x4_row_2x2_group_packed_msb, 0x000, 0x10 ) // [0] Sprites
GFXDECODE_END


/***************************************************************************
                                Sand Scorpion
***************************************************************************/


void sandscrp_state::sandscrp(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 12_MHz_XTAL);    /* TMP68HC000N-12 */
	m_maincpu->set_addrmap(AS_PROGRAM, &sandscrp_state::sandscrp_mem);
	m_maincpu->set_vblank_int("screen", FUNC(sandscrp_state::interrupt));

	Z80(config, m_audiocpu, 4000000);   /* Z8400AB1, Reads the DSWs: it can't be disabled */
	m_audiocpu->set_addrmap(AS_PROGRAM, &sandscrp_state::sandscrp_soundmem);
	m_audiocpu->set_addrmap(AS_IO, &sandscrp_state::sandscrp_soundport);

	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_seconds(3));  /* a guess, and certainly wrong */

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time( ATTOSECONDS_IN_USEC(2500) /* not accurate */ );
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 0+16, 256-16-1);
	screen.set_screen_update(FUNC(sandscrp_state::screen_update));
	screen.screen_vblank().set(FUNC(sandscrp_state::screen_vblank));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xGRB_555, 2048);

	KANEKO_TMAP(config, m_view2);
	m_view2->set_colbase(0x400);
	m_view2->set_offset(0x5b, 0, 256, 224);
	m_view2->set_palette("palette");

	KANEKO_HIT(config, "calc1_mcu").set_type(0);

	KANEKO_PANDORA(config, m_pandora, 0, "palette", gfx_sandscrp_spr);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch[0]);
	m_soundlatch[0]->data_pending_callback().set_inputline(m_audiocpu, INPUT_LINE_NMI);

	GENERIC_LATCH_8(config, m_soundlatch[1]);

	OKIM6295(config, "oki", 12000000/6, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 0.5);

	/* YM3014B + YM2203C */
	ym2203_device &ymsnd(YM2203(config, "ymsnd", 4000000));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.port_a_read_callback().set_ioport("DSW1");
	ymsnd.port_b_read_callback().set_ioport("DSW2");
	ymsnd.add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************

                                Sand Scorpion

***************************************************************************/

ROM_START( sandscrp ) /* Z03VA-003 PCB */
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "11.bin", 0x000000, 0x040000, CRC(9b24ab40) SHA1(3187422dbe8b15d8053be4cb20e56d3e6afbd5f2) ) /* Location is IC4 */
	ROM_LOAD16_BYTE( "12.bin", 0x000001, 0x040000, CRC(ad12caee) SHA1(83267445b89c3cf4dc317106aa68763d2f29eff7) ) /* Location is IC5 */

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "8.ic51", 0x00000, 0x20000, CRC(6f3e9db1) SHA1(06a04fa17f44319986913bff70433510c89e38f1) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "5.ic16", 0x000000, 0x080000, CRC(9bb675f6) SHA1(c3f6768cfd99a0e19ca2224fff9aa4e27ec0da24) )
	ROM_LOAD( "6.ic17", 0x080000, 0x080000, CRC(7df2f219) SHA1(e2a59e201bfededa92d6c86f8dc1b212527ef66f) )

	ROM_REGION( 0x100000, "view2", 0 )   /* Layers */
	ROM_LOAD16_BYTE( "3.ic33", 0x000000, 0x080000, CRC(adf20fa0) SHA1(67a7a2be774c86916cbb97e4c9b16c2e48125780) )
	ROM_LOAD16_BYTE( "4.ic32", 0x000001, 0x080000, CRC(b9222ff2) SHA1(a445da3f7f5dea5ff64bb0b048f624f947875a39) )

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "7.ic55", 0x000000, 0x040000, CRC(9870ab12) SHA1(5ea3412cbc57bfaa32a1e2552b2eb46f4ceb5fa8) )
ROM_END

ROM_START( sandscrpa ) /* Z03VA-003 PCB, earlier program version */
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "1.ic4", 0x000000, 0x040000, CRC(c0943ae2) SHA1(04dac4e1f116cd96d6292daa61ef40efc7eba919) )
	ROM_LOAD16_BYTE( "2.ic5", 0x000001, 0x040000, CRC(6a8e0012) SHA1(2350b11c9bd545c8ba4b3c25cd6547ba2ad474b5) )

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "8.ic51", 0x00000, 0x20000, CRC(6f3e9db1) SHA1(06a04fa17f44319986913bff70433510c89e38f1) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "5.ic16", 0x000000, 0x080000, CRC(9bb675f6) SHA1(c3f6768cfd99a0e19ca2224fff9aa4e27ec0da24) )
	ROM_LOAD( "6.ic17", 0x080000, 0x080000, CRC(7df2f219) SHA1(e2a59e201bfededa92d6c86f8dc1b212527ef66f) )

	ROM_REGION( 0x100000, "view2", 0 )   /* Layers */
	ROM_LOAD16_BYTE( "3.ic33", 0x000000, 0x080000, CRC(adf20fa0) SHA1(67a7a2be774c86916cbb97e4c9b16c2e48125780) )
	ROM_LOAD16_BYTE( "4.ic32", 0x000001, 0x080000, CRC(b9222ff2) SHA1(a445da3f7f5dea5ff64bb0b048f624f947875a39) )

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "7.ic55", 0x000000, 0x040000, CRC(9870ab12) SHA1(5ea3412cbc57bfaa32a1e2552b2eb46f4ceb5fa8) )
ROM_END

//快打獅子皇帝/Kuài dǎ shīzi huángdì
ROM_START( sandscrpb ) /* Different rev PCB */
	ROM_REGION( 0x080000, "maincpu", 0 )        /* 68000 Code */
	ROM_LOAD16_BYTE( "11.ic4", 0x000000, 0x040000, CRC(80020cab) SHA1(4f1f4d8ea07ad745f2d6d3f800686f07fe4bf20f) ) /* Chinese title screen */
	ROM_LOAD16_BYTE( "12.ic5", 0x000001, 0x040000, CRC(8df1d42f) SHA1(2a9db5c4b99a8a3f62bffa9ddd96a95e2042602b) ) /* Game & test menu in English */
	/* internet translators come up with "fighter lion king" and / or "Hits lion Emperor Quickly" */

	ROM_REGION( 0x20000, "audiocpu", 0 )        /* Z80 Code */
	ROM_LOAD( "8.ic51", 0x00000, 0x20000, CRC(6f3e9db1) SHA1(06a04fa17f44319986913bff70433510c89e38f1) )

	ROM_REGION( 0x100000, "sprites", 0 )
	ROM_LOAD( "ss502.ic16", 0x000000, 0x100000, CRC(d8012ebb) SHA1(975bbb3b57a09e41d2257d4fa3a64097144de554) )

	ROM_REGION( 0x100000, "view2", 0 )   /* Layers */
	ROM_LOAD( "ss501.ic30", 0x000000, 0x100000, CRC(0cf9f99d) SHA1(47f7f120d2bc075bedaff0a44306a8f46a1d848c) )

	ROM_REGION( 0x040000, "oki", 0 )    /* Samples */
	ROM_LOAD( "7.ic55", 0x000000, 0x040000, CRC(9870ab12) SHA1(5ea3412cbc57bfaa32a1e2552b2eb46f4ceb5fa8) )
ROM_END

} // anonymous namespace


GAME( 1992, sandscrp,  0,        sandscrp, sandscrp, sandscrp_state, empty_init, ROT90, "Face",   "Sand Scorpion", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sandscrpa, sandscrp, sandscrp, sandscrp, sandscrp_state, empty_init, ROT90, "Face",   "Sand Scorpion (Earlier)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sandscrpb, sandscrp, sandscrp, sandscrp, sandscrp_state, empty_init, ROT90, "Face",   "Kuai Da Shizi Huangdi (China?, Revised Hardware)", MACHINE_SUPPORTS_SAVE )
