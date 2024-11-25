// license:BSD-3-Clause
// copyright-holders:Mirko Buffoni
/***************************************************************************

Mario Bros driver by Mirko Buffoni


Memory map (preliminary):

0000-5fff ROM
6000-6fff RAM
7000-73ff Sprite RAM
7400-77ff Video RAM
f000-ffff ROM

read:
7c00      IN0
7c80      IN1
7f80      DSW

*
 * IN0 (bits NOT inverted)
 * bit 7 : TEST
 * bit 6 : START 2
 * bit 5 : START 1
 * bit 4 : JUMP player 1
 * bit 3 : ? DOWN player 1 ?
 * bit 2 : ? UP player 1 ?
 * bit 1 : LEFT player 1
 * bit 0 : RIGHT player 1
 *
*
 * IN1 (bits NOT inverted)
 * bit 7 : ?
 * bit 6 : COIN 2
 * bit 5 : COIN 1
 * bit 4 : JUMP player 2
 * bit 3 : ? DOWN player 2 ?
 * bit 2 : ? UP player 2 ?
 * bit 1 : LEFT player 2
 * bit 0 : RIGHT player 2
 *
*
 * DSW (bits NOT inverted)
 * bit 7 : \ difficulty
 * bit 6 : / 00 = easy  01 = medium  10 = hard  11 = hardest
 * bit 5 : \ bonus
 * bit 4 : / 00 = 20000  01 = 30000  10 = 40000  11 = none
 * bit 3 : \ coins per play
 * bit 2 : /
 * bit 1 : \ 00 = 3 lives  01 = 4 lives
 * bit 0 : / 10 = 5 lives  11 = 6 lives
 *

write:
7d00      vertical scroll (pow)
7d80      ?
7e00      sound
7e80-7e87 misc. triggers (see mcfg)
7f00-7f07 sound triggers

I/O ports

write:
00        Z80 DMA


The sound MCU can be easily replaced with a ROMless one such as I8039
(or just force EA high), by doing a 1-byte patch to the external ROM:

offset $01: change $00 to $01 (call $100 -> call $101)

***************************************************************************/

#include "emu.h"
#include "mario.h"

#include "cpu/z80/z80.h"
#include "machine/74259.h"
#include "machine/z80dma.h"
#include "screen.h"


/*************************************
 *
 *  statics
 *
 *************************************/

uint8_t mario_state::memory_read_byte(offs_t offset)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.read_byte(offset);
}

void mario_state::memory_write_byte(offs_t offset, uint8_t data)
{
	address_space& prog_space = m_maincpu->space(AS_PROGRAM);
	return prog_space.write_byte(offset, data);
}

void mario_state::nmi_mask_w(int state)
{
	m_nmi_mask = state;
	if (!state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void mario_state::coin_counter_1_w(int state)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

void mario_state::coin_counter_2_w(int state)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

/*************************************
 *
 *  Main CPU memory handlers
 *
 *************************************/

void mario_state::mario_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6fff).ram();
	map(0x7000, 0x73ff).ram().share("spriteram"); /* physical sprite ram */
	map(0x7400, 0x77ff).ram().w(FUNC(mario_state::mario_videoram_w)).share("videoram");
	map(0x7c00, 0x7c00).portr("IN0").w(FUNC(mario_state::mario_sh1_w)); /* Mario run sample */
	map(0x7c80, 0x7c80).portr("IN1").w(FUNC(mario_state::mario_sh2_w)); /* Luigi run sample */
	map(0x7d00, 0x7d00).w(FUNC(mario_state::mario_scroll_w));
	map(0x7e00, 0x7e00).w(FUNC(mario_state::mario_sh_tuneselect_w));
	map(0x7e80, 0x7e87).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x7f00, 0x7f07).w(FUNC(mario_state::mario_sh3_w)); /* Sound port */
	map(0x7f80, 0x7f80).portr("DSW");    /* DSW */
	map(0xf000, 0xffff).rom();
}

void mario_state::masao_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x6fff).ram();
	map(0x7000, 0x73ff).ram().share("spriteram"); /* physical sprite ram */
	map(0x7400, 0x77ff).ram().w(FUNC(mario_state::mario_videoram_w)).share("videoram");
	map(0x7c00, 0x7c00).portr("IN0");
	map(0x7c80, 0x7c80).portr("IN1");
	map(0x7d00, 0x7d00).w(FUNC(mario_state::mario_scroll_w));
	map(0x7e00, 0x7e00).w(m_soundlatch, FUNC(generic_latch_8_device::write));
	map(0x7e80, 0x7e87).w("mainlatch", FUNC(ls259_device::write_d0));
	map(0x7f00, 0x7f00).w(FUNC(mario_state::masao_sh_irqtrigger_w));
	map(0x7f80, 0x7f80).portr("DSW");    /* DSW */
	map(0xf000, 0xffff).rom();
}

void mario_state::mario_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).rw(m_z80dma, FUNC(z80dma_device::read), FUNC(z80dma_device::write));  /* dma controller */
}


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( mario )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_SERVICE_NO_TOGGLE( 0x80, IP_ACTIVE_HIGH )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_2WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSW")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!3,!4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "20k only" )
	PORT_DIPSETTING(    0x10, "30k only" )
	PORT_DIPSETTING(    0x20, "40k only" )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPSETTING(    0x30, DEF_STR( None ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Medium ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Hardest ) )

	PORT_START("MONITOR")
	PORT_CONFNAME( 0x01, 0x00, "Monitor" )
	PORT_CONFSETTING(    0x00, "Nintendo" )
	PORT_CONFSETTING(    0x01, "Std 15.72Khz" )

INPUT_PORTS_END

static INPUT_PORTS_START( mariof )
	PORT_INCLUDE( mario )

	PORT_MODIFY( "DSW" )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!5,!6")
	PORT_DIPSETTING(    0x00, "20k 40k 20k+" )
	PORT_DIPSETTING(    0x10, "30k 50k 20k+" )
	PORT_DIPSETTING(    0x20, "40k 60k 20k+" )
INPUT_PORTS_END


static INPUT_PORTS_START( marioe )
	PORT_INCLUDE( mario )

	PORT_MODIFY ( "IN1" )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_COIN2 )  /* doesn't work in game, but does in service mode */
INPUT_PORTS_END

static INPUT_PORTS_START( marioj )
	PORT_INCLUDE( marioe )

	PORT_MODIFY( "DSW" )
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:!1,!2")
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x01, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x03, "6" )
	PORT_DIPNAME( 0x1c, 0x00, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:!3,!4,!5")
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x14, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x1c, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x20, 0x20, "2 Players Game" )        PORT_DIPLOCATION("SW1:!6")
	PORT_DIPSETTING(    0x00, "1 Credit" )
	PORT_DIPSETTING(    0x20, "2 Credits" )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:!7,!8")
	PORT_DIPSETTING(    0x00, "20k 50k 30k+" )
	PORT_DIPSETTING(    0x40, "30k 60k 30k+" )
	PORT_DIPSETTING(    0x80, "40k 70k 30k+" )
	PORT_DIPSETTING(    0xc0, DEF_STR( None ) )
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	512,    /* 512 characters */
	2,  /* 2 bits per pixel */
	{ 512*8*8, 0 }, /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 }, /* pretty straightforward layout */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};


static const gfx_layout spritelayout =
{
	16,16,  /* 16*16 sprites */
	256,    /* 256 sprites */
	3,  /* 3 bits per pixel */
	{ 2*256*16*16, 256*16*16, 0 },  /* the bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,       /* the two halves of the sprite are separated */
			256*16*8+0, 256*16*8+1, 256*16*8+2, 256*16*8+3, 256*16*8+4, 256*16*8+5, 256*16*8+6, 256*16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8    /* every sprite takes 16 consecutive bytes */
};

static GFXDECODE_START( gfx_mario )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,   0, 32 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 0, 32 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void mario_state::vblank_irq(int state)
{
	if (state && m_nmi_mask)
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

void mario_state::mario_base(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, Z80_CLOCK); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &mario_state::mario_map);
	m_maincpu->set_addrmap(AS_IO, &mario_state::mario_io_map);
	downcast<z80_device &>(*m_maincpu).busack_cb().set(m_z80dma, FUNC(z80dma_device::bai_w));

	/* devices */
	Z80DMA(config, m_z80dma, Z80_CLOCK);
	m_z80dma->out_busreq_callback().set_inputline(m_maincpu, Z80_INPUT_LINE_BUSRQ);
	m_z80dma->in_mreq_callback().set(FUNC(mario_state::memory_read_byte));
	m_z80dma->out_mreq_callback().set(FUNC(mario_state::memory_write_byte));

	ls259_device &mainlatch(LS259(config, "mainlatch")); // 2L (7E80H)
	mainlatch.q_out_cb<0>().set(FUNC(mario_state::gfx_bank_w));         // ~T ROM
	mainlatch.q_out_cb<1>().set_nop();                                  // 2 PSL
	mainlatch.q_out_cb<2>().set(FUNC(mario_state::flip_w));             // FLIP
	mainlatch.q_out_cb<3>().set(FUNC(mario_state::palette_bank_w));     // CREF 0
	mainlatch.q_out_cb<4>().set(FUNC(mario_state::nmi_mask_w));         // NMI EI
	mainlatch.q_out_cb<5>().set("z80dma", FUNC(z80dma_device::rdy_w));  // DMA SET
	mainlatch.q_out_cb<6>().set(FUNC(mario_state::coin_counter_1_w));   // COUNTER 2 (misnumbered on schematic)
	mainlatch.q_out_cb<7>().set(FUNC(mario_state::coin_counter_2_w));   // COUNTER 1 (misnumbered on schematic)

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(PIXEL_CLOCK, HTOTAL, HBEND, HBSTART, VTOTAL, VBEND, VBSTART);
	screen.set_screen_update(FUNC(mario_state::screen_update));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(mario_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_mario);
	PALETTE(config, m_palette, FUNC(mario_state::mario_palette), 256);
}

void mario_state::mario(machine_config &config)
{
	mario_base(config);
	mario_audio(config);
}

void mario_state::masao(machine_config &config)
{
	mario_base(config);
	m_maincpu->set_clock(4000000); /* 4.000 MHz (?) */
	m_maincpu->set_addrmap(AS_PROGRAM, &mario_state::masao_map);

	/* sound hardware */
	masao_audio(config);
}


/*************************************
 *
 *  ROM definitions
 *
 *************************************/

ROM_START( mario )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tma1-c-7f_g.7f", 0x0000, 0x2000, CRC(c0c6e014) SHA1(36a04f9ca1c2a583477cb8a6f2ef94e044e08296) ) /* Unknown revision */
	ROM_LOAD( "tma1-c-7e_g.7e", 0x2000, 0x2000, CRC(116b3856) SHA1(e372f846d0e5a2b9b47ebd0330293fcc8a12363f) )
	ROM_LOAD( "tma1-c-7d_g.7d", 0x4000, 0x2000, CRC(dcceb6c1) SHA1(b19804e69ce2c98cf276c6055c3a250316b96b45) )
	ROM_LOAD( "tma1-c-7c_g.7c", 0xf000, 0x1000, CRC(4a63d96b) SHA1(b09060b2c84ab77cc540a27b8f932cb60ec8d442) )

	ROM_REGION( 0x0800, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m58715-051p.5l", 0x0000, 0x0800, NO_DUMP ) // internal rom

	ROM_REGION( 0x1000, "soundrom", 0 )
	ROM_LOAD( "tma1-c-6k_e.6k", 0x0000, 0x1000, CRC(06b9ff85) SHA1(111a29bcb9cda0d935675fa26eca6b099a88427f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tma1-v-3f.3f",   0x0000, 0x1000, CRC(28b0c42c) SHA1(46749568aff88a28c3b6a1ac423abd1b90742a4d) )
	ROM_LOAD( "tma1-v-3j.3j",   0x1000, 0x1000, CRC(0c8cc04d) SHA1(15fae47d701dc1ef15c943cee6aa991776ecffdf) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1-v-7m.7m",   0x0000, 0x1000, CRC(22b7372e) SHA1(4a1c1e239cb6d483e76f50d7a3b941025963c6a3) )
	ROM_LOAD( "tma1-v-7n.7n",   0x1000, 0x1000, CRC(4f3a1f47) SHA1(0747d693b9482f6dd28b0bc484fd1d3e29d35654) )
	ROM_LOAD( "tma1-v-7p.7p",   0x2000, 0x1000, CRC(56be6ccd) SHA1(15a6e16c189d45f72761ebcbe9db5001bdecd659) )
	ROM_LOAD( "tma1-v-7s.7s",   0x3000, 0x1000, CRC(56f1d613) SHA1(9af6844dbaa3615433d0595e9e85e72493e31a54) )
	ROM_LOAD( "tma1-v-7t.7t",   0x4000, 0x1000, CRC(641f0008) SHA1(589fe108c7c11278fd897f2ded8f0498bc149cfd) )
	ROM_LOAD( "tma1-v-7u.7u",   0x5000, 0x1000, CRC(7baf5309) SHA1(d9194ff7b89a18273d37b47228fc7fb7e2a0ed1f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p.4p",   0x0000, 0x0200, CRC(afc9bd41) SHA1(90b739c4c7f24a88b6ac5ca29b06c032906a2801) )

	ROM_REGION( 0x0020, "decoder_prom", 0 )
	ROM_LOAD( "tma1-c-5p.5p",   0x0000, 0x0020, CRC(58d86098) SHA1(d654995004b9052b12d3b682a2b39530e70030fc) ) /* BPROM was a TBP18S030N read as 82S123, main cpu memory map decoding prom */
ROM_END

ROM_START( mariof )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tma1-c-7f_f.7f", 0x0000, 0x2000, CRC(c0c6e014) SHA1(36a04f9ca1c2a583477cb8a6f2ef94e044e08296) )
	ROM_LOAD( "tma1-c-7e_f.7e", 0x2000, 0x2000, CRC(94fb60d6) SHA1(e74d74aa27f87a164bdd453ab0076efeeb7d4ea3) )
	ROM_LOAD( "tma1-c-7d_f.7d", 0x4000, 0x2000, CRC(dcceb6c1) SHA1(b19804e69ce2c98cf276c6055c3a250316b96b45) )
	ROM_LOAD( "tma1-c-7c_f.7c", 0xf000, 0x1000, CRC(4a63d96b) SHA1(b09060b2c84ab77cc540a27b8f932cb60ec8d442) )

	ROM_REGION( 0x0800, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m58715-051p.5l", 0x0000, 0x0800, NO_DUMP ) // internal rom

	ROM_REGION( 0x1000, "soundrom", 0 )
	ROM_LOAD( "tma1-c-6k_e.6k", 0x0000, 0x1000, CRC(06b9ff85) SHA1(111a29bcb9cda0d935675fa26eca6b099a88427f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tma1-v-3f.3f",   0x0000, 0x1000, CRC(28b0c42c) SHA1(46749568aff88a28c3b6a1ac423abd1b90742a4d) )
	ROM_LOAD( "tma1-v-3j.3j",   0x1000, 0x1000, CRC(0c8cc04d) SHA1(15fae47d701dc1ef15c943cee6aa991776ecffdf) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1-v-7m.7m",   0x0000, 0x1000, CRC(22b7372e) SHA1(4a1c1e239cb6d483e76f50d7a3b941025963c6a3) )
	ROM_LOAD( "tma1-v-7n.7n",   0x1000, 0x1000, CRC(4f3a1f47) SHA1(0747d693b9482f6dd28b0bc484fd1d3e29d35654) )
	ROM_LOAD( "tma1-v-7p.7p",   0x2000, 0x1000, CRC(56be6ccd) SHA1(15a6e16c189d45f72761ebcbe9db5001bdecd659) )
	ROM_LOAD( "tma1-v-7s.7s",   0x3000, 0x1000, CRC(56f1d613) SHA1(9af6844dbaa3615433d0595e9e85e72493e31a54) )
	ROM_LOAD( "tma1-v-7t.7t",   0x4000, 0x1000, CRC(641f0008) SHA1(589fe108c7c11278fd897f2ded8f0498bc149cfd) )
	ROM_LOAD( "tma1-v-7u.7u",   0x5000, 0x1000, CRC(7baf5309) SHA1(d9194ff7b89a18273d37b47228fc7fb7e2a0ed1f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p_1.4p", 0x0000, 0x0200, CRC(8187d286) SHA1(8a6d8e622599f1aacaeb10f7b1a39a23c8a840a0) ) /* BPROM was a MB7124E read as 82S147 */

	ROM_REGION( 0x0020, "decoder_prom", 0 )
	ROM_LOAD( "tma1-c-5p.5p",   0x0000, 0x0020, CRC(58d86098) SHA1(d654995004b9052b12d3b682a2b39530e70030fc) ) /* BPROM was a TBP18S030N read as 82S123, main cpu memory map decoding prom */
ROM_END

ROM_START( marioe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tma1-c-7f_e-1.7f", 0x0000, 0x2000, CRC(c0c6e014) SHA1(36a04f9ca1c2a583477cb8a6f2ef94e044e08296) )
	ROM_LOAD( "tma1-c-7e_e-3.7e", 0x2000, 0x2000, CRC(b09ab857) SHA1(35b91cd1c4c3dd2d543a1ea8ff7b951715727792) )
	ROM_LOAD( "tma1-c-7d_e-1.7d", 0x4000, 0x2000, CRC(dcceb6c1) SHA1(b19804e69ce2c98cf276c6055c3a250316b96b45) )
	ROM_LOAD( "tma1-c-7c_e-3.7c", 0xf000, 0x1000, CRC(0d31bd1c) SHA1(a2e238470ba2ea3c81225fec687f61f047c68c59) )

	ROM_REGION( 0x0800, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m58715-051p.5l",   0x0000, 0x0800, NO_DUMP ) // internal rom

	ROM_REGION( 0x1000, "soundrom", 0 )
	ROM_LOAD( "tma1-c-6k_e.6k",   0x0000, 0x1000, CRC(06b9ff85) SHA1(111a29bcb9cda0d935675fa26eca6b099a88427f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tma1-v-3f.3f",     0x0000, 0x1000, CRC(28b0c42c) SHA1(46749568aff88a28c3b6a1ac423abd1b90742a4d) )
	ROM_LOAD( "tma1-v-3j.3j",     0x1000, 0x1000, CRC(0c8cc04d) SHA1(15fae47d701dc1ef15c943cee6aa991776ecffdf) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1-v.7m",        0x0000, 0x1000, CRC(d01c0e2c) SHA1(cd6cc9d69c36db15543601f5da2abf109cde43c9) )
	ROM_LOAD( "tma1-v-7n.7n",     0x1000, 0x1000, CRC(4f3a1f47) SHA1(0747d693b9482f6dd28b0bc484fd1d3e29d35654) )
	ROM_LOAD( "tma1-v-7p.7p",     0x2000, 0x1000, CRC(56be6ccd) SHA1(15a6e16c189d45f72761ebcbe9db5001bdecd659) )
	ROM_LOAD( "tma1-v.7s",        0x3000, 0x1000, CRC(ff856e6f) SHA1(2bc9ff18bb1842e8de2bc61ac828f1b175290bed) )
	ROM_LOAD( "tma1-v-7t.7t",     0x4000, 0x1000, CRC(641f0008) SHA1(589fe108c7c11278fd897f2ded8f0498bc149cfd) )
	ROM_LOAD( "tma1-v.7u",        0x5000, 0x1000, CRC(d2dbeb75) SHA1(676cf3e15252cd0d9e926ca15c3aa0caa39be269) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p_1.4p",   0x0000, 0x0200, CRC(8187d286) SHA1(8a6d8e622599f1aacaeb10f7b1a39a23c8a840a0) ) /* BPROM was a MB7124E read as 82S147 */

	ROM_REGION( 0x0020, "decoder_prom", 0 )
	ROM_LOAD( "tma1-c-5p.5p",     0x0000, 0x0020, CRC(58d86098) SHA1(d654995004b9052b12d3b682a2b39530e70030fc) ) /* BPROM was a TBP18S030N read as 82S123, main cpu memory map decoding prom */
ROM_END

ROM_START( marioj )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "tma1c-a1.7f",    0x0000, 0x2000, CRC(b64b6330) SHA1(f7084251ac325bbfa3fb804da16a50622e1fd213) )
	ROM_LOAD( "tma1c-a2.7e",    0x2000, 0x2000, CRC(290c4977) SHA1(5af266be0ddc883c6548c90e4a9084024a1e91a0) )
	ROM_LOAD( "tma1c-a1.7d",    0x4000, 0x2000, CRC(f8575f31) SHA1(710d0e72fcfce700ed2a22fb9c7c392cc76b250b) )
	ROM_LOAD( "tma1c-a2.7c",    0xf000, 0x1000, CRC(a3c11e9e) SHA1(d0612b0f8c2ea4e798f551922a04a324f4ed5f3d) )

	ROM_REGION( 0x0800, "audiocpu", ROMREGION_ERASE00 )
	ROM_LOAD( "m58715-051p.5l", 0x0000, 0x0800, NO_DUMP ) // internal rom

	ROM_REGION( 0x1000, "soundrom", 0 )
	ROM_LOAD( "tma1c-a.6k",     0x0000, 0x1000, CRC(06b9ff85) SHA1(111a29bcb9cda0d935675fa26eca6b099a88427f) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tma1v-a.3f",     0x0000, 0x1000, CRC(adf49ee0) SHA1(11fc2cd197bfe3ecb6af55c3c7a326c94988d2bd) )
	ROM_LOAD( "tma1v-a.3j",     0x1000, 0x1000, CRC(a5318f2d) SHA1(e42f5e51804195c64a56addb18b7ad12c57bb09a) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1v-a.7m",     0x0000, 0x1000, CRC(186762f8) SHA1(711fdd37392656bdd5027e020d51d083ccd7c407) )
	ROM_LOAD( "tma1v-a.7n",     0x1000, 0x1000, CRC(e0e08bba) SHA1(315eba2c10d426c9c0bb4e36987bf8ebed7df9a0) )
	ROM_LOAD( "tma1v-a.7p",     0x2000, 0x1000, CRC(7b27c8c1) SHA1(3fb2613ce19e353fbcc77b6817927794fb35810f) )
	ROM_LOAD( "tma1v-a.7s",     0x3000, 0x1000, CRC(912ba80a) SHA1(351fb5b160216eb10e281815d05a7165ca0e5909) )
	ROM_LOAD( "tma1v-a.7t",     0x4000, 0x1000, CRC(5cbb92a5) SHA1(a78a378e6d3060143dc456e9c33a5068da648331) )
	ROM_LOAD( "tma1v-a.7u",     0x5000, 0x1000, CRC(13afb9ed) SHA1(b29dcd91cf5e639ee50b734afc7a3afce79634df) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p.4p",   0x0000, 0x0200, CRC(afc9bd41) SHA1(90b739c4c7f24a88b6ac5ca29b06c032906a2801) )

	ROM_REGION( 0x0020, "decoder_prom", 0 )
	ROM_LOAD( "tma1-c-5p.5p",   0x0000, 0x0020, CRC(58d86098) SHA1(d654995004b9052b12d3b682a2b39530e70030fc) ) /* BPROM was a TBP18S030N read as 82S123, main cpu memory map decoding prom */
ROM_END

ROM_START( masao )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "masao-4.rom",  0x0000, 0x2000, CRC(07a75745) SHA1(acc760242a8862d177e3cff90aa32c4f3dac4e65) )
	ROM_LOAD( "masao-3.rom",  0x2000, 0x2000, CRC(55c629b6) SHA1(1f5b5699821871aadacc511663cb4bd4e357e215) )
	ROM_LOAD( "masao-2.rom",  0x4000, 0x2000, CRC(42e85240) SHA1(bc8cdf867b743c5ee58fcacb63a44f826c8f8c1a) )
	ROM_LOAD( "masao-1.rom",  0xf000, 0x1000, CRC(b2817af9) SHA1(95e83752e544671a68df2107fae1010b187f04a6) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* 64k for sound */
	ROM_LOAD( "masao-5.rom",  0x0000, 0x1000, CRC(bd437198) SHA1(ebae88461984afc97bbc103fc6d95bc3c1865eec) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "masao-6.rom",  0x0000, 0x1000, CRC(1c9e0be2) SHA1(b4a650412dad90c6f6d79e93cde49055703b7f3e) )
	ROM_LOAD( "masao-7.rom",  0x1000, 0x1000, CRC(747c1349) SHA1(54674f78edf86953b7d500b66393483d1a5ce8ab) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "tma1v-a.7m",   0x0000, 0x1000, CRC(186762f8) SHA1(711fdd37392656bdd5027e020d51d083ccd7c407) )
	ROM_LOAD( "masao-9.rom",  0x1000, 0x1000, CRC(50be3918) SHA1(73e22eee67a03732ff57e523f900f20c6aee0491) )
	ROM_LOAD( "mario.7p",     0x2000, 0x1000, CRC(56be6ccd) SHA1(15a6e16c189d45f72761ebcbe9db5001bdecd659) )
	ROM_LOAD( "tma1v-a.7s",   0x3000, 0x1000, CRC(912ba80a) SHA1(351fb5b160216eb10e281815d05a7165ca0e5909) )
	ROM_LOAD( "tma1v-a.7t",   0x4000, 0x1000, CRC(5cbb92a5) SHA1(a78a378e6d3060143dc456e9c33a5068da648331) )
	ROM_LOAD( "tma1v-a.7u",   0x5000, 0x1000, CRC(13afb9ed) SHA1(b29dcd91cf5e639ee50b734afc7a3afce79634df) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tma1-c-4p.4p", 0x0000, 0x0200, CRC(afc9bd41) SHA1(90b739c4c7f24a88b6ac5ca29b06c032906a2801) )
ROM_END


/*************************************
 *
 *  Game drivers
 *
 *************************************/

GAME( 1983, mario,  0,     mario, mario,  mario_state, empty_init, ROT0, "Nintendo of America", "Mario Bros. (US, Revision G)",    MACHINE_SUPPORTS_SAVE )
GAME( 1983, mariof, mario, mario, mariof, mario_state, empty_init, ROT0, "Nintendo of America", "Mario Bros. (US, Revision F)",    MACHINE_SUPPORTS_SAVE )
GAME( 1983, marioe, mario, mario, marioe, mario_state, empty_init, ROT0, "Nintendo of America", "Mario Bros. (US, Revision E)",    MACHINE_SUPPORTS_SAVE )
GAME( 1983, marioj, mario, mario, marioj, mario_state, empty_init, ROT0, "Nintendo",            "Mario Bros. (Japan, Revision C)", MACHINE_SUPPORTS_SAVE )
GAME( 1983, masao,  mario, masao, mario,  mario_state, empty_init, ROT0, "bootleg",             "Masao",                           MACHINE_SUPPORTS_SAVE )
