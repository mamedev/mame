/***************************************************************************

  Namco System 10 - Arcade PSX Hardware
  =====================================
  Driver by smf

Namco System 10 Hardware Overview
---------------------------------
Note! This document is a Work-In-Progress and will be updated from time to time when more dumps are available.

This document covers all the known Namco System 10 games, including....
*Drum Master                                     (C) Namco, 2001
*Drum Master 2                                   (C) Namco, 2001
*Drum Master 3                                   (C) Namco, 2002
*Drum Master 4                                   (C) Namco, 2003
*Drum Master 5                                   (C) Namco, 2003
*Drum Master 6                                   (C) Namco, 2004
*Drum Master 7                                   (C) Namco, 2005
*GAHAHA Ippatsu-dou                              (C) Namco/Metro, 2000
*GAHAHA Ippatsu-dou 2                            (C) Namco/Metro, 2001
Gamshara (10021 Ver.A)                           (C) Mitchell, 2003
Gekitoride-Jong Space (10011 Ver.A)              (C) Namco/Metro, 2001
*Golgo-13 3 : Juusei no Chinkonka                (C) Namco/8ing/Raizing, 2001
*Hard Puncher Hajime no Ippo 2 Ouja e no Chousen (C) Namco/Taito, 2002
*Honne Hakkenki                                  (C) Namco, 2001
Kotoba no Puzzle Mojipittan (KPM1 Ver.A)         (C) Namco, 2001
*Kurukuru Food                                   (C) Namco, 2002
Mr Driller 2 (DR21 Ver.A)                        (C) Namco, 2000
Mr Driller 2 (DR22 Ver.A)                        (C) Namco, 2000
Mr Driller G (DRG1 Ver.A)                        (C) Namco, 2001
*NFL Classic Football                            (C) Namco, 2003
*Panicuru Panekuru (PPA1 Ver.A)                  (C) Namco, 2001
*Photo Battole                                   (C) Namco, 2001
*Ren-ai Quiz High School Angel                   (C) Namco, 2002
*Seishun Quiz Colorful High School               (C) Namco, 2002
Star Trigon (STT1 Ver.A)                         (C) Namco, 2002
*Taiko No Tatsujin                               (C) Namco, 2001
*Taiko No Tatsujin 2                             (C) Namco, 2001
*Taiko No Tatsujin 3                             (C) Namco, 2002
*Taiko No Tatsujin 4                             (C) Namco, 2003
*Taiko No Tatsujin 5                             (C) Namco, 2003
*Taiko No Tatsujin 6                             (C) Namco, 2004
*Tsukkomi Yousei Gips Nice Tsukkomi              (C) Namco/Metro, 2002
*Uchuu Daisakusen : Chocovader Contactee (CVC1 Ver.A) (C) Namco, 2002

* - denotes not dumped yet. If you can help with the remaining undumped S10 games,
    please contact me at http://guru.mameworld.info/

The Namco System 10 system comprises 2 PCB's....
MAIN PCB - This is the mother board PCB. It holds the main CPU/GPU & SPU and all sound circuitry, program & video RAM,
           controller/input logic and video output circuitry. Basically everything except the ROMs.
           There are three known revisions of this PCB so far. The differences seem very minor. The 2nd and 3rd revision
           have an updated CPLD revision.
           The 3rd revision has an updated model Sony chip. The only other _noticable_ difference is some component
           shuffling in the sound amplification section to accomodate two extra 1000uF capacitors and one 470uF capacitor
           has been replaced by a 1000uF capacitor. Everything else, including all the PLDs appears to be identical.
           Note there are no ROMs on the Main PCB and also no custom Namco chips on System10, which seem to have been
           phased out. Instead, they have been replaced by (custom programmed) CPLDs, probably due to cost-cutting
           measures within the company, or to reduce the cost of System10 to an entry-level no-frills development platform.
MEM PCB  - There are two known revisions of this PCB (so far). They're mostly identical except for the type/number of ROMs
           used and the contents of the CPLD.  The 2nd revision also has a RAM chip on it.
           Each game has a multi-letter code assigned to it which is printed on a small sticker and placed on the top side
           of the MEM PCB.
           This code is then proceeded by a number (only '1' & '2' seen so far), then 'Ver.' then A/B/C/D/E (only 'A' seen so far)
           which denotes the software revision, and in some cases a sub-revision such as 1 or 2 (usually only listed in the
           test mode).
           The first 1 denotes a Japanese version. Other numbers denote a World version.
           For World versions, only the main program changes, the rest of the (graphics) ROMs use the Japanese version ROMs.
           Any System 10 MEM PCB can be swapped to run on any System 10 Main PCB regardless of the main board revision.
           The high scores are stored on the MEM PCB (probably inside the main program EEPROMs/FlashROMs or maybe the CPLD?).
           Also, on all System 10 games, there is a sticker with a serial number on it and the program ROMs also contain
           that same serial number. I'm not sure why, they're not exactly _easily_ tracable and no one cares either way ;-)
           See the Main PCB and ROM Daughterboard PCB texts below for more details.


Main PCB Layout
---------------

Revision 1
SYSTEM10 MAIN PCB 8906960103 (8906970103)

Revision 2
SYSTEM10 MAIN PCB 8906960104 (8906970104)

Revision 3
SYSTEM10 MAIN PCB 8906962400 (8906972400)
  |----------------------------------------------------------|
  |   LA4705    VR1                     J201                 |
  |                           |----------------------|       |
  |           NJM3414         |----------------------|       |
|-|                                                          |
|       BA3121                                               |
|             NJM3414            54V25632      54V25632    J1|
|                     CXD1178Q                               |
|J JP4                          |---------|   |-------|      |
|A                              |         |   |       |      |
|M         CXA2067AS            |CXD8561CQ|   |CY37128|      |
|M                 53.693175MHz |         |   |VP160  |      |
|A                              |         |   |       |      |
|                               |---------|   |-------|      |
|                                                            |
|                            101.4912MHz                     |
|-|          MAX734  IS41LV16100                             |
  |                              |---------|                 |
  |   DSW1           IS41LV16100 |         |                 |
  |                              |CXD8606BQ|                 |
  |        GAL16V8D              |         |                 |
  |J5      |-|           *       |         |          PST592 |
  |        | |                   |---------| |--------|      |
  |        | |                               |        |      |
  |        | |           *                   |CXD2938Q|      |
  |J4      | |J202             IS41LV16256   |        |      |
  |        | |                               |        |      |
  |        | |      EPM3064                  |--------|      |
  |        | |                                               |
  |        |-|                 PQ30RV21                      |
  |                                                  J103    |
  |----------------------------------------------------------|
Notes:
------
      CXD8606BQ   : SONY CXD8606BQ Central Processing Unit / GTE     (QFP208)
                     - replaced by CXD8606CQ on Revision 3 Main PCB
      CXD8561CQ   : SONY CXD8561CQ Graphics Processsor Unit          (QFP208)
      CXD2938Q    : SONY CXD2938Q  Sound Processor Unit              (QFP208)
      CXD1178Q    : SONY CXD1178Q  8-bit RGB 3-channel D/A converter (QFP48)
      CXA2067AS   : SONY CXA2067AS TV/Video circuit RGB Pre-Driver   (SDIP30)
      CY37128VP160: CYPRESS CY37128VP160 Complex Programmable Logic Device (TQFP160, stamped 'S10MA1')
                     - replaced by an updated revision on Revision 2 & 3 Main PCB and stamped 'S10MA1B'
      EPM3064     : Altera MAX EPM3064ATC100-10 Complex Programmable Logic Device (TQFP100, stamped 'S10MA2A')
      GAL16V8D    : GAL16V8D PAL (PLCC20, stamped 'S10MA3A')
      IS41LV16100 : ISSI IS41LV16100S-50T 1M x16 EDO DRAM (x2, TSOP50(44) Type II)
      IS41LV16256 : ISSI IS41LV16256-50T 256k x16 EDO DRAM (TSOP44(40) Type II)
      54V25632    : OKI 54V25632 256K x32 SGRAM (x2, QFP100)
      PQ30RV31    : Sharp PQ30RV31 5 Volt to 3.3 Volt Voltage Regulator
      LA4705      : LA4705 15W 2-channel Power Amplifier (SIP18)
      MAX734      : MAX734 +12V 120mA Flash Memory Programming Supply Switching Regulator (SOIC8)
      PST592      : PST592J System Reset IC with 2.7V detection circuit (SOIC4)
      BA3121      : Ground Isolation Amplifier/ Noise Eliminator (SOIC8)
      NJM3414     : 70mA Dual Op Amp (x2, SOIC8)
      DSW1        : 8 position dip switch
      JP4         : 2 position jumper, set to NC, alt. position labelled SYNC (Note: changing the jumper position has
                    no visual effect)
      J1          : 40 Pin IDC connector for plugging of a flat 40-wire cable (not used, purpose unknown, possible
                    CDROM/DVD)
      J4          : 10 pin header for extra controls etc  / (note: custom Namco 48 pin edge connector is not on
                    System10 PCBs)
      J5          : 4 pin header for stereo sound out     \
      J201        : 100 pin custom Namco connector for mounting of MEM PCB. This connector is surface-mounted, not a
                    thru-hole type.
      J202        : 80 pin custom Namco connector for mounting of another board. This connector is surface-mounted,
                    not a thru-hole type.
                    (not used, purpose unknown)
      J103        : 6-pin JAMMA2 power plug (Note none of the other JAMMA2 standard connectors are present)
      VR1         : Volume potentiometer
      *           : Unpopulated position for IS41LV16100 1M x16 EDO DRAM

Additional Notes:
                1. In test mode (Display Test) the screen can be set to interlace or non-interlace mode. The graphics in
                   interlace mode are visually much smoother with noticeable screen flickering. Non-interlace modes gives
                   a much blockier graphic display (i.e. lower resolution) but without screen flickering.
                2. There is no dedicated highscore/options EEPROM present on the PCB, the game stores it's settings on the
                   game board (probably in the program EEPROMs/FlashROMs or maybe the CPLD?).

ROM Daughterboard PCB
---------------------
This PCB holds all the ROMs.
There are two known types of ROM daughterboards used on S10 games (so far).
All of the PCBs are the same size (approx 5" x 5") containing one custom connector surface-mounted to the underside of
the PCB, some MASKROMs/FlashROMs, a CPLD (which seems to be the customary 'KEYCUS' chip, and on the 2nd type a RAM
chip also.

********
*Type 1*
********
System10 MEM(M) PCB 8906961000 (8906970700)
|-------------------------------------|
|                                     |
|                       |-------|     +-
|                       |       |     +-
|                       |CY37128|     +-
|                       |VP160  |     +-
|                       |       |     +-
|     7E     7D         |-------|     +-
|                                     |
|     6E     6D                       |
|                                     |
|     5E     5D                 5A    |
|                                     |
|     4E     4D                 4A    |
|                                     |
|     3E     3D                       |
|                                     |
|     2E     2D                 2A    |
|                                     |
|     1E     1D                 1A    |
|                                     |
|-------------------------------------|
Notes:
      CY37128VP160: CY37128VP160 Cypress Complex Programmable Logic Device (TQFP160)
      1A - 5A     : Intel Flash DA28F640J5 64MBit Flash EEPROM (SSOP56)
      1D - 7E     : Samsung Electronics K3N9V1000A-YC 128MBit MASK ROM (TSOP48) (see note 3)
      6 pin header: (purpose unknown, probably for programming the CPLD)

This PCB is used on:

              Software     MEM PCB
Game          Revision     Sticker      KEYCUS   ROMs Populated
------------------------------------------------------------------------------------
Mr Driller 2  DR21/VER.A3  DR21 Ver.A   KC001A   DR21VERA.1A, DR21MA1.1D, DR21MA2.2D
Mr Driller 2  DR22/VER.A3  DR22 Ver.A   KC001A   DR22VERA.1A, DR21MA1.1D, DR21MA2.2D

      Note
      1. The ROM PCB has locations for 4x 64MBit program ROMs, but only 1A is populated.
      2. The ROM PCB has locations for 14x 128MBit GFX ROMs (Total capacity = 2048MBits) but only 1D and 2D are populated.
      3. These ROMs are only 18mm long, dumping them requires a special custom adapter

********
*Type 2*
********
System10 MEM(N) PCB 8906961402 (8906971402)
|-------------------------------------|
|                                     |
|                    |---------|      +-
|                    |         |      +-
|                    |CY37256  |      +-
|     8E     8D      |VP208    |      +-
|                    |         |      +-
|     7E     7D      |---------|      +-
|                                     |
|     6E     6D                       |
|                                     |
|     5E     5D       CY7C1019        |
|                                     |
|     4E     4D                       |
|                                     |
|     3E     3D                       |
|                                     |
|     2E     2D                       |
|                                     |
|     1E     1D                       |
|                                     |
|-------------------------------------|
Notes:
      CY37256VP208: Cypress CY37256VP208 Complex Programmable Logic Device (TQFP208)
      CY7C1019    : Cypress CY7C1019BV33-15VC or Samsung Electronics K6R1008V1C-JC15 128k x8 bit 3.3V High Speed CMOS Static Ram (SOJ32)
      1D - 8E     : Samsung Electronics K9F2808U0B-YCBO 128MBit NAND Flash EEPROM (TSOP48)
      6 pin header: (purpose unknown, probably for programming the CPLD)

This PCB is used on:

                                      MEM PCB
Game                                  Sticker       KEYCUS   ROMs Populated
---------------------------------------------------------------------------
Gamshara                              10021 Ver.A   KC020A   8E, 8D
Gekitoride-Jong Space                 10011 Ver.A   KC003A   8E, 8D, 7E, 7D
Mr.Driller G                          DRG1  Ver.A   KC007A   8E, 8D, 7E
Kotoba no Puzzle Mojipittan           KPM1  Ver.A   KC012A   8E, 8D, 7E
Panicuru Panekuru                     PPA1  Ver.A   KC017A   8E, 8D, 7E
Star Trigon                           STT1  Ver.A   KC019A   8E, 8D
Utyuu Daisakusen Chocovader Contactee CVC1  Ver.A   KC022A

      Note
      1. The ROM PCB has locations for 16x 128MBit FlashROMs (Total capacity = 2048MBits) but usually only a few are populated.
*/

#include "emu.h"
#include "cpu/mips/psx.h"
#include "includes/psx.h"

class namcos10_state : public psx_state
{
public:
	namcos10_state(running_machine &machine, const driver_device_config_base &config)
		: psx_state(machine, config) { }
};

static ADDRESS_MAP_START( namcos10_map, ADDRESS_SPACE_PROGRAM, 32 )
	AM_RANGE(0x00000000, 0x003fffff) AM_RAM AM_SHARE("share1") /* ram */
	AM_RANGE(0x1f800000, 0x1f8003ff) AM_RAM /* scratchpad */
	AM_RANGE(0x1f801000, 0x1f801007) AM_WRITENOP
	AM_RANGE(0x1f801008, 0x1f80100b) AM_RAM /* ?? */
	AM_RANGE(0x1f80100c, 0x1f80102f) AM_WRITENOP
	AM_RANGE(0x1f801010, 0x1f801013) AM_READNOP
	AM_RANGE(0x1f801014, 0x1f801017) AM_READNOP
	AM_RANGE(0x1f801040, 0x1f80105f) AM_READWRITE(psx_sio_r, psx_sio_w)
	AM_RANGE(0x1f801060, 0x1f80106f) AM_WRITENOP
	AM_RANGE(0x1f801070, 0x1f801077) AM_READWRITE(psx_irq_r, psx_irq_w)
	AM_RANGE(0x1f801080, 0x1f8010ff) AM_READWRITE(psx_dma_r, psx_dma_w)
	AM_RANGE(0x1f801100, 0x1f80112f) AM_READWRITE(psx_counter_r, psx_counter_w)
	AM_RANGE(0x1f801810, 0x1f801817) AM_READWRITE(psx_gpu_r, psx_gpu_w)
	AM_RANGE(0x1f801820, 0x1f801827) AM_READWRITE(psx_mdec_r, psx_mdec_w)
	AM_RANGE(0x1f801c00, 0x1f801dff) AM_NOP
	AM_RANGE(0x1f802020, 0x1f802033) AM_RAM /* ?? */
	AM_RANGE(0x1f802040, 0x1f802043) AM_WRITENOP
	AM_RANGE(0x1fc00000, 0x1fffffff) AM_ROM AM_SHARE("share2") AM_REGION("user1", 0) /* bios */
	AM_RANGE(0x80000000, 0x803fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0x9fc00000, 0x9fffffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xa0000000, 0xa03fffff) AM_RAM AM_SHARE("share1") /* ram mirror */
	AM_RANGE(0xbfc00000, 0xbfffffff) AM_ROM AM_SHARE("share2") /* bios mirror */
	AM_RANGE(0xfffe0130, 0xfffe0133) AM_WRITENOP
ADDRESS_MAP_END

static void memcpy32le( UINT32 *dst, UINT8 *src, int len )
{
	while( len > 0 )
	{
		*( dst ) = ( *( src + 0 ) << 0 ) | ( *( src + 1 ) << 8 ) | ( *( src + 2 ) << 16 ) | ( *( src + 3 ) << 24 );
		dst++;
		src += 4;
		len -= 4;
	}
}

static void memm_driver_init( running_machine *machine )
{
	psx_driver_init(machine);
}

static void memn_driver_init( running_machine *machine )
{
	UINT8 *BIOS = (UINT8 *)machine->region( "user1" )->base();
	UINT8 *ROM = (UINT8 *)machine->region( "user2" )->base();

	memcpy32le( (UINT32 *)( BIOS + 0x0000000 ), ROM + 0x08000, 0x001c000 );
	memcpy32le( (UINT32 *)( BIOS + 0x0020000 ), ROM + 0x24000, 0x03dffff );

	psx_driver_init(machine);
}

static void decrypt_bios( running_machine *machine, int b15, int b14, int b13, int b12, int b11, int b10, int b9, int b8,
	int b7, int b6, int b5, int b4, int b3, int b2, int b1, int b0 )
{
	UINT16 *BIOS = (UINT16 *)machine->region( "user1" )->base();
	int len = machine->region( "user1" )->bytes() / 2;
	int i;

	for( i = 0; i < len; i++ )
	{
		BIOS[ i ] = BITSWAP16( BIOS[ i ] ^ 0xaaaa,
			b15, b14, b13, b12, b11, b10, b9, b8, b7, b6, b5, b4, b3, b2, b1, b0 );
	}
}

static DRIVER_INIT( mrdrilr2 )
{
	memm_driver_init(machine);
	decrypt_bios( machine, 0xc, 0xd, 0xf, 0xe, 0xb, 0xa, 0x9, 0x8, 0x7, 0x6, 0x4, 0x1, 0x2, 0x5, 0x0, 0x3 );
}

static DRIVER_INIT( gjspace )
{
	memn_driver_init(machine);
	decrypt_bios( machine, 0x0, 0x2, 0xe, 0xd, 0xf, 0x6, 0xc, 0x7, 0x5, 0x1, 0x9, 0x8, 0xa, 0x3, 0x4, 0xb );
}

static DRIVER_INIT( mrdrilrg )
{
	memn_driver_init(machine);
	decrypt_bios( machine, 0x6, 0x4, 0x7, 0x5, 0x2, 0x1, 0x0, 0x3, 0xc, 0xd, 0xe, 0xf, 0x8, 0x9, 0xb, 0xa );
}

static DRIVER_INIT( knpuzzle )
{
	memn_driver_init(machine);
	decrypt_bios( machine, 0x6, 0x7, 0x4, 0x5, 0x2, 0x0, 0x3, 0x1, 0xc, 0xd, 0xe, 0xf, 0x9, 0xb, 0x8, 0xa );
}

static DRIVER_INIT( startrgn )
{
	memn_driver_init(machine);
	decrypt_bios( machine, 0x6, 0x5, 0x4, 0x7, 0x1, 0x3, 0x0, 0x2, 0xc, 0xd, 0xe, 0xf, 0x8, 0xb, 0xa, 0x9 );
}

static DRIVER_INIT( gamshara )
{
	memn_driver_init(machine);
	decrypt_bios( machine, 0x5, 0x4, 0x7, 0x6, 0x0, 0x1, 0x3, 0x2, 0xd, 0xf, 0xc, 0xe, 0x8, 0x9, 0xa, 0xb );
}

static MACHINE_RESET( namcos10 )
{
	psx_machine_init(machine);
}

static MACHINE_CONFIG_START( namcos10, namcos10_state )
	/* basic machine hardware */
	MCFG_CPU_ADD( "maincpu", PSXCPU, XTAL_101_4912MHz )
	MCFG_CPU_PROGRAM_MAP( namcos10_map)
	MCFG_CPU_VBLANK_INT("screen", psx_vblank)

	MCFG_MACHINE_RESET( namcos10 )

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 60 )
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE( 1024, 1024 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 639, 0, 479 )

	MCFG_PALETTE_LENGTH( 65536 )

	MCFG_PALETTE_INIT( psx )
	MCFG_VIDEO_START( psx_type2 )
	MCFG_VIDEO_UPDATE( psx )

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")
MACHINE_CONFIG_END

static INPUT_PORTS_START( namcos10 )
	/* IN 0 */
	PORT_START("SYSTEM")
	PORT_SERVICE_NO_TOGGLE( 0x8000, IP_ACTIVE_HIGH )
	PORT_BIT( 0x7fff, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* IN 1 */
	PORT_START("P1")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE1 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 )
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 )
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* IN 2 */
	PORT_START("P2")
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( On ) )
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_SERVICE2 )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( On ) )
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x000f, IP_ACTIVE_HIGH, IPT_UNUSED )

	/* IN 3 */
	PORT_START("COINS")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 )
INPUT_PORTS_END

ROM_START( mrdrilr2 )
	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* main prg */
	ROM_LOAD( "dr21vera.1a",  0x000000, 0x800000, CRC(f93532a2) SHA1(8b72f2868978be1f0e0abd11425a3c8b2b0c4e99) )

	ROM_REGION( 0x4000000, "user2", 0 ) /* main prg */
        ROM_LOAD( "dr21ma1.1d", 0x0000000, 0x1000000, CRC(26dc6f55) SHA1(a9cedf547fa7a4d5850b9b3b867d46e577a035e0) )
        ROM_LOAD( "dr21ma2.2d", 0x1000000, 0x1000000, CRC(702556ff) SHA1(c95defd5fd6a9b406fc8d8f28ecfab732ef1ff42) )
ROM_END

ROM_START( mrdrlr2a )
	ROM_REGION32_LE( 0x800000, "user1", 0 ) /* main prg */
        ROM_LOAD( "dr22vera.1a",  0x000000, 0x800000, CRC(f2633388) SHA1(42e56c9758ee833390003d4b41956f75f5a22760) )

	ROM_REGION( 0x4000000, "user2", 0 ) /* main prg */
        ROM_LOAD( "dr21ma1.1d", 0x0000000, 0x1000000, CRC(26dc6f55) SHA1(a9cedf547fa7a4d5850b9b3b867d46e577a035e0) )
        ROM_LOAD( "dr21ma2.2d", 0x1000000, 0x1000000, CRC(702556ff) SHA1(c95defd5fd6a9b406fc8d8f28ecfab732ef1ff42) )
ROM_END

ROM_START( gjspace )
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION( 0x4000000, "user2", 0 ) /* main prg */
	ROM_LOAD( "10011a_0.bin", 0x0000000, 0x1000000, CRC(853e329e) SHA1(dd272ab2de9052c5aeee9e0ead4ee3d14347593e) )
	ROM_LOAD( "10011a_1.bin", 0x1000000, 0x1000000, CRC(7d4e96f7) SHA1(41b0d27b93662f15547311c8723ac5141e129cab) )
	ROM_LOAD( "10011a_2.bin", 0x2000000, 0x1000000, CRC(83111de6) SHA1(d760fd41644104f6d3a2806ab4664df2c77bdc42) )
	ROM_LOAD( "10011a_3.bin", 0x3000000, 0x1000000, CRC(b6ea8be7) SHA1(3e65deda2b0e6758ce0ed05419ed096efb8ad755) )
ROM_END

ROM_START( mrdrilrg )
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION( 0x3000000, "user2", 0 ) /* main prg */
	ROM_LOAD( "drg1a_0.bin",  0x0000000, 0x1000000, CRC(1e5595f1) SHA1(0373ccb6e4b6fe96010afbb9657c3c0b2b02dd57) )
	ROM_LOAD( "drg1a_1.bin",  0x1000000, 0x1000000, CRC(4db3a361) SHA1(d6a4fb682ab75cd98a9a960c244e94cacd367f12) )
	ROM_LOAD( "drg1a_2.bin",  0x2000000, 0x1000000, CRC(c99ffa30) SHA1(fdcc0a348fe705c5ed1611170570f2fd9068525a) )
ROM_END

ROM_START( knpuzzle )
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION( 0x3000000, "user2", 0 ) /* main prg */
	ROM_LOAD( "kpm1a_0.bin",  0x0000000, 0x1000000, CRC(606d001b) SHA1(5a94e56a164a9132af7f9bd51cae0674dbe0e036) )
	ROM_LOAD( "kpm1a_1.bin",  0x1000000, 0x1000000, CRC(a506d767) SHA1(f4830edc8eaa42ff80e83aca2fa38a4d2807c667) )
	ROM_LOAD( "kpm1a_2.bin",  0x2000000, 0x1000000, CRC(5f40c402) SHA1(2a56421c0f0c9cc5b7b93ddacfc6f3d214a946d9) )
ROM_END

ROM_START( startrgn )
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION( 0x2000000, "user2", 0 ) /* main prg */
	ROM_LOAD( "stt1a_0.bin",  0x0000000, 0x1000000, CRC(07513d66) SHA1(baed445ff81e7f687e39b26648b3fe2ff601826b) )
	ROM_LOAD( "stt1a_1.bin",  0x1000000, 0x1000000, CRC(f6e8e641) SHA1(37c8c1482d652b46a744252721299448b15e1027) )
ROM_END

ROM_START( gamshara )
	ROM_REGION32_LE( 0x400000, "user1", 0 ) /* bios */
	ROM_FILL( 0x0000000, 0x400000, 0x55 )

	ROM_REGION( 0x2000000, "user2", 0 ) /* main prg */
	ROM_LOAD( "10021a.8e",    0x0000000, 0x1000000, CRC(db0a3687) SHA1(757360c43fba247e6c0c0fc66bb089575e19d646) )
	ROM_LOAD( "10021a.8d",    0x1000000, 0x1000000, CRC(bebafef6) SHA1(6c965c09f3186523d27c50894a15b9c1dd02a794) )
ROM_END

GAME( 2000, mrdrilr2,  0,        namcos10, namcos10, mrdrilr2, ROT0, "Namco", "Mr. Driller 2 (Japan, DR21 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2000, mrdrlr2a,  mrdrilr2, namcos10, namcos10, mrdrilr2, ROT0, "Namco", "Mr. Driller 2 (Japan, DR22 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2001, gjspace,   0,        namcos10, namcos10, gjspace,  ROT0, "Namco / Metro", "Gekitoride-Jong Space (10011 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2001, mrdrilrg,  0,        namcos10, namcos10, mrdrilrg, ROT0, "Namco", "Mr. Driller G (Japan, DRG1 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2001, knpuzzle,  0,        namcos10, namcos10, knpuzzle, ROT0, "Namco", "Kotoba no Puzzle Mojipittan (Japan, KPM1 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2002, startrgn,  0,        namcos10, namcos10, startrgn, ROT0, "Namco", "Star Trigon (Japan, STT1 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
GAME( 2003, gamshara,  0,        namcos10, namcos10, gamshara, ROT0, "Mitchell", "Gamshara (10021 Ver.A)", GAME_NOT_WORKING | GAME_NO_SOUND )
