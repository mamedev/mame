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
#include "cpu/m68000/m68000.h"
#include "machine/eepromser.h"
#include "sound/es5506.h"
#include "audio/taito_en.h"
#include "includes/groundfx.h"


/***********************************************************
                INTERRUPTS
***********************************************************/

void groundfx_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	switch (id)
	{
	case TIMER_GROUNDFX_INTERRUPT5:
		m_maincpu->set_input_line(5, HOLD_LINE); //from 5... ADC port
		break;
	default:
		assert_always(FALSE, "Unknown id in groundfx_state::device_timer");
	}
}


/**********************************************************
            GAME INPUTS
**********************************************************/

CUSTOM_INPUT_MEMBER(groundfx_state::frame_counter_r)
{
	return m_frame_counter;
}

CUSTOM_INPUT_MEMBER(groundfx_state::coin_word_r)
{
	return m_coin_word;
}

WRITE32_MEMBER(groundfx_state::groundfx_input_w)
{
	switch (offset)
	{
		case 0x00:
			if (ACCESSING_BITS_24_31)   /* $500000 is watchdog */
			{
				machine().watchdog_reset();
			}

			if (ACCESSING_BITS_0_7)
			{
				ioport("EEPROMOUT")->write(data, 0xff);
			}

			break;

		case 0x01:
			if (ACCESSING_BITS_24_31)
			{
				machine().bookkeeping().coin_lockout_w(0,~data & 0x01000000);
				machine().bookkeeping().coin_lockout_w(1,~data & 0x02000000);
				machine().bookkeeping().coin_counter_w(0, data & 0x04000000);
				machine().bookkeeping().coin_counter_w(1, data & 0x08000000);
				m_coin_word = (data >> 16) &0xffff;
			}
			break;
	}
}

READ32_MEMBER(groundfx_state::groundfx_adc_r)
{
	return (ioport("AN0")->read() << 8) | ioport("AN1")->read();
}

WRITE32_MEMBER(groundfx_state::groundfx_adc_w)
{
	/* One interrupt per input port (4 per frame, though only 2 used).
	    1000 cycle delay is arbitrary */
	timer_set(downcast<cpu_device *>(&space.device())->cycles_to_attotime(1000), TIMER_GROUNDFX_INTERRUPT5);
}

WRITE32_MEMBER(groundfx_state::rotate_control_w)/* only a guess that it's rotation */
{
		if (ACCESSING_BITS_0_15)
		{
			m_rotate_ctrl[m_port_sel] = data;
			return;
		}

		if (ACCESSING_BITS_16_31)
		{
			m_port_sel = (data &0x70000) >> 16;
		}
}


WRITE32_MEMBER(groundfx_state::motor_control_w)
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

static ADDRESS_MAP_START( groundfx_map, AS_PROGRAM, 32, groundfx_state )
	AM_RANGE(0x000000, 0x1fffff) AM_ROM
	AM_RANGE(0x200000, 0x21ffff) AM_RAM AM_SHARE("ram") /* main CPUA ram */
	AM_RANGE(0x300000, 0x303fff) AM_RAM AM_SHARE("spriteram") /* sprite ram */
	AM_RANGE(0x400000, 0x400003) AM_WRITE(motor_control_w)  /* gun vibration */
	AM_RANGE(0x500000, 0x500003) AM_READ_PORT("BUTTONS")
	AM_RANGE(0x500004, 0x500007) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x500000, 0x500007) AM_WRITE(groundfx_input_w) /* eeprom etc. */
	AM_RANGE(0x600000, 0x600003) AM_READWRITE(groundfx_adc_r,groundfx_adc_w)
	AM_RANGE(0x700000, 0x7007ff) AM_RAM AM_SHARE("snd_shared")
	AM_RANGE(0x800000, 0x80ffff) AM_DEVREADWRITE("tc0480scp", tc0480scp_device, long_r, long_w)      /* tilemaps */
	AM_RANGE(0x830000, 0x83002f) AM_DEVREADWRITE("tc0480scp", tc0480scp_device, ctrl_long_r, ctrl_long_w)  // debugging
	AM_RANGE(0x900000, 0x90ffff) AM_DEVREADWRITE("tc0100scn", tc0100scn_device, long_r, long_w)    /* 6bpp tilemaps */
	AM_RANGE(0x920000, 0x92000f) AM_DEVREADWRITE("tc0100scn", tc0100scn_device, ctrl_long_r, ctrl_long_w)
	AM_RANGE(0xa00000, 0xa0ffff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0xb00000, 0xb003ff) AM_RAM                     // ?? single bytes, blending ??
	AM_RANGE(0xc00000, 0xc00007) AM_READNOP /* Network? */
	AM_RANGE(0xd00000, 0xd00003) AM_WRITE(rotate_control_w) /* perhaps port based rotate control? */
	/* f00000 is seat control? */
ADDRESS_MAP_END

/***********************************************************
             INPUT PORTS (dips in eprom)
***********************************************************/

static INPUT_PORTS_START( groundfx )
	PORT_START("BUTTONS")
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, groundfx_state,frame_counter_r, NULL)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_BUTTON3 )      /* shift hi */
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_BUTTON1 )      /* brake */
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON2 )      /* shift low */
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)

	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x00000001, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffff0000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, groundfx_state,coin_word_r, NULL)

	PORT_START("AN0")   /* IN 2, steering wheel */
	PORT_BIT( 0xff, 0x7f, IPT_AD_STICK_X ) PORT_SENSITIVITY(25) PORT_KEYDELTA(15) PORT_REVERSE PORT_PLAYER(1)

	PORT_START("AN1")   /* IN 3, accel */
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(1)

	PORT_START("AN2")   /* IN 4, sound volume */
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_X ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_REVERSE PORT_PLAYER(2)

	PORT_START("AN3")   /* IN 5, unknown */
	PORT_BIT( 0xff, 0x00, IPT_AD_STICK_Y ) PORT_SENSITIVITY(20) PORT_KEYDELTA(10) PORT_PLAYER(2)
INPUT_PORTS_END

/**********************************************************
                GFX DECODING
**********************************************************/

static const gfx_layout tile16x16_layout =
{
	16,16,  /* 16*16 sprites */
	RGN_FRAC(1,2),
	5,  /* 5 bits per pixel */
	{ RGN_FRAC(1,2), 0, 8, 16, 24 },
	{ 32, 33, 34, 35, 36, 37, 38, 39, 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*64, 1*64,  2*64,  3*64,  4*64,  5*64,  6*64,  7*64,
		8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	64*16   /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout charlayout =
{
	16,16,    /* 16*16 characters */
	RGN_FRAC(1,1),
	4,        /* 4 bits per pixel */
	{ 0, 1, 2, 3 },
	{ 1*4, 0*4, 5*4, 4*4, 3*4, 2*4, 7*4, 6*4, 9*4, 8*4, 13*4, 12*4, 11*4, 10*4, 15*4, 14*4 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64, 8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8     /* every sprite takes 128 consecutive bytes */
};

static const gfx_layout scclayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,2),
	6,      /* 4 bits per pixel */
	{ RGN_FRAC(1,2), RGN_FRAC(1,2)+1, 0, 1, 2, 3 },
	{ 2*4, 3*4, 0*4, 1*4, 6*4, 7*4, 4*4, 5*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8    /* every sprite takes 32 consecutive bytes */
};

static GFXDECODE_START( groundfx )
	GFXDECODE_ENTRY( "gfx2", 0x0, tile16x16_layout,  4096, 512 )
	GFXDECODE_ENTRY( "gfx1", 0x0, charlayout,        0, 512 )
	GFXDECODE_ENTRY( "gfx3", 0x0, scclayout,         0, 512 )
GFXDECODE_END


/***********************************************************
                 MACHINE DRIVERS
***********************************************************/

INTERRUPT_GEN_MEMBER(groundfx_state::groundfx_interrupt)
{
	m_frame_counter^=1;
	device.execute().set_input_line(4, HOLD_LINE);
}

static MACHINE_CONFIG_START( groundfx, groundfx_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68EC020, XTAL_40MHz/2) /* 20MHz - verified */
	MCFG_CPU_PROGRAM_MAP(groundfx_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", groundfx_state,  groundfx_interrupt)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0, 40*8-1, 3*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(groundfx_state, screen_update_groundfx)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", groundfx)
	MCFG_PALETTE_ADD("palette", 16384)
	MCFG_PALETTE_FORMAT(XRGB)

	MCFG_DEVICE_ADD("tc0100scn", TC0100SCN, 0)
	MCFG_TC0100SCN_GFX_REGION(2)
	MCFG_TC0100SCN_TX_REGION(3)
	MCFG_TC0100SCN_OFFSETS(50, 8)
	MCFG_TC0100SCN_GFXDECODE("gfxdecode")
	MCFG_TC0100SCN_PALETTE("palette")

	MCFG_DEVICE_ADD("tc0480scp", TC0480SCP, 0)
	MCFG_TC0480SCP_GFX_REGION(1)
	MCFG_TC0480SCP_TX_REGION(4)
	MCFG_TC0480SCP_OFFSETS(0x24, 0)
	MCFG_TC0480SCP_OFFSETS_TX(-1, 0)
	MCFG_TC0480SCP_GFXDECODE("gfxdecode")

	/* sound hardware */
	MCFG_DEVICE_ADD("taito_en", TAITO_EN, 0)
MACHINE_CONFIG_END

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
	ROM_LOAD16_BYTE( "d51-08.35", 0x000000, 0x200000, CRC(835b7a0f) SHA1(0131fceabd73b0045b5d4ae0bb2f03efdd407962) )    /* SCR 16x16 tiles */
	ROM_LOAD16_BYTE( "d51-09.34", 0x000001, 0x200000, CRC(6dabd83d) SHA1(3dbd7ea36b9900faa6420af1f1600efe295db74c) )

	ROM_REGION( 0x1000000, "gfx2", 0 )
	ROM_LOAD32_BYTE( "d51-03.47", 0x800000, 0x200000, CRC(629a5c99) SHA1(cfc1c0b07ecefd6eddb83edcbcf710e8b8de19e4) )    /* OBJ 16x16 tiles */
	ROM_LOAD32_BYTE( "d51-04.48", 0x000000, 0x200000, CRC(f49b14b7) SHA1(31129771159c1295a074c8311344ece525302289) )
	ROM_LOAD32_BYTE( "d51-05.49", 0x000001, 0x200000, CRC(3a2e2cbf) SHA1(ed2c1ca9211b1d70b4767a54e08263a3e4867199) )
	ROM_LOAD32_BYTE( "d51-06.50", 0x000002, 0x200000, CRC(d33ce2a0) SHA1(92c4504344672ea798cd6dd34f4b46848bf9f82b) )
	ROM_LOAD32_BYTE( "d51-07.51", 0x000003, 0x200000, CRC(24b2f97d) SHA1(6980e67b435d189ce897c0301e0411763410ab47) )

	ROM_REGION( 0x400000, "gfx3", 0 )
	ROM_LOAD16_BYTE( "d51-10.95", 0x000000, 0x100000, CRC(d5910604) SHA1(8efe13884cfdef208394ddfe19f43eb1b9f78ff3) )    /* SCC 8x8 tiles, 6bpp */
	ROM_LOAD16_BYTE( "d51-11.96", 0x000001, 0x100000, CRC(fee5f5c6) SHA1(1be88747f9c71c348dd61a8f0040007df3a3e6a6) )
	ROM_LOAD       ( "d51-12.97", 0x300000, 0x100000, CRC(d630287b) SHA1(2fa09e1821b7280d193ca9a2a270759c3c3189d1) )
	ROM_FILL       (              0x200000, 0x100000, 0x00 )

	ROM_REGION16_LE( 0x80000, "user1", 0 )
	ROM_LOAD16_WORD( "d51-13.7", 0x00000,  0x80000,  CRC(36921b8b) SHA1(2130120f78a3b984618a53054fc937cf727177b9) ) /* STY, spritemap */

	ROM_REGION16_BE( 0x1000000, "ensoniq.0", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "d51-01.73", 0x000000, 0x200000, CRC(92f09155) SHA1(8015e1997818bb480174394eb43840bf26679bcf) )    /* Ensoniq samples */
	ROM_LOAD16_BYTE( "d51-02.74", 0xc00000, 0x200000, CRC(20a9428f) SHA1(c9033d02a49c72f704808f5f899101617d5814e5) )

	ROM_REGION16_BE( 0x80, "eeprom", 0 )
	ROM_LOAD( "93c46.164", 0x0000, 0x0080, CRC(6f58851d) SHA1(33bd4478f097dca6b5d222adb89699c6d35ed009) )
ROM_END


READ32_MEMBER(groundfx_state::irq_speedup_r_groundfx)
{
	cpu_device *cpu = downcast<cpu_device *>(&space.device());
	int ptr;
	offs_t sp = cpu->sp();
	if ((sp&2)==0) ptr=m_ram[(sp&0x1ffff)/4];
	else ptr=(((m_ram[(sp&0x1ffff)/4])&0x1ffff)<<16) |
	(m_ram[((sp&0x1ffff)/4)+1]>>16);

	if (cpu->pc()==0x1ece && ptr==0x1b9a)
		cpu->spin_until_interrupt();

	return m_ram[0xb574/4];
}


DRIVER_INIT_MEMBER(groundfx_state,groundfx)
{
	UINT32 offset,i;
	UINT8 *gfx = memregion("gfx3")->base();
	int size=memregion("gfx3")->bytes();
	int data;

	/* Speedup handlers */
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x20b574, 0x20b577, read32_delegate(FUNC(groundfx_state::irq_speedup_r_groundfx),this));

	/* make SCC tile GFX format suitable for gfxdecode */
	offset = size/2;
	for (i = size/2+size/4; i<size; i++)
	{
		int d1,d2,d3,d4;

		/* Expand 2bits into 4bits format */
		data = gfx[i];
		d1 = (data>>0) & 3;
		d2 = (data>>2) & 3;
		d3 = (data>>4) & 3;
		d4 = (data>>6) & 3;

		gfx[offset] = (d1<<2) | (d2<<6);
		offset++;

		gfx[offset] = (d3<<2) | (d4<<6);
		offset++;
	}
}


GAME( 1992, groundfx, 0, groundfx, groundfx, groundfx_state, groundfx, ROT0, "Taito Corporation", "Ground Effects / Super Ground Effects (Japan)", 0 )
