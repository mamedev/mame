/***************************************************************************

The Simpsons (c) 1991 Konami Co. Ltd

Preliminary driver by:
Ernesto Corvi
someone@secureshell.com

PCB Layout
----------

GX072  PWB352346B
|-----------------------------------------------------|
| MB3722   072D04.1D  072D05.1F        8464           |
|  VOL VOL     YM2151                        072B08.3N|
|           YM3012     053260       053246            |
|CN3                 3.579545MHz                      |
|              Z80     072E03.6G                      |
|J 052535                 8416      053247            |
|A 052535                 2018               072B09.8N|
|M 052535 053994          2018      053251            |
|M 051550 053995                                      |
|A                   8464                             |
|    ER5911               052109    051962  072B10.12N|
|TEST  072M13.13C                                     |
|      072L12.15C                                     |
|CN6   072G02.16C  053248  8464  072B06.16H 072B11.16L|
|CN7   072G01.17C    24MHz 8464  072B07.18H           |
|-----------------------------------------------------|
Notes:
       CN3 - 4 pin connector for sound output for right speaker (left speaker
             is via the JAMMA connector)
   CN6/CN7 - 15 pin connector for player 3 and player 4 controls
    ER5911 - EEPROM (128bx8 == 93C46)
      8464 - 8kx8 SRAM
      8416 - 2kx8 SRAM
      2018 - 2kx8 SRAM
    MB3722 - Audio Power AMP
       Z80 - clock 3.579545MHz
    YM2151 - clock 3.579545MHz
    YM3012 - clock 1.7897725MHz [3.579545/2]
Custom ICs - 053260        - sound chip (QFP80)
             053246/053247 - sprite generators (QFP120)
             053251        - priority encoder (QFP100)
             052535        - RGB DAC (ceramic encased SIP9)
             051550        - I/O DAC (ceramic encased SIP23)
             053994/053995 - PALs (MMI PAL16L8, DIP20)
             052109/051962 - Tilemap Generators (QFP120)
             053248        - CPU (QFP80), clock input 12.000MHz and 3.000MHz [24/2 and 24/8]
      ROMs - 072D04 \  256kx8 DIP40 MaskROM
             072D05 /  1Mx8 DIP40 MaskROM (Sound Samples)
             072E03 -  32kx8 MaskROM (Z80 Sound Program)
             072B08 \
             072B09  |
             072B10  | 512kx16 DIP40 MaskROM (Sprites)
             072B11 /
             072B06 \
             072B07 /  256kx16 DIP40 MaskROM (Tiles)
             072M13 \
             072L12  |
             072G02  | 128kx8 DIP32 MaskROM (Main Program)
             072G01 /

     VSync - 59.1856Hz
     HSync - 15.1566kHz

***************************************************************************/

#include "driver.h"
#include "cpu/konami/konami.h" /* for the callback and the firq irq definition */
#include "cpu/z80/z80.h"
#include "video/konamiic.h"
#include "sound/2151intf.h"
#include "sound/k053260.h"
#include "includes/simpsons.h"
#include "konamipt.h"


/***************************************************************************

  Memory Maps

***************************************************************************/

static ADDRESS_MAP_START( main_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM
	AM_RANGE(0x1f80, 0x1f80) AM_READ_PORT("COIN")
	AM_RANGE(0x1f81, 0x1f81) AM_READ(simpsons_eeprom_r)
	AM_RANGE(0x1f90, 0x1f90) AM_READ_PORT("P1")
	AM_RANGE(0x1f91, 0x1f91) AM_READ_PORT("P2")
	AM_RANGE(0x1f92, 0x1f92) AM_READ_PORT("P3")
	AM_RANGE(0x1f93, 0x1f93) AM_READ_PORT("P4")
	AM_RANGE(0x1fa0, 0x1fa7) AM_WRITE(K053246_w)
	AM_RANGE(0x1fb0, 0x1fbf) AM_WRITE(K053251_w)
	AM_RANGE(0x1fc0, 0x1fc0) AM_WRITE(simpsons_coin_counter_w)
	AM_RANGE(0x1fc2, 0x1fc2) AM_WRITE(simpsons_eeprom_w)
	AM_RANGE(0x1fc4, 0x1fc4) AM_READ(simpsons_sound_interrupt_r)
	AM_RANGE(0x1fc6, 0x1fc7) AM_DEVREADWRITE("konami", simpsons_sound_r, k053260_w)
	AM_RANGE(0x1fc8, 0x1fc9) AM_READ(K053246_r)
	AM_RANGE(0x1fca, 0x1fca) AM_READ(watchdog_reset_r)
	AM_RANGE(0x2000, 0x3fff) AM_RAMBANK(4)
	AM_RANGE(0x0000, 0x3fff) AM_READWRITE(K052109_r, K052109_w)
	AM_RANGE(0x4000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x7fff) AM_ROMBANK(1)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static WRITE8_HANDLER( z80_bankswitch_w )
{
	memory_set_bank(space->machine, 2, data & 7);
}

#if 0
static int nmi_enabled;

static void sound_nmi_callback( running_machine *machine, int param )
{
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_NMI, ( nmi_enabled ) ? CLEAR_LINE : ASSERT_LINE );

	nmi_enabled = 0;
}
#endif

static TIMER_CALLBACK( nmi_callback )
{
	cputag_set_input_line(machine, "audiocpu", INPUT_LINE_NMI, ASSERT_LINE);
}

static WRITE8_HANDLER( z80_arm_nmi_w )
{
//  sound_nmi_enabled = 1;
	cputag_set_input_line(space->machine, "audiocpu", INPUT_LINE_NMI, CLEAR_LINE);
	timer_set(space->machine, ATTOTIME_IN_USEC(25), NULL,0,nmi_callback);	/* kludge until the K053260 is emulated correctly */
}

static ADDRESS_MAP_START( z80_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK(2)
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xf801) AM_DEVREADWRITE("ym", ym2151_r, ym2151_w)
	AM_RANGE(0xfa00, 0xfa00) AM_WRITE(z80_arm_nmi_w)
	AM_RANGE(0xfc00, 0xfc2f) AM_DEVREADWRITE("konami", k053260_r, k053260_w)
	AM_RANGE(0xfe00, 0xfe00) AM_WRITE(z80_bankswitch_w)
ADDRESS_MAP_END

/***************************************************************************

    Input Ports

***************************************************************************/

static INPUT_PORTS_START( simpsons )
	PORT_START("P1")
	KONAMI8_B12_START(1)

	PORT_START("P2")
	KONAMI8_B12_START(2)

	PORT_START("P3")
	KONAMI8_B12_START(3)

	PORT_START("P4")
	KONAMI8_B12_START(4)

	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE1 Unused
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE2 Unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE3 Unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE4 Unused

	PORT_START("TEST")
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static INPUT_PORTS_START( simpsn2p )
	PORT_START("P1")
	KONAMI8_B12_START(1)

	PORT_START("P2")
	KONAMI8_B12_START(2)

	PORT_START("P3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("COIN") /* IN4 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) //COIN3 Unused
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) //COIN4 Unused
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE2 Unused
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE3 Unused
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN ) //SERVICE4 Unused

	PORT_START("TEST") /* IN5 */
	PORT_SERVICE_NO_TOGGLE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0xfe, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END



/***************************************************************************

    Machine Driver

***************************************************************************/

static void simpsons_objdma(void)
{
	int counter, num_inactive;
	UINT16 *src, *dst;

	K053247_export_config(&dst, 0, 0, 0, &counter);
	src = spriteram16;
	num_inactive = counter = 256;

	do {
		if ((*src & 0x8000) && (*src & 0xff))
		{
			memcpy(dst, src, 0x10);
			dst += 8;
			num_inactive--;
		}
		src += 8;
	}
	while (--counter);

	if (num_inactive) do { *dst = 0; dst += 8; } while (--num_inactive);
}

static TIMER_CALLBACK( dmaend_callback )
{
	if (simpsons_firq_enabled)
		cputag_set_input_line(machine, "maincpu", KONAMI_FIRQ_LINE, HOLD_LINE);
}

static INTERRUPT_GEN( simpsons_irq )
{
	if (K053246_is_IRQ_enabled())
	{
		simpsons_objdma();

		// 32+256us delay at 8MHz dotclock; artificially shortened since actual V-blank length is unknown
		timer_set(device->machine, ATTOTIME_IN_USEC(30), NULL, 0, dmaend_callback);
	}

	if (K052109_is_IRQ_enabled())
		cpu_set_input_line(device, KONAMI_IRQ_LINE, HOLD_LINE);
}

static MACHINE_DRIVER_START( simpsons )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", KONAMI, XTAL_24MHz/2/4) /* pin 18 of konami cpu is 12Mhz, while pin 17 is 3mhz. Clock probably divided internally by 4  */
	MDRV_CPU_PROGRAM_MAP(main_map)
	MDRV_CPU_VBLANK_INT("screen", simpsons_irq)	/* IRQ triggered by the 052109, FIRQ by the sprite hardware */

	MDRV_CPU_ADD("audiocpu", Z80, XTAL_3_579545MHz)	/* verified on pcb */
	MDRV_CPU_PROGRAM_MAP(z80_map)
								/* NMIs are generated by the 053260 */

	MDRV_MACHINE_RESET(simpsons)
	MDRV_NVRAM_HANDLER(simpsons)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_HAS_SHADOWS | VIDEO_HAS_HIGHLIGHTS | VIDEO_UPDATE_AFTER_VBLANK)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )

	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_START(simpsons)
	MDRV_VIDEO_UPDATE(simpsons)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MDRV_SOUND_ADD("ym", YM2151, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_ROUTE(0, "lspeaker", 1.0)	/* only left channel is connected */
	MDRV_SOUND_ROUTE(0, "rspeaker", 1.0)
	MDRV_SOUND_ROUTE(1, "lspeaker", 0.0)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.0)

	MDRV_SOUND_ADD("konami", K053260, XTAL_3_579545MHz) /* verified on pcb */
	MDRV_SOUND_ROUTE(0, "lspeaker", 0.75)
	MDRV_SOUND_ROUTE(1, "rspeaker", 0.75)
MACHINE_DRIVER_END


/***************************************************************************

  Game ROMs

***************************************************************************/

ROM_START( simpsons ) /* World 4 Player */
	ROM_REGION( 0x8b000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "072-g02.16c", 0x10000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) )
	ROM_LOAD( "072-g01.17c", 0x30000, 0x20000, CRC(9f843def) SHA1(858432b59101b0577c5cec6ac0c7c20ab0780c9a) )
	ROM_LOAD( "072-j13.13c", 0x50000, 0x20000, CRC(aade2abd) SHA1(10f178d5ed399b4866266e075d91ca3db26798f8) )
	ROM_LOAD( "072-j12.15c", 0x70000, 0x18000, CRC(479e12f2) SHA1(15a6cb12e68b4773a29ab463640a43f8e814de59) )
	ROM_CONTINUE(		 0x08000, 0x08000 )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-e03.6g", 0x00000, 0x08000, CRC(866b7a35) SHA1(98905764eb4c7d968ccc17618a1f24ee12e33c0e) )
	ROM_CONTINUE(		0x10000, 0x18000 )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )	/* tiles */
	ROM_LOAD( "072-b06.16h", 0x080000, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) )	/* sprites */
	ROM_LOAD( "072-b09.8n",  0x100000, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD( "072-b10.12n", 0x200000, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD( "072-b11.16l", 0x300000, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "konami", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )
ROM_END

ROM_START( simpsons4pa ) /* World 4 Player, later? (by use of later leters) */
	ROM_REGION( 0x8b000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "072-g02.16c", 0x10000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) )
	ROM_LOAD( "072-g01.17c", 0x30000, 0x20000, CRC(9f843def) SHA1(858432b59101b0577c5cec6ac0c7c20ab0780c9a) )
	ROM_LOAD( "072-m13.13c", 0x50000, 0x20000, CRC(f36c9423) SHA1(4a7311ffcb2e6916006c1e79dfc231e7fc570781) )
	ROM_LOAD( "072-l12.15c", 0x70000, 0x18000, CRC(84f9d9ba) SHA1(d52f999b7c8125daea5e9b5754c6e82c17861d1b) )
	ROM_CONTINUE(            0x08000, 0x08000 )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-e03.6g", 0x00000, 0x08000, CRC(866b7a35) SHA1(98905764eb4c7d968ccc17618a1f24ee12e33c0e) )
	ROM_CONTINUE(     	0x10000, 0x18000 )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) ) /* tiles */
	ROM_LOAD( "072-b06.16h", 0x080000, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) ) /* sprites */
	ROM_LOAD( "072-b09.8n",  0x100000, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD( "072-b10.12n", 0x200000, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD( "072-b11.16l", 0x300000, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "konami", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )
ROM_END


ROM_START( simpsons2p ) /* World 2 Player */
	ROM_REGION( 0x8b000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "072-g02.16c",  0x10000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) )
	ROM_LOAD( "072-p01.17c",  0x30000, 0x20000, CRC(07ceeaea) SHA1(c18255ae1d578c2d53de80d6323cdf41cbe47b57) )
	ROM_LOAD( "072-013.13c",  0x50000, 0x20000, CRC(8781105a) SHA1(ef2f16f7a56d3715536511c674df4b3aab1be2bd) )
	ROM_LOAD( "072-012.15c",  0x70000, 0x18000, CRC(244f9289) SHA1(eeda7f5c7340cbd1a1cd576af48cd5d1a629914a) )
	ROM_CONTINUE(		  0x08000, 0x08000 )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-g03.6g", 0x00000, 0x08000, CRC(76c1850c) SHA1(9047c6b26c4e33c74eb7400a807d3d9f206f7bbe) )
	ROM_CONTINUE(		0x10000, 0x18000 )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )	/* tiles */
	ROM_LOAD( "072-b06.16h", 0x080000, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) )	/* sprites */
	ROM_LOAD( "072-b09.8n",  0x100000, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD( "072-b10.12n", 0x200000, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD( "072-b11.16l", 0x300000, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "konami", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )
ROM_END

ROM_START( simpsons2p2 ) /* World 2 Player, alt */
	ROM_REGION( 0x8b000, "maincpu", 0 ) /* code + banked roms + banked ram */
        ROM_LOAD( "072-g02.16c", 0x10000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) )
	ROM_LOAD( "072-p01.17c", 0x30000, 0x20000, CRC(07ceeaea) SHA1(c18255ae1d578c2d53de80d6323cdf41cbe47b57) )
        ROM_LOAD( "072-_13.13c", 0x50000, 0x20000, CRC(54e6df66) SHA1(1b83ae56cf1deb51b04880fa421f06568c938a99) ) /* Uknown revision/region code */
        ROM_LOAD( "072-_12.15c", 0x70000, 0x18000, CRC(96636225) SHA1(5de95606e5c9337f18bc42f4df791cacafa20399) ) /* Uknown revision/region code */
	ROM_CONTINUE(		 0x08000, 0x08000 )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-g03.6g", 0x00000, 0x08000, CRC(76c1850c) SHA1(9047c6b26c4e33c74eb7400a807d3d9f206f7bbe) )
	ROM_CONTINUE(		0x10000, 0x18000 )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )	/* tiles */
	ROM_LOAD( "072-b06.16h", 0x080000, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) )	/* sprites */
	ROM_LOAD( "072-b09.8n",  0x100000, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD( "072-b10.12n", 0x200000, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD( "072-b11.16l", 0x300000, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "konami", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )
ROM_END

ROM_START( simpsons2pa ) /* Asia 2 Player */
	ROM_REGION( 0x8b000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "072-g02.16c", 0x10000, 0x20000, CRC(580ce1d6) SHA1(5b07fb8e8041e1663980aa35d853fdc13b22dac5) ) /* Same as both world 2p sets */
	ROM_LOAD( "072-p01.17c", 0x30000, 0x20000, CRC(07ceeaea) SHA1(c18255ae1d578c2d53de80d6323cdf41cbe47b57) ) /* Same as both world 2p sets */
	ROM_LOAD( "072-113.13c", 0x50000, 0x20000, CRC(8781105a) SHA1(ef2f16f7a56d3715536511c674df4b3aab1be2bd) ) /* Same as world set simpsn2p */
	ROM_LOAD( "072-112.15c", 0x70000, 0x18000, CRC(3bd69404) SHA1(e055fed7e9bde8315ae2f9b2d35bc05fece6b80b) )
	ROM_CONTINUE(		 0x08000, 0x08000 )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-e03.6g", 0x00000, 0x08000, CRC(866b7a35) SHA1(98905764eb4c7d968ccc17618a1f24ee12e33c0e) )
	ROM_CONTINUE(		0x10000, 0x18000 )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )	/* tiles */
	ROM_LOAD( "072-b06.16h", 0x080000, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) )	/* sprites */
	ROM_LOAD( "072-b09.8n",  0x100000, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD( "072-b10.12n", 0x200000, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD( "072-b11.16l", 0x300000, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "konami", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )
ROM_END

ROM_START( simpsons2pj ) /* Japan 2 Player */
	ROM_REGION( 0x8b000, "maincpu", 0 ) /* code + banked roms + banked ram */
	ROM_LOAD( "072-s02.16c", 0x10000, 0x20000, CRC(265f7a47) SHA1(d39c19a5e303f822313409343b209947f4c47ae4) )
	ROM_LOAD( "072-t01.17c", 0x30000, 0x20000, CRC(91de5c2d) SHA1(1e18a5585ed821ec7cda69bdcdbfa4e6c71455c6) )
	ROM_LOAD( "072-213.13c", 0x50000, 0x20000, CRC(b326a9ae) SHA1(f222c33f2e8b306f2f0ef6f0da9febbf8219e1a4) )
	ROM_LOAD( "072-212.15c", 0x70000, 0x18000, CRC(584d9d37) SHA1(61b9df4dfb323b7284894e5e1eb9d713ebf64721) )
	ROM_CONTINUE(		 0x08000, 0x08000 )

	ROM_REGION( 0x28000, "audiocpu", 0 ) /* Z80 code + banks */
	ROM_LOAD( "072-g03.6g", 0x00000, 0x08000, CRC(76c1850c) SHA1(9047c6b26c4e33c74eb7400a807d3d9f206f7bbe) )
	ROM_CONTINUE(		0x10000, 0x18000 )

	ROM_REGION( 0x100000, "gfx1", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "072-b07.18h", 0x000000, 0x080000, CRC(ba1ec910) SHA1(0805ccb641271dea43185dc0365732260db1763d) )	/* tiles */
	ROM_LOAD( "072-b06.16h", 0x080000, 0x080000, CRC(cf2bbcab) SHA1(47afea47f9bc8cb5eb1c7b7fbafe954b3e749aeb) )

	ROM_REGION( 0x400000, "gfx2", 0 ) /* graphics ( dont dispose as the program can read them, 0 ) */
	ROM_LOAD( "072-b08.3n",  0x000000, 0x100000, CRC(7de500ad) SHA1(61b76b8f402e3bde1509679aaaa28ef08cafb0ab) )	/* sprites */
	ROM_LOAD( "072-b09.8n",  0x100000, 0x100000, CRC(aa085093) SHA1(925239d79bf607021d371263352618876f59c1f8) )
	ROM_LOAD( "072-b10.12n", 0x200000, 0x100000, CRC(577dbd53) SHA1(e603e03e3dcba766074561faa92afafa5761953d) )
	ROM_LOAD( "072-b11.16l", 0x300000, 0x100000, CRC(55fab05d) SHA1(54db8559d71ed257de9a29c8808654eaea0df9e2) )

	ROM_REGION( 0x140000, "konami", 0 ) /* samples for the 053260 */
	ROM_LOAD( "072-d05.1f", 0x000000, 0x100000, CRC(1397a73b) SHA1(369422c84cca5472967af54b8351e29fcd69f621) )
	ROM_LOAD( "072-d04.1d", 0x100000, 0x040000, CRC(78778013) SHA1(edbd6d83b0d1a20df39bb160b92395586fa3c32d) )
ROM_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

static DRIVER_INIT( simpsons )
{
	konami_rom_deinterleave_2(machine, "gfx1");
	konami_rom_deinterleave_4(machine, "gfx2");
}

// the region warning, if one exists, is shown after the high-score screen in attract mode
GAME( 1991, simpsons,    0,        simpsons, simpsons, simpsons, ROT0, "Konami", "The Simpsons (4 Players World, set 1)", 0 )
GAME( 1991, simpsons4pa, simpsons, simpsons, simpsons, simpsons, ROT0, "Konami", "The Simpsons (4 Players World, set 2)", 0 )
GAME( 1991, simpsons2p,  simpsons, simpsons, simpsn2p, simpsons, ROT0, "Konami", "The Simpsons (2 Players World, set 1)", 0 )
GAME( 1991, simpsons2p2, simpsons, simpsons, simpsons, simpsons, ROT0, "Konami", "The Simpsons (2 Players World, set 2)", 0 )
GAME( 1991, simpsons2pa, simpsons, simpsons, simpsn2p, simpsons, ROT0, "Konami", "The Simpsons (2 Players Asia)", 0 )
GAME( 1991, simpsons2pj, simpsons, simpsons, simpsn2p, simpsons, ROT0, "Konami", "The Simpsons (2 Players Japan)", 0 )
