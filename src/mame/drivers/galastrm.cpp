// license:BSD-3-Clause
// copyright-holders:Hau
/*

Galactic Storm
(c)1992 Taito

----------------------------------------------------------
MAIN PCB
CPU:MC68EC020RP25
TC0480SCP
TC0100SCN
TC0510NIO
TC0580PIV x2
TC0110PCR
TC0470LIN x2
TC0570SPC
TC0610
ADC0809CCN

OSC1:32MHz
OSC2:20MHz
----------------------------------------------------------
SOUND BOARD
CPU:MC68000P12F,MC68681P
ENSONIQ 5701,5510,5505

OSC1:16MHz
OSC2:30.47618MHz
----------------------------------------------------------
based on driver from drivers/gunbustr.cpp by Bryan McPhail & David Graves
Written by Hau
07/03/2008


tips
$300.b debugmode
$305.b invincibility

TODO:
- device-ify TC0610? (no other known users)
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/adc0808.h"
#include "machine/eepromser.h"
#include "machine/taitoio.h"
#include "sound/es5506.h"
#include "audio/taito_en.h"
#include "includes/galastrm.h"


/*********************************************************************/

INTERRUPT_GEN_MEMBER(galastrm_state::interrupt)
{
	m_frame_counter ^= 1;
	device.execute().set_input_line(5, HOLD_LINE);
}

template<int Chip>
void galastrm_state::tc0610_w(offs_t offset, u16 data)
{
	if (offset == 0)
		m_tc0610_addr[Chip] = data;
	else if (m_tc0610_addr[Chip] < 8)
		m_tc0610_ctrl_reg[Chip][m_tc0610_addr[Chip]] = data;
}


READ_LINE_MEMBER(galastrm_state::frame_counter_r)
{
	return m_frame_counter;
}

void galastrm_state::coin_word_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0, ~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1, ~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x04);
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

void galastrm_state::main_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x21ffff).ram();                             /* main CPU ram */
	map(0x300000, 0x303fff).ram().share("spriteram");
	map(0x400000, 0x400007).rw("tc0510nio", FUNC(tc0510nio_device::read), FUNC(tc0510nio_device::write));
	map(0x40fff0, 0x40fff3).nopw();
	map(0x500000, 0x500007).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w)).umask32(0xffffffff);
	map(0x600000, 0x6007ff).rw("taito_en:dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w)); /* Sound shared ram */
	map(0x800000, 0x80ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));        /* tilemaps */
	map(0x830000, 0x83002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));
	map(0x900000, 0x900003).rw(m_tc0110pcr, FUNC(tc0110pcr_device::word_r), FUNC(tc0110pcr_device::step1_rbswap_word_w));                               /* TC0110PCR */
	map(0xb00000, 0xb00003).w(FUNC(galastrm_state::tc0610_w<0>));                              /* TC0610 */
	map(0xc00000, 0xc00003).w(FUNC(galastrm_state::tc0610_w<1>));
	map(0xd00000, 0xd0ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));        /* piv tilemaps */
	map(0xd20000, 0xd2000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
}

/***********************************************************
             INPUT PORTS (dips in eprom)
***********************************************************/

static INPUT_PORTS_START( galastrm )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(galastrm_state, frame_counter_r)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
//  PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(1)  /* Freeze input */
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)

	PORT_START("IN2")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)

	PORT_START("STICKX")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_X ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_PLAYER(1)

	PORT_START("STICKY")
	PORT_BIT( 0xff, 0x80, IPT_AD_STICK_Y ) PORT_SENSITIVITY(60) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)
INPUT_PORTS_END


/***********************************************************
                GFX DECODING
**********************************************************/

static const gfx_layout tile16x16_layout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,1),
	4,  /* 4 bits per pixel */
	{ STEP4(0,16) },
	{ STEP16(0,1) },
	{ STEP16(0,16*4) },
	64*16   /* every sprite takes 128 consecutive bytes */
};

static GFXDECODE_START( gfx_galastrm )
	GFXDECODE_ENTRY( "sprites", 0x0, tile16x16_layout, 0, 4096/16 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

/***************************************************************************/

void galastrm_state::galastrm(machine_config &config)
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, 16000000); /* 16 MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &galastrm_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(galastrm_state::interrupt)); /* VBL */

	EEPROM_93C46_16BIT(config, "eeprom");

	adc0809_device &adc(ADC0809(config, "adc", 500000)); // unknown clock
	adc.eoc_ff_callback().set_inputline("maincpu", 6);
	adc.in_callback<0>().set_ioport("STICKX");
	adc.in_callback<1>().set_ioport("STICKY");

	tc0510nio_device &tc0510nio(TC0510NIO(config, "tc0510nio", 0));
	tc0510nio.read_2_callback().set_ioport("IN0");
	tc0510nio.read_3_callback().set_ioport("IN1");
	tc0510nio.write_3_callback().set("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(5);
	tc0510nio.write_3_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(6);
	tc0510nio.write_3_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	tc0510nio.write_4_callback().set(FUNC(galastrm_state::coin_word_w));
	tc0510nio.read_7_callback().set_ioport("IN2");

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(64*8, 50*8);
	m_screen->set_visarea(0+96, 40*8-1+96, 3*8+60, 32*8-1+60);
	m_screen->set_screen_update(FUNC(galastrm_state::screen_update));
	m_screen->set_palette(m_tc0110pcr);

	GFXDECODE(config, m_gfxdecode, m_tc0110pcr, gfx_galastrm);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_offsets(-48, -56);
	m_tc0100scn->set_palette(m_tc0110pcr);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_palette(m_tc0110pcr);
	m_tc0480scp->set_offsets(-40, -3);

	TC0110PCR(config, m_tc0110pcr, 0);

	/* sound hardware */
	TAITO_EN(config, "taito_en", 0);
}

/***************************************************************************/

ROM_START( galastrm )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "c99_15.ic105", 0x00000, 0x40000,  CRC(7eae8efd) SHA1(6bbb3da697dfcd93337b53895678e2a4ff2de457) )
	ROM_LOAD32_BYTE( "c99_12.ic102", 0x00001, 0x40000,  CRC(e059d1ee) SHA1(560951f95f270f0559b5289dda7f4ba74538cfcb) )
	ROM_LOAD32_BYTE( "c99_13.ic103", 0x00002, 0x40000,  CRC(885fcb35) SHA1(be10e109c461c1f776e98efa1b2a4d588aa0c41c) )
	ROM_LOAD32_BYTE( "c99_14.ic104", 0x00003, 0x40000,  CRC(457ef6b1) SHA1(06c2613d46addacd380a0f2413cd795b17ac9474) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )
	ROM_LOAD16_BYTE( "c99_23.ic8",  0x100000, 0x20000,  CRC(5718ee92) SHA1(33cfa60c5bceb1525498f27b598067d2dc620431) )
	ROM_LOAD16_BYTE( "c99_22.ic7",  0x100001, 0x20000,  CRC(b90f7c42) SHA1(e2fa9ee10ad61ae1a672c3357c0072b79ec7fbcb) )

	ROM_REGION( 0x100, "tc0100scn", ROMREGION_ERASE00 ) // no roms for tc0100scn, dummy

	ROM_REGION( 0x200000, "tc0480scp", 0 )
	ROM_LOAD32_WORD( "c99-05.ic1",  0x000000, 0x100000, CRC(a91ffba4) SHA1(467af9646ddad5fbb520b6bc13517ed4deacf479) )  /* SCR 16x16 tiles */
	ROM_LOAD32_WORD( "c99-06.ic2",  0x000002, 0x100000, CRC(812ed3ae) SHA1(775904dd42643d0e3a30890590d5f8eac1fe78db) )

	ROM_REGION( 0x400000, "sprites", 0 )
	ROM_LOAD64_WORD_SWAP( "c99-02.ic50", 0x000000, 0x100000, CRC(81e9fc6f) SHA1(4495a7d130b755b5a48eaa814d884d6bb8243bcb) )  /* OBJ 16x16 tiles */
	ROM_LOAD64_WORD_SWAP( "c99-01.ic51", 0x000002, 0x100000, CRC(9dda1267) SHA1(c639ba064496dcadf5f1e55332a12bb442e9dc86) )
	ROM_LOAD64_WORD_SWAP( "c99-04.ic66", 0x000004, 0x100000, CRC(a681760f) SHA1(23d4fc7eb778c8a25c4bc7cee1d0c8cdd828a996) )
	ROM_LOAD64_WORD_SWAP( "c99-03.ic67", 0x000006, 0x100000, CRC(a2807a27) SHA1(977e395ea2ab2fb82807d3cf5fe5f1dbbde99da0) )

	ROM_REGION16_LE( 0x80000, "sprmaprom", 0 )
	ROM_LOAD16_WORD( "c99-11.ic90",  0x00000,  0x80000, CRC(26a6926c) SHA1(918860e2829131e9ecfe983b2ae3e49e1c9ecd72) )  /* STY, spritemap */

	ROM_REGION16_BE( 0x1000000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "c99-08.ic3",  0x000000, 0x100000, CRC(fedb4187) SHA1(83563e4af795a0dfeb261a62c31b6fed72f45a4d) )  /* Ensoniq samples */
	ROM_LOAD16_BYTE( "c99-09.ic4",  0x200000, 0x100000, CRC(ba70b86b) SHA1(ffbb9547d6b6e47a3ef23206b5f40c57f3ea7619) )
	ROM_LOAD16_BYTE( "c99-10.ic5",  0x400000, 0x100000, CRC(da016f1e) SHA1(581ef158c6f6576618dd75429b1d3aa92cd3581d) )
	ROM_LOAD16_BYTE( "c99-07.ic2",  0x680000, 0x040000, CRC(4cc3136f) SHA1(d9d7556bbe6af161fa0651b1fbd72e7dbf0a8e82) )
	ROM_CONTINUE( 0x600000, 0x040000 )
	ROM_CONTINUE( 0x780000, 0x040000 )
	ROM_CONTINUE( 0x700000, 0x040000 )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD16_WORD( "eeprom-galastrm.bin", 0x0000, 0x0080, CRC(94efa7a6) SHA1(5870b988cb364065e8bd779efbdadca8d3ffc17c) )
ROM_END


GAME( 1992, galastrm, 0, galastrm, galastrm, galastrm_state, empty_init, ROT0, "Taito Corporation", "Galactic Storm (Japan)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
