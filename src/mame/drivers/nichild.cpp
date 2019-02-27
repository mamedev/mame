// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    'Nichibutsu LD' HW (c) 199? Nichibutsu

    TODO:
    - Understand how MMU works (both games jumps to invalid areas, probably
      port A remaps memory for the whole space!);
    - (if ld check patched) memory error printed by ldquiz4, most likely
      related to above;
    - Unknown LaserDisc type;
    - Verify irq vector for vblank irq, and make it work with daisy chain;
    - Unknown irq vector for LaserDisc strobe (ldquiz4 sets a flag at $f014,
      the only place it clears it is at snippet 0x0ED6);
    - hookup audio CPU (same as niyanpai HW?)

=============================================================================

    1 x TMPZ84C011AF-6 main CPU
    1 x 21.47727MHz OSC
    1 x Z0840004PSC audio CPU
    1 x 4.000MHz OSC
    1 x Yamaha V9938
    1 x uPC1352C
    1 x Yamaha YM3812
    2 x 8 dip switch banks

***************************************************************************/


#include "emu.h"
#include "cpu/z80/tmpz84c011.h"
#include "video/v9938.h"
#include "screen.h"
#include "speaker.h"

#define MAIN_CLOCK XTAL(21'477'272)
#define SOUND_CLOCK XTAL(4'000'000)

class nichild_state : public driver_device
{
public:
	nichild_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_v9938(*this, "v9938")
		, m_gfxrom(*this, "gfx")
	{
	}

	void nichild(machine_config &config);

private:
	DECLARE_READ8_MEMBER(gfx_r);
	DECLARE_READ8_MEMBER(mux_r);
	DECLARE_WRITE8_MEMBER(mux_w);
	DECLARE_WRITE8_MEMBER(porta_w);
	DECLARE_WRITE8_MEMBER(portb_w);
	DECLARE_WRITE8_MEMBER(portc_w);
	DECLARE_WRITE8_MEMBER(portd_w);
	DECLARE_WRITE8_MEMBER(gfxbank_w);
	INTERRUPT_GEN_MEMBER(vdp_irq);

	void nichild_io(address_map &map);
	void nichild_map(address_map &map);

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<tmpz84c011_device> m_maincpu;
	required_device<v9938_device> m_v9938;
	required_region_ptr<uint8_t> m_gfxrom;
	uint32_t m_gfx_bank;
};


READ8_MEMBER(nichild_state::gfx_r)
{
	uint32_t gfx_offset;

	gfx_offset  = ((offset & 0x007f) << 8);
	gfx_offset |= ((offset & 0xff00) >> 8);
	gfx_offset |= m_gfx_bank;

	//printf("%08x %02x\n",gfx_offset,m_gfx_bank);

	return m_gfxrom[gfx_offset];
}

//#include "debugger.h"

WRITE8_MEMBER(nichild_state::porta_w)
{
	printf("PORTA %02x\n",data);
//  machine().debug_break();
}

WRITE8_MEMBER(nichild_state::portb_w)
{
	printf("PORTB %02x\n",data);
}

WRITE8_MEMBER(nichild_state::portc_w)
{
	printf("PORTC %02x\n",data);
}

WRITE8_MEMBER(nichild_state::portd_w)
{
	printf("PORTD %02x\n",data);
}

WRITE8_MEMBER(nichild_state::gfxbank_w)
{
	// TODO: ldquiz4 checks up to 0x30, what for?
	m_gfx_bank = data * 0x8000;
}

READ8_MEMBER(nichild_state::mux_r)
{
	// TODO
	return 0xff;
}

WRITE8_MEMBER(nichild_state::mux_w)
{
	// ...
}

void nichild_state::nichild_map(address_map &map)
{
	// TODO: MMU :/
	map(0x0000, 0x1fff).rom().region("ipl", 0x0000);
	map(0x2000, 0x3fff).rom().region("ipl", 0x2000);
	map(0x4000, 0x5fff).rom().region("ipl", 0x4000);
	map(0x6000, 0x7fff).rom().region("ipl", 0x6000);
	map(0x8000, 0x9fff).rom().region("ipl", 0x8000);
	map(0xc000, 0xdfff).rom().region("ipl", 0x0000);
	map(0xe000, 0xffff).ram();
}

void nichild_state::nichild_io(address_map &map)
{
//  map.global_mask(0xff);
	map(0x60, 0x60).mirror(0xff00).w(FUNC(nichild_state::mux_w));
	// shabdama accesses 0x70-0x73, ldquiz4 0x7c-0x7f
	map(0x70, 0x73).mirror(0xff0c).rw(m_v9938, FUNC(v9938_device::read), FUNC(v9938_device::write));
	map(0x80, 0xff).select(0xff00).r(FUNC(nichild_state::gfx_r));
}

static INPUT_PORTS_START( nichild )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
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

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


void nichild_state::machine_start()
{
}

void nichild_state::machine_reset()
{
}

#if 0
static const z80_daisy_config daisy_chain_main[] =
{
	TMPZ84C011_DAISY_INTERNAL,
	{ nullptr }
};
#endif


INTERRUPT_GEN_MEMBER(nichild_state::vdp_irq)
{
	m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0x76);
}


void nichild_state::nichild(machine_config &config)
{
	/* basic machine hardware */
	TMPZ84C011(config, m_maincpu, MAIN_CLOCK/4);
	//m_maincpu->set_daisy_config(daisy_chain_main);
	m_maincpu->set_addrmap(AS_PROGRAM, &nichild_state::nichild_map);
	m_maincpu->set_addrmap(AS_IO, &nichild_state::nichild_io);
	m_maincpu->set_vblank_int("screen", FUNC(nichild_state::vdp_irq));
	m_maincpu->in_pb_callback().set(FUNC(nichild_state::mux_r));
	m_maincpu->out_pa_callback().set(FUNC(nichild_state::porta_w));
	m_maincpu->out_pb_callback().set(FUNC(nichild_state::portb_w));
	m_maincpu->out_pc_callback().set(FUNC(nichild_state::portc_w));
	m_maincpu->out_pd_callback().set(FUNC(nichild_state::portd_w));
	m_maincpu->out_pe_callback().set(FUNC(nichild_state::gfxbank_w));

	/* video hardware */
	V9938(config, m_v9938, MAIN_CLOCK);
	m_v9938->set_screen_ntsc("screen");
	m_v9938->set_vram_size(0x40000);
//  m_v9938->int_cb().set_inputline("maincpu", 0);
//  m_v9938->int_cb().set(FUNC(nichild_state::vdp_irq));
	SCREEN(config, "screen", SCREEN_TYPE_RASTER);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
//  YM3812(config, "fmsnd", SOUND_CLOCK).add_route(ALL_OUTPUTS, "speaker", 0.7);
}


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( shabdama )
	ROM_REGION( 0x10000, "ipl", ROMREGION_ERASE00 )
	ROM_LOAD( "1.bin",        0x000000, 0x010000, CRC(e49e3d73) SHA1(6d17d60e1b6f8aee96f7a09f45113030064d3bdb) )

	ROM_REGION( 0x20000, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "3.bin",        0x000000, 0x010000, CRC(e8233c6e) SHA1(fbfdb03dc9f4e3e80e161b8522b676485ffb1c95) )
	ROM_LOAD( "2.bin",        0x010000, 0x010000, CRC(3e0b5344) SHA1(eeae36fc4fca091065c1d51f05c2d11f44fe6d13) )

	ROM_REGION( 0x200000, "gfx", ROMREGION_ERASE00 )
	ROM_LOAD( "10.bin",       0x060000, 0x010000, CRC(5da10b82) SHA1(72974d083110fc6c583bfa1c22ce3abe02ba86f6) )
	ROM_LOAD( "9.bin",        0x050000, 0x010000, CRC(1afdc5bf) SHA1(b07b32656ffc96b7f7d4bd242b2a6e0e105ab67a) )
	ROM_LOAD( "8.bin",        0x040000, 0x010000, CRC(3e75423e) SHA1(62e24849ddeb004ed8570d2884afa4ab257cdf07) )
	ROM_LOAD( "7.bin",        0x030000, 0x010000, CRC(7f08e3a6) SHA1(127018442183332175c9e1f558274cd2cb5f0147) )
	ROM_LOAD( "6.bin",        0x020000, 0x010000, CRC(0fece809) SHA1(1fe8436af8ead02a3b517b6306f9824cd64b2d26) )
	ROM_LOAD( "5.bin",        0x010000, 0x010000, CRC(0706386a) SHA1(29eee363775869dcc9c46285632e8bf745c9110b) )
	ROM_LOAD( "4.bin",        0x000000, 0x010000, CRC(199e2127) SHA1(2514d51cb06438b312d1f328c72baa739280416a) )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "shabdama", 0, NO_DUMP )
ROM_END


// LD QUIZ 第4弾 答えたもん勝ち!

ROM_START( ldquiz4 )
	ROM_REGION( 0x10000, "ipl", 0 ) // 27512
	ROM_LOAD( "1.e3", 0x00000,  0x10000, CRC(49255f66) SHA1(bdd01987331c2aadea7f588d39c48c70cd43fc71) )

	ROM_REGION( 0x20000, "audiocpu", 0 ) // 27512
	ROM_LOAD( "3.e7", 0x00000,  0x10000, CRC(b033eb6a) SHA1(2c11b2b998117f68a1fbbd110d3f67ab472e133d) )
	ROM_LOAD( "2.e6", 0x10000,  0x10000, CRC(6c83cad6) SHA1(c38f60fb4fdbda76ea3459644bf491cc305a7ae6) )

	ROM_REGION( 0x200000, "gfx", ROMREGION_ERASE00 ) // 27010
	ROM_LOAD( "4.k1",   0x000000, 0x20000, CRC(9cdf8114) SHA1(99e6b9bb43c6df320fdb1ea8599967b707f7f18d) )
	ROM_LOAD( "5.k2",   0x020000, 0x20000, CRC(7746a909) SHA1(c69a45159d15e8897a5999e57b519ed6fc0d9812) )
	ROM_LOAD( "6.k3",   0x040000, 0x20000, CRC(3b3e63ad) SHA1(92898ade77ad267978a469b03c9113f4e5a47288) )
	ROM_LOAD( "7.k4",   0x060000, 0x20000, CRC(cdcaae32) SHA1(bfc07524a9859592bf7c6397fd80570b4e5e15fc) )
	ROM_LOAD( "8.k5",   0x080000, 0x20000, CRC(c08b90e6) SHA1(add35812ea98ac44299b7f165efeee268aa57132) )
	ROM_LOAD( "9.k6",   0x0a0000, 0x20000, CRC(72c1a283) SHA1(8dbfc5892d719033dff82c70f13c1d7c63173240) )
	ROM_LOAD( "10.k8",  0x0c0000, 0x20000, CRC(c7437125) SHA1(55b161ce2432d04531ed0afab973f892b571ef88) )
	ROM_LOAD( "11.k9",  0x0e0000, 0x20000, CRC(6feeab93) SHA1(d77325c1eecb677c48d11bf8d5f73b238f2896e6) )
	ROM_LOAD( "12.k10", 0x100000, 0x20000, CRC(c7f9bf98) SHA1(103b78b0e126ea4249982bf114010f57e5ffa70a) )

	ROM_REGION( 0x800, "plds", 0 ) // all protected
	ROM_LOAD( "pal16l8.0", 0x000, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.1", 0x200, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.2", 0x400, 0x104, NO_DUMP )
	ROM_LOAD( "pal16l8.3", 0x600, 0x104, NO_DUMP )

	DISK_REGION( "laserdisc" )
	DISK_IMAGE_READONLY( "ldquiz4", 0, NO_DUMP )
ROM_END

GAME( 1991, shabdama, 0,   nichild, nichild, nichild_state, empty_init, ROT0, "Nichibutsu", "LD Mahjong #4 Shabon-Dama (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1992, ldquiz4,  0,   nichild, nichild, nichild_state, empty_init, ROT0, "Nichibutsu", "LD Quiz dai 4-dan - Kotaetamon Gachi! (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
