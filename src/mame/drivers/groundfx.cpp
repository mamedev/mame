// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, David Graves
/***************************************************************************

    Ground Effects / Super Ground FX                    (c) 1993 Taito

    Driver by Bryan McPhail & David Graves.

    Board Info:

    K1100744A
    J1100316A
    Sticker - K11J0744A
    ------------------------------------------------------------------------------------------------
    | 2018  2088        43256      43256   MC68EC020RP25   DIP-SW (8) COM20020  SLIDE-SW  LAN-IN   |           Connector G
    | 2018  2088        D51-21     D51-22              D51-14                                      |           -----------
    | 2018  2088        43256      43256               (PAL16L8)                                   |       Parts    |   Solder
    |                                                  D51-15              ADC0809        LAN-OUT  |       ------------------
    | 2018              D51-23     D51-24              (PAL16L8)                                   |        GND    1 A   GND
    |                                                  D51-16-1                          JP4       |        GND    2 B   GND
    | 2018  TC0570SPC                                  (PAL16L8)            EEPROM.164   1-2       |        +12    3 C   +12
    | 2018                                                                               2-3       |         +5    4 D   +5
    |                                                                 TC0510NIO          2-3     --|         +5    5 E   +5
    |    D51-13       D51-03                                                             2-3     |           +5    6 F   +5
    |                 D51-04                             43256                           1-2     |                 7 H
    |    TC0470LIN    D51-05                                                             1-2     --|        +13    8 J   +13
    |                 D51-06                             43256   TC0650FCA               1-2       |        +13    9 K   +13
    |                 D51-07                                                             2-3       |      SPK_R+  10 L  SPK_R-
    |    TC0580PIV              43256                    43256      2018                 2-3       |      SPK_L+  11 M  SPK_L-
    |                           43256     D51-17                                                   |   SPK_REAR-  12 N  SPK_REAR-
    | 514256          TC0480SCP           (PAL16L8)                                                |              13 P
    | 514256                                                                                      G|         RED  14 R  GREEN
    | 514256                              D51-10                                                   |        BLUE  15 S  SYNC
    | 514256   D51-09                     D51-11          TC0620SCC                                |       V-GND  16 T
    | 514256   D51-08                     D51012                                                   |      METER1  17 U  METER2
    | 514256          43256                                                                        |    LOCKOUT1  18 V  LOCKOUT2
    |          MB8421                                        43256                                 |              19 W
    | D51-18   MB8421             ENSONIC                                                        --|       COIN1  20 X  COIN2
    | (PAL20L8)       D51-29      OTISR2                     43256      MB87078                  |       SERVICE  21 Y  TEST
    |                         D51-01                                                             |                22 Z
    | D51-19          43256           ENSONIC     ENSONIC                                        --|    SHIFT UP  23 a  BRAKE
    | (PAL20L8)               D51-02  SUPER GLU   ESP-R6    TC511664    TL074  TL074               |   HANDLE VR  24 b  ACCEL VR
    |                 D51-30                                                                       |    SHIFT DN  25 c
    | MC68000P12F16MHz                                                  TDA1543 TDA1543            |              26 d
    |                 40MHz    16MHz  30.476MHz    MC68681                                         |         GND  27 e  GND
    | D51-20                                                                                       |         GND  28 f  GND
    | (PAL20L8)                                                                                    |
    ------------------------------------------------------------------------------------------------


    Ground Effects combines the sprite system used in Taito Z games with
    the TC0480SCP tilemap chip plus some features from the Taito F3 system.
    It has an extra TC0620SCC tilemap chip which is a 6bpp version of the
    TC0100SCN (check the inits), like Under Fire.

    Ground Effects is effectively a 30Hz game - though the vblank interrupts
    still come in at 60Hz, the game uses a hardware frame counter to limit
    itself to 30Hz (it only updates things like the video registers every
    other vblank).  There isn't enough cpu power in a 20MHz 68020 to run
    this game at 60Hz.

    Ground Effects has a network mode - probably uses IRQ 6 and the unknown
    ports at 0xc00000.

***************************************************************************/

#include "emu.h"
#include "includes/groundfx.h"
#include "audio/taito_en.h"
#include "machine/taitoio.h"

#include "cpu/m68000/m68000.h"
#include "machine/adc0808.h"
#include "machine/eepromser.h"
#include "sound/es5506.h"
#include "screen.h"


/**********************************************************
            GAME INPUTS
**********************************************************/

READ_LINE_MEMBER(groundfx_state::frame_counter_r)
{
	return m_frame_counter;
}

void groundfx_state::coin_word_w(u8 data)
{
	machine().bookkeeping().coin_lockout_w(0,~data & 0x01);
	machine().bookkeeping().coin_lockout_w(1,~data & 0x02);
	machine().bookkeeping().coin_counter_w(0, data & 0x04);
	machine().bookkeeping().coin_counter_w(1, data & 0x08);
}

void groundfx_state::rotate_control_w(offs_t offset, u16 data) /* only a guess that it's rotation */
{
	if (offset & 1)
	{
		m_rotate_ctrl[m_port_sel] = data;
		return;
	}
	else
	{
		m_port_sel = data & 0x7;
	}
}

void groundfx_state::motor_control_w(u32 data)
{
/*
    Standard value poked is 0x00910200 (we ignore lsb and msb
    which seem to be always zero)

    0x0, 0x8000 and 0x9100 are written at startup

    Two bits are written in test mode to this middle word
    to test gun vibration:

    ........ .x......   P1 gun vibration
    ........ x.......   P2 gun vibration
*/
}


/***********************************************************
             MEMORY STRUCTURES
***********************************************************/

void groundfx_state::groundfx_map(address_map &map)
{
	map(0x000000, 0x1fffff).rom();
	map(0x200000, 0x21ffff).ram().share("ram"); /* main CPUA ram */
	map(0x300000, 0x303fff).ram().share("spriteram"); /* sprite ram */
	map(0x400000, 0x400003).w(FUNC(groundfx_state::motor_control_w));  /* gun vibration */
	map(0x500000, 0x500007).rw("tc0510nio", FUNC(tc0510nio_device::read), FUNC(tc0510nio_device::write));
	map(0x600000, 0x600007).rw("adc", FUNC(adc0808_device::data_r), FUNC(adc0808_device::address_offset_start_w)).umask32(0xffffffff);
	map(0x700000, 0x7007ff).rw("taito_en:dpram", FUNC(mb8421_device::left_r), FUNC(mb8421_device::left_w));
	map(0x800000, 0x80ffff).rw(m_tc0480scp, FUNC(tc0480scp_device::ram_r), FUNC(tc0480scp_device::ram_w));      /* tilemaps */
	map(0x830000, 0x83002f).rw(m_tc0480scp, FUNC(tc0480scp_device::ctrl_r), FUNC(tc0480scp_device::ctrl_w));  // debugging
	map(0x900000, 0x90ffff).rw(m_tc0100scn, FUNC(tc0100scn_device::ram_r), FUNC(tc0100scn_device::ram_w));    /* 6bpp tilemaps */
	map(0x920000, 0x92000f).rw(m_tc0100scn, FUNC(tc0100scn_device::ctrl_r), FUNC(tc0100scn_device::ctrl_w));
	map(0xa00000, 0xa0ffff).ram().w(m_palette, FUNC(palette_device::write32)).share("palette");
	map(0xb00000, 0xb003ff).ram();                     // ?? single bytes, blending ??
	map(0xc00000, 0xc00007).nopr(); /* Network? */
	map(0xd00000, 0xd00003).w(FUNC(groundfx_state::rotate_control_w)); /* perhaps port based rotate control? */
	/* f00000 is seat control? */
}

/***********************************************************
             INPUT PORTS (dips in eprom)
***********************************************************/

static INPUT_PORTS_START( groundfx )
	PORT_START("BUTTONS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Shift Hi")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Brake")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Shift Low")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("WHEEL")
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("ACCEL")
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(1)
INPUT_PORTS_END

/**********************************************************
                GFX DECODING
**********************************************************/

static const gfx_layout tile16x16_layout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,5),
	5,  /* 5 bits per pixel */
	{ 0, RGN_FRAC(1,5), RGN_FRAC(2,5), RGN_FRAC(3,5), RGN_FRAC(4,5) },
	{ STEP16(0,1) },
	{ STEP16(0,16) },
	16*16   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	16,16,    /* 16*16 characters */
	RGN_FRAC(1,1),
	4,        /* 4 bits per pixel */
	{ STEP4(0,1) },
	{ STEP8(7*4,-4), STEP8(15*4,-4) },
	{ STEP16(0,16*4) },
	16*16*4     /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout scclayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,2),
	6,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2), RGN_FRAC(1,2)+1, STEP4(0,1) },
	{ STEP8(0,4) },
	{ STEP8(0,4*8) },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( gfx_groundfx )
	GFXDECODE_ENTRY( "gfx2", 0x0, tile16x16_layout,  4096, 512 )
	GFXDECODE_ENTRY( "gfx1", 0x0, charlayout,        0, 512 )
	GFXDECODE_ENTRY( "gfx3", 0x0, scclayout,         0, 512 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

INTERRUPT_GEN_MEMBER(groundfx_state::interrupt)
{
	m_frame_counter^=1;
	device.execute().set_input_line(4, HOLD_LINE);
}

void groundfx_state::groundfx(machine_config &config)
{
	/* basic machine hardware */
	M68EC020(config, m_maincpu, XTAL(40'000'000) / 2); /* 20MHz - verified */
	m_maincpu->set_addrmap(AS_PROGRAM, &groundfx_state::groundfx_map);
	m_maincpu->set_vblank_int("screen", FUNC(groundfx_state::interrupt));

	EEPROM_93C46_16BIT(config, "eeprom");

	adc0809_device &adc(ADC0809(config, "adc", 500000)); // unknown clock
	adc.eoc_ff_callback().set_inputline("maincpu", 5);
	adc.in_callback<0>().set_constant(0); // unknown
	adc.in_callback<1>().set_constant(0); // unknown (used to be labeled 'volume' - but doesn't seem to affect it
	adc.in_callback<2>().set_ioport("WHEEL");
	adc.in_callback<3>().set_ioport("ACCEL");

	tc0510nio_device &tc0510nio(TC0510NIO(config, "tc0510nio", 0));
	tc0510nio.read_2_callback().set_ioport("BUTTONS");
	tc0510nio.read_3_callback().set("eeprom", FUNC(eeprom_serial_93cxx_device::do_read)).lshift(7);
	tc0510nio.read_3_callback().append(FUNC(groundfx_state::frame_counter_r)).lshift(0);
	tc0510nio.write_3_callback().set("eeprom", FUNC(eeprom_serial_93cxx_device::clk_write)).bit(5);
	tc0510nio.write_3_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::di_write)).bit(6);
	tc0510nio.write_3_callback().append("eeprom", FUNC(eeprom_serial_93cxx_device::cs_write)).bit(4);
	tc0510nio.write_4_callback().set(FUNC(groundfx_state::coin_word_w));
	tc0510nio.read_7_callback().set_ioport("SYSTEM");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0, 40*8-1, 3*8, 32*8-1);
	screen.set_screen_update(FUNC(groundfx_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_groundfx);
	PALETTE(config, m_palette).set_format(palette_device::xRGB_888, 16384);

	TC0100SCN(config, m_tc0100scn, 0);
	m_tc0100scn->set_gfx_region(2);
	m_tc0100scn->set_offsets(50, 8);
	m_tc0100scn->set_gfxdecode_tag(m_gfxdecode);
	m_tc0100scn->set_palette(m_palette);

	TC0480SCP(config, m_tc0480scp, 0);
	m_tc0480scp->set_gfx_region(1);
	m_tc0480scp->set_palette(m_palette);
	m_tc0480scp->set_offsets(0x24, 0);
	m_tc0480scp->set_offsets_tx(-1, 0);
	m_tc0480scp->set_gfxdecode_tag(m_gfxdecode);

	/* sound hardware */
	TAITO_EN(config, "taito_en", 0);
}

/***************************************************************************
                    DRIVERS
***************************************************************************/

ROM_START( groundfx )
	ROM_REGION( 0x200000, "maincpu", 0 )    /* 2048K for 68020 code (CPU A) */
	ROM_LOAD32_BYTE( "d51-24.79", 0x00000, 0x80000, CRC(5caaa031) SHA1(03e727e26df701e3f5e16c5f933d5b29a528945a) )
	ROM_LOAD32_BYTE( "d51-23.61", 0x00001, 0x80000, CRC(462e3c9b) SHA1(7f116ee755748497b911868a948d3e3b5134e475) )
	ROM_LOAD32_BYTE( "d51-22.77", 0x00002, 0x80000, CRC(b6b04d88) SHA1(58685ee8fd788dcbfe318f1e3c06d93e2128034c) )
	ROM_LOAD32_BYTE( "d51-21.59", 0x00003, 0x80000, CRC(21ecde2b) SHA1(c6d3738f34c8e24346e7784b14aeff300ae2d225) )

	ROM_REGION( 0x180000, "taito_en:audiocpu", 0 )
	ROM_LOAD16_BYTE( "d51-29.54", 0x100000, 0x40000,  CRC(4b64f41d) SHA1(040427668d13f7320d23805098d6d0e1aa8d121e) )
	ROM_LOAD16_BYTE( "d51-30.56", 0x100001, 0x40000,  CRC(45f339fe) SHA1(cc7adfb2b86070f5bb426542e3b7ed2a50b3c39e) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD32_WORD_SWAP( "d51-08.35", 0x000002, 0x200000, CRC(835b7a0f) SHA1(0131fceabd73b0045b5d4ae0bb2f03efdd407962) )    /* SCR 16x16 tiles */
	ROM_LOAD32_WORD_SWAP( "d51-09.34", 0x000000, 0x200000, CRC(6dabd83d) SHA1(3dbd7ea36b9900faa6420af1f1600efe295db74c) )

	ROM_REGION( 0xa00000, "gfx2", 0 )
	ROM_LOAD16_WORD_SWAP( "d51-03.47", 0x000000, 0x200000, CRC(629a5c99) SHA1(cfc1c0b07ecefd6eddb83edcbcf710e8b8de19e4) )    /* OBJ 16x16 tiles */
	ROM_LOAD16_WORD_SWAP( "d51-04.48", 0x200000, 0x200000, CRC(f49b14b7) SHA1(31129771159c1295a074c8311344ece525302289) )
	ROM_LOAD16_WORD_SWAP( "d51-05.49", 0x400000, 0x200000, CRC(3a2e2cbf) SHA1(ed2c1ca9211b1d70b4767a54e08263a3e4867199) )
	ROM_LOAD16_WORD_SWAP( "d51-06.50", 0x600000, 0x200000, CRC(d33ce2a0) SHA1(92c4504344672ea798cd6dd34f4b46848bf9f82b) )
	ROM_LOAD16_WORD_SWAP( "d51-07.51", 0x800000, 0x200000, CRC(24b2f97d) SHA1(6980e67b435d189ce897c0301e0411763410ab47) )

	ROM_REGION( 0x400000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "d51-10.95", 0x000001, 0x100000, CRC(d5910604) SHA1(8efe13884cfdef208394ddfe19f43eb1b9f78ff3) )    /* SCC 8x8 tiles, 6bpp */
	ROM_LOAD16_BYTE( "d51-11.96", 0x000000, 0x100000, CRC(fee5f5c6) SHA1(1be88747f9c71c348dd61a8f0040007df3a3e6a6) )
	ROM_LOAD       ( "d51-12.97", 0x300000, 0x100000, CRC(d630287b) SHA1(2fa09e1821b7280d193ca9a2a270759c3c3189d1) )
	ROM_FILL       (              0x200000, 0x100000, 0x00 )

	ROM_REGION16_LE( 0x80000, "spritemap", 0 )
	ROM_LOAD16_WORD( "d51-13.7", 0x00000,  0x80000,  CRC(36921b8b) SHA1(2130120f78a3b984618a53054fc937cf727177b9) ) /* STY, spritemap */

	ROM_REGION16_BE( 0x1000000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d51-01.73", 0x000000, 0x200000, CRC(92f09155) SHA1(8015e1997818bb480174394eb43840bf26679bcf) )    /* Ensoniq samples */
	ROM_LOAD16_BYTE( "d51-02.74", 0xc00000, 0x200000, CRC(20a9428f) SHA1(c9033d02a49c72f704808f5f899101617d5814e5) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.164", 0x0000, 0x0080, CRC(6f58851d) SHA1(33bd4478f097dca6b5d222adb89699c6d35ed009) )
ROM_END

/*The World version of Ground Effects is not dumped and has the following EPROM labels:
'D51 28' @ IC59
'D51 31' @ IC77
'D51 32' @ IC61
'D51 33' @ IC79*/

READ32_MEMBER(groundfx_state::irq_speedup_r)
{
	int ptr;
	offs_t sp = m_maincpu->sp();
	if ((sp & 2) == 0) ptr = m_ram[(sp & 0x1ffff) / 4];
	else ptr = (((m_ram[(sp & 0x1ffff) / 4]) & 0x1ffff) << 16) |
	(m_ram[((sp & 0x1ffff) / 4) + 1] >> 16);

	if (m_maincpu->pc() == 0x1ece && ptr == 0x1b9a)
		m_maincpu->spin_until_interrupt();

	return m_ram[0xb574 / 4];
}


void groundfx_state::init_groundfx()
{
	u8 *gfx = memregion("gfx3")->base();
	const u32 size = memregion("gfx3")->bytes();

	/* Speedup handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x20b574, 0x20b577, read32_delegate(FUNC(groundfx_state::irq_speedup_r),this));

	/* make SCC tile GFX format suitable for gfxdecode */
	u32 offset = size/2;
	for (u32 i = size/2 + size / 4; i < size; i++)
	{
		/* Expand 2bits into 4bits format */
		const u8 data = gfx[i];
		const u8 d1 = (data >> 0) & 3;
		const u8 d2 = (data >> 2) & 3;
		const u8 d3 = (data >> 4) & 3;
		const u8 d4 = (data >> 6) & 3;

		gfx[offset] = (d3 << 2) | (d4 << 6);
		offset++;

		gfx[offset] = (d1 << 2) | (d2 << 6);
		offset++;
	}
}


GAME( 1992, groundfx, 0, groundfx, groundfx, groundfx_state, init_groundfx, ROT0, "Taito Corporation", "Ground Effects / Super Ground Effects (Japan)", MACHINE_NODEVICE_LAN )
